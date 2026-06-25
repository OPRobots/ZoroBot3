# ZoroBot3

Robot micromouse de alto rendimiento — 3ª versión del equipo OPRobots.
8 medallas de oro en competiciones europeas (Portugal, España, Rumanía, 2023–2026).

## Stack tecnológico
- **Microcontrolador**: STM32F405RGT6 ARM Cortex-M4F @ 168 MHz
- **Framework**: LibOpenCM3
- **Entorno**: PlatformIO + VSCode
- **Lenguaje**: C11
- **Sensores**: 4× IR (SFH-4550 + ST-1KL3A), AS5145B-HSST (encoders magnéticos 12-bit), LSM6DSR (IMU/giroscopio SPI)
- **Actuadores**: 2× motores DC con drivers DRV8212, ventilador de succión
- **PCB**: KiCad (diseño propio)
- **Chasis**: PCB + piezas 3D (PLA), engranajes en resina

## Estructura del repositorio
- `source_code/` — firmware (PlatformIO + LibOpenCM3)
  - `include/` — 28 headers (sensores, control, movimiento, floodfill, menú, ...)
  - `src/` — 30 archivos .c
  - `lib/` — librerías externas (lsm6dsr_reg, mmsim_api)
  - `boards/` — board definition STM32F405RGT6
  - `utils/` — simulador standalone (C) + visualizador (Python)
- `pcb_files/` — diseño electrónico KiCad + gerbers + BOM
- `3d_model/` — piezas mecánicas STL
- `docs/` — documentación técnica MkDocs Material (15 documentos)
- `scripts/` — calibración y análisis (Jupyter notebooks)
- `images/` — fotos, banderas, sprites del simulador

## Frecuencias del sistema
- **CPU**: 168 MHz
- **SysTick / Control**: 1 kHz
- **ADC**: 16 kHz (4 sensores IR en secuencia)
- **PWM motores**: 20 kHz
- **SPI IMU**: ODR 1666 Hz, filtro paso-bajo ~100 Hz
- **UART debug**: 115200 baud

## Convenciones de código
- C11 con tipos explícitos (`uint16_t`, `int32_t`, `float`)
- Funciones con prefijo del módulo: `sensors_*`, `move_*`, `control_*`, `floodfill_*`
- ISR del SysTick en `main.c` — distribuye tareas por fases a 1 kHz
- Compilación condicional: `#ifdef SIMULATOR`, `#ifdef DEBUG`
- Configuración persistente en sector flash 11 (EEPROM emulada)

## Documentación
- Usa `/doc-init` para regenerar la documentación desde cero
- Usa `/doc-review` para revisar cambios incrementales y actualizar known issues
- La documentación sigue el estándar OPRobots (MkDocs Material + estilo ZoroBot3)
- Construir localmente: `mkdocs build` (sitio en `site/`)
- Despliegue automático en GitHub Pages (docs.oprobots.org) vía monorepo OPRobots/docs

## Notas
- El simulador MMSIM está integrado en el firmware vía compilación condicional
- Los scripts de calibración (`scripts/sensors-profiles/`, `scripts/turn-profiles/`) son Jupyter notebooks
- La documentación de known issues (`15-known-issues.md`) tiene 47 issues documentados
- El robot usa un sistema de succión (ventilador centrífugo) para mejorar el agarre en curvas rápidas
