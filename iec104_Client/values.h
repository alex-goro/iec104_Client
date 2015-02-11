#pragma once

void Values_Init();
int Values_Add(char type, char addr, char *ptr);
int Values_GetNum(char type);
int Values_GetValue(char type, int index, char **ptr);
int Values_GetValueByAddr(char type, int addr, char **ptr, char *index);
int Values_SetValue(char type, char index, char value);