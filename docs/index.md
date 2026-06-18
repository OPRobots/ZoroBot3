# Documentación Técnica de ZoroBot3

> Robot Micromouse de alto rendimiento con STM32F4, encoders magnéticos de alta resolución, ventilador de succión y 4 sensores infrarrojos.
>
> **Versión del documento**: 2026-06-18
> **Branch**: `develop`

---

## 🏆 Resultados en Competición

| 🥇 | 🥈 | 🥉 |
|:--:|:--:|:--:|
| **8** | **0** | **0** |

Competiciones ganadas: OSHWDem 2023-2025, RoboChallenge 2024-2025, Portuguese Contest 2025-2026.

---

## 📚 Índice de Documentación

Cada sección enlaza a un documento detallado independiente.

### 1. Hardware
- **[Hardware](01-hardware.md)** — Especificaciones del robot: MCU, sensores, motores, encoders, giroscopio, batería, chasis, PCB, versiones del robot (A/B/C).

### 2. Arquitectura Software
- **[Arquitectura Software](02-software-architecture.md)** — Bucle principal, SysTick ISR, distribución de tareas a 1 kHz, flujo de carrera, detección de inicio de competición, algoritmos de exploración.

### 3. Sensores
- **[Sistema de Sensores](03-sensors.md)** — Hardware IR (SFH-4550 + ST-1KL3A), máquina de estados ADC, linealización a distancia, detección de paredes con histéresis, calibración, cálculo de errores para control PID.

### 4. Movimiento
- **[Sistema de Movimiento](04-movement.md)** — Tipos de movimiento (26), perfiles de giro sinusoidales, movimientos rectos multi-celda, orquestración de speed run, wall loss correction.

### 5. Floodfill
- **[Algoritmo Floodfill](05-floodfill.md)** — Tipos (BASIC, DIAGONAL, TIME_BASED), cálculo de pesos cinemáticos, BFS con cola de prioridad, path following, exploración, speed run.

### 6. Control PID
- **[Sistema de Control PID](06-control-system.md)** — Arquitectura de 7 lazos PID en cascada, fórmula PID, anti-windup, compensación de batería, rampas de aceleración, detección de saturación.

### 7. Menú y Algoritmos
- **[Sistema de Menú](07-menu-system.md)** — Navegación por menú con botón, selección de algoritmo (HandWall, FloodFill, TimeTrial, DragRace), configuración de velocidad, opciones de debug.

### 8. Debug
- **[Sistema de Debug](08-debug-system.md)** — LEDs de información, LED RGB, puerto serie USART, funciones de debug, toma de valores raw.

### 9. Calibración
- **[Calibraciones](09-calibration.md)** — Calibración frontal, frontal media, lateral, calibración de giroscopio, persistencia en EEPROM.

### 10. EEPROM
- **[Gestión de EEPROM](10-eeprom.md)** — Diseño de almacenamiento, checksum (complemento a 2), carga/guardado de calibraciones, maze persistente, ocupación.

### 11. Encoders y Giroscopio
- **[Encoders y Giroscopio](11-encoders-gyro.md)** — AS5145B-HSST (12-bit), lectura por timer en cuadratura, max_likelihood_counter_diff, LSM6DSR (SPI, 1000-4000dps), filtro paso-bajo, integración angular.

### 12. Batería y LEDs
- **[Batería y LEDs](12-battery-leds.md)** — Monitorización de voltaje, divisor de tensión, LEDs de estado, LED RGB, control de ventilador.

### 13. Simulador
- **[Simulador MMSIM](13-simulator.md)** — Integración con Micromouse Simulator, API de paredes virtuales, estimación de tiempo.

### 14. Cinemática y Estrategias de Velocidad
- **[Cinemática](14-kinematics.md)** — Estrategias de velocidad (EXPLORE a HAKI), perfiles de aceleración, parámetros de giro, degradación de velocidad.

### 15. Problemas Conocidos
- **[Registro de Issues](15-known-issues.md)** — Issues documentados con IDs, descripciones, impacto y soluciones.

---

## 🔧 Stack Tecnológico

| Componente | Detalle |
|-----------|---------|
| **MCU** | STM32F405RGT6 @ 168 MHz (ARM Cortex-M4F) |
| **Framework** | LibOpenCM3 + PlatformIO |
| **Lenguaje** | C11 |
| **Compilador** | GCC ARM Embedded (arm-none-eabi-gcc) |
| **IDE** | VSCode |
| **Control de versiones** | Git (branch `develop`) |
