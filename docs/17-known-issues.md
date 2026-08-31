# Problemas Conocidos

> **Fecha de análisis**: 2026-06-11
> **Última actualización**: 2026-08-25
> **Total**: 64 issues — 10 críticos, 24 moderados, 30 leves

> **Última revisión**: 2026-08-25 (commits `aa1e633`..`2bbbe13`)
> **Issues nuevos**: 17 (HW-01..HW-05, SW-01..SW-03, SP-01..SP-09)
> **Issues corregidos**: 0
> **Issues actualizados**: 0

---

## Correcciones Recientes ✅

| ID | Fecha | Descripción |
|----|-------|-------------|
| SS-01 | 2026-06-11 | Desbordamiento de tabla LUT — añadido bounds checking |
| SS-02 | 2026-06-11 | Análisis de timing: margen 97x, fallo improbable |
| SS-03 | 2026-06-11 | Filtro mediana N=3 + EMA adaptativo (α=0.2/0.6, umbral 10mm) |
| SS-07 | 2026-06-11 | Threshold raw corregido con ley 1/d² (factor 1.5× distancia) |
| SS-15 | 2026-06-11 | Checksum aditivo (complemento a 2) en EEPROM + warning_eeprom |

---

## 🔴 Críticos (10 issues)

### Floodfill

<a id="ff-01"></a>
#### FF-01 — Estado insuficiente en el BFS para modo TIME_BASED

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/floodfill.c#L340"><code>floodfill.c:340</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>El floodfill almacena un único <code>float</code> por celda. En TIME_BASED, el coste de transición depende del estado (dirección + count). Un camino con tiempo ligeramente superior pero mejor dirección se descarta.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Alto</strong> — puede no encontrar la ruta óptima global.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Condición <code>&gt;=</code> permite caminos con mismo tiempo pero distinto estado.</td></tr>
<tr><td><strong>Solución</strong></td><td>Ampliar estado a <code>floodfill[celda][direccion]</code> o usar A*.</td></tr>
</table>

<a id="ff-02"></a>
#### FF-02 — Desbordamiento de la cola de prioridad

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/include/floodfill.h#L63"><code>floodfill.h:63</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>Cola con capacidad fija de <code>MAZE_CELLS</code>. Cada celda puede encolarse hasta 4 veces. 16×16 = 256 slots, peor caso ~1024 pushes.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Alto</strong> — corrupción de memoria.</td></tr>
<tr><td><strong>Solución</strong></td><td>Redimensionar a <code>MAZE_CELLS × 4</code> o cola circular con bounds checking.</td></tr>
</table>

<a id="ff-03"></a>
#### FF-03 — Penalización de giro ausente en transición ortogonal→ortogonal

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/floodfill.c#L515-L520"><code>floodfill.c:515-520</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>El caso <code>from_orthogonal &amp;&amp; to_orthogonal</code> no verifica si la dirección cambió.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo en práctica (cubierto por otros casos del switch).</td></tr>
</table>

### Sensores

<a id="ss-01"></a>
#### SS-01 — Desbordamiento de tabla LUT ✅ CORREGIDO

<table>
<tr><td><strong>Solución</strong></td><td>Añadido bounds checking: <code>if (ln_index &gt;= LOG_LINEARIZATION_TABLE_SIZE) ln_index = LOG_LINEARIZATION_TABLE_SIZE − 1</code>.</td></tr>
</table>

<a id="ss-02"></a>
#### SS-02 — Pérdida de conversiones ADC ✅ CORREGIDO

<table>
<tr><td><strong>Solución</strong></td><td>Análisis de timing confirmó margen 97x. Sin riesgo real.</td></tr>
</table>

<a id="ss-03"></a>
#### SS-03 — Sin filtrado de lecturas ADC ✅ CORREGIDO

<table>
<tr><td><strong>Solución</strong></td><td>Añadido: mediana N=3 + EMA adaptativo.</td></tr>
</table>

### Movimiento

<a id="mv-01"></a>
#### MV-01 — PID sin anti-windup (6 integradores)

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/control.c#L392-L451"><code>control.c:392-451</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>Los 6 integradores acumulan error sin clamping. Tras saturación, overshoot masivo.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Alto</strong> — inestabilidad severa.</td></tr>
<tr><td><strong>Solución</strong></td><td>Implementar clamping condicional (back-calculation o conditional integration).</td></tr>
</table>

<a id="mv-02"></a>
#### MV-02 — `move_inplace_turn()` siempre gira a la izquierda

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/move.c#L126-L143"><code>move.c:126-143</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>MOVE_BACK, MOVE_BACK_WALL y MOVE_BACK_STOP tienen <code>sign = −1</code> fijo.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio-Alto — desviación sistemática en exploración.</td></tr>
</table>

<a id="mv-03"></a>
#### MV-03 — `lsm6dsr_read_gyro_z_raw()` depende de auto-incremento SPI no verificado

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/lsm6dsr.c#L66-L73"><code>lsm6dsr.c:66-73</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>Asume que IF_INC está habilitado para leer ZL+ZH consecutivos.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio — datos incorrectos si IF_INC no está configurado.</td></tr>
</table>

<a id="mv-04"></a>
#### MV-04 — `platform_write()` y `platform_read()` ignoran parámetro `len`

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/lsm6dsr.c#L48-L59"><code>lsm6dsr.c:48-59</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>Solo transfieren 1 byte independientemente del parámetro <code>len</code>.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio — frágil ante cambios de la librería LSM6DSR.</td></tr>
</table>

---

## 🟡 Moderados (24 issues)

### Floodfill

<a id="ff-04"></a>
#### FF-04 — `total_time` incorrecto en tabla de pesos

<table>
<tr><td><strong>Descripción</strong></td><td><code>total_time</code> incorrecto en tabla de pesos (primera celda contada 2×).</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo.</td></tr>
</table>

<a id="ff-05"></a>
#### FF-05 — Índice incorrecto en `mmsim_get_estimated_time()`

<table>
<tr><td><strong>Descripción</strong></td><td><code>mmsim_get_estimated_time()</code> usa índice incorrecto (off-by-one).</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio.</td></tr>
</table>

<a id="ff-06"></a>
#### FF-06 — Modelo de penalización de giro inexacto

<table>
<tr><td><strong>Descripción</strong></td><td>Modelo de penalización de giro inexacto (asume deceleración completa).</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio.</td></tr>
</table>

<a id="ff-07"></a>
#### FF-07 — Orientación inicial no considerada al planificar

<table>
<tr><td><strong>Descripción</strong></td><td>Floodfill no considera orientación inicial del robot al planificar.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio.</td></tr>
</table>

### Sensores

<a id="ss-04"></a>
#### SS-04 — Fórmula con posible división por ~0

<table>
<tr><td><strong>Descripción</strong></td><td>Fórmula con posible división por ~0.</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-05"></a>
#### SS-05 — Parámetro c negativo (mitigado)

<table>
<tr><td><strong>Descripción</strong></td><td>Parámetro c negativo (mitigado).</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-06"></a>
#### SS-06 — Filtro EMA sustituido por adaptativo

<table>
<tr><td><strong>Descripción</strong></td><td>Filtro EMA sustituido por adaptativo en SS-03.</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-07"></a>
#### SS-07 — Umbral raw corregido con ley 1/d² ✅ CORREGIDO

<table>
<tr><td><strong>Descripción</strong></td><td>Umbral raw corregido con ley 1/d² (factor 1.5× distancia).</td></tr>
</table>

<a id="ss-08"></a>
#### SS-08 — Crosstalk óptico potencial

<table>
<tr><td><strong>Descripción</strong></td><td>Crosstalk óptico potencial.</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-09"></a>
#### SS-09 — Umbrales cableados en `get_side_sensors_error()`

<table>
<tr><td><strong>Descripción</strong></td><td>Umbrales cableados en <code>get_side_sensors_error()</code>.</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-10"></a>
#### SS-10 — Fórmula no validada contra datasheet

<table>
<tr><td><strong>Descripción</strong></td><td>Fórmula no validada contra datasheet (validada empíricamente).</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-15"></a>
#### SS-15 — Sin validación de datos EEPROM ✅ CORREGIDO

<table>
<tr><td><strong>Descripción</strong></td><td>Sin validación de datos EEPROM. Corregido con checksum aditivo (complemento a 2) + <code>warning_eeprom</code>.</td></tr>
</table>

### Movimiento

<a id="mv-05"></a>
#### MV-05 — Distancia de frenada usa velocidad ideal

<table>
<tr><td><strong>Descripción</strong></td><td>Distancia de frenada usa velocidad ideal, no real.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio.</td></tr>
</table>

<a id="mv-06"></a>
#### MV-06 — Filtros independientes en velocidad angular

<table>
<tr><td><strong>Descripción</strong></td><td>Velocidad angular con filtros independientes (desfase entre canales).</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo-Medio.</td></tr>
</table>

<a id="mv-07"></a>
#### MV-07 — PI con 4 decimales

<table>
<tr><td><strong>Descripción</strong></td><td>PI con 4 decimales (error 0.003%).</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo.</td></tr>
</table>

<a id="mv-08"></a>
#### MV-08 — Fórmulas de `move_inplace_turn()` no cinemáticamente exactas

<table>
<tr><td><strong>Descripción</strong></td><td>Fórmulas <code>move_inplace_turn()</code> no cinemáticamente exactas.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio.</td></tr>
</table>

<a id="mv-09"></a>
#### MV-09 — Degradación de giro solo hacia abajo

<table>
<tr><td><strong>Descripción</strong></td><td>Degradación de giro solo hacia abajo.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Medio.</td></tr>
</table>

<a id="mv-10"></a>
#### MV-10 — Posible inconsistencia de signos en control angular

<table>
<tr><td><strong>Descripción</strong></td><td>Posible inconsistencia de signos en control angular.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Alto potencial.</td></tr>
</table>

<a id="mv-11"></a>
#### MV-11 — `move_straight()` negativo no comprueba wall loss

<table>
<tr><td><strong>Descripción</strong></td><td><code>move_straight()</code> negativo no comprueba wall loss.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo.</td></tr>
</table>

### Hardware

<a id="hw-01"></a>
#### HW-01 — Esquemático y PCB desincronizados

<table>
<tr><td><strong>Archivos</strong></td><td><code>pcb_files/kicad_project/ZoroBot3.kicad_sch</code>, <code>pcb_files/kicad_project/ZoroBot3.kicad_pcb</code></td></tr>
<tr><td><strong>Descripción</strong></td><td>El commit <code>ced009d</code> añade la SRAM 23AA04M al esquemático (SRAM1 + RRAM1 + CSRAM1 + net <code>NSS_SRAM</code>) y elimina los amplificadores de corriente AD8418, pero el PCB no se ha actualizado: la SRAM no está colocada y los footprints <code>AD8418AWBZ_1/_2</code> siguen presentes. El diff del PCB en ese commit es solo migración de formato KiCad 8→10 (198 footprints idénticos).</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio-Alto</strong> — fabricar la placa en este estado produciría un PCB sin SRAM y con amplificadores sin función; riesgo de olvido si no se sincroniza antes de fabricar.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna — el desfase es silencioso hasta exportar gerbers.</td></tr>
<tr><td><strong>Solución</strong></td><td>Ejecutar <em>Update PCB from Schematic</em> y recolocar/rutear las piezas nuevas y huérfanas antes de la próxima fabricación.</td></tr>
</table>

<a id="hw-02"></a>
#### HW-02 — Lectura de corriente sin amplificador front-end

<table>
<tr><td><strong>Archivos</strong></td><td><code>pcb_files/kicad_project/ZoroBot3.kicad_sch</code> (nets <code>CURR_SEN_MI</code>/<code>CURR_SEN_MD</code>)</td></tr>
<tr><td><strong>Descripción</strong></td><td>El commit <code>ced009d</code> anunciaba "añadir lectura de corriente" pero elimina los amplificadores AD8418 y los shunts (<code>SHUNT_MI</code>/<code>SHUNT_MD</code>). Los canales ADC CH14/CH15 (PC4/PC5) quedan conectados sin shunt ni etapa de ganancia, por lo que no miden corriente utilizable.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — la lectura de corriente de motores queda inoperativa; los canales ADC quedan sin señal útil.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna — el firmware no usa estos canales actualmente.</td></tr>
<tr><td><strong>Solución</strong></td><td>Rediseñar el front-end (p. ej. reinsertar AD8418 con shunt, o un amplificador de nueva generación) o eliminar los canales y su documentación.</td></tr>
</table>

<a id="hw-03"></a>
#### HW-03 — Diodo de protección del ventilador de succión ausente

<table>
<tr><td><strong>Archivos</strong></td><td><code>pcb_files/kicad_project/ZoroBot3.kicad_sch</code>, <code>ZoroBot3.kicad_pro</code></td></tr>
<tr><td><strong>Descripción</strong></td><td>El commit <code>ced009d</code> menciona un "diodo en succión" pero ningún fichero guardado contiene ese diodo: <code>D14</code> solo existe como designador fantasma en <code>used_designators</code> del proyecto KiCad. El ventilador queda sin diodo de libre circulación/protección.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — picos de tensión inducidos por el motor del ventilador pueden dañar el MOSFET de control (Q1).</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna en el diseño actual.</td></tr>
<tr><td><strong>Solución</strong></td><td>Añadir el diodo al esquemático (en paralelo con el ventilador, cátodo a positivo) y propagar a PCB/BOM.</td></tr>
</table>

### Scripts Python

<a id="sp-01"></a>
#### SP-01 — Dependencias de los scripts sin declarar

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/turn-profiles/*.py</code>, <code>scripts/turn-profiles/*.ipynb</code>, <code>scripts/sensors-profiles/sensors-profiles.ipynb</code></td></tr>
<tr><td><strong>Descripción</strong></td><td>Los scripts importan numpy, matplotlib y pandas (y <code>scipy.optimize</code> en sensors-profiles) sin que exista ninguna declaración de dependencias en el repo: no hay <code>requirements.txt</code> ni <code>pyproject.toml</code> en la raíz ni en <code>scripts/</code> (los únicos requirements son de docs y solo contienen mkdocs). Un entorno limpio falla con <code>ModuleNotFoundError</code> y no queda registro del contrato de runtime.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — los scripts no son reproducibles en una máquina nueva.</td></tr>
<tr><td><strong>Solución</strong></td><td>Añadir un <code>requirements.txt</code> (numpy, pandas, matplotlib, scipy) en <code>scripts/</code> o en la raíz y referenciarlo en los README de los scripts.</td></tr>
</table>

<a id="sp-02"></a>
#### SP-02 — Imports dependientes del directorio de trabajo

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/turn-profiles/aux/verify_profiles.py:9</code>, <code>aux/deep_analysis.py:7</code>, <code>aux/analyze_all_pairs.py:10</code>, <code>aux/test_new_exit.py:14</code>, <code>turn-profiles.ipynb</code> (celda 2)</td></tr>
<tr><td><strong>Descripción</strong></td><td>Los cuatro scripts auxiliares hacen <code>sys.path.insert(0, '.')</code> e importan la biblioteca principal: solo funciona si el CWD es exactamente <code>scripts/turn-profiles/</code>. El notebook tiene la misma dependencia oculta (importa el módulo sin manipular el path). Desde cualquier otro directorio falla con <code>ModuleNotFoundError</code> sin diagnóstico útil.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — scripts frágiles ante la forma de invocación.</td></tr>
<tr><td><strong>Solución</strong></td><td>Derivar la ruta del módulo desde <code>__file__</code> (p. ej. <code>sys.path.insert(0, str(Path(__file__).resolve().parent.parent))</code>) o empaquetar el módulo; documentar el requisito de CWD en el notebook.</td></tr>
</table>

---

## 🟢 Leves (30 issues)

### Floodfill

<a id="ff-08"></a>
#### FF-08 — Código inalcanzable tras `return` en switches

<table>
<tr><td><strong>Descripción</strong></td><td>Código inalcanzable tras <code>return</code> en switches.</td></tr>
</table>

<a id="ff-09"></a>
#### FF-09 — `#pragma GCC diagnostic` ignora `-Wswitch`

<table>
<tr><td><strong>Descripción</strong></td><td><code>#pragma GCC diagnostic</code> ignora <code>-Wswitch</code>.</td></tr>
</table>

<a id="ff-10"></a>
#### FF-10 — `time_penalty` retorna 0 si `init_speed >= speed`

<table>
<tr><td><strong>Descripción</strong></td><td><code>time_penalty</code> retorna 0 si <code>init_speed &gt;= speed</code>.</td></tr>
</table>

<a id="ff-11"></a>
#### FF-11 — `get_direction_value()` devuelve 0 para diagonales

<table>
<tr><td><strong>Descripción</strong></td><td><code>get_direction_value()</code> devuelve 0 para diagonales.</td></tr>
</table>

<a id="ff-12"></a>
#### FF-12 — Sesgo direccional en `floodfill_run()`

<table>
<tr><td><strong>Descripción</strong></td><td>Sesgo direccional en <code>floodfill_run()</code> (NORTH siempre gana empates).</td></tr>
</table>

<a id="ff-13"></a>
#### FF-13 — `cells_to_max_speed` puede exceder el buffer de pesos

<table>
<tr><td><strong>Descripción</strong></td><td><code>cells_to_max_speed</code> puede exceder <code>FLOODFILL_MAX_WEIGHTS_COUNT</code>.</td></tr>
</table>

### Sensores

<a id="ss-11"></a>
#### SS-11 — `update_side_sensors_leds()` con código repetitivo

<table>
<tr><td><strong>Descripción</strong></td><td><code>update_side_sensors_leds()</code> con código repetitivo.</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-12"></a>
#### SS-12 — `all_sensors_take_values()` bucle infinito (debug)

<table>
<tr><td><strong>Descripción</strong></td><td><code>all_sensors_take_values()</code> bucle infinito (debug).</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-13"></a>
#### SS-13 — Flag `sensors_taking_values` nunca a false

<table>
<tr><td><strong>Descripción</strong></td><td>Flag <code>sensors_taking_values</code> nunca a false.</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

<a id="ss-14"></a>
#### SS-14 — División entera en calibración

<table>
<tr><td><strong>Descripción</strong></td><td>División entera en calibración (error ±1 sobre ~2000).</td></tr>
<tr><td><strong>Estado</strong></td><td>NO APLICA.</td></tr>
</table>

### Movimiento

<a id="mv-12"></a>
#### MV-12 — `avg_micrometers`/`avg_millimeters` nunca actualizadas

<table>
<tr><td><strong>Descripción</strong></td><td><code>avg_micrometers</code>/<code>avg_millimeters</code> nunca actualizadas (código muerto).</td></tr>
</table>

<a id="mv-13"></a>
#### MV-13 — `volatile` innecesario en variables de control PID

<table>
<tr><td><strong>Descripción</strong></td><td><code>volatile</code> innecesario en variables de control PID.</td></tr>
</table>

<a id="mv-14"></a>
#### MV-14 — Expresión redundante `true &&` en `move_arc_turn()`

<table>
<tr><td><strong>Descripción</strong></td><td>Expresión redundante <code>true &amp;&amp;</code> en <code>move_arc_turn()</code>.</td></tr>
</table>

<a id="mv-15"></a>
#### MV-15 — Número mágico `1.75f` en `move_back()`

<table>
<tr><td><strong>Descripción</strong></td><td>Número mágico <code>1.75f</code> en <code>move_back()</code>.</td></tr>
</table>

<a id="mv-16"></a>
#### MV-16 — Parámetros `start`/`end` negativos sin validación de rango

<table>
<tr><td><strong>Descripción</strong></td><td>Parámetros <code>start</code>/<code>end</code> negativos sin validación de rango.</td></tr>
</table>

<a id="mv-17"></a>
#### MV-17 — `check_wall_loss_correction()` solo paredes laterales

<table>
<tr><td><strong>Descripción</strong></td><td><code>check_wall_loss_correction()</code> solo paredes laterales.</td></tr>
</table>

<a id="mv-18"></a>
#### MV-18 — Frecuencia acoplada rígidamente a SysTick = 16 kHz

<table>
<tr><td><strong>Descripción</strong></td><td>Frecuencia acoplada rígidamente a SysTick = 16 kHz.</td></tr>
</table>

<a id="mv-19"></a>
#### MV-19 — Wrap-around de timer de encoders (verificado correcto ✅)

<table>
<tr><td><strong>Descripción</strong></td><td>Wrap-around de timer de encoders (verificado correcto ✅).</td></tr>
</table>

### Hardware

<a id="hw-04"></a>
#### HW-04 — BOM desactualizado

<table>
<tr><td><strong>Archivos</strong></td><td><code>pcb_files/gerbers/bom.csv</code> (+ <code>designators.csv</code>, <code>positions.csv</code>)</td></tr>
<tr><td><strong>Descripción</strong></td><td>El BOM generado es anterior al rango revisado: lista los AD8418 eliminados del esquemático y no incluye SRAM1/RRAM1/CSRAM1 (23AA04M, 10 kΩ, 100 nF).</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — pedido de componentes incorrecto si se usa el BOM sin regenerar.</td></tr>
<tr><td><strong>Solución</strong></td><td>Regenerar el BOM desde el esquemático actual (export BOM de KiCad) cuando el diseño se estabilice.</td></tr>
</table>

<a id="hw-05"></a>
#### HW-05 — SRAM sin soporte en firmware

<table>
<tr><td><strong>Archivos</strong></td><td><code>source_code/</code> (sin driver); <code>pcb_files/kicad_project/ZoroBot3.kicad_sch</code> (SRAM1)</td></tr>
<tr><td><strong>Descripción</strong></td><td>La SRAM 23AA04M-I/ST (4 Mbit) está conectada al bus SPI3 compartido con CS dedicado en PB12 (pull-up 10 kΩ), pero no existe driver ni inicialización en el firmware. PB12 está libre en el código actual.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — la SRAM no es utilizable hasta añadir el driver.</td></tr>
<tr><td><strong>Solución</strong></td><td>Implementar driver SPI (modo byte) con CS en PB12 e integrar el uso previsto de la SRAM.</td></tr>
</table>

### Arquitectura Software

<a id="sw-01"></a>
#### SW-01 — Comentarios de `setup_spi()` incorrectos

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/setup.c#L328-L332"><code>setup.c:328-332</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>El comentario documenta CPHA 0 ("los datos de entrada se capturan en el flanco ascendente") pero el código usa <code>SPI_CR1_CPHA_CLK_TRANSITION_1</code> (CPHA 1, captura en el segundo flanco). Además los comentarios de frecuencia mezclan PCLK1 = 36 MHz con 84 MHz para el cálculo del divisor SPI.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — riesgo de configuraciones erróneas en cambios futuros guiados por el comentario.</td></tr>
<tr><td><strong>Solución</strong></td><td>Corregir el comentario al valor real del registro (CPHA 1) y unificar las frecuencias de reloj citadas.</td></tr>
</table>

<a id="sw-02"></a>
#### SW-02 — `setup_spi_low_speed()` código muerto

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/setup.c#L375-L378"><code>setup.c:375-378</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>La función <code>setup_spi_low_speed()</code> está definida pero nunca se llama en todo el firmware.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — mantenibilidad.</td></tr>
<tr><td><strong>Solución</strong></td><td>Eliminarla o usarla donde corresponda.</td></tr>
</table>

<a id="sw-03"></a>
#### SW-03 — openocd_reset.cfg referencia STM32F401

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/openocd_reset.cfg"><code>openocd_reset.cfg</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>El comentario sobre flash de banco único menciona "STM32F401"; el target real es un STM32F405 (también de banco único, por lo que el comportamiento descrito es correcto).</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — confusión al leer la configuración.</td></tr>
<tr><td><strong>Solución</strong></td><td>Cambiar el comentario a STM32F405.</td></tr>
</table>

### Scripts Python

<a id="sp-03"></a>
#### SP-03 — `ValueError` sin mensaje en perfiles slalom infeasibles

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/turn-profiles/micromouse_sinusoidal_turn_profiles_aux.py:299</code>, <code>aux/test_new_exit.py:199-200</code></td></tr>
<tr><td><strong>Descripción</strong></td><td>Cuando un perfil slalom es infeasible, se lanza <code>raise ValueError</code> sin mensaje. El capturador en <code>test_new_exit.py</code> imprime <code>turn_shift failed (lines parallel) - {e}</code> con mensaje vacío, atribuyendo la causa a líneas paralelas cuando en realidad es la comprobación de velocidad angular de la transición.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — diagnóstico engañoso.</td></tr>
<tr><td><strong>Solución</strong></td><td><code>raise ValueError(f'transition-only angular velocity {max_angular_velocity_transition:.3f} &lt; required {max_angular_velocity:.3f}')</code>.</td></tr>
</table>

<a id="sp-04"></a>
#### SP-04 — `except Exception` genérico oculta bugs como fallos de estrategia

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/turn-profiles/aux/test_new_exit.py:232</code></td></tr>
<tr><td><strong>Descripción</strong></td><td>El <code>except Exception as e</code> exterior del bucle de estrategias imprime <code>ERROR - {e}</code> y hace <code>continue</code>. Cualquier error de programación real (KeyError, NaN, typo) dentro del try se reporta como fallo de la estrategia de shift, sin traceback ni re-raise. El <code>except ValueError</code> interno (líneas 194-201) ya cubre el caso esperado.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — bugs reales se ocultan como resultados legítimos del análisis.</td></tr>
<tr><td><strong>Solución</strong></td><td>Eliminar el <code>except Exception</code> exterior, o re-lanzar tras imprimir con <code>traceback.print_exc()</code>.</td></tr>
</table>

<a id="sp-05"></a>
#### SP-05 — Código muerto y comentado sin explicación

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/turn-profiles/aux/test_new_exit.py:82</code>, <code>micromouse_sinusoidal_turn_profiles_aux.py:288</code></td></tr>
<tr><td><strong>Descripción</strong></td><td>Línea muerta errónea en <code>test_new_exit.py:82</code> (usa la columna <code>y</code> en vez de <code>x</code>, marcada como "wrong, let me recalc", nunca usada — el cálculo correcto sigue en 85-86) y fórmula comentada en la biblioteca (línea 288) sin explicación de por qué se sustituyó.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — mantenibilidad; la línea errónea induciría a error si se reactivara.</td></tr>
<tr><td><strong>Solución</strong></td><td>Eliminar la línea 82 y el comentario de la 288 (o documentar el motivo de la sustitución).</td></tr>
</table>

<a id="sp-06"></a>
#### SP-06 — Encoding corrupto en `verify_profiles.py`

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/turn-profiles/aux/verify_profiles.py</code></td></tr>
<tr><td><strong>Descripción</strong></td><td>El archivo contiene 1428 bytes de <code>?</code> literales donde debería haber ✓/✗/═/acentos: "Script de verificaci??n", separadores "????????", y los marcadores pass/fail muestran "???" en ambas ramas. El veredicto del script — su función principal — queda ilegible.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — el lector debe inferir el resultado de los ratios numéricos.</td></tr>
<tr><td><strong>Solución</strong></td><td>Reescribir los caracteres (✓/✗, ═, verificación) y verificar la codificación UTF-8 al guardar.</td></tr>
</table>

<a id="sp-07"></a>
#### SP-07 — Clasificación "PERPENDICULAR" errónea en `analyze_all_pairs.py`

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/turn-profiles/aux/analyze_all_pairs.py:297-306</code></td></tr>
<tr><td><strong>Descripción</strong></td><td>Tras normalizar los ángulos a [0, π/2], el código clasifica como perpendiculares los pares con <code>abs(angle_between - π/4) &lt; 0.01</code>. Las líneas perpendiculares difieren π/2 (90°), no π/4: pares a 45° se etiquetan "PERPENDICULAR", alimentando el razonamiento de PART 4 sobre qué pares de giro necesitan tramo recto intermedio.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — decisiones de diseño basadas en salida incorrecta.</td></tr>
<tr><td><strong>Solución</strong></td><td><code>is_perpendicular = abs(angle_between - π/2) &lt; 0.01</code>.</td></tr>
</table>

<a id="sp-08"></a>
#### SP-08 — Notebook de turn-profiles con 3 MB de figuras y títulos con mojibake

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/turn-profiles/turn-profiles.ipynb</code> (celdas 7, 9, 11, 13, 15, 17)</td></tr>
<tr><td><strong>Descripción</strong></td><td>42 figuras PNG de 1600×1600 embebidas en el notebook (3.06 MB total) y 36 títulos con la secuencia "Âº" (mojibake del carácter °): p. ej. <code>plot_trajectory(title = "90Âº NORMAL")</code>. El notebook pesa 3.1 MB, renderiza lento y los títulos guardados están corruptos.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — repo inflado y salida degradada.</td></tr>
<tr><td><strong>Solución</strong></td><td>Reducir a <code>dpi=80-100</code> o guardar figuras en <code>images/</code> y referenciarlas; reemplazar <code>Âº</code> por <code>°</code>.</td></tr>
</table>

<a id="sp-09"></a>
#### SP-09 — Structs de calibración solo en stdout, sin artefacto diffable

<table>
<tr><td><strong>Archivos</strong></td><td><code>scripts/sensors-profiles/sensors-profiles.ipynb</code> (celda 8)</td></tr>
<tr><td><strong>Descripción</strong></td><td>Los coeficientes a/b/c que deben aterrizar en <code>sensors.c</code> se emiten solo como salida impresa de la celda 8. No existe artefacto en fichero ni comprobación contra los valores commitados en el firmware: cambiar <code>ACTIVE_DATASETS</code> o los rangos de ajuste modifica silenciosamente los structs emitidos sin dejar rastro diffable.</td></tr>
<tr><td><strong>Impacto</strong></td><td>Bajo — las constantes del firmware pueden derivar del análisis sin detectarse.</td></tr>
<tr><td><strong>Solución</strong></td><td>Hacer que la celda 8 escriba a un fichero (p. ej. <code>generated/sensors_calibration.h</code>) o afirme igualdad con los valores del firmware; añadir nota de orden de ejecución.</td></tr>
</table>

---

## Resumen por Prioridades

| Prioridad | Cantidad | Issues |
|-----------|:--------:|--------|
| 🔴 Críticos | 10 | FF-01..03, SS-01..03, MV-01..04 |
| 🟡 Moderados | 24 | FF-04..07, SS-04..10/15, MV-05..11, HW-01..03, SP-01..02 |
| 🟢 Leves | 30 | FF-08..13, SS-11..14, MV-12..19, HW-04..05, SW-01..03, SP-03..09 |
| **TOTAL** | **64** | |

---

*Documento actualizado el 2026-08-25. El registro de issues se mantiene en este mismo documento, con anchors estables (`#xx-NN`) para referencias cruzadas desde el resto de la documentación.*
