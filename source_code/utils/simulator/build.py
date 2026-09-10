"""
PlatformIO script para compilar el simulador standalone después del firmware.
Se ejecuta como post-build script tras cada 'pio run'.
"""

import os
import platform
import subprocess
from SCons.Script import ARGUMENTS

Import("env")

SIMULATOR_DIR = os.path.join(env["PROJECT_DIR"], "utils", "simulator")


def build_simulator(target, source, env):
    """Compila el simulador usando el Makefile correspondiente al SO."""
    system = platform.system()

    if system == "Windows":
        makefile = "Makefile.windows"
    else:
        makefile = "Makefile.linux"

    print(f"\n--- Compilando simulador ({makefile}) ---")
    result = subprocess.run(
        ["make", "-f", makefile, "-C", SIMULATOR_DIR],
        capture_output=True,
        text=True,
    )

    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr)

    if result.returncode != 0:
        print(f"--- ERROR: Compilacion del simulador fallo (codigo {result.returncode}) ---")
        env.Exit(1)

    print("--- Simulador compilado correctamente ---")
    return None


# Registrar como action post-build
env.AddPostAction("$PROGPATH", build_simulator)
