# Rutas de inclusión
INCLUDE_DIRS := -I../../lib/mmsim_api -I../../include

# Definiciones del compilador
CC := gcc
CFLAGS := $(INCLUDE_DIRS) -D MMSIM_ENABLED

# Archivos fuente
SRCS := \
    mmsim_api.c \
    ../../src/control.c \
    ../../src/floodfill.c \
    ../../src/maze.c \
    ../../src/move.c \
    ../../src/floodfill_weigths.c \
    ../../src/sensors.c \
    ../../src/menu_run.c \
    mmsim.c

# Nombre del ejecutable
OUT := mmsim.exe

# Objetivo principal
.PHONY: all
all:
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT)
	@echo Done.
