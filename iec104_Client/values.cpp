#include "stdafx.h"
#include <WinSock2.h>
#include <iostream>
#include "iec104.h"
#include "values.h"

using namespace std;

#define VARS_NUM 50

char singlePointArrAddr[VARS_NUM];
char singlePointArr[VARS_NUM];
char singlePointIndex = 0;

char doublePointArrAddr[VARS_NUM];
char doublePointArr[VARS_NUM];
char doublePointIndex = 0;

char bitStringArrAddr[VARS_NUM];
char bitStringArr[VARS_NUM][4];
char bitStringIndex = 0;

char singleCommandArrAddr[VARS_NUM];
char singleCommandArr[VARS_NUM];
char singleCommandIndex = 0;

char valuesAddr=1;
/*
#define C_IC_NA_1 100 команда опроса
#define C_SC_NA_1 45 однопозиционная команда
#define M_SP_NA_1 1 одноэлементная информация
#define M_DP_NA_1 3 двухэлементная информация
#define M_BO_NA_1 7 32 битовая строка
*/

void Values_Init()
{
	int i;
	char val = 0;

	valuesAddr=1;

	cout << endl<< "Values init...." << endl;

	for(i=0; i<2; i++)
	{
	  Values_Add(M_DP_NA_1, valuesAddr ++, &val);
	  val++;
	  if(val==4)
		  val=0;
	}
	cout << "Total values M_DP_NA_1: "<< i <<  endl << endl;

	for(i=0; i<2; i++)
	{
	  Values_Add(C_SC_NA_1, valuesAddr ++,  &val);
	  val++;
	  if(val==4)
		  val=0;

	}
	cout << "Total values C_SC_NA_1: "<< i <<  endl << endl;
}

int Values_Add(char type, char addr, char *ptr)
{
	int ret = 1;
	switch(type)
	{
	case M_SP_NA_1:
		//single point

		if(singlePointIndex < (VARS_NUM - 1))
		{
			singlePointArr[singlePointIndex] = *ptr;
			singlePointArrAddr[singlePointIndex ++] = addr;
			printf("    M_SP_NA_1 single point: addr: %d value: %d\n", addr, *ptr);
		}
		else
			ret = 0;
		break;
	
	case M_DP_NA_1:
		//double point

		if(doublePointIndex < (VARS_NUM - 1))
		{
			doublePointArr[doublePointIndex] = *ptr;
			doublePointArrAddr[doublePointIndex ++] = addr;
			printf("    M_DP_NA_1 double point: addr: %d value: %d\n", addr, *ptr);
		}
		else
			ret = 0;
		break;

	case M_BO_NA_1:
		//32 bit string

		if(bitStringIndex < (VARS_NUM - 1))
		{
			memcpy(ptr, bitStringArr[bitStringIndex], 4);
			bitStringArrAddr[bitStringIndex ++] = addr;
			printf("    M_BO_NA_1 32 bit string: addr: %d, %d%d%d%d\n", addr, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
		}
		else 
			ret = 0;
		break;
		
	case C_SC_NA_1:
		//single command

		if(singleCommandIndex < (VARS_NUM - 1))
		{
			singleCommandArr[singleCommandIndex] = *ptr;
			singleCommandArrAddr[singleCommandIndex ++]  = addr;
			printf("    C_SC_NA_1 single command: addr: %d %d\n", addr, *ptr);
		}
		else
			ret = 0;
		break;
	}

	return ret;
}

int Values_GetNum(char type)
{
	int ret;

	switch(type)
	{
	case M_SP_NA_1:
		//single point
		ret = singlePointIndex;
		break;
	
	case M_DP_NA_1:
		//double point
		ret = doublePointIndex;
		break;

	case M_BO_NA_1:
		//32 bit string
		ret = bitStringIndex;
		break;

	case C_SC_NA_1:
		//single command
		ret = singleCommandIndex;
		break;
	}

	return ret;
}

int Values_GetValue(char type, int index, char **ptr)
{
	int ret = 1;

	switch(type)
	{
	case M_SP_NA_1:
		//single point
		if(index <= singlePointIndex)
		{
			ret = 1;
			*ptr = (singlePointArr + index);
		}
		else
		{
			ret = 0;
			ptr = NULL;
		}
		break;

	case M_DP_NA_1:
		//double point
		if(index <= doublePointIndex)
		{
			ret = 1;
			*ptr = (doublePointArr + index);
		}
		else
		{
			ret = 0;
			ptr = NULL;
		}
		break;

	case M_BO_NA_1:
		//32 bit string		
		if(index <= bitStringIndex)
		{
			ret = 1;
			*ptr = (char *)(bitStringArr + index);
		}
		else
		{
			ret = 0;
			ptr = NULL;
		}
		break;

	case C_SC_NA_1:
		//single command
		if(index <= singleCommandIndex)
		{
			ret = 1;
			*ptr = (char *)(singleCommandArr + index);
		}
		else
		{
			ret = 0;
			ptr = NULL;
		}
		break;

	}

	return ret;
}


int Values_GetAddr(char type, int index, char **ptr)
{
	int ret = 1;

	switch(type)
	{
	case M_SP_NA_1:
		//single point
		if(index <= singlePointIndex)
		{
			ret = 1;
			*ptr = (singlePointArrAddr + index);
		}
		else
		{
			ret = 0;
			ptr = NULL;
		}
		break;

	case M_DP_NA_1:
		//double point
		if(index <= doublePointIndex)
		{
			ret = 1;
			*ptr = (doublePointArrAddr + index);
		}
		else
		{
			ret = 0;
			ptr = NULL;
		}
		break;

	case M_BO_NA_1:
		//32 bit string		
		if(index <= bitStringIndex)
		{
			ret = 1;
			*ptr = (char *)(bitStringArrAddr + index);
		}
		else
		{
			ret = 0;
			ptr = NULL;
		}
		break;

	case C_SC_NA_1:
		//single command
		if(index <= singleCommandIndex)
		{
			ret = 1;
			*ptr = (singleCommandArrAddr + index);
		}
		else
		{
			ret = 0;
			ptr = NULL;
		}
		break;

	}

	return ret;

}

int Values_GetValueByAddr(char type, int addr, char **ptr, char *index)
{
	int ret = 0;
	int i=0;
	char *adrPtr;
	char num;

	switch(type)
	{
	case M_SP_NA_1:
		//single point

		num = Values_GetNum(M_SP_NA_1); 

		for(i=0; i<num; i++)
		{
			if(Values_GetAddr(M_SP_NA_1, i, &adrPtr))
			{
				if(*adrPtr == addr)
				{
					*ptr = (singlePointArr + i);
					*index = i;
					return 1;
				}
			}
		}		
		break;

	case M_DP_NA_1:
		//double point

		num = Values_GetNum(M_DP_NA_1); 
		
		for(i=0; i<num; i++)
		{
			if(Values_GetAddr(M_DP_NA_1, i, &adrPtr))
			{
				if(*adrPtr == addr)
				{
					*ptr = (doublePointArr + i);
					*index = i;
					return 1;
				}
			}
		}
		break;

	case M_BO_NA_1:
		//32 bit string		

		num = Values_GetNum(M_BO_NA_1); 

		for(i=0; i<num; i++)
		{
			if(Values_GetAddr(M_BO_NA_1, i, &adrPtr))
			{
				if(*adrPtr == addr)
				{
					*ptr = (char *)(bitStringArrAddr + i);
					*index = i;
					return 1;
				}
			}
		}

		break;

	case C_SC_NA_1:
		//single command
		num = Values_GetNum(C_SC_NA_1); 

		for(i=0; i<num; i++)
		{
			if(Values_GetAddr(C_SC_NA_1, i, &adrPtr))
			{
				if(*adrPtr == addr)
				{
					*ptr = (singleCommandArrAddr + i);
					*index = i;
					return 1;
				}
			}
		}		
		break;

	}

	return 0;
}

int Values_SetValue(char type, char index, char value)
{
	int ret = 0;
	int i=0;
	
	switch(type)
	{
	case C_SC_NA_1:		

		if(index <= singleCommandIndex)
		{
			ret = 1;
			*(singlePointArrAddr + index) = value;
		}		
		break;
	}
	return ret;
}