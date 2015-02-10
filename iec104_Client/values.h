#pragma once

void Values_Init();
int Values_Add(char type, char *ptr);
int Values_GetNum(char type);
int Values_GetValue(char type, int index, char **ptr);