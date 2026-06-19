# Gestión de EEPROM


## Hardware y Configuración

El STM32F405RGT6 no tiene EEPROM dedicada. Se emula usando un sector de flash:

| Parámetro | Valor |
|-----------|-------|
| Sector flash | 11 |
| Dirección base | `0x080E0000` |
| Tamaño del sector | 128 KB |
| Ocupación actual | ~0.9% (283 palabras = 1132 bytes) |

Archivos: [`eeprom.c`](../source_code/src/eeprom.c), [`eeprom.h`](../source_code/include/eeprom.h).

---

## Layout de Datos

La EEPROM emulada almacena un bloque contiguo de palabras de 32 bits con checksum al final:

```
Offset  Contenido                          Tamaño (words)
──────  ─────────────────────────────────  ──────────────
0       Offset Z giroscopio                MPU_DATA_LENGTH
        (3 valores: 1000/2000/4000 dps)
───
N       Offsets de distancia (4 sensores)  4
───
        Umbrales raw de detección (×4)     4
───
        Distancias medias objetivo (×4)     4
───
        Laberinto (MAZE_CELLS celdas)      256
───
        Configuración del menú RUN         7
───
        Datos RC5                          5
───
        Checksum (complemento a 2)         1
```

### Índices de Acceso

```c
DATA_INDEX_GYRO_Z                         // Offset giroscopio
DATA_INDEX_SENSORS_OFFSETS                // Offsets de distancia
DATA_INDEX_SENSORS_RAW_THRESHOLDS         // Umbrales raw
DATA_INDEX_SENSORS_MIDDLE_TARGET_DISTANCE // Distancias medias
DATA_INDEX_MAZE                           // Mapa del laberinto
DATA_INDEX_MENU_RUN                       // Config del menú
DATA_INDEX_RC5                            // Códigos RC5
```

---

## Checksum

### Algoritmo

**Checksum aditivo con complemento a 2**:

```c
// Al guardar:
uint32_t sum = Σ(data[0..DATA_LENGTH−1]);
checksum = −sum;  // Complemento a 2

// Al cargar:
if (Σ(data[0..DATA_LENGTH]) == 0 && data_no_es_todo_ceros)
    ✓ Válido
else
    ✗ Corrupto → warning_eeprom()
```

### Validación

En `eeprom_load()`:
1. Suma todas las palabras incluyendo el checksum.
2. Si `suma != 0`: datos corruptos → llama a `warning_eeprom()` (LED de advertencia).
3. Si `suma == 0` y no todo ceros: datos válidos.
4. Si todo ceros: EEPROM vacía (primer arranque).

---

## Operaciones

### `eeprom_save()`
1. Parpadea LED de estado.
2. Calcula checksum.
3. **Desbloquea** el sector flash.
4. **Borra** el sector completo (128 KB).
5. **Programa** palabra por palabra usando `flash_program_word()`.
6. **Bloquea** el sector flash.

⚠️ El borrado del sector es **lento** (~1-2 segundos) y **bloqueante**. Toda la EEPROM se reescribe completa en cada `save()`.

### `eeprom_load()`
1. Lee desde flash al buffer RAM.
2. Valida checksum.
3. Si es válido:
   - Carga offsets del giroscopio → `lsm6dsr_load_eeprom()`
   - Carga offsets, umbrales y distancias de sensores → `sensors_load_eeprom()`
   - Carga maze → `floodfill_load_maze()`
   - Carga config del menú → `menu_run_load_values()`
   - Carga datos RC5 → `rc5_load_eeprom()`
4. Si no es válido: warning + imprime "EEPROM vacía o corrupta".
5. Imprime estadísticas de ocupación por USART.

### `eeprom_clear()`
Borra el sector flash (sin reprogramar).

### `eeprom_backup()`
Imprime el contenido actual como array C para backup externo:
```c
int16_t backup[] = {1234, -567, 890, ...};
```

### `eeprom_restore()`
Sobrescribe el buffer con un array hardcodeado de backup.

### `eeprom_set_data(index, data, length)`
Copia datos al buffer en el offset especificado. No escribe a flash (requiere `eeprom_save()` posterior).

---

## Carga durante el Arranque

En `main()`:

```c
setup();              // Inicializar hardware
eeprom_load();        // Cargar TODOS los datos persistidos
handle_robot_version(); // Detectar versión A/B/C
```

Esto asegura que calibraciones, maze y configuración estén disponibles antes de entrar al menú.

---

## Subsistemas que Usan EEPROM

| Subsistema | Datos guardados | Cuándo |
|-----------|----------------|--------|
| **Giroscopio** | Offset Z calibrado | `lsm6dsr_gyro_z_calibration()` |
| **Sensores** | Offsets, umbrales, distancias medias | `front_sensors_calibration()`, `side_sensors_calibration()` |
| **Floodfill** | Laberinto explorado | `floodfill_explore_finish(true)`, `go_to_target()` al llegar a meta |
| **Menú** | Configuración actual | `menu_run_load_values()` |
| **RC5** | Códigos de control remoto | Configuración RC5 |

---

## Issue Relacionado

- **SS-15** ✅ CORREGIDO: Se añadió checksum aditivo (complemento a 2) para validar integridad de datos. Anteriormente no había validación, lo que podía causar uso de datos corruptos sin detección.

---

*Documento generado el 2026-06-12. Ver también [Calibración](09-calibration.md), [Sensores](03-sensors.md), [Floodfill](05-floodfill.md).*
