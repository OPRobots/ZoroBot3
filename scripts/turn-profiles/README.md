# Micromouse Sinusoidal Turn Profiles

Generador de perfiles de giro con aceleración angular sinusoidal para un robot micromouse de alto rendimiento (ZoroBot3).

## Índice

- [Descripción general](#descripción-general)
- [Arquitectura](#arquitectura)
- [Sistema de coordenadas](#sistema-de-coordenadas)
- [Flujo de generación de un perfil](#flujo-de-generación-de-un-perfil)
- [Estructuras de datos clave](#estructuras-de-datos-clave)
- [Convención de signos de `.start` y `.end`](#convención-de-signos-de-start-y-end)
- [Uso del notebook](#uso-del-notebook)
- [Estrategias de velocidad](#estrategias-de-velocidad)
- [Tipos de giro](#tipos-de-giro)
- [Parámetros de salida](#parámetros-de-salida)
- [Problemas conocidos](#problemas-conocidos)
- [Archivos](#archivos)

---

## Descripción general

Este sistema genera perfiles de trayectoria para giros slalom alrededor de postes en un laberinto de micromouse. El perfil usa **aceleración angular sinusoidal** para producir transiciones suaves (jerk limitado), lo que minimiza el deslizamiento de las ruedas y el estrés mecánico.

Cada perfil describe:
- La trayectoria `(x, y)` del centro del robot
- Las trayectorias de los bordes izquierdo y derecho del robot
- El margen de seguridad respecto al poste
- Las fuerzas centrífuga y de aceleración angular
- Las distancias de entrada/salida necesarias para encadenar giros

## Arquitectura

```
micromouse_sinusoidal_turn_profiles_aux.py
│
├── Maze               — dimensiones de la celda y poste
├── RobotPhysics       — masa, inercia, geometría del robot
├── Line               — recta definida por punto de referencia y ángulo
├── lines_intersection — intersección de dos rectas
├── TurnProfile        — perfil de giro básico (in-place)
├── SlalomTurnProfile  — perfil de giro slalom con entrada/salida
├── turn_profile()     — genera perfil de velocidad angular sinusoidal
├── complete_profile() — integra velocidades → posiciones y fuerzas
├── complete_slalom_profile() — alinea con líneas de entrada/salida
├── turn_shift()       — calcula desplazamiento para alinear perfil
└── Simulator          — orquesta la generación de perfiles
```

### Clases principales

| Clase | Responsabilidad |
|-------|----------------|
| `Maze` | Dimensiones físicas: `cell` (0.18m), `post` (0.012m) |
| `RobotPhysics` | Parámetros del robot: masa, inercia, ancho, separación de ruedas, velocidad angular máxima |
| `Line` | Modela una línea por su punto de referencia `(x, y)` y su `angle`. Calcula `slope` e `intercept`. |
| `Simulator` | Punto de entrada. Métodos: `inplace()` (giro sin desplazamiento) y `slalom()` (giro con trayectoria curva) |
| `SlalomTurnProfile` | Contiene el perfil generado más las líneas de entrada/salida. Métodos: `describe()`, `describe_profile()`, `plot_trajectory()`, `plot_forces()` |

## Sistema de coordenadas

Las coordenadas de entrada/salida se definen en **unidades de celda** relativas al poste:

```
                Norte
              (0, +0.5) 
                  |
Oeste (-0.5, 0) --+-- (+0.5, 0) Este
                  |
              (0, -0.5)
                 Sur
```

- `(0, 0)` = centro del poste
- `(0, -0.5)` = centro del borde de la celda sur
- `(+0.5, 0)` = centro del borde de la celda este
- Una unidad de celda = 0.18 m (se multiplica dentro de `Simulator.slalom()`)

Los ángulos siguen la convención matemática estándar:
- `0` = Este (+x)
- `π/2` = Norte (+y)
- `π` = Oeste (-x)
- `3π/2` = Sur (-y)

## Flujo de generación de un perfil

### 1. `Simulator.slalom(entry, exit, radius, linear_velocity, shift=None)`

1. Convierte coordenadas de celda a metros
2. Calcula el ángulo total del giro: `angle = exit.angle - entry.angle`
3. Calcula la fuerza centrífuga necesaria: `F = m·v² / (2·r)`
4. Calcula la velocidad angular máxima: `ω_max = v / r` (limitada por `robot.max_angular_velocity`)
5. Calcula la aceleración angular máxima: `α_max = F · d_ruedas / I`
6. Verifica que la transición sinusoidal pueda alcanzar `ω_max`
7. Genera el perfil con `turn_profile()`
8. Completa el perfil con `complete_profile()`
9. Alinea el perfil con `complete_slalom_profile()`

### 2. `turn_profile(angle, ω_max, α_max, dt)`

Genera un perfil de velocidad angular con tres fases:

```
         ω_max  ───────────────
               ╱                ╲
             ╱                    ╲
    0  ─────                      ─────
    |--transition--|--arc--|--transition--|
```

- **Transición de entrada**: sinusoidal, `ω(t) = ω_max · sin(t/transition · π/2)`
- **Arco constante**: `ω(t) = ω_max` (velocidad angular máxima)
- **Transición de salida**: sinusoidal decreciente

Parámetros calculados:
- `duration = ω_max / α_max · π`
- `transition_angle = duration · ω_max / π`
- `arc = (|angle| - 2·transition_angle) / ω_max` (duración del arco)
- `transition = duration / 2` (duración de cada transición)

### 3. `complete_profile(profile, entry_angle, radius, v, robot, dt)`

Integra las velocidades para obtener posiciones, ángulos y fuerzas:

- **Ángulo**: `angle[t] = Σ(ω[t]·dt) + entry_angle`
- **Posición X**: `x[t] = Σ(v·cos(angle[t])·dt)`
- **Posición Y**: `y[t] = Σ(v·sin(angle[t])·dt)`
- **Fuerza centrífuga**: `Fc = m·v·ω / 2`
- **Fuerza aceleración angular**: `Fα = |I·α / d_ruedas|`
- **Fuerza total**: `F = √(Fc² + Fα²)`

### 4. `complete_slalom_profile(profile, entry, exit, robot, maze, shift=None)`

1. **Calcula el desplazamiento** (`shift`):
   - Si `shift=None`: usa `turn_shift()` automáticamente
   - Si se proporciona `shift` manual: `shift_final = shift + entry.reference`
2. **Aplica el desplazamiento** a todas las coordenadas x,y
3. **Calcula los bordes del robot**: añade/substrae `robot.width/2` perpendicular a la trayectoria
4. **Calcula el margen al poste**: distancia mínima de los bordes al poste más cercano

### 5. `turn_shift(entry, exit, profile)`

```python
expected = lines_intersection(entry, exit)
start = Line(profile[0].x, profile[0].y, angle=entry.angle)
stop  = Line(profile[-1].x, profile[-1].y, angle=exit.angle)
actual = lines_intersection(start, stop)
return expected - actual
```

Alinea el "centro de giro" del perfil con el punto de intersección de las líneas de entrada y salida del laberinto.

**Limitación**: solo funciona cuando las líneas de entrada y salida NO son paralelas. Para giros de 180° (líneas paralelas), se requiere un `shift` manual.

## Convención de signos de `.start` y `.end`

Los valores `.start` y `.end` representan la distancia (en mm) desde el primer/último punto del perfil hasta el **sensing point** (línea divisora de casillas) de entrada/salida, **con signo**. El signo indica si el perfil *reduce* o *aumenta* el tramo recto anterior/posterior:

| Efecto sobre el tramo recto | `.start` | `.end` |
|-----------------------------|----------|--------|
| **Negativo** → reduce el tramo recto anterior/posterior | El perfil empieza **antes** del sensing point (se acerca a él; el tramo recto previo se acorta) | El perfil termina **después** del sensing point (ya lo pasó; el tramo recto posterior se acorta) |
| **Positivo** → aumenta el tramo recto anterior/posterior | El perfil empieza **después** del sensing point (ya lo pasó; el tramo recto previo se alarga) | El perfil termina **antes** del sensing point (aún no llega; el tramo recto posterior se alarga) |

El punto de referencia es siempre la **línea divisoria** entre casillas (sensing point), no el centro del poste ni la línea central del pasillo.

- **`.start` negativo** (`-68.8 mm`): el perfil comienza 68.8 mm **antes** de cruzar la línea divisoria de entrada → el tramo recto de aproximación se **reduce** en 68.8 mm porque el robot empieza a girar antes de llegar a ella.
- **`.start` positivo** (`+47.9 mm`): el perfil comienza 47.9 mm **después** de cruzar la línea divisoria de entrada → el tramo recto de aproximación se **alarga** en 47.9 mm porque el robot ya ha entrado en la celda cuando empieza a girar.
- **`.end` negativo** (`-68.8 mm`): el perfil termina 68.8 mm **después** de cruzar la línea divisoria de salida → el tramo recto posterior se **reduce** en 68.8 mm porque el robot ya ha salido de la celda cuando termina el giro.
- **`.end` positivo** (`+47.9 mm`): el perfil termina 47.9 mm **antes** de cruzar la línea divisoria de salida → el tramo recto posterior se **alarga** en 47.9 mm porque el robot termina el giro antes de llegar a ella.

> **Regla mnemotécnica**: negativo = el perfil se *come* parte del tramo recto (lo reduce); positivo = el perfil *deja espacio* antes/después del sensing point (lo aumenta).

El signo se determina comparando la distancia del primer punto del perfil con la del punto siguiente (para `.start`), o la del último punto con la del punto anterior (para `.end`). Si la distancia al sensing point disminuye → el robot se acerca a él (signo negativo para `.start`, positivo para `.end`); si aumenta → se aleja (signo positivo para `.start`, negativo para `.end`).

## Uso del notebook

### Requisitos

```python
pip install ipykernel numpy pandas matplotlib
```

En caso de usar Arch Linux, instalar los paquetes del sistema:
```python
paru -S python-ipykernel python-numpy python-pandas python-matplotlib
```


### Ejecución

Abrir `turn-profiles.ipynb` en Jupyter y ejecutar todas las celdas en orden:

1. **Setup** (celda 2): define el laberinto, robot y simulador
2. **Definición de perfiles** (celda 5): genera todos los perfiles para cada velocidad
3. **Visualización** (celdas 8-18): muestra trayectorias y genera estructuras C

### Personalización

Para añadir una nueva velocidad o modificar parámetros:

```python
# En la celda 5, añadir:
nueva_velocidad90 = simulate.slalom(
    entry=(0, -.5, 0),      # Entrada: sur, rumbo este
    exit=(.5, 0, pi/2),      # Salida: este, rumbo norte
    radius=0.135,            # Radio de giro (m)
    linear_velocity=2.0      # Velocidad lineal (m/s)
)
```

### Robot físico

Parámetros actuales del robot:

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| Masa | 0.070 kg | Masa total del robot |
| Momento de inercia | 8.75×10⁻⁵ kg·m² | I = m·r²/2 (disco) |
| Ancho | 0.0702 m | Ancho total |
| Separación ruedas | 0.062 m | Distancia entre ruedas |
| ω_max | 40 rad/s | Velocidad angular máxima |

## Estrategias de velocidad

Cada estrategia define velocidades lineales crecientes para el mismo conjunto de giros:

| Estrategia | v_90 (m/s) | v_180 (m/s) | v_45 (m/s) | v_135 (m/s) | v_45to45 (m/s) |
|-----------|-----------|------------|-----------|------------|---------------|
| EXPLORE   | 0.800     | —          | —         | —          | —             |
| NORMAL    | 1.696     | 1.350      | 1.644     | 1.118      | 0.955         |
| MEDIUM    | 1.932     | 1.575      | 1.865     | 1.305      | 1.114         |
| FAST      | 2.220     | 1.800      | 2.146     | 1.491      | 1.273         |
| SUPER     | 2.490     | 2.025      | 2.404     | 1.678      | 1.432         |
| HAKI      | 2.745     | 2.250      | 2.647     | 1.864      | 1.591         |

Las velocidades más altas producen fuerzas laterales mayores; el radio de giro y la velocidad deben equilibrarse para no superar los límites de fricción.

## Tipos de giro

### Giros principales

| Tipo | Ángulo | Entry | Exit | Radio (m) | Notas |
|------|--------|-------|------|-----------|-------|
| **MOVE_90** | 90° | `(0, -.5, 0)` | `(.5, 0, π/2)` | 0.135 | Giro estándar en L |
| **MOVE_180** | 180° | `(0, -.5, 0)` | `(0, .5, π)` | 0.090 | Giro en U. **Usa shift manual** |
| **MOVE_TO_45** | 45° | `(0, -.5, 0)` | `(.5, 0, π/4)` | 0.120 | Giro de 45° |
| **MOVE_TO_135** | 135° | `(0, -.5, 0)` | `(0, .5, 3π/4)` | 0.07456 | Giro de 135° |
| **MOVE_45_TO_45** | 90° | `(0, -.5, π/4)` | `(0, .5, 3π/4)` | 0.06364 | Giro diagonal-diagonal |
| **MOVE_FROM_45** | 45° | `(0, -.5, π/4)` | `(.5, 0, π/2)` | 0.120 | Complementario de TO_45 |
| **MOVE_FROM_45_180** | 135° | `(0, -.5, π/4)` | `(0, .5, π)` | 0.07456 | Complementario de TO_135 |

### Líneas de entrada y salida

Cada giro se define sobre dos líneas del laberinto (los ejes centrales de los pasillos). Todas las entradas están al **sur** del poste; las salidas al **este** o al **norte**:

| Línea | Ecuación | Dirección | Tipo | Usada por |
|-------|----------|-----------|------|-----------|
| **E-H** (Entry Horizontal) | `y = -90 mm` | Este (0°) | Entrada | MOVE_90, MOVE_180, MOVE_TO_45, MOVE_TO_135 |
| **E-D** (Entry Diagonal) | `y = x - 90 mm` | Noreste (45°) | Entrada | MOVE_45_TO_45, MOVE_FROM_45, MOVE_FROM_45_180 |
| **X-V** (Exit Vertical) | `x = 90 mm` | Norte (90°) | Salida | MOVE_90, MOVE_FROM_45 |
| **X-H** (Exit Horizontal) | `y = 90 mm` | Oeste (180°) | Salida | MOVE_180, MOVE_FROM_45_180 |
| **X-D** (Exit Diagonal NE) | `y = x - 90 mm` | Noreste (45°) | Salida | MOVE_TO_45 |
| **X-Dnw** (Exit Diagonal NW) | `y = -x + 90 mm` | Noroeste (135°) | Salida | MOVE_TO_135, MOVE_45_TO_45 |

> **Nota**: X-D (salida de MOVE_TO_45) es exactamente la misma línea que E-D (entrada de los giros diagonales). Esta es la **única** coincidencia entre una línea de salida y una de entrada.

### Encadenamiento de giros

Para encadenar dos giros A→B, la línea de salida de A debe conectar geométricamente con la línea de entrada de B. Se distinguen dos casos:

#### Caso 1: Línea compartida (misma recta)

La salida de A y la entrada de B están en **la misma línea** geométrica. Solo ocurre con X-D = E-D (`y = x - 90 mm`):

| Giro A | Giro B | Gap físico | Segmento recto necesario |
|--------|--------|-----------|--------------------------|
| MOVE_TO_45 | MOVE_45_TO_45 | 34.1 mm | 34.1 mm recto diagonal |
| MOVE_TO_45 | MOVE_FROM_45 | 18.9 mm | 18.9 mm recto diagonal |
| MOVE_TO_45 | MOVE_FROM_45_180 | 25.1 mm | 25.1 mm recto diagonal |

En los tres casos, los puntos de referencia de salida de A `(90, 0) mm` y de entrada de B `(0, -90) mm` están separados por `CELL_HALF_DIAGONAL = 127.28 mm` sobre la misma diagonal. Los offsets `.end` y `.start` posicionan los puntos físicos del perfil a lo largo de esta línea, dejando gaps pequeños (19-34 mm) que deben recorrerse en recto.

#### Caso 2: Líneas diferentes — se requiere trayectoria intermedia

Para todos los demás 39 pares, la línea de salida y la de entrada son **diferentes**. La conexión requiere navegar por la geometría del laberinto. Se han identificado 7 combinaciones distintas de líneas:

| # | Salida de A | Entrada de B | Relación | Distancia entre refs | Conexión necesaria |
|---|------------|-------------|----------|---------------------|-------------------|
| 1 | X-Dnw (`y=-x+90`) | E-H (`y=-90`) | ⊥ (45°) | 127.28 mm | Recta diagonal CELL_HALF_DIAGONAL (127.28 mm) |
| 2 | X-Dnw (`y=-x+90`) | E-D (`y=x-90`) | ⊥ (90°) | 127.28 mm | Giro de 90° en el poste + recta |
| 3 | X-H (`y=90`) | E-H (`y=-90`) | ∥ (paralelas) | 180.00 mm | 1 CELL recto vertical |
| 4 | X-H (`y=90`) | E-D (`y=x-90`) | ⊥ (45°) | 180.00 mm | Navegación por celda |
| 5 | X-D (`y=x-90`) | E-H (`y=-90`) | ⊥ (45°) | 127.28 mm | Recta diagonal CELL_HALF_DIAGONAL |
| 6 | X-V (`x=90`) | E-H (`y=-90`) | ⊥ (90°) | 127.28 mm | Giro de 90° en el poste |
| 7 | X-V (`x=90`) | E-D (`y=x-90`) | ⊥ (45°) | 127.28 mm | Recta diagonal CELL_HALF_DIAGONAL |

Donde:
- **⊥ (45°)**: líneas a 45° (una horizontal/vertical y una diagonal) — requieren CELL_HALF_DIAGONAL
- **⊥ (90°)**: líneas perpendiculares (horizontal y vertical) — el poste está en la intersección
- **∥**: líneas paralelas — requieren CELL de separación

**El caso TO_135 → FROM_45_180**: la salida de TO_135 es por X-Dnw (`y = -x + 90mm`, ref en `(0, 90mm)`) y la entrada de FROM_45_180 es por E-D (`y = x - 90mm`, ref en `(0, -90mm)`). Son líneas diagonales perpendiculares que se cruzan en el poste. Con los puntos de referencia corregidos (ver [Problema #2](#2-move_to_135--move_from_45_180-referencia-de-salida-incorrecta)), `.end` y `.start` coinciden (+47.97 mm para NORMAL), por lo que los offsets ya codifican correctamente los tramos rectos de entrada/salida sin requerir un gap intermedio adicional.

**El caso FROM_45_180 → TO_135** (inverso): la salida de FROM_45_180 es por X-H (`y = 90mm`) y la entrada de TO_135 es por E-H (`y = -90mm`). Son líneas paralelas separadas 180 mm (1 CELL). Con los valores corregidos, `.start = -26.59 mm` y `.end = -26.59 mm` también coinciden entre ambos giros, completando la simetría inversa del par.

### Tabla resumen de offsets para el firmware

Valores de `.start` y `.end` para todas las velocidades (NORMAL, en mm):

| Giro | `.start` | `.end` | Notas |
|------|---------|--------|-------|
| MOVE_90 | -68.83 | -68.83 | Simétrico. Ambos negativos → reducen los tramos rectos de entrada y salida |
| MOVE_180 | -44.99 | -45.06 | Simétrico. Ambos negativos → reducen los tramos rectos de entrada y salida. **Requiere shift manual** |
| MOVE_TO_45 | -73.07 | +54.21 | Reduce tramo recto de entrada, alarga el de salida |
| MOVE_TO_135 | -26.59 | +47.97 | Reduce tramo recto de entrada, alarga el de salida |
| MOVE_45_TO_45 | +39.02 | +39.01 | Simétrico. Ambos positivos → alargan los tramos rectos de entrada y salida |
| MOVE_FROM_45 | +54.21 | -73.07 | Alarga tramo recto de entrada, reduce el de salida. Inverso de TO_45 |
| MOVE_FROM_45_180 | +47.97 | -26.59 | Alarga tramo recto de entrada, reduce el de salida. Inverso de TO_135 |

## Parámetros de salida

Cada perfil genera una estructura C con estos campos:

```c
struct turn_params {
    float start;              // Distancia de entrada (mm, con signo)
    float end;                // Distancia de salida (mm, con signo)
    uint16_t linear_speed;    // Velocidad lineal (mm/s)
    float max_angular_speed;  // Velocidad angular máxima (rad/s)
    float transition;         // Duración de transición (ms)
    float arc;                // Duración del arco (ms)
    int8_t sign;              // +1 = giro derecha, -1 = giro izquierda
};
```

Ejemplo de salida para NORMAL_LEFT_90:
```c
[MOVE_LEFT_90] = {
    .start = -68.8257,         // 68.8 mm antes de la referencia de entrada
    .end = -68.8336,           // 68.8 mm después de la referencia de salida
    .linear_speed = 1696,      // 1.696 m/s
    .max_angular_speed = 12.5630,
    .transition = 63.3541,     // 63.4 ms por transición
    .arc = 131.4061,           // 131.4 ms de arco constante
    .sign = -1,                // giro a izquierda
},
```

## Problemas conocidos

### 1. MOVE_180: Asimetría en `.start` / `.end`

**Estado**: ✅ CORREGIDO (2026-06-13)

**Descripción**: Para giros de 180°, el `exit` original era `(.5, 0, π)` (línea de salida horizontal `y=0` pasando por el centro del poste). Con radio 0.09m, el perfil tiene un desplazamiento vertical natural de ~182 mm pero la distancia entre las líneas de entrada (`y=-90mm`) y salida (`y=0mm`) era solo 90 mm — un desajuste geométrico de 2× que causaba `.start` ≠ `.end` (ratio 3.64×) y un error de 92.5 mm en la línea de salida.

**Solución aplicada**: Cambiar el `exit` de `(.5, 0, π)` → `(0, .5, π)` (salida al norte, línea `y=0.09`, heading oeste). La distancia vertical entre líneas pasa a ser 0.18 m, acercándose al desplazamiento natural del perfil (~0.1825 m).

**Resultados tras la corrección**:

| Velocidad | \|start\| | \|end\| | Ratio | Error salida |
|-----------|----------|--------|-------|-------------|
| NORMAL | 44.99 mm | 45.06 mm | 1.002× | 2.55 mm |
| MEDIUM | 44.98 mm | 45.06 mm | 1.002× | 2.55 mm |
| FAST | 44.98 mm | 45.07 mm | 1.002× | 2.55 mm |
| SUPER | 44.98 mm | 45.06 mm | 1.002× | 2.55 mm |
| HAKI | 44.98 mm | 45.07 mm | 1.002× | 2.55 mm |

**Nota**: El error residual de ~2.55 mm en la línea de salida se debe a que el desplazamiento vertical del perfil (182.55 mm) no es exactamente 180 mm. Ajustar el componente `y` del shift manual a `-0.00127` en vez de `0` eliminaría este error repartiéndolo simétricamente (1.27 mm en cada extremo). No obstante, 2.55 mm es aceptable para la aplicación.

### 2. MOVE_TO_135 / MOVE_FROM_45_180: Referencia de salida incorrecta

**Estado**: ✅ CORREGIDO (2026-06-15)

**Descripción**: El notebook generaba `TO_135.end = -79.31 mm` porque usaba `exit=(.5, 0, 3π/4)` — referencia de salida en (90mm, 0). El valor correcto debe ser `+47.97 mm` para que coincida con `FROM_45_180.start`, como exige el firmware (comentario en `move.c:111`: *"El .end tiene que ser el .start y el .start tiene que ser el .end del MOVE_****_FROM_45_180"*).

**Causa raíz**: La línea de salida de TO_135 es `y = -x + 90mm`. Tanto (90, 0) como (0, 90) están sobre esta misma línea, pero el `.end` mide la distancia desde el punto de referencia, no desde la línea. Con la referencia incorrecta en (90, 0), el perfil termina en (33.92, 56.08) y la distancia es 79.31mm (signo negativo porque ya pasó la referencia). Con la referencia corregida en (0, 90), la distancia al mismo punto físico es 47.97mm (signo positivo porque aún no llega a la referencia).

**Solución aplicada**: Cambiar `exit=(.5, 0, 3π/4)` → `exit=(0, .5, 3π/4)` en las 5 velocidades del notebook. Esto NO cambia la trayectoria física (misma línea, mismo `turn_shift()`, mismas posiciones) — solo cambia desde qué punto de la línea se mide `.end`.

**Resultados tras la corrección**:

| Velocidad | TO_135.end | FROM_45_180.start | ¿Coinciden? |
|-----------|-----------|-------------------|-------------|
| NORMAL | +47.97 mm | +47.97 mm | ✅ |
| MEDIUM | +47.97 mm | +47.97 mm | ✅ |
| FAST | +47.97 mm | +47.97 mm | ✅ |
| SUPER | +47.97 mm | +47.97 mm | ✅ |
| HAKI | +47.97 mm | +47.97 mm | ✅ |

**Interpretación geométrica correcta**:

```
        (0, 90mm) ← TO_135.exit_ref CORREGIDO
        ×
        |\
        | \        TO_135: el robot recorre 47.97mm recto
        |  \       desde el fin del arco hasta el límite de celda
        |   \
        |    \     FROM_45_180: el robot empieza 47.97mm después
        ×-----×    de la referencia de entrada, ya dentro de la celda
  (0,0)       (0, -90mm)
  poste       FROM_45_180.entry_ref
```

- **TO_135**: entra por el sur (heading E), gira 135° alrededor del poste, y el arco termina en (33.92, 56.08) mm. Desde ahí, el robot recorre **47.97 mm recto en dirección NW** hasta alcanzar el límite de la celda en (0, 90) mm.
- **FROM_45_180**: el robot entra en la celda por (0, -90) mm heading NE, y ya ha avanzado **47.97 mm** dentro de la celda cuando empieza el arco del giro (en (33.92, -56.08) mm).
- `.start = -26.59 mm` y `.end = -26.59 mm` también coinciden entre TO_135 y FROM_45_180 respectivamente, completando la simetría inversa del par.

**Por qué el análisis anterior era incorrecto**: La diferencia de 127.28 mm (CELL_HALF_DIAGONAL) entre `|-79.31|` y `|+47.97|` era un artefacto de usar el punto de referencia equivocado. No existe ningún "gap" que requiera un movimiento recto diagonal intermedio — los valores `.start`/`.end` ya codifican correctamente los tramos rectos de aproximación y salida cuando se usa la referencia adecuada.

### 3. Dependencia de shift manual para 180°

**Estado**: ⚠️ Limitación de diseño (funcional con la geometría corregida)

El `shift` manual `(-0.045, 0)` para giros de 180° sigue siendo necesario porque `turn_shift()` no funciona con líneas paralelas (entry angle=0, exit angle=π → slope=0 ambas). Con la geometría corregida (`exit=(0, .5, π)`), el shift actual produce resultados casi simétricos (ratio ~1.002, error <3mm). Un ajuste fino del componente `y` del shift a `-0.00127` daría simetría perfecta, pero no es necesario para la aplicación práctica.

### 4. Limitación de `turn_shift()` con líneas paralelas

`lines_intersection()` lanza `ValueError` cuando las pendientes son iguales. Esto afecta a cualquier configuración donde las líneas de entrada y salida sean paralelas (ángulos que difieren en múltiplos de π).

## Archivos

### `micromouse_sinusoidal_turn_profiles_aux.py` — Biblioteca principal

Motor de cálculo de perfiles de giro sinusoidal para micromouse. Contiene todas las clases y funciones necesarias para generar, alinear, analizar y visualizar trayectorias de giro.

**Clases:**

| Clase | Descripción |
|-------|-------------|
| `Maze` | Parámetros del laberinto: `cell` (0.18m), `post` (0.012m) |
| `RobotPhysics` | Parámetros físicos: masa, momento de inercia, ancho, separación de ruedas, velocidad angular máxima |
| `Line` | Línea recta infinita definida por punto de referencia `(x, y)` y `angle`. Calcula `slope` e `intercept` |
| `TurnProfile` | Clase base con DataFrame `profile` y método `plot_forces()` |
| `SlalomTurnProfile(TurnProfile)` | Perfil completo con referencias `entry`/`exit`. Métodos: `describe()`, `describe_profile()`, `plot_trajectory()`, `plot_forces()` |
| `Simulator` | Orquestador. Métodos: `inplace()` para giros en el sitio, `slalom()` para giros con desplazamiento |

**Funciones principales:**

| Función | Descripción |
|---------|-------------|
| `lines_intersection(l0, l1)` | Calcula el punto de intersección de dos líneas. Lanza `ValueError` si son paralelas |
| `turn_shift(entry, exit, profile)` | Calcula el vector de desplazamiento para alinear la trayectoria generada con las líneas de entrada/salida deseadas. Compara la intersección esperada con la real |
| `turn_profile(angle, max_av, max_aa, period)` | Genera perfil de velocidad angular sinusoidal: aceleración (seno 0→π/2), constante, deceleración (seno π/2→0) |
| `complete_profile(profile, start_angle, radius, lv, robot, period)` | Integra cinemáticamente: ángulo, posición (x,y), fuerzas (centrífuga, aceleración angular, total) |
| `complete_slalom_profile(profile, entry, exit, maze, robot, shift)` | Aplica `turn_shift()`, desplaza la trayectoria, calcula trayectorias de bordes del robot, margen al poste |

---

### `turn-profiles.ipynb` — Notebook principal

Define, genera y visualiza los 7 perfiles de giro para cada una de las 5 estrategias de velocidad (EXPLORE, NORMAL, MEDIUM, FAST, SUPER, HAKI). Para cada velocidad imprime los structs C listos para copiar al firmware.

**Estructura del notebook:**

| Celda | Contenido |
|-------|-----------|
| 0-4 | Setup: imports, constantes del robot, simulador |
| 5 | Definición de los 7 perfiles × 6 velocidades |
| 6-18 | Salida de structs C + gráficas de trayectoria para cada velocidad |

**Perfiles definidos (todos los movimientos MOVE_LEFT_* y MOVE_RIGHT_*):**

| Variable | Movimiento | entry | exit | Ángulo | Radio |
|----------|-----------|-------|------|--------|-------|
| `*90` | 90° estándar | `(0, -.5, 0)` | `(.5, 0, π/2)` | 90° | 0.135m |
| `*180` | 180° | `(0, -.5, 0)` | `(0, .5, π)` | 180° | 0.09m |
| `*45` / `*To45` | TO_45 | `(0, -.5, 0)` | `(.5, 0, π/4)` | 45° | 0.12m |
| `*135` / `*To135` | TO_135 | `(0, -.5, 0)` | `(0, .5, 3π/4)` | 135° | 0.07456m |
| `*45to45` | 45_TO_45 | `(0, -.5, π/4)` | `(0, .5, 3π/4)` | 90° | 0.06364m |
| `*From45` | FROM_45 | `(0, -.5, π/4)` | `(.5, 0, π/2)` | 45° | 0.12m |
| `*From45to180` | FROM_45_180 | `(0, -.5, π/4)` | `(0, .5, π)` | 135° | 0.07456m |

**Coordenadas de celda:** Los valores `.5` y `-.5` se escalan por `maze.cell` (0.18m):
- `(0, -.5)` → `(0, -90mm)` — centro del pasillo sur
- `(.5, 0)` → `(90mm, 0)` — centro del pasillo este
- `(0, .5)` → `(0, 90mm)` — centro del pasillo norte
- `(0, .5)` → `(0, 90mm)` — centro del pasillo norte

---

### `verify_profiles.py` — Verificación de casos problemáticos

Script que comprueba sistemáticamente las relaciones esperadas entre pares de perfiles de giro para las 5 velocidades.

**Casos verificados:**

| Caso | Verificación | Criterio |
|------|-------------|----------|
| **CASE 1** — MOVE_180 | `|.start| ≈ |.end|` | Simetría del giro de 180° |
| **CASE 2** — TO_135 ↔ FROM_45_180 | `.start₁ ≈ .end₂` y `.end₁ ≈ .start₂` | Correspondencia de par inverso |
| **CASE 3** — TO_45 ↔ FROM_45 | `.start₁ ≈ -.end₂` y `.end₁ ≈ -.start₂` | Par simétrico de referencia |

**Análisis adicional:**
- Geometría de líneas de entrada/salida para cada caso
- Verificación de que los puntos inicial/final están sobre sus respectivas líneas (error en mm)
- Relación entre la línea de salida de TO_135 y la de entrada de FROM_45_180
- Análisis de por qué los giros de 180° requieren `shift` manual (líneas paralelas)

**Uso:**
```bash
python3 verify_profiles.py
```

---

### `deep_analysis.py` — Trazado paso a paso de la generación de perfiles

Script que desglosa el proceso interno de generación de perfiles para entender exactamente cómo se calculan los desplazamientos, las distancias y los signos.

**Partes del análisis:**

| Parte | Contenido |
|-------|-----------|
| **PART 1** — 180° deep trace | Por qué el perfil de 180° es asimétrico: el desplazamiento vertical natural del perfil (~0.1825m) es ~2× el requerido (0.09m con el exit antiguo) |
| **PART 2** — TO_135 vs FROM_45_180 | Comparación de ángulos de giro (ambos 135°), shifts aplicados, posiciones físicas de los endpoints, diferencia con el par TO_45/FROM_45 que sí comparte línea |
| **PART 3** — Sign convention | Análisis detallado de cómo se determinan los signos de `.start` y `.end`: compara `norm(p[1]-ref)` con `norm(p[0]-ref)` para determinar si el robot se aproxima o se aleja |

**Uso:**
```bash
python3 deep_analysis.py
```

---

### `analyze_all_pairs.py` — Análisis exhaustivo de encadenamiento de giros

Script que analiza todas las combinaciones posibles de pares de giros (7×7 = 49 pares) para determinar cuáles pueden encadenarse directamente y qué segmento recto intermedio necesita cada par.

**Partes del análisis:**

| Parte | Contenido |
|-------|-----------|
| **PART 1** — Entry/Exit lines | Lista todas las líneas de entrada y salida de cada tipo de giro, mostrando ecuación, referencia y ángulo |
| **PART 2** — Direct chains | Identifica pares donde la línea de salida de A coincide exactamente con la línea de entrada de B (misma pendiente e intercepto) |
| **PART 3** — Direct chain analysis | Para cada par encadenable, calcula offsets, gaps físicos, y verifica la consistencia geométrica |
| **PART 4** — Non-direct pairs | Para pares no encadenables, analiza el ángulo entre líneas (paralelas, perpendiculares, 90°), punto de intersección, y distancias |
| **PART 5** — Reference distance matrix | Matriz 7×7 de distancias entre referencia de salida de A y referencia de entrada de B (mm) |
| **PART 6** — Straight segment analysis | Para pares encadenables, calcula la distancia recta necesaria entre el fin físico de A y el inicio físico de B |

**Uso:**
```bash
python3 analyze_all_pairs.py
```

---

### `test_new_exit.py` — Prueba de corrección de geometría para MOVE_180

Script que evalúa el cambio de `exit` para giros de 180°: de `(.5, 0, π)` (línea horizontal y=0 que pasa por el centro del poste) a `(0, .5, π)` (línea horizontal y=0.09 al norte del poste).

**Partes del análisis:**

| Parte | Contenido |
|-------|-----------|
| **PART 1** — OLD vs NEW | Compara `.start`/`.end` y simetría para las 5 velocidades con ambas definiciones de exit |
| **PART 2** — Optimal shift | Calcula el shift óptimo para la nueva geometría minimizando el error en ambas líneas |
| **PART 3** — Shift strategies | Prueba 4 estrategias: shift actual, shift óptimo, shift en Y solamente, sin shift |
| **PART 4** — Safety check | Verifica que el margen al poste se mantiene seguro para todas las velocidades |
| **PART 5** — Summary | Resumen de la mejora geométrica: con el nuevo exit, la distancia vertical entre líneas (0.18m) se aproxima mucho más al desplazamiento natural del perfil (~0.1825m) |

**Uso:**
```bash
python3 test_new_exit.py
```

---

### Flujo de trabajo típico

```
1. Definir/ajustar perfiles en turn-profiles.ipynb
2. Ejecutar el notebook para generar structs C y gráficas
3. Verificar con verify_profiles.py que los pares cumplen las relaciones esperadas
4. Si hay discrepancias, depurar con deep_analysis.py
5. Para análisis global de encadenamiento, usar analyze_all_pairs.py
6. Copiar los structs generados al firmware (move.c)
```

### Dependencias

- Python ≥ 3.10
- NumPy, Pandas, Matplotlib
- Jupyter (para el notebook)

Los scripts `.py` son independientes y no requieren Jupyter — solo importan la biblioteca auxiliar.

---

## Referencias

- **Robot**: ZoroBot3 — micromouse de competición
- **Laberinto**: Micromouse clásico (celda 0.18m, poste 0.012m)
- **Modelo físico**: Aceleración angular sinusoidal para minimizar jerk
- **Formato de salida**: Estructuras C para el firmware del robot
