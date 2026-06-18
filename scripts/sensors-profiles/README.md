# Sensors Profiles — Calibración y Linealización de Sensores IR

## Índice

1. [Resumen](#resumen)
2. [Modelo matemático](#modelo-matemático)
3. [Estructura del notebook](#estructura-del-notebook)
4. [Flujo de trabajo](#flujo-de-trabajo)
5. [Uso en el firmware](#uso-en-el-firmware)
6. [Guía de uso](#guía-de-uso)
7. [Datasets de calibración](#datasets-de-calibración)
8. [Referencias](#referencias)

---

## Resumen

Este notebook de Jupyter (`sensors-profiles.ipynb`) calcula las constantes de calibración `(a, b, c)` que permiten convertir la lectura raw del ADC de los sensores IR del micromouse en distancia real en milímetros, mediante un modelo logarítmico empírico.

La salida del notebook son las constantes que se copian directamente en el firmware (`source_code/src/sensors.c`, función `set_sensors_robot_calibration()`), donde se usan en tiempo real para el control del robot.

---

## Modelo matemático

### Ecuación

```
distance (m) = a / ln(raw + c) - b
```

Donde:
- `raw` = valor del ADC (0–4095, lectura diferencial: sensor ON − sensor OFF)
- `a` = factor de escala (controla la forma de la curva logarítmica)
- `b` = offset (desplaza la curva verticalmente)
- `c` = offset del raw (desplaza horizontalmente, compensa el offset del sensor)

### Conversión a milímetros en el firmware

```c
// sensors.c:505-509
int16_t ln_index = (raw_filtered + calib.c) / 4;   // división entera
float ln = get_ln_value(ln_index);                  // lookup table: devuelve ln(ln_index * 4)
int16_t distance_mm = (int16_t)((calib.a / ln - calib.b) * 1000.0f);
```

El firmware usa una **lookup table de 1024 entradas** (`LOG_LINEARIZATION_TABLE_SIZE = 4096/4`) que almacena `ln(index * 4)`. La división entera por 4 introduce un error de cuantificación típicamente < 0.5% en distancia. El notebook incluye la función `raw_to_distance_firmware()` que simula esta cuantificación y reporta el error introducido (celda 6).

### Justificación física

Los sensores IR de reflectancia siguen aproximadamente la ley de la inversa del cuadrado: la intensidad reflejada es proporcional a `1/d²`. Esto implica que `raw ∝ 1/d²`, o equivalentemente `d ∝ 1/√raw`. El modelo `a/ln(raw+c) - b` es una aproximación empírica con asíntotas similares pero mejor comportamiento en los extremos del rango.

---

## Estructura del notebook

| Celda | Título | Contenido |
|-------|--------|-----------|
| 0 | _Imports_ | `numpy`, `pandas`, `matplotlib.pyplot`, `matplotlib.ticker`, `scipy.optimize.curve_fit` |
| 1 | Datos de calibración | `DATASETS` (lista de `{label, front_left, front_right, side_left, side_right}`), `ACTIVE_DATASETS`, `PLOT_MODE`. Invierte arrays y construye los DataFrames en `dfs`. |
| 2 | Visión general | Grid 2×2 con los valores raw de todos los robots activos superpuestos (`plt.cm.tab10`). |
| 3 | Funciones de modelo y utilidades | `raw_to_distance()`, `raw_to_distance_firmware()`, `estimate_p0()`, `compute_metrics()` |
| 4 | Selección de rangos | Define `RANGE_FRONT_*` y `RANGE_SIDE_*`. Crea DataFrames filtrados en `range_dfs`. Imprime tabla de raw mínimos por robot. |
| 5 | Ajuste de la curva | Define `fit_sensor()` y ajusta los 4 sensores × N datasets activos. Imprime parámetros ± incertidumbre + R²/RMSE. Almacena resultados en `results`. |
| 6 | Análisis de error | Calcula distancias ajustadas y errores para todos los datasets. Imprime `.describe()`, RMSE separado por rangos ajustados/excluidos, y error de cuantificación de la LUT. |
| 7 | Gráficas de error y curvas ajustadas | Genera 2 juegos de gráficas (error + curvas ajustadas), cada uno respetando `PLOT_MODE` (`"by_sensor"` o `"by_robot"`). |
| 8 | Structs C para el firmware | Imprime todos los structs C agrupados por robot con banner destacado, listos para copiar a `sensors.c`. |

---

## Flujo de trabajo

### 1. Obtención de datos

Se toman lecturas raw del ADC para cada sensor a distancias conocidas (0–250 mm, en pasos de 10 mm). Las lecturas se almacenan como arrays de 26 valores en `DATASETS` (celda 1), donde el primer valor corresponde a 250 mm (lejos) y el último a 0 mm (cerca). La inversión al orden ascendente se aplica automáticamente.

### 2. Filtrado por rangos de interés

El ajuste de curvas NO se hace sobre todos los datos. Se definen rangos prioritarios donde se necesita más precisión:

| Sensor | Rangos de ajuste | Rango excluido |
|--------|-----------------|----------------|
| Frontales | 20–70 mm y 100–150 mm | 70–100 mm |
| Laterales | 5–90 mm y 100–170 mm | 90–100 mm |

Los rangos excluidos corresponden a distancias de transición donde la precisión es menos crítica para la navegación en el laberinto. La celda 6 reporta el RMSE por separado para los rangos ajustados y los excluidos, permitiendo validar que la exclusión no degrada la precisión.

### 3. Ajuste de curvas

Se usa `scipy.optimize.curve_fit` con el método de mínimos cuadrados no lineales (Levenberg-Marquardt). Cada sensor se ajusta de forma independiente mediante `fit_sensor()` (celda 5), que encapsula todo el proceso:

1. `estimate_p0(raw_vals, dist_vals)` — estima `p0=[a, b, c=0]` resolviendo el modelo a partir de los extremos del rango de datos. `c=0` evita el colapso del logaritmo con valores raw bajos.
2. `curve_fit(...)` — ajusta los parámetros con bounds adecuados para cada tipo de sensor.
3. `np.sqrt(np.diag(pcov))` — calcula la incertidumbre de cada parámetro.
4. `compute_metrics(...)` — calcula R² y RMSE sobre los datos de ajuste.

- **Frontales**: `p0` automático, bounds `([-10, -1, -200], [10, 1, 200])`
- **Laterales**: `p0` automático, bounds `([-5, -2, -300], [5, 2, 300])`

### 4. Validación

La celda 6 realiza la validación completa sobre el dataset entero (no solo los rangos de ajuste):

- **Estadísticas descriptivas** (`.describe()`) del error de cada sensor
- **RMSE separado** por rangos ajustados vs excluidos, con conteo de muestras
- **Error de cuantificación de la LUT**: `raw_to_distance_firmware()` simula la división entera del firmware y se compara con el modelo float de precisión. Se reporta el error máximo y RMSE. Si el error máx > 3 mm, el notebook recomienda reajustar con el modelo firmware.

La celda 7 genera las gráficas de error y curvas ajustadas, respetando la variable `PLOT_MODE`:
- `"by_sensor"` — grid 2×2 con todos los robots superpuestos por colores
- `"by_robot"` — un figure por robot con sus 4 sensores

---

## Uso en el firmware

### Dónde se copian las constantes

Archivo: [`source_code/src/sensors.c`](../../source_code/src/sensors.c)

Función: `set_sensors_robot_calibration(uint16_t version)` (línea 67)

Las constantes se almacenan por versión de robot (`ZOROBOT3_A`, `ZOROBOT3_B`, `ZOROBOT3_C`, etc.). La celda 8 del notebook imprime los structs C listos para copiar, agrupados por robot con su etiqueta identificativa.

### Estructura de datos

```c
struct sensors_distance_calibration {
    float a;
    float b;
    float c;
};
```

### Procesamiento en tiempo real

En `update_sensors_magics()` (línea 486), cada ciclo:
1. Se calcula `raw_filtered` (mediana de 3 lecturas)
2. Se obtiene `ln_index = (raw_filtered + c) / 4`
3. Se busca `ln = get_ln_value(ln_index)` en la LUT
4. Se calcula `distance_mm = (a / ln - b) * 1000`
5. Se añaden offsets: distancia frontal del robot (`ROBOT_FRONT_LENGTH`) o ancho medio (`ROBOT_MIDDLE_WIDTH`)
6. Se aplica un filtro paso bajo (LPF) con α adaptativo (0.2 normal, 0.6 para cambios grandes)
7. Se aplica el offset de calibración guardado en EEPROM (`sensors_distance_offset`)

---

## Guía de uso

### Requisitos

```bash
pip install numpy pandas matplotlib scipy
```

En Arch Linux:
```bash
paru -S python-ipykernel python-numpy python-pandas python-matplotlib python-scipy
```

### Procedimiento para calibrar un nuevo robot

1. **Tomar medidas**: Colocar el robot frente a una pared a distancias conocidas (0–250 mm, pasos de 10 mm) y registrar el valor raw de cada sensor.

2. **Añadir datos al notebook**:
   - Añadir una nueva entry al array `DATASETS` en la celda 1 con el `label` del robot y sus 4 arrays de valores raw (orden: lejos → cerca)
   - Usar `ACTIVE_DATASETS` para seleccionar qué robots procesar (por defecto todos)

3. **Ajustar rangos si es necesario**: Modificar `RANGE_FRONT_*` y `RANGE_SIDE_*` en la celda 4 según las distancias críticas para este robot.

4. **Revisar `p0`**: La función `estimate_p0()` calcula automáticamente buenos valores iniciales. La celda 5 imprime los `p0` estimados para verificar que son razonables.

5. **Ejecutar todas las celdas**: `Cell > Run All`

6. **Verificar errores**:
   - RMSE < 5 mm en rangos ajustados → calibración aceptable
   - RMSE > 10 mm en rangos ajustados → revisar datos o modelo
   - Si algún `a` sale negativo → los datos no siguen el modelo logarítmico, revisar mediciones
   - Error de cuantificación LUT (celda 6) > 3 mm → considerar reajustar con `raw_to_distance_firmware()`

7. **Copiar constantes al firmware**:
   - Copiar la salida de la **celda 8** (structs C agrupados al final)
   - Pegar en `set_sensors_robot_calibration()` en `sensors.c`
   - Añadir un nuevo `case ZOROBOT3_X:` con las constantes

### Solución de problemas

| Síntoma | Causa probable | Solución |
|---------|---------------|----------|
| `a` negativo en el ajuste | Datos no siguen el modelo logarítmico | Revisar mediciones, verificar que `raw` decrece con la distancia |
| Error grande en rangos excluidos | La curva no extrapola bien a esos rangos | Ampliar los rangos de ajuste en la celda 4 |
| RMSE > 10 mm en todos los sensores | Datos de calibración ruidosos o mal tomados | Repetir las mediciones, verificar iluminación |
| La LUT del firmware da resultados distintos | Error de cuantificación por división entera | Revisar el error reportado en la celda 6; si > 3 mm, usar `raw_to_distance_firmware()` en `curve_fit` |

---

## Referencias

- Firmware: [`source_code/src/sensors.c`](../../source_code/src/sensors.c) — `set_sensors_robot_calibration()`, `update_sensors_magics()`
- LUT de logaritmo: [`source_code/src/utils.c`](../../source_code/src/utils.c) — `ln_lookup[]`, `get_ln_value()`
- Constantes: [`source_code/include/constants.h`](../../source_code/include/constants.h) — `ADC_RESOLUTION`, `LOG_LINEARIZATION_TABLE_STEP`
- Estructura de calibración: [`source_code/include/sensors.h`](../../source_code/include/sensors.h) — `struct sensors_distance_calibration`
