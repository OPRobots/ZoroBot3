"""Descubrimiento y validacion del manifiesto CLI de los programas de ZoroBot3.

Cada programa soporta `--describe`, que imprime en stdout un manifiesto JSON con
sus opciones y descripciones (ver `cli_manifest.schema.json`). Este modulo ofrece
un cliente sencillo para que herramientas externas lo lean de forma sistematica.

Uso como libreria:
    from cli_manifest import load
    manifest = load("simulator/maze_sim")
    manifest = load(["python", "visualizer/paths_visualizer.py"])
    for option in manifest["options"]:
        print(option["name"], "-", option["description"])

Uso por terminal (desde source_code/utils):
    python cli_manifest.py simulator/maze_sim
    python cli_manifest.py visualizer/paths_visualizer.py
"""

import json
import os
import subprocess
import sys

SCHEMA_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                           "cli_manifest.schema.json")


class CliManifestError(RuntimeError):
    """Error al ejecutar --describe o al validar el manifiesto."""


def _build_command(program):
    if isinstance(program, (list, tuple)):
        cmd = [str(part) for part in program]
    else:
        path = str(program)
        cmd = [sys.executable, path] if path.endswith(".py") else [path]
    return cmd + ["--describe"]


def load(program, timeout=30):
    """Ejecuta `<programa> --describe` y devuelve el manifiesto como dict.

    `program` puede ser la ruta a un ejecutable, la ruta a un script .py, o una
    lista de tokens (p.ej. ["python", "paths_visualizer.py"]).
    """
    cmd = _build_command(program)
    try:
        result = subprocess.run(cmd, capture_output=True, text=True,
                                encoding="utf-8", timeout=timeout)
    except FileNotFoundError as error:
        raise CliManifestError(f"No se encuentra el programa: {cmd[0]}") from error
    except subprocess.TimeoutExpired as error:
        raise CliManifestError(
            f"El programa no respondio a --describe en {timeout}s: {' '.join(cmd)}"
        ) from error

    if result.returncode != 0:
        stderr = result.stderr.strip()
        raise CliManifestError(
            f"'{' '.join(cmd)}' fallo (codigo {result.returncode}).\n"
            f"{stderr if stderr else '(sin stderr)'}"
        )

    try:
        manifest = json.loads(result.stdout)
    except json.JSONDecodeError as error:
        raise CliManifestError(f"La salida de --describe no es JSON valido: {error}") from error

    validate(manifest)
    return manifest


def validate(manifest, schema_path=SCHEMA_PATH):
    """Valida el manifiesto. Usa `jsonschema` si esta disponible; si no, una
    comprobacion estructural minima. Devuelve True o lanza CliManifestError."""
    try:
        import jsonschema
    except ImportError:
        _minimal_validate(manifest)
        return True

    with open(schema_path, "r", encoding="utf-8") as schema_file:
        schema = json.load(schema_file)
    try:
        jsonschema.validate(instance=manifest, schema=schema)
    except jsonschema.ValidationError as error:
        raise CliManifestError(f"Manifiesto invalido: {error.message}") from error
    return True


def _minimal_validate(manifest):
    if not isinstance(manifest, dict):
        raise CliManifestError("El manifiesto no es un objeto JSON")

    required_top = ["schema_version", "program", "description", "arg_style",
                    "positionals", "options"]
    for key in required_top:
        if key not in manifest:
            raise CliManifestError(f"Falta la clave obligatoria '{key}'")

    if manifest["schema_version"] != 1:
        raise CliManifestError(f"schema_version no soportada: {manifest['schema_version']}")
    if manifest["arg_style"] not in ("equals", "space"):
        raise CliManifestError(f"arg_style invalido: {manifest['arg_style']}")
    if not isinstance(manifest["positionals"], list) or not isinstance(manifest["options"], list):
        raise CliManifestError("'positionals' y 'options' deben ser listas")

    required_option = ["name", "aliases", "type", "default", "required",
                       "repeatable", "value_name", "choices", "description"]
    for option in manifest["options"]:
        for key in required_option:
            if key not in option:
                raise CliManifestError(f"Opcion '{option.get('name', '?')}' sin clave '{key}'")


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if not argv or argv[0] in ("-h", "--help"):
        print("Uso: python cli_manifest.py <programa> [args...]")
        print("  Ej.: python cli_manifest.py simulator/maze_sim")
        print("       python cli_manifest.py visualizer/paths_visualizer.py")
        return 0 if argv else 1

    program = argv[0] if len(argv) == 1 else argv
    try:
        manifest = load(program)
    except CliManifestError as error:
        print(f"[ERROR] {error}", file=sys.stderr)
        return 1

    print(json.dumps(manifest, indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    sys.exit(main())
