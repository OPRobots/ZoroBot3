import os
import subprocess
import re
import glob
import sys
from pathlib import Path

def parse_total_distance(output):
    """Extrae el valor de Total Distance del output del simulador"""
    if not output:
        return None
    pattern = r'Total Distance:\s*(\d+)'
    match = re.search(pattern, output)
    if match:
        return int(match.group(1))
    return None

def run_simulator(maze_file, floodfill_type, simulator_path):
    """Ejecuta el simulador con un tipo de floodfill específico"""
    cmd = [simulator_path, f'-floodfill-type={floodfill_type}', maze_file]
    try:
        # FIX: encoding='utf-8' con errors='ignore' y verificar None
        result = subprocess.run(
            cmd, 
            capture_output=True, 
            text=True, 
            timeout=30,
            encoding='utf-8',
            errors='ignore'
        )
        # Verificar que stdout y stderr no sean None antes de concatenar
        stdout = result.stdout if result.stdout is not None else ""
        stderr = result.stderr if result.stderr is not None else ""
        return stdout + stderr
    except subprocess.TimeoutExpired:
        print(f"  Timeout floodfill {floodfill_type}")
        return ""
    except Exception as e:
        print(f"  Error floodfill {floodfill_type}: {str(e)[:50]}")
        return ""

def process_maze(simulator_path, maze_folder, output_file):
    """Procesa todos los laberintos de la carpeta"""
    # Buscar archivos .map y .txt
    maze_files = []
    for ext in ['*.map', '*.txt']:
        maze_files.extend(glob.glob(os.path.join(maze_folder, ext)))
    
    maze_files = sorted(set(maze_files))  # Eliminar duplicados y ordenar
    
    if not maze_files:
        print("No se encontraron archivos .map o .txt")
        return
    
    results = []
    
    print(f"Procesando {len(maze_files)} laberintos...")
    
    for maze_file in maze_files:
        print(f"Procesando: {os.path.basename(maze_file)}")
        
        distances = []
        for floodfill_type in [0, 1, 2, 3]:
            output = run_simulator(maze_file, floodfill_type, simulator_path)
            distance = parse_total_distance(output)
            
            if distance is not None:
                distances.append(str(distance))
                print(f"  Floodfill {floodfill_type}: {distance}")
            else:
                distances.append("ERROR")
                print(f"  Floodfill {floodfill_type}: ERROR")
        
        # Guardar resultado
        maze_name = os.path.basename(maze_file)
        result_line = f"{distances[0]}\t{distances[1]}\t{distances[2]}\t{distances[3]}\t\"{maze_name}\""
        results.append(result_line)
    
    # Escribir resultados al archivo
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("0\t1\t2\t3\tnombre\n")
            for line in results:
                f.write(line + "\n")
        print(f"\nResultados guardados en: {output_file}")
    except Exception as e:
        print(f"Error escribiendo archivo: {e}")
    
    print(f"Total laberintos procesados: {len(results)}")

def main():
    # Verificar argumentos
    if len(sys.argv) != 2:
        print("Uso: python procesar_laberintos.py <carpeta_laberintos>")
        print("Ejemplo: python procesar_laberintos.py \"C:\\mis_laberintos\"")
        sys.exit(1)
    
    maze_folder = sys.argv[1]
    SIMULATOR_PATH = r".\maze_sim.exe"
    OUTPUT_FILE = "resultados_floodfill.txt"
    
    # Verificar que la carpeta existe
    if not os.path.exists(maze_folder):
        print(f"ERROR: No se encuentra la carpeta {maze_folder}")
        sys.exit(1)
    
    # Verificar que el simulador existe
    if not os.path.exists(SIMULATOR_PATH):
        print(f"ERROR: No se encuentra el simulador en {SIMULATOR_PATH}")
        print("Asegúrate de que maze_sim.exe está en la carpeta simulator/")
        sys.exit(1)
    
    process_maze(SIMULATOR_PATH, maze_folder, OUTPUT_FILE)

if __name__ == "__main__":
    main()