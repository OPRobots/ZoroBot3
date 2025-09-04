#ifndef MMMSIM_API_H
#define MMMSIM_API_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 32

int API_mazeWidth(void);
int API_mazeHeight(void);

int API_wallFront(void);
int API_wallRight(void);
int API_wallLeft(void);

int API_moveForward(void); // Returns 0 if crash, else returns 1
void API_turnRight(void);
void API_turnLeft(void);
void API_moveBack(void);

void API_setWall(int x, int y, char direction);
void API_clearWall(int x, int y, char direction);

void API_setColor(int x, int y, char color);
void API_clearColor(int x, int y);
void API_clearAllColor(void);

void API_setText(int x, int y, char *str);
void API_setFloodFill(int x, int y, float value);
void API_clearText(int x, int y);
void API_clearAllText(void);

int API_wasReset(void);
void API_ackReset(void);

void API_log(char *text);

#endif