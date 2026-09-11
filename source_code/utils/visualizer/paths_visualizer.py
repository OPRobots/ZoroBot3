import subprocess
import ast
import json
import os
import re
from PIL import Image, ImageDraw
import sys

if any(arg.startswith('-f') or 'kernel' in arg.lower() for arg in sys.argv):
    # Solo en Colab/Jupyter
    try:
        get_ipython()
        # Ejecuta el comando shell solo en Colab/Jupyter
        from IPython import get_ipython
        get_ipython().system('chmod +x ./maze_sim')
    except Exception:
        pass

# Bits de pared
NORTH, EAST, SOUTH, WEST = 16, 2, 4, 8

# Tabla de peligrosidad para cada tipo de movimiento
MOVEMENT_DANGER = {
    # Movimientos seguros (0 puntos)
    'MOVE_NONE': 0,
    'MOVE_START': 0,
    'MOVE_HOME': 0,
    'MOVE_END': 0,
    'MOVE_FRONT': 0,
    
    # Riesgo muy bajo (1-2 puntos)
    'MOVE_LEFT_90': 1,
    'MOVE_RIGHT_90': 1,
    'MOVE_LEFT_180': 2,
    'MOVE_RIGHT_180': 2,
    'MOVE_DIAGONAL_LEFT': 1,
    'MOVE_DIAGONAL_RIGHT': 1,
    
    # Riesgo medio (3-5 puntos)
    'MOVE_LEFT_TO_45': 3,
    'MOVE_RIGHT_TO_45': 3,
    'MOVE_LEFT_FROM_45': 3,
    'MOVE_RIGHT_FROM_45': 3,
    
    # Riesgo alto (6-10 puntos)
    'MOVE_LEFT_TO_135': 6,
    'MOVE_RIGHT_TO_135': 6,
    'MOVE_LEFT_45_TO_45': 8,
    'MOVE_RIGHT_45_TO_45': 8,
    'MOVE_LEFT_FROM_45_180': 6,
    'MOVE_RIGHT_FROM_45_180': 6,
    'MOVE_LEFT_DIAGONAL_TO_DIAGONAL': 8,
    'MOVE_RIGHT_DIAGONAL_TO_DIAGONAL': 8,
}

# ============================================
# SISTEMA DE SPRITES PARA ACCIONES
# ============================================

def bytes_to_matrix(byte_array, width, height):
    """
    Convierte array de bytes hexadecimales a matriz binaria.
    - byte_array: Lista de enteros hexadecimales (ej: [0x0FE0, 0x0000, ...])
    - width: Ancho en píxeles (debe ser múltiplo de 4 para empaque en hex)
    - height: Alto en píxeles
    Returns: Lista de listas de 0/1 (height filas × width columnas)
    """
    matrix = []
    for row_idx in range(height):
        row = []
        row_bytes = byte_array[row_idx]
        # Extraer bits del más significativo al menos significativo
        for bit_idx in range(width - 1, -1, -1):
            bit = (row_bytes >> bit_idx) & 1
            row.append(bit)
        matrix.append(row)
    return matrix

def matrix_to_bytes(matrix):
    """
    Convierte matriz binaria a array de bytes hexadecimales.
    - matrix: Lista de listas de 0/1
    Returns: Lista de enteros hexadecimales
    """
    byte_array = []
    for row in matrix:
        row_bytes = 0
        width = len(row)
        for col_idx, pixel in enumerate(row):
            if pixel:
                row_bytes |= (1 << (width - 1 - col_idx))
        byte_array.append(row_bytes)
    return byte_array

def rotate_matrix_90_cw(matrix):
    """
    Rota matriz 90° en sentido horario.
    NORTH (0) -> EAST (1) -> SOUTH (2) -> WEST (3)
    """
    height = len(matrix)
    width = len(matrix[0]) if height > 0 else 0
    rotated = []
    for c in range(width):
        new_row = []
        for r in range(height - 1, -1, -1):
            new_row.append(matrix[r][c])
        rotated.append(new_row)
    return rotated

def flip_matrix_horizontal(matrix):
    """Espeja matriz horizontalmente (izquierda <-> derecha)"""
    return [row[::-1] for row in matrix]

def flip_matrix_vertical(matrix):
    """Espeja matriz verticalmente (arriba <-> abajo)"""
    return matrix[::-1]

# Definición de sprites para cada acción (orientadas a NORTE)
# Formato: hexadecimal, 12 bits de ancho (0x000 a 0xFFF)
# Placeholder: flecha vertical (^) para verificar rotaciones
SPRITE_1CELL_PLACEHOLDER = [
    0x000,    # Fila 0:		............
    0x000,    # Fila 1:		............
    0x462,    # Fila 2:		.#...##...#.
    0x5E2,    # Fila 3:		.#.####...#.
    0x1E0,    # Fila 4:		...####.....
    0x462,    # Fila 5:		.#...##...#.
    0x462,    # Fila 6:		.#...##...#.
    0x060,    # Fila 7:		.....##.....
    0x5FA,    # Fila 8:		.#.######.#.
    0x5FA,    # Fila 9:		.#.######.#.
    0x000,    # Fila 10:	............
    0x000,    # Fila 11:	............
]

SPRITE_2CELL_PLACEHOLDER = [
    0x000,    # Fila 0:		.......................
    0x000,    # Fila 1:		.......................
    0x2308F2, # Fila 2:		.#...##....#...####..#.
    0x2F09FA, # Fila 3:		.#.####....#..######.#.
    0xF0198,  # Fila 4:		...####.......##..##...
    0x230832, # Fila 5:		.#...##....#.....##..#.
    0x230862, # Fila 6:		.#...##....#....##...#.
    0x300C0,  # Fila 7:		.....##........##......
    0x2FC9FA, # Fila 8:		.#.######..#..######.#.
    0x2FC9FA, # Fila 9:		.#.######..#..######.#.
    0x000,    # Fila 10:	.......................
    0x000,    # Fila 11:	.......................
]

SPRITE_FRONT = [
    0x060,    # Fila 0:		.....##.....
    0x060,    # Fila 1:		.....##.....
    0x060,    # Fila 2:		.....##.....
    0x060,    # Fila 3:		.....##.....
    0x060,    # Fila 4:		.....##.....
    0x060,    # Fila 5:		.....##.....
    0x060,    # Fila 6:		.....##.....
    0x060,    # Fila 7:		.....##.....
    0x060,    # Fila 8:		.....##.....
    0x060,    # Fila 9:		.....##.....
    0x060,    # Fila 10:  .....##.....
    0x060,    # Fila 11:  .....##.....
]

SPRITE_LEFT_90 = [
    0x000,    # Fila 0:		............
    0x000,    # Fila 1:		............
    0x000,    # Fila 2:		............
    0x000,    # Fila 3:		............
    0x000,    # Fila 4:		............
    0xE00,    # Fila 5:		###.........
    0xF00,    # Fila 6:		####........
    0x180,    # Fila 7:		...##.......
    0x0C0,    # Fila 8:		....##......
    0x060,    # Fila 9:		.....##.....
    0x060,    # Fila 10:  .....##.....
    0x060,    # Fila 11:  .....##.....
]

SPRITE_DIAGONAL_LEFT = [
    0x000,    # Fila 0:		............
    0x000,    # Fila 1:		............
    0x000,    # Fila 2:		............
    0x000,    # Fila 3:		............
    0x000,    # Fila 4:		............
    0x800,    # Fila 5:		#...........
    0xC00,    # Fila 6:		##..........
    0x600,    # Fila 7:		.##.........
    0x300,    # Fila 8:		..##........
    0x180,    # Fila 9:		...##.......
    0x0C0,    # Fila 10:  ....##......
    0x060,    # Fila 11:  .....##.....
]

SPRITE_LEFT_TO_45 = [
    0x000,    # Fila 0:		............
    0x000,    # Fila 1:		............
    0x000,    # Fila 2:		............
    0x000,    # Fila 3:		............
    0x000,    # Fila 4:		............
    0x800,    # Fila 5:		#...........
    0xC00,    # Fila 6:		##..........
    0x600,    # Fila 7:		.##.........
    0x300,    # Fila 8:		..##........
    0x1C0,    # Fila 9:		...###......
    0x0E0,    # Fila 10:  ....###.....
    0x060,    # Fila 11:  .....##.....
]

SPRITE_LEFT_FROM_45 = [
    0x000,  # Fila 0: 	............
    0x000,  # Fila 1: 	............
    0x000,  # Fila 2: 	............
    0x000,  # Fila 3: 	............
    0x000,  # Fila 4: 	............
    0xC00,  # Fila 5: 	##..........
    0xE00,  # Fila 6: 	###.........
    0x600,  # Fila 7: 	.##.........
    0x300,  # Fila 8: 	..##........
    0x180,  # Fila 9: 	...##.......
    0x0C0,  # Fila 10: 	....##......
    0x060,  # Fila 11: 	.....##.....
]

SPRITE_START = [
    0x060,    # Fila 0:		.....##.....
    0x060,    # Fila 1:		.....##.....
    0x060,    # Fila 2:		.....##.....
    0x060,    # Fila 3:		.....##.....
    0x0F0,    # Fila 4:		....####....
    0x0F0,    # Fila 5:		....####....
    0x0F0,    # Fila 6:		....####....
    0x0F0,    # Fila 7:		....####....
    0x000,    # Fila 8:		............
    0x000,    # Fila 9:		............
    0x000,    # Fila 10:	............
    0x000,    # Fila 11:	............
]

SPRITE_LEFT_TO_135 = [
    0x000,    # Fila 0:   .......................
    0x000,    # Fila 1:   .......................
    0x000,    # Fila 2:   .......................
    0x000,    # Fila 3:   .......................
    0x000,    # Fila 4:   .......................
    0xE00,    # Fila 5:   ...........###.........
    0x1F00,   # Fila 6:   ..........#####........
    0x3180,   # Fila 7:   .........##...##.......
    0x60C0,   # Fila 8:   ........##.....##......
    0xC060,   # Fila 9:   .......##.......##.....
    0x18060,  # Fila 10:  ......##........##.....
    0x30060,  # Fila 11:  .....##.........##.....
]

SPRITE_LEFT_DIAGONAL_TO_DIAGONAL = [
    0x000,    # Fila 0:		.......................
    0x000,    # Fila 1:		.......................
    0x000,    # Fila 2:		.......................
    0x000,    # Fila 3:		.......................
    0x000,    # Fila 4:		.......................
    0x000,    # Fila 5:		.......................
    0x000,    # Fila 6:		.......................
    0x3E00,   # Fila 7:		.........#####.........
    0x7F00,   # Fila 8:		........#######........
    0xC180,   # Fila 9:		.......##.....##.......
    0x180C0,  # Fila 10:	......##.......##......
    0x30060,  # Fila 11:	.....##.........##.....
]

SPRITE_LEFT_180 = [
    0x000,    # Fila 0:		.......................
    0x000,    # Fila 1:		.......................
    0x000,    # Fila 2:		.......................
    0x000,    # Fila 3:		.......................
    0x000,    # Fila 4:		.......................
    0x3E00,   # Fila 5:		.........#####.........
    0x7F00,   # Fila 6:		........#######........
    0xC180,   # Fila 7:		.......##.....##.......
    0x180C0,  # Fila 8:		......##.......##......
    0x30060,  # Fila 9:		.....##.........##.....
    0x30060,  # Fila 10:	.....##.........##.....
    0x30060,  # Fila 11:	.....##.........##.....
]

SPRITE_HOME = [
    0xFFF,  # Fila 0:   ############
    0xFFF,  # Fila 1:   ############
    0xB9D,  # Fila 2:   #.###..###.#
    0xA1D,  # Fila 3:   #.#....###.#
    0xE1F,  # Fila 4:   ###....#####
    0xB9D,  # Fila 5:   #.###..###.#
    0xB9D,  # Fila 6:   #.###..###.#
    0xF9F,  # Fila 7:   #####..#####
    0xA05,  # Fila 8:   #.#......#.#
    0xA05,  # Fila 9:   #.#......#.#
    0xFFF,  # Fila 10:  ############
    0xFFF,  # Fila 11:  ############
]

SPRITE_HOME_REAL = [ # ;)
    [(255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255)], 
    [(255, 255, 255), (255, 255, 255), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (120, 159, 92), (255, 255, 255), (255, 255, 255), (255, 255, 255)], 
    [(255, 255, 255), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (120, 159, 92), (255, 255, 255), (255, 255, 255)], 
    [(255, 255, 255), (119, 160, 90), (139, 186, 106), (157, 212, 121), (157, 212, 121), (139, 186, 106), (157, 212, 121), (139, 186, 106), (139, 186, 106), (157, 212, 121), (120, 159, 92), (255, 255, 255)], 
    [(255, 255, 255), (97, 134, 79), (119, 160, 90), (157, 212, 121), (139, 186, 106), (119, 160, 90), (139, 186, 106), (139, 186, 106), (139, 186, 106), (139, 186, 106), (120, 159, 92), (255, 255, 255)], 
    [(255, 255, 255), (97, 134, 79), (97, 134, 79), (139, 186, 106), (119, 160, 90), (139, 186, 106), (119, 160, 90), (139, 186, 106), (119, 160, 90), (235, 217, 195), (97, 134, 79), (255, 255, 255)], 
    [(255, 255, 255), (97, 134, 79), (97, 134, 79), (119, 160, 90), (235, 217, 195), (235, 217, 195), (235, 217, 195), (139, 186, 106), (235, 217, 195), (235, 217, 195), (97, 134, 79), (255, 255, 255)], 
    [(255, 255, 255), (97, 134, 79), (235, 217, 195), (119, 160, 90), (33, 33, 33), (33, 33, 33), (235, 217, 195), (235, 217, 195), (33, 33, 33), (33, 33, 33), (255, 255, 255), (255, 255, 255)], 
    [(255, 255, 255), (255, 255, 255), (235, 217, 195), (215, 191, 161), (240, 241, 223), (78, 74, 81), (215, 191, 161), (215, 191, 161), (240, 241, 223), (78, 74, 81), (255, 255, 255), (255, 255, 255)], 
    [(255, 255, 255), (255, 255, 255), (255, 255, 255), (195, 169, 132), (215, 191, 161), (235, 217, 195), (235, 217, 195), (235, 217, 195), (235, 217, 195), (215, 191, 161), (255, 255, 255), (255, 255, 255)], 
    [(255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (174, 143, 106), (195, 169, 132), (195, 169, 132), (195, 169, 132), (195, 169, 132), (255, 255, 255), (255, 255, 255), (255, 255, 255)], 
    [(255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255)]
]

ACTION_SPRITES = {
    # Acciones de 1 celda (12×12 píxeles)
    'MOVE_START': {
        'sprite': SPRITE_START,
        'width': 12,
        'height': 12,
        'layout': 'forward',  # Ocupa celda actual
    },
    'MOVE_FRONT': {
        'sprite': SPRITE_FRONT,
        'width': 12,
        'height': 12,
        'layout': 'forward',
    },
    'MOVE_DIAGONAL_LEFT': {
        'sprite': SPRITE_DIAGONAL_LEFT,
        'width': 12,
        'height': 12,
        'layout': 'forward',
    },
    'MOVE_DIAGONAL_RIGHT': {
        'reference': 'MOVE_DIAGONAL_LEFT',
        'transform': 'flip_horizontal',
        'layout': 'forward',
    },
    'MOVE_LEFT_90': {
        'sprite': SPRITE_LEFT_90,
        'width': 12,
        'height': 12,
        'layout': 'forward',
    },
    'MOVE_RIGHT_90': {
        'reference': 'MOVE_LEFT_90',
        'transform': 'flip_horizontal',
        'layout': 'forward',
    },
    
    # Acciones de 2 celdas (23×12 píxeles)
    'MOVE_LEFT_TO_45': {
        'sprite': SPRITE_LEFT_TO_45,
        'width': 12,
        'height': 12,
        'layout': 'forward',
    },
    'MOVE_RIGHT_TO_45': {
        'reference': 'MOVE_LEFT_TO_45',
        'transform': 'flip_horizontal',
        'layout': 'forward',
    },
    'MOVE_LEFT_FROM_45': {
        'sprite': SPRITE_LEFT_FROM_45,
        'width': 12,
        'height': 12,
        'layout': 'forward',
    },
    'MOVE_RIGHT_FROM_45': {
        'reference': 'MOVE_LEFT_FROM_45',
        'transform': 'flip_horizontal',
        'layout': 'forward',
    },
    'MOVE_LEFT_DIAGONAL_TO_DIAGONAL': {
        'sprite': SPRITE_LEFT_DIAGONAL_TO_DIAGONAL,
        'width': 23,
        'height': 12,
        'layout': 'forward_left',
    },
    'MOVE_RIGHT_DIAGONAL_TO_DIAGONAL': {
        'reference': 'MOVE_LEFT_DIAGONAL_TO_DIAGONAL',
        'layout': 'forward_right',
    },
    'MOVE_LEFT_TO_45_TO_DIAGONAL': {
        'sprite': SPRITE_2CELL_PLACEHOLDER,
        'width': 23,
        'height': 12,
        'layout': 'forward_left',
    },
    'MOVE_RIGHT_TO_45_TO_DIAGONAL': {
        'reference': 'MOVE_LEFT_TO_45_TO_DIAGONAL',
        'transform': 'flip_horizontal',
        'layout': 'forward_right',
    },
    'MOVE_LEFT_FROM_DIAGONAL_TO_45': {
        'sprite': SPRITE_2CELL_PLACEHOLDER,
        'width': 23,
        'height': 12,
        'layout': 'forward_left',
    },
    'MOVE_RIGHT_FROM_DIAGONAL_TO_45': {
        'reference': 'MOVE_LEFT_FROM_DIAGONAL_TO_45',
        'transform': 'flip_horizontal',
        'layout': 'forward_right',
    },
    'MOVE_LEFT_180': {
        'sprite': SPRITE_LEFT_180,
        'width': 23,
        'height': 12,
        'layout': 'forward_left',  # Celda actual + izquierda
    },
    'MOVE_RIGHT_180': {
        'reference': 'MOVE_LEFT_180',
        'transform': 'flip_horizontal',
        'layout': 'forward_right',
    },
    'MOVE_FRONT_180': {
        'sprite': SPRITE_2CELL_PLACEHOLDER,
        'width': 12,
        'height': 12,
        'layout': 'forward',  # Celda actual + adelante (2 celdas)
    },
    
    # Acciones adicionales del simulador
    'MOVE_DIAGONAL': {
        'sprite': SPRITE_1CELL_PLACEHOLDER,
        'width': 12,
        'height': 12,
        'layout': 'forward',  # Movimiento diagonal continuo
    },
    'MOVE_HOME': {
        'sprite': SPRITE_HOME,
        'width': 12,
        'height': 12,
        'layout': 'forward',  # Marca de llegada a casa
    },
    'MOVE_LEFT_TO_135': {
        'sprite': SPRITE_LEFT_TO_135,
        'width': 23,
        'height': 12,
        'layout': 'forward_left',
    },
    'MOVE_RIGHT_TO_135': {
        'reference': 'MOVE_LEFT_TO_135',
        'transform': 'flip_horizontal',
        'layout': 'forward_right',
    },
    'MOVE_LEFT_45_TO_45': {
        'reference': 'MOVE_LEFT_DIAGONAL_TO_DIAGONAL',
        'layout': 'forward_left',
    },
    'MOVE_RIGHT_45_TO_45': {
        'reference': 'MOVE_RIGHT_DIAGONAL_TO_DIAGONAL',
        'layout': 'forward_right',
    },
    'MOVE_LEFT_FROM_45_180': {
        'reference': 'MOVE_LEFT_TO_135',
        'transform': 'flip_horizontal',
        'layout': 'forward_right',
    },
    'MOVE_RIGHT_FROM_45_180': {
        'reference': 'MOVE_LEFT_FROM_45_180',
        'transform': 'flip_horizontal',
        'layout': 'forward_left',
    },
}

def resolve_sprite_definition(action_name):
    """
    Resuelve la definición completa de un sprite, siguiendo referencias si es necesario.
    Returns: (sprite_bytes, width, height, layout)
    """
    if action_name not in ACTION_SPRITES:
        raise ValueError(f"Acción desconocida: {action_name}")
    
    definition = ACTION_SPRITES[action_name]
    
    # Si tiene referencia, resolver recursivamente
    if 'reference' in definition:
        ref_sprite, ref_width, ref_height, _ = resolve_sprite_definition(definition['reference'])
        # Aplicar transformación si existe
        if 'transform' in definition:
            matrix = bytes_to_matrix(ref_sprite, ref_width, ref_height)
            if definition['transform'] == 'flip_horizontal':
                matrix = flip_matrix_horizontal(matrix)
            elif definition['transform'] == 'flip_vertical':
                matrix = flip_matrix_vertical(matrix)
            ref_sprite = matrix_to_bytes(matrix)
        return ref_sprite, ref_width, ref_height, definition['layout']
    else:
        return definition['sprite'], definition['width'], definition['height'], definition['layout']

def get_cell_positions_for_action(current_row, current_col, direction, layout):
    """
    Calcula las posiciones de celdas que ocupa un sprite según su layout.
    
    Args:
        current_row, current_col: Posición actual del robot
        direction: 0=NORTE, 1=ESTE, 2=SUR, 3=OESTE
        layout: 'forward', 'forward2', 'left', 'right', 'forward_left', 'forward_right'
    
    Returns: Lista de (row, col) que ocupa el sprite
    """
    # Vectores direccionales (NORTE=arriba, ESTE=derecha, SUR=abajo, OESTE=izquierda)
    dir_vectors = {
        0: (1, 0),   # NORTE: adelante = +fila
        1: (0, 1),   # ESTE: adelante = +columna
        2: (-1, 0),  # SUR: adelante = -fila
        3: (0, -1),  # OESTE: adelante = -columna
    }
    
    forward_dr, forward_dc = dir_vectors[direction]
    # Izquierda es 90° CCW respecto a adelante
    left_dr, left_dc = dir_vectors[(direction - 1) % 4]
    # Derecha es 90° CW respecto a adelante
    right_dr, right_dc = dir_vectors[(direction + 1) % 4]
    
    if layout == 'forward':
        return [(current_row, current_col)]
    elif layout == 'forward_left':
        diag_row = current_row + forward_dr + left_dr
        diag_col = current_col + forward_dc + left_dc
        return [(current_row, current_col), (diag_row, diag_col)]
    elif layout == 'forward_right':
        diag_row = current_row + forward_dr + right_dr
        diag_col = current_col + forward_dc + right_dc
        return [(current_row, current_col), (diag_row, diag_col)]
    else:
        raise ValueError(f"Layout desconocido: {layout}")

def render_colored_sprite(draw, sprite_matrix, cell_positions, direction, columns, rows, cell_interior=10, wall_width=1):
    if len(cell_positions) == 0:
        return

    # Rotar la matriz si es necesario (NORTE=0 no rota)
    matrix = sprite_matrix
    for _ in range(direction):
        matrix = [list(row) for row in zip(*matrix[::-1])]  # 90° CW

    min_row = min(r for r, c in cell_positions)
    max_row = max(r for r, c in cell_positions)
    min_col = min(c for r, c in cell_positions)
    max_col = max(c for r, c in cell_positions)

    step = cell_interior + wall_width
    x0 = min_col * step
    y0 = (rows - 1 - max_row) * step

    sprite_height = len(matrix)
    sprite_width = len(matrix[0]) if sprite_height > 0 else 0

    for sprite_y in range(sprite_height):
        for sprite_x in range(sprite_width):
            color = matrix[sprite_y][sprite_x]
            if isinstance(color, tuple) and len(color) == 3 and color != (255, 255, 255):
                img_x = x0 + sprite_x
                img_y = y0 + sprite_y
                draw.point((img_x, img_y), fill=color)

def render_action_sprite(draw, action_name, cell_positions, direction, 
                        columns, rows, cell_interior=10, wall_width=1, color="yellow"):
    """
    Renderiza un sprite de acción en el laberinto.
    
    Args:
        draw: ImageDraw object
        action_name: Nombre de la acción (ej: 'MOVE_START')
        cell_positions: Lista de (row, col) donde dibujar el sprite
        direction: Dirección actual del robot (0=NORTE, 1=ESTE, 2=SUR, 3=OESTE)
        columns, rows: Dimensiones del laberinto
        cell_interior: Píxeles interiores de celda
        wall_width: Grosor de paredes
        color: Color del sprite
    """
    if len(cell_positions) == 0:
        return
    
    # Resolver sprite
    sprite_bytes, width, height, layout = resolve_sprite_definition(action_name)
    
    # Convertir a matriz
    matrix = bytes_to_matrix(sprite_bytes, width, height)
    
    # Rotar según dirección (NORTE=0 no rota, cada incremento es 90° CW)
    for _ in range(direction):
        matrix = rotate_matrix_90_cw(matrix)
    
    # Calcular bounding box de todas las celdas ocupadas
    min_row = min(r for r, c in cell_positions)
    max_row = max(r for r, c in cell_positions)
    min_col = min(c for r, c in cell_positions)
    max_col = max(c for r, c in cell_positions)
    
    step = cell_interior + wall_width
    
    # Conversión: (row, col) del laberinto -> (x, y) de la imagen
    # Dibujar desde la esquina superior-izquierda del bounding box
    # En imagen: row mayor -> y menor (imagen tiene Y=0 arriba)
    # El sprite incluye bordes, así que empezamos en el borde, no en el interior
    x0 = min_col * step  # Incluye el borde izquierdo
    y0 = (rows - 1 - max_row) * step  # Incluye el borde superior
    
    # Dibujar píxel por píxel
    sprite_height = len(matrix)
    sprite_width = len(matrix[0]) if sprite_height > 0 else 0
    
    for sprite_y in range(sprite_height):
        for sprite_x in range(sprite_width):
            if matrix[sprite_y][sprite_x]:
                img_x = x0 + sprite_x
                img_y = y0 + sprite_y
                draw.point((img_x, img_y), fill=color)

def parse_initial_position(sim_output):
    """
    Extrae la posición y dirección inicial del robot.
    Busca líneas como: "Current position: 0" y "Current direction: 16"
    Returns: (position, direction) donde position es el índice de celda
    """
    pos_match = re.search(r'Current position:\s*(\d+)', sim_output)
    dir_match = re.search(r'Current direction:\s*(-?\d+)', sim_output)
    
    position = int(pos_match.group(1)) if pos_match else 0
    direction_raw = int(dir_match.group(1)) if dir_match else 0
    
    # La dirección es el offset en el array lineal (16x16)
    # +16 = NORTE (subir fila), +1 = ESTE (siguiente col)
    # -16 = SUR (bajar fila), -1 = OESTE (anterior col)
    direction_map = {
        16: 0,   # NORTE
        1: 1,    # ESTE
        -16: 2,  # SUR
        -1: 3,   # OESTE
    }
    
    direction = direction_map.get(direction_raw, 0)
    
    print(f"  [DEBUG] Direction raw: {direction_raw}, mapped to: {direction} ({['NORTE', 'ESTE', 'SUR', 'OESTE'][direction]})")
    
    return position, direction

def parse_actions_from_output(sim_output):
    """
    Parsea las acciones directamente del output del simulador.
    Busca líneas como: "MOVE_START > 14xMOVE_FRONT > MOVE_RIGHT_90 > ..."
    Returns: Lista de nombres de acciones expandidas
    """
    # Buscar la línea que empieza con MOVE_
    match = re.search(r'^(MOVE_\w+.*?)$', sim_output, re.MULTILINE)
    if not match:
        return []
    
    action_line = match.group(1)
    line_num = sim_output[:match.start()].count('\n')
    print(f"  [DEBUG] Encontrada línea de acciones en línea {line_num}: {action_line[:80]}...")
    
    # Dividir por " > " para obtener cada parte
    parts = action_line.split(' > ')
    
    actions = []
    for part in parts:
        part = part.strip()
        if not part:
            continue
        
        # Verificar si tiene multiplicador (ej: "14xMOVE_FRONT")
        mult_match = re.match(r'(\d+)x(\w+)', part)
        if mult_match:
            count = int(mult_match.group(1))
            action_name = mult_match.group(2)
            actions.extend([action_name] * count)
        else:
            # Acción simple
            actions.append(part)
    
    return actions

def interpolate_color(value, gradient):
    """
    Interpola color en un gradiente según un valor normalizado (0-1).
    
    Args:
        value: float entre 0 y 1
        gradient: lista de tuplas (R, G, B)
    
    Returns:
        tupla (R, G, B)
    """
    value = max(0.0, min(1.0, value))  # Clamp entre 0 y 1
    
    if value == 0.0:
        return gradient[0]
    if value == 1.0:
        return gradient[-1]
    
    # Determinar entre qué dos colores interpolar
    num_segments = len(gradient) - 1
    segment_size = 1.0 / num_segments
    segment_idx = int(value / segment_size)
    
    # Prevenir índice fuera de rango
    if segment_idx >= num_segments:
        segment_idx = num_segments - 1
    
    # Valor local dentro del segmento (0-1)
    local_value = (value - segment_idx * segment_size) / segment_size
    
    # Interpolación lineal RGB
    c1 = gradient[segment_idx]
    c2 = gradient[segment_idx + 1]
    
    r = int(c1[0] + (c2[0] - c1[0]) * local_value)
    g = int(c1[1] + (c2[1] - c1[1]) * local_value)
    b = int(c1[2] + (c2[2] - c1[2]) * local_value)
    
    return (r, g, b)

def calculate_danger_colors(actions):
    """
    Calcula el color de cada acción basándose en su peligrosidad y concatenación.
    
    Args:
        actions: Lista de nombres de acciones
    
    Returns:
        Lista de colores (tuplas RGB) correspondientes a cada acción
    """
    # Gradiente Roronoa (Marimo a Enma)
    gradient = [
        (40, 182, 27),    # Verde Marimo (#28B61B)
        (0, 255, 204),    # Cian Eléctrico (#00FFCC)
        (0, 127, 255),    # Azul Azure (#007FFF)
        (79, 0, 255),     # Índigo Eléctrico (#4F00FF)
        (160, 32, 240),   # Lila Enma (#A020F0)
        (255, 0, 255),    # Fucsia haki (#FF00FF)
    ]

    colors = []
    consecutive_danger_count = 0
    
    for action in actions:
        base_danger = MOVEMENT_DANGER.get(action, 0)
        
        # Determinar si es movimiento peligroso (danger >= 3)
        if base_danger >= 3:
            consecutive_danger_count += 1
            
            # Aplicar multiplicador exponencial basado en concatenación
            if consecutive_danger_count > 1:
                multiplier = 1 + pow((consecutive_danger_count - 1) * 0.3, 1.2)
            else:
                multiplier = 1.0
            
            danger_points = base_danger * multiplier
        else:
            # Movimiento seguro o de bajo riesgo
            danger_points = base_danger
            consecutive_danger_count = 0  # Reset contador
        
        # Normalizar danger_points a rango 0-1
        # Máximo esperado: ~10 puntos base * ~2.5 multiplicador = 25 puntos
        # Usar 30 como techo para saturar en rosa solo en casos extremos
        normalized_danger = min(danger_points / 30.0, 1.0)
        
        # Interpolar color
        color_rgb = interpolate_color(normalized_danger, gradient)
        colors.append(color_rgb)
    
    return colors

def get_action_color(action_name):
    """
    Devuelve un color único para cada tipo de curva.
    - Lados (left/right) agrupados.
    - "from" y "to" diferenciados.
    """
    # Curvas agrupadas por tipo
    curve_colors = {
        # START - END
        'MOVE_START': 'yellow',
        'MOVE_HOME': 'yellow',
        # FRONT
        'MOVE_FRONT': 'green',
        # 90°
        'MOVE_LEFT_90': 'brown',
        'MOVE_RIGHT_90': 'brown',
        # 180°
        'MOVE_LEFT_180': 'violet',
        'MOVE_RIGHT_180': 'violet',
        # 45° TO - FROM
        'MOVE_LEFT_TO_45': 'cyan',
        'MOVE_RIGHT_TO_45': 'cyan',
        'MOVE_LEFT_FROM_45': 'cyan',
        'MOVE_RIGHT_FROM_45': 'cyan',
        # 135°
        'MOVE_LEFT_TO_135': 'magenta',
        'MOVE_RIGHT_TO_135': 'magenta',
        # Diagonal
        'MOVE_DIAGONAL_LEFT': 'darkviolet',
        'MOVE_DIAGONAL_RIGHT': 'darkviolet',
        # Diagonal to diagonal
        'MOVE_LEFT_DIAGONAL_TO_DIAGONAL': 'royalblue',
        'MOVE_RIGHT_DIAGONAL_TO_DIAGONAL': 'royalblue',
        # TO_45_TO_DIAGONAL
        'MOVE_LEFT_TO_45_TO_DIAGONAL': 'lime',
        'MOVE_RIGHT_TO_45_TO_DIAGONAL': 'lime',
        # FROM_DIAGONAL_TO_45
        'MOVE_LEFT_FROM_DIAGONAL_TO_45': 'magenta',
        'MOVE_RIGHT_FROM_DIAGONAL_TO_45': 'magenta',
        # 45_TO_45
        'MOVE_LEFT_45_TO_45': 'royalblue',
        'MOVE_RIGHT_45_TO_45': 'royalblue',
        # FROM_45_180
        'MOVE_LEFT_FROM_45_180': 'magenta',
        'MOVE_RIGHT_FROM_45_180': 'magenta',
    }
    if action_name in curve_colors:
        return curve_colors[action_name]
    else:
        return 'white'  # Otros

def simulate_actions_to_positions(actions, path_cells, start_direction=0):
    """
    Asocia acciones con celdas del path óptimo.
    
    Args:
        actions: Lista de nombres de acciones
        path_cells: Lista ordenada de (row, col) del path óptimo
        start_direction: Dirección inicial (0=NORTE, 1=ESTE, 2=SUR, 3=OESTE)
    
    Returns: Lista de tuplas (action_name, cell_positions, direction)
    """
    result = []
    current_dir = start_direction
    path_idx = 0
    
    dir_names = {0: 'NORTE', 1: 'ESTE', 2: 'SUR', 3: 'OESTE'}
    
    print(f"  [DEBUG] Asociando {len(actions)} acciones con {len(path_cells)} celdas del path")

    last_turn_dir = 0  # Puede ser +1 (derecha), -1 (izquierda)

    for idx, action in enumerate(actions):
        # Obtener layout de la acción para saber cuántas celdas consume
        try:
            _, _, _, layout = resolve_sprite_definition(action)
        except ValueError as e:
            print(f"  [ERROR] Acción {idx}: {action} - {e}")
            continue
        
        # Determinar cuántas celdas del path consume esta acción
        if layout in ['forward', 'left', 'right']:
            cells_needed = 1
        elif layout in ['forward2', 'forward_left', 'forward_right']:
            cells_needed = 2
        else:
            cells_needed = 1
        
        # Extraer las celdas del path para esta acción
        if path_idx < len(path_cells):
            if path_idx + cells_needed <= len(path_cells):
                cell_positions = path_cells[path_idx:path_idx + cells_needed]
            else:
                # Si no hay suficientes celdas, tomar las que queden
                cell_positions = path_cells[path_idx:]
            
            path_idx += cells_needed
        else:
            # No hay más celdas disponibles
            cell_positions = []
            if idx < 10:
                print(f"    [{idx}] {action}: ¡Sin celdas disponibles!")
        
        # Guardar acción con sus celdas
        if action == 'MOVE_DIAGONAL':
            if last_turn_dir == -1:
                result.append(('MOVE_DIAGONAL_LEFT', cell_positions, (current_dir - 1) % 4))
            elif last_turn_dir == +1:
                result.append(('MOVE_DIAGONAL_RIGHT', cell_positions, (current_dir + 1) % 4))
        else:
            result.append((action, cell_positions, current_dir))
        
        # Debug: mostrar primeras 10 acciones
       # print(f"    [{idx}] {action}: dir={dir_names[current_dir]} cells={cell_positions} layout={layout}")
        
        # Actualizar dirección según la acción

        if action == 'MOVE_HOME':
            pass
        elif action == 'MOVE_LEFT_90':
            current_dir = (current_dir - 1) % 4
            last_turn_dir = -1
        elif action == 'MOVE_RIGHT_90':
            current_dir = (current_dir + 1) % 4
            last_turn_dir = +1
        elif action == 'MOVE_LEFT_TO_45':
            current_dir = (current_dir - 1) % 4
            last_turn_dir = -1
        elif action == 'MOVE_RIGHT_TO_45':
            current_dir = (current_dir + 1) % 4
            last_turn_dir = +1
        elif action == 'MOVE_LEFT_TO_135':
            current_dir = (current_dir - 2) % 4
            last_turn_dir = -1
        elif action == 'MOVE_RIGHT_TO_135':
            current_dir = (current_dir + 2) % 4
            last_turn_dir = +1
        elif action == 'MOVE_LEFT_180':
            current_dir = (current_dir - 2) % 4
            last_turn_dir = -1
        elif action == 'MOVE_RIGHT_180':
            current_dir = (current_dir + 2) % 4
            last_turn_dir = +1
        elif action == 'MOVE_LEFT_FROM_45':
            current_dir = (current_dir - 1) % 4
            last_turn_dir = -1
        elif action == 'MOVE_RIGHT_FROM_45':
            current_dir = (current_dir + 1) % 4
            last_turn_dir = +1
        elif action == 'MOVE_LEFT_45_TO_45':
            current_dir = (current_dir - 2) % 4
            last_turn_dir = -1
        elif action == 'MOVE_RIGHT_45_TO_45':
            current_dir = (current_dir + 2) % 4
            last_turn_dir = +1
        elif action == 'MOVE_LEFT_FROM_45_180':
            current_dir = (current_dir - 2) % 4
            last_turn_dir = -1
        elif action == 'MOVE_RIGHT_FROM_45_180':
            current_dir = (current_dir + 2) % 4
            last_turn_dir = +1
        elif action == 'MOVE_DIAGONAL':
            if last_turn_dir == -1:
                current_dir = (current_dir + 1) % 4
                last_turn_dir = +1
            elif last_turn_dir == +1:
                current_dir = (current_dir - 1) % 4
                last_turn_dir = -1
                
       # print(f"         -> nueva dir={dir_names[current_dir]}")
    
    if result and result[-1][0] == 'MOVE_HOME' and not result[-1][1]:
        if len(result) >= 2 and result[-2][1]:
            last_cell, last_dir = result[-2][1][-1], result[-2][2]
            dr, dc = {0: (1, 0), 1: (0, 1), 2: (-1, 0), 3: (0, -1)}[last_dir]
            next_cell = (last_cell[0] + dr, last_cell[1] + dc)
            result[-1] = (result[-1][0], [next_cell], 0)  # 0 = NORTE (vertical)

    print(f"  [DEBUG] Asociación completada. Celdas consumidas: {path_idx}/{len(path_cells)}")
    
    return result

def extract_array_from_map(map_path):
    with open(map_path, 'r') as f:
        content = f.read()
    # Buscar el array entre corchetes al final
    match = re.search(r'\[([\d,\s]+)\]', content)
    if not match:
        raise ValueError('No se encontró el array en el fichero .map')
    array_str = match.group(1)
    array = ast.literal_eval('[' + array_str + ']')
    return array

def call_simulator(sim_path, map_path, floodfill_type=0, explore_type=0):
    if not os.path.isfile(sim_path):
        raise RuntimeError(f"No se encuentra el simulador: '{sim_path}'")

    if not os.access(sim_path, os.X_OK):
        raise RuntimeError(f"El simulador no es ejecutable: '{sim_path}'")

    result = subprocess.run([
        sim_path, 
        f'-floodfill-type={floodfill_type}',
        f'-explore-type={explore_type}',
        map_path
    ], capture_output=True, text=True, encoding="utf-8")

    if result.returncode != 0:
        stderr = result.stderr.strip()
        raise RuntimeError(
            f"El simulador falló (código {result.returncode}) con "
            f"floodfill-type={floodfill_type}, explore-type={explore_type}.\n"
            f"{stderr if stderr else '(sin salida de error)'}"
        )

    return result.stdout

def parse_times_map(sim_output, columns=16):
    """Extrae el mapa de tiempos del simulador"""
    times_block = re.search(r'=== TIEMPOS \(FLOODFILL\) ===\s*\n(.+?)\n\n', sim_output, re.DOTALL)
    if not times_block:
        return None
    
    block = times_block.group(1)
    times = []
    
    for line in block.splitlines():
        first = line.find('║')
        last = line.rfind('║')
        if first != -1 and last != -1 and last > first:
            inner = line[first+1:last]
            # Buscar todos los números flotantes
            numbers = re.findall(r'\d+\.\d+', inner)
            if len(numbers) == columns:
                times.append([float(n) for n in numbers])
    
    # Invertir porque el output tiene fila 0 arriba
    times = times[::-1]
    return times

def parse_optimal_path(sim_output, columns=16):
    # Buscar el bloque de camino óptimo
    path_block = re.search(r'=== CELDAS VISITADAS Y CAMINO ÓPTIMO ===\s*\n(.+)', sim_output, re.DOTALL)
    if not path_block:
        print('No se encontró el camino óptimo en la salida del simulador')
        return []
    block = path_block.group(1)
    rows = []
    
    for line in block.splitlines():
        first = line.find('║')
        last = line.rfind('║')
        if first != -1 and last != -1 and last > first:
            inner = line[first+1:last]
            # Calcular el ancho esperado por celda
            total_width = len(inner)
            cell_width = total_width / columns
            
            # Buscar todas las apariciones de '███' y 'V' y determinar su columna
            cells = [None] * columns
            
            # Buscar '███'
            for match in re.finditer(r'█{3}', inner):
                pos = match.start() + 1.5  # Centro del símbolo
                col = int(pos / cell_width)
                if 0 <= col < columns:
                    cells[col] = '███'
            
            # Buscar 'V' (solo si no hay ya un '███')
            for match in re.finditer(r'V', inner):
                pos = match.start()
                col = int(pos / cell_width)
                if 0 <= col < columns and cells[col] is None:
                    cells[col] = 'V'
            
            rows.append(cells)
    
    # Invertir las filas porque el output del simulador tiene la fila 0 arriba
    # pero el array del laberinto tiene la fila 0 abajo
    rows = rows[::-1]
    
    # Extraer mapa de tiempos
    times = parse_times_map(sim_output, columns)
    
    # Extraer coordenadas del camino óptimo con sus tiempos
    path_with_times = []
    for r, row in enumerate(rows):
        for c, cell in enumerate(row):
            if cell == '███':
                time = times[r][c] if times and r < len(times) and c < len(times[r]) else 0
                path_with_times.append((r, c, time))
    
    # Ordenar por tiempo decreciente (desde inicio hasta meta)
    path_with_times.sort(key=lambda x: x[2], reverse=True)
    
    # Extraer solo las coordenadas
    path_cells = [(r, c) for r, c, _ in path_with_times]
    
    return path_cells

def highlight_special_walls_and_posts(cell_array, draw, columns, cell_interior, wall_width, color):
    rows = len(cell_array) // columns
    step = cell_interior + wall_width

    # Special cells: start and four centers
    special_cells = [(0, 0), (7, 7), (7, 8), (8, 7), (8, 8)]

    # CORRECCIÓN: Usar las constantes correctas
    NORTH_BIT, EAST_BIT, SOUTH_BIT, WEST_BIT = 16, 2, 4, 8
    wall_names = {NORTH_BIT: "NORTH", EAST_BIT: "EAST", SOUTH_BIT: "SOUTH", WEST_BIT: "WEST"}

    for r, c in special_cells:
        bits = cell_array[r * columns + c]
        x0 = wall_width + c * step
        y0 = wall_width + (rows - 1 - r) * step

        print(f"[DEBUG] Cell ({r},{c}): bits={bits} ({bits:05b})")

        # Walls
        for wall_bit, name in wall_names.items():
            if bits & wall_bit:
                print(f"  [DEBUG] Wall {name} exists, repainting in {color}")
                if wall_bit == NORTH_BIT:
                    draw.rectangle([x0, y0 - wall_width, x0 + cell_interior - 1, y0 - 1], fill=color)
                elif wall_bit == SOUTH_BIT:
                    draw.rectangle([x0, y0 + cell_interior, x0 + cell_interior - 1, y0 + cell_interior + wall_width - 1], fill=color)
                elif wall_bit == WEST_BIT:
                    draw.rectangle([x0 - wall_width, y0, x0 - 1, y0 + cell_interior - 1], fill=color)
                elif wall_bit == EAST_BIT:
                    draw.rectangle([x0 + cell_interior, y0, x0 + cell_interior + wall_width - 1, y0 + cell_interior - 1], fill=color)
            else:
                print(f"  [DEBUG] Wall {name} does NOT exist")

        # Posts (corners)
        print(f"  [DEBUG] Repainting posts for cell ({r},{c}) in {color}")
        draw.rectangle([x0 - wall_width, y0 - wall_width, x0 - 1, y0 - 1], fill=color)
        draw.rectangle([x0 + cell_interior, y0 - wall_width, x0 + cell_interior + wall_width - 1, y0 - 1], fill=color)
        draw.rectangle([x0 - wall_width, y0 + cell_interior, x0 - 1, y0 + cell_interior + wall_width - 1], fill=color)
        draw.rectangle([x0 + cell_interior, y0 + cell_interior, x0 + cell_interior + wall_width - 1, y0 + cell_interior + wall_width - 1], fill=color)

def draw_maze_from_array(cell_array,
                         output_path,
                         paths_with_colors=None,
                         actions_with_colors=None,
                         columns=16,
                         cell_interior=10,
                         wall_width=2):
    """
    Dibuja el laberinto con caminos o acciones.
    
    Args:
        paths_with_colors: Lista de tuplas (path_cells, color) para dibujar líneas
        actions_with_colors: Lista de tuplas (action_list, color) para dibujar sprites
            donde action_list es una lista de (action_name, cell_positions, direction)
    """
    rows = len(cell_array) // columns
    step = cell_interior + wall_width
    img_size = rows * cell_interior + (rows+1) * wall_width
    img = Image.new("RGB", (img_size, img_size), "black")
    draw = ImageDraw.Draw(img)

    # Dibujar postes en cruces
    for ry in range(rows+1):
        for cx in range(columns+1):
            x = cx * step
            y = ry * step
            draw.rectangle([x, y, x + wall_width - 1, y + wall_width - 1], fill="red")

    # Dibujar muros de cada celda
    for r in range(rows):
        for c in range(columns):
            bits = cell_array[r*columns + c]
            x0 = wall_width + c * step
            y0 = wall_width + (rows - 1 - r) * step
            if bits & NORTH:
                draw.rectangle([x0, y0 - wall_width, x0 + cell_interior - 1, y0 - 1], fill="red")
            if bits & SOUTH:
                draw.rectangle([x0, y0 + cell_interior, x0 + cell_interior - 1, y0 + cell_interior + wall_width - 1], fill="red")
            if bits & WEST:
                draw.rectangle([x0 - wall_width, y0, x0 - 1, y0 + cell_interior - 1], fill="red")
            if bits & EAST:
                draw.rectangle([x0 + cell_interior, y0, x0 + cell_interior + wall_width - 1, y0 + cell_interior - 1], fill="red")

    # Dibujar marco exterior completo
    draw.rectangle([0, 0, img_size - 1, wall_width - 1], fill="red")
    y_bot = img_size - wall_width
    draw.rectangle([0, y_bot, img_size - 1, img_size - 1], fill="red")
    draw.rectangle([0, 0, wall_width - 1, img_size - 1], fill="red")
    x_bot = img_size - wall_width
    draw.rectangle([x_bot, 0, img_size - 1, img_size - 1], fill="red")

    highlight_special_walls_and_posts(cell_array, draw, columns, cell_interior, wall_width, "yellow")

    # Dibujar caminos óptimos (múltiples colores)
    if paths_with_colors:
        for path_cells, color in paths_with_colors:
            for i, (r, c) in enumerate(path_cells):
                if i < len(path_cells) - 1:
                    r_next, c_next = path_cells[i + 1]
                    
                    x0 = wall_width + c * step
                    y0 = wall_width + (rows - 1 - r) * step
                    x0_next = wall_width + c_next * step
                    y0_next = wall_width + (rows - 1 - r_next) * step
                    
                    dx = c_next - c
                    dy = r_next - r
                    
                    is_first = (i == 0)
                    is_last = (i == len(path_cells) - 2)
                    
                    # Primera: 2 del centro + 10 hasta siguiente centro + 1 medio = 13
                    # Intermedia: 1 medio + 10 + 1 medio = 12
                    # Última: 1 medio + 10 + 2 del centro = 13
                    
                    if dx > 0:  # Derecha
                        start_x = x0 + 4 if is_first else x0 + 5
                        end_x = x0_next + 5 if is_last else x0_next + 4
                        start_y = end_y = y0 + 4
                    elif dx < 0:  # Izquierda
                        start_x = x0 + 4
                        end_x = x0_next + 4 if is_last else x0_next + 5
                        start_y = end_y = y0 + 5
                    elif dy > 0:  # Arriba (r aumenta, y disminuye en imagen)
                        start_y = y0 + 5 if is_first else y0 + 4
                        end_y = y0_next + 4 if is_last else y0_next + 5
                        start_x = end_x = x0 + 5
                    else:  # Abajo (r disminuye, y aumenta en imagen)
                        start_y = y0 + 5
                        end_y = y0_next + 5 if is_last else y0_next + 4
                        start_x = end_x = x0 + 4
                    
                    draw.line([(start_x, start_y), (end_x, end_y)], fill=color, width=2)

    # Dibujar acciones con sprites
    if actions_with_colors:
        for action_list, color in actions_with_colors:
            for action_name, cell_positions, direction in action_list:
                if action_name == 'MOVE_HOME':
                    render_colored_sprite(draw, SPRITE_HOME_REAL, cell_positions, direction, columns, rows, cell_interior, wall_width)
                else:
                    render_action_sprite(draw, action_name, cell_positions, direction, columns, rows, cell_interior, wall_width, color)

    dirname = os.path.dirname(output_path)
    if dirname:
        os.makedirs(dirname, exist_ok=True)
    img_grande = img.resize((img.width*4, img.height*4), Image.NEAREST)
    img_grande.save(output_path)
    print(f"Laberinto reconstruido guardado en: {output_path}")

# ============================================
# MANIFIESTO CLI (--describe)
# ============================================

PATH_OPTIONS = {"--map", "--sim", "--output"}

# Choices documentados (informativos; no modifican la validación de argparse)
FLOODFILL_CHOICES = [
    {"value": 0, "label": "BASIC", "description": "Distancia Manhattan (1.0 por celda ortogonal)."},
    {"value": 1, "label": "DIAGONAL", "description": "Coste 1.0 ortogonal / 0.7 diagonal."},
    {"value": 2, "label": "TIME", "description": "Coste en tiempo real según cinemática y penalizaciones de giro."},
]

EXPLORE_CHOICES = [
    {"value": 0, "label": "SIMPLE", "description": "Ir directo a la meta explorando en el camino."},
    {"value": 1, "label": "HOME", "description": "Explorar y volver al inicio."},
    {"value": 2, "label": "COMPLETE", "description": "Explorar todas las celdas no visitadas antes de volver."},
    {"value": 3, "label": "INFINITE", "description": "Exploración continua (puede no terminar)."},
]

INFO_ONLY_CHOICES = {
    "--floodfill": FLOODFILL_CHOICES,
    "--explore": EXPLORE_CHOICES,
}

CHOICE_DESCRIPTIONS = {
    "sprites": "Dibuja cada movimiento con su sprite real del robot.",
    "lines": "Une los centros de las celdas del camino con líneas.",
    "parts": "Una ruta con cada segmento en su color según el tipo de curva.",
    "single": "Una ruta completa en un único color.",
    "danger": "Colorea cada movimiento según su peligrosidad (gradiente).",
}


def _argparse_type_name(action):
    if action.type is int:
        return "integer"
    if action.type is float:
        return "number"
    return "string"


def build_cli_manifest(parser, program="paths_visualizer.py"):
    """Construye un manifiesto JSON de las opciones del parser de argparse."""
    import argparse

    options = []
    positionals = []

    for action in parser._actions:
        if isinstance(action, argparse._HelpAction) or action.dest == "describe":
            continue

        if not action.option_strings:
            positionals.append({
                "name": action.dest,
                "type": _argparse_type_name(action),
                "required": action.default is None,
                "description": (action.help or "").strip(),
            })
            continue

        name = next((s for s in action.option_strings if s.startswith("--")),
                    action.option_strings[0])
        aliases = [s for s in action.option_strings if s != name]

        if name in INFO_ONLY_CHOICES:
            choices = INFO_ONLY_CHOICES[name]
            opt_type = "enum"
        elif name in PATH_OPTIONS:
            choices = None
            opt_type = "path"
        elif action.choices:
            choices = [
                {"value": c, "label": str(c), "description": CHOICE_DESCRIPTIONS.get(str(c), "")}
                for c in action.choices
            ]
            opt_type = "enum"
        else:
            choices = None
            opt_type = _argparse_type_name(action)

        value_name = action.metavar or name.lstrip("-").upper().replace("-", "_")

        options.append({
            "name": name,
            "aliases": aliases,
            "type": opt_type,
            "default": action.default,
            "required": bool(getattr(action, "required", False)),
            "repeatable": action.nargs in ("+", "*"),
            "value_name": value_name,
            "choices": choices,
            "description": (action.help or "").strip(),
        })

    return {
        "schema_version": 1,
        "program": program,
        "description": parser.description or "",
        "arg_style": "space",
        "positionals": positionals,
        "options": options,
    }


def main(map_path="Portuguese Micromouse Contest 2025.map",
         sim_path="./maze_sim",
         output_path="maze_paths.bmp",
         floodfill_types=None,
         explore_types=None,
         render_mode="sprites",  # "sprites" o "lines"
         color_mode="parts"):  # "parts", "single", o "danger"
    """
    Genera visualización del laberinto con paths de floodfill.
    
    Args:
        map_path: Ruta al archivo .map
        sim_path: Ruta al ejecutable del simulador
        output_path: Ruta de salida de la imagen
        floodfill_types: Lista de tipos de floodfill a ejecutar (0-N).
        explore_types: Lista de tipos de exploración a ejecutar (0-N).
        render_mode: "sprites" para bloques de movimientos reales, "lines" para unir centros
        color_mode: 
            - "parts": Una ruta, cada segmento con su color
            - "single": Una ruta, todos los segmentos del mismo color
            - "danger": Color por peligrosidad
        El modo 'grouped' se elimina; se gestiona automáticamente según floodfill_types
    """
    
    array = extract_array_from_map(map_path)
    
    # Configurar tipos de floodfill y explore a ejecutar
    if floodfill_types is None:
        floodfill_types = [0]
    if explore_types is None:
        explore_types = [0]
    
    # Asegurar que ambas listas tengan la misma longitud
    if len(explore_types) == 1 and len(floodfill_types) > 1:
        explore_types = explore_types * len(floodfill_types)
    
    # Colores disponibles
    all_colors = ["cyan", "lime", "saddlebrown", "darkviolet", "orange", "pink", "white", "gold"]
    
    # Preparar estructuras según el modo de color
    paths_with_colors = []
    actions_with_colors = []
    routes_rendered = 0
    
    for idx, (ff_type, exp_type) in enumerate(zip(floodfill_types, explore_types)):
        print(f"\nEjecutando simulador con floodfill-type={ff_type}, explore-type={exp_type}...")
        sim_output = call_simulator(sim_path, map_path, ff_type, exp_type)
        
        # Parsear posición inicial
        position, direction = parse_initial_position(sim_output)
        start_row = position // 16
        start_col = position % 16
        start_dir = direction
        
        # Parsear acciones
        action_list = parse_actions_from_output(sim_output)
        
        if not action_list:
            print("  [ADVERTENCIA] No se encontraron acciones en la salida del simulador")
            continue
        
        print(f"  Acciones parseadas: {len(action_list)}")
        print(f"  Posición inicial: ({start_row}, {start_col}), Dirección: {start_dir}")
        
        # Obtener path óptimo
        path_cells = parse_optimal_path(sim_output, columns=16)
        
        if not path_cells:
            print("  [ADVERTENCIA] No se encontró el camino óptimo")
            continue

        routes_rendered += 1

        # Asociar acciones con celdas del path
        action_positions = simulate_actions_to_positions(action_list, path_cells, start_direction=start_dir)
        
        # VALIDACIÓN
        action_cells = set()
        for action_name, cell_positions, direction in action_positions:
            for cell in cell_positions:
                action_cells.add(cell)
        
        path_cells_set = set(path_cells)
        
        print(f"  [VALIDACIÓN] Celdas en path óptimo: {len(path_cells_set)}")
        print(f"  [VALIDACIÓN] Celdas en acciones: {len(action_cells)}")
        
        in_path_not_actions = path_cells_set - action_cells
        in_actions_not_path = action_cells - path_cells_set
        
        if in_path_not_actions:
            print(f"  [ADVERTENCIA] Celdas en path pero NO en acciones: {sorted(in_path_not_actions)[:10]}")
        if in_actions_not_path:
            print(f"  [ADVERTENCIA] Celdas en acciones pero NO en path: {sorted(in_actions_not_path)[:10]}")
        
        if not in_path_not_actions and not in_actions_not_path:
            print(f"  [OK] ✓ Las celdas coinciden perfectamente!")
        
        # Preparar datos según el modo
        if color_mode == "parts":
            # Una ruta, cada segmento con su color
            for action_name, cell_positions, direction in action_positions:
                action_color = get_action_color(action_name)
                actions_with_colors.append(([(action_name, cell_positions, direction)], action_color))
            
            # Para lines mode, usar el path completo
            if render_mode == "lines":
                paths_with_colors.append((path_cells, all_colors[idx % len(all_colors)]))
        
        elif color_mode == "single":
            # Una ruta, todos los segmentos del mismo color
            single_color = all_colors[idx % len(all_colors)]
            actions_with_colors.append((action_positions, single_color))
            paths_with_colors.append((path_cells, single_color))
        
        elif color_mode == "danger":
            # Color por peligrosidad (solo para sprites)
            action_names_only = [name for name, _, _ in action_positions]
            danger_colors = calculate_danger_colors(action_names_only)
            
            # Asignar color individual a cada acción
            for i, (action_name, cell_positions, direction) in enumerate(action_positions):
                color_rgb = danger_colors[i]
                actions_with_colors.append(([(action_name, cell_positions, direction)], color_rgb))
        
        # El modo 'grouped' se elimina; se gestiona automáticamente según floodfill_types

    if routes_rendered == 0:
        raise RuntimeError(
            "El simulador no devolvió ningún camino válido. "
            "Revisa el tipo de floodfill/explore o la salida del simulador."
        )

    # Dibujar según el modo de render
    if render_mode == "sprites":
        print(f"\n[INFO] Dibujando con sprites ({color_mode} color mode)")
        draw_maze_from_array(array, output_path, 
                           paths_with_colors=None,
                           actions_with_colors=actions_with_colors,
                           columns=16, cell_interior=10, wall_width=1)
    elif render_mode == "lines":
        print(f"\n[INFO] Dibujando con líneas ({color_mode} color mode)")
        draw_maze_from_array(array, output_path,
                           paths_with_colors=paths_with_colors,
                           actions_with_colors=None,
                           columns=16, cell_interior=10, wall_width=1)

if __name__ == "__main__":
    import sys
    import argparse
    
    # Si hay argumentos de Jupyter/Colab, no intentar parsear
    if any(arg.startswith('-f') or 'kernel' in arg.lower() for arg in sys.argv):
        # Colab: No hacer nada, el usuario llamará a main() manualmente
        pass
    else:
        # Modo CLI: parsear argumentos
        parser = argparse.ArgumentParser(
            description="Visualizador de caminos del simulador ZoroBot3 (Micromouse).")
        
        # Archivos
        parser.add_argument("--map", type=str, default="Portuguese Micromouse Contest 2025.map", 
                           help="Ruta del archivo .map")
        parser.add_argument("--sim", type=str, default="./maze_sim",
                           help="Ruta al ejecutable del simulador")
        parser.add_argument("--output", type=str, default="maze_paths.bmp", 
                           help="Ruta de salida de la imagen")
        
        # Algoritmos
        parser.add_argument("--floodfill", type=int, nargs="+", default=[0],
                           help="Tipos de floodfill a ejecutar (ej: --floodfill 0 1 2)")
        parser.add_argument("--explore", type=int, nargs="+", default=[0],
                           help="Tipos de exploración a ejecutar (ej: --explore 0 1)")
        
        # Modos de visualización
        parser.add_argument("--render", type=str, choices=["sprites", "lines"], default="sprites",
                           help="Modo de renderizado: 'sprites' (bloques) o 'lines' (unir centros)")
        parser.add_argument("--color", type=str, choices=["parts", "single", "danger"], default="parts",
                   help="Modo de color: 'parts' (cada segmento su color), 'single' (una ruta un color), 'danger' (por peligrosidad, gradiente azul-rosa). Si hay más de un floodfill, se colorea automáticamente cada ruta distinta.")

        # Descubrimiento de opciones para herramientas externas
        parser.add_argument("--describe", action="store_true",
                   help="Imprime el manifiesto JSON de opciones y sale (uso para herramientas externas).")

        args = parser.parse_args()

        if args.describe:
            manifest = build_cli_manifest(parser, program="paths_visualizer.py")
            print(json.dumps(manifest, indent=2, ensure_ascii=False))
            sys.exit(0)

        try:
            main(map_path=args.map,
                 sim_path=args.sim,
                 output_path=args.output,
                 floodfill_types=args.floodfill,
                 explore_types=args.explore,
                 render_mode=args.render,
                 color_mode=args.color)
        except RuntimeError as error:
            print(f"\n[ERROR] {error}", file=sys.stderr)
            sys.exit(1)


'''
Como utilizar:
En collan
crear celda debajo y poner:

################################################################

# Ejecutar con los parámetros deseados
main(
    map_path="Portuguese Micromouse Contest 2025.map",
    sim_path="./maze_sim",
    output_path="maze_paths.bmp",
    floodfill_types=[0, 1, 2, 3],  # Los 4 tipos de floodfill
    explore_types=[0],               # Explore tipo 0
    render_mode="sprites",           # "sprites" o "lines"
    color_mode="parts"             # "parts", "single", o "danger"
)

# Mostrar la imagen generada
from PIL import Image
from IPython.display import display

img = Image.open("maze_paths.bmp")
display(img)

# Versión escalada x3
img_grande = img.resize((img.width*3, img.height*3), Image.NEAREST)
display(img_grande)

################################################################


Por terminal:

python "array and solve to image.py" --map "Portuguese Micromouse Contest 2025.map" --sim "./maze_sim" --output "maze_paths_terminal.bmp" --floodfill 0 1 2 3 --explore 2 --render sprites --color parts

'''