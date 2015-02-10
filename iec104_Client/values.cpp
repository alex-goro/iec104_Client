#include "stdafx.h"
#include <WinSock2.h>
#include <iostream>
#include "iec104.h"
#include "values.h"

using namespace std;

#define VARS_NUM 50

char singlePointArr[VARS_NUM];
char singlePointIndex = 0;

char bitStringArr[VARS_NUM][4];
char bitStringIndex = 0;


void Values_Init()
{
	int i;
	char val = 0;

	cout << endl<< "Values init...." << endl;

	for(i=0; i<40; i++)
	{
	  Values_Add(M_SP_NA_1, &val);
	  val++;
	  if(val==2)
		  val=0;

	}
	cout << "Total values: "<< i <<  endl << endl;
}

int Values_Add(char type, char *ptr)
{
	int ret = 1;
	switch(type)
	{
	case M_SP_NA_1:
		//single point

		if(singlePointIndex < (VARS_NUM - 1))
		{
			singlePointArr[singlePointIndex ++] = *ptr;
			printf("    M_SP_NA_1 single point: %d\n", *ptr);
		}
		else
			ret = 0;
		break;

	case M_BO_NA_1:
		//32 bit string

		if(bitStringIndex < (VARS_NUM - 1))
		{
			memcpy(ptr, bitStringArr[bitStringIndex ++], 4);
			printf("    M_BO_NA_1 32 bit string: %d%d%d%d\n",*ptr, *(ptr+1), *(ptr+2), *(ptr+3));
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

	case M_BO_NA_1:
		//32 bit string
		ret = bitStringIndex;
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

	}

	return ret;
}