#include "stdafx.h"
#include <WinSock2.h>
#include <iostream>

using namespace std;

#define U_FORMAT_STARTDT_ACT (0x01 << 2)
#define U_FORMAT_STARTDT_CON (0x01 << 3)
#define U_FORMAT_STOPDT_ACT (0x01 << 4)
#define U_FORMAT_STOPDT_CON (0x01 << 5)
#define U_FORMAT_TESTFR_ACT (0x01 << 6)
#define U_FORMAT_TESTFR_CON (0x01 << 7)

typedef enum APDU_Type
{
	U_Type = 0,
	S_Type = 1,
	I_Type = 2
}
APDU_Type_TypeDef;

typedef struct APDU_Frame
{
	APDU_Type_TypeDef type;
	char byte1;
	char byte2;
	char byte3;
	char byte4;

	int txNum;
	int rxNum;

	char typeId;
	char sq;
	char numIx;
	char causeTx;
	char negative;
	char test;
	char OA;
	int addr;
	char ioa[3];
	char qoi;
}
APDU_Frame_TypeDef;

#define APDU_ARR_MAX_SIZE 20
APDU_Frame_TypeDef apduArr[APDU_ARR_MAX_SIZE];

int apduArrIndexRec = 0;
int apduArrIndexTrm = 0;
int apduNumRec = 0;
int apduNumTrm = 0;
int apduArrOf = 0;
int apduArrNewNum = 0;


int RxNum = 0;
int txNum = 0;




int parseIec104(char *recvBuf, int size);
void newUFormat(char byte);
void processApduArr(char *sendPtr, int *sendLen);
void addApduArr(APDU_Type_TypeDef type, char byte1, char byte2, char byte3, char byte4);

int main(int argc, char *argv[])
{
//	cout << "Hello Worlld" << endl;
//	getchar();    

	WORD wVersionRequested;
	WSADATA wsaData;
	int wsaerr;

	wVersionRequested = MAKEWORD(2,2);
	wsaerr = WSAStartup(wVersionRequested, &wsaData);
	if (wsaerr != 0)
	{
		printf("The Winsock DLL not found\n");
		return 0;
	}
	else
	{
		printf("The Winsock DLL found\n");
		printf("The status: %s\n", wsaData.szSystemStatus);
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("The dll do not supports the Winsock version %u.%u", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		WSACleanup();
		return 0;
	}
	else
	{
		printf("The dll supports the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		printf("The highest version this dll can support: %u.%u\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
	}	

    SOCKET m_socket;

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(m_socket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}
	else
	{
		printf("socket() is OK\n");
	}

	char buf[128];
	hostent *h;
	
	
    sockaddr_in service;

	if (gethostname(buf, 128) == 0)
        {
            h = gethostbyname(buf);
            if (h != NULL)
			{
				service.sin_addr.s_addr = inet_addr(inet_ntoa(*(reinterpret_cast<in_addr *>(*(h->h_addr_list)))));
				cout << "IP address: ";
                cout << inet_ntoa (*(reinterpret_cast<in_addr *>(*(h->h_addr_list)))) << endl;
			}
            else 
                cout << "You have not any IP." << endl;
        }
		

	
	service.sin_family = AF_INET;
//	service.sin_addr.s_addr=inet_addr("127.0.0.1");
	service.sin_port =htons(2404);
	

	if (bind(m_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		printf("bind() failed: %ld\n", WSAGetLastError());
		closesocket(m_socket);
		return 0;
	}
	else
	{
		printf("bind() is OK\n");
	}

	if (listen(m_socket, 1) == SOCKET_ERROR)
	{
		printf("listen(): Error listening on socket %ld\n", WSAGetLastError());
	}
	else
	{
		printf("listen() is OK, waiting for connections...\n");
	}

	SOCKET AcceptSocket;

	printf("Server: waiting for a client to connect...\n");

	while(1)
	{
		AcceptSocket = SOCKET_ERROR;
		while(AcceptSocket == SOCKET_ERROR)
		{
			AcceptSocket = accept(m_socket, NULL, NULL);
		}

		printf("Server: client connected!\n");
		m_socket = AcceptSocket;
		break;
	}


//	IEC_Frame_TypeDef iecFrame, iecFrameSend;

	int bytesSent;	
	int bytesRecv = SOCKET_ERROR;

	int sendbufLen = 10;
	int recIndex =0;

	char sendbuf[1460]="abcdefghig";
	char recvbuf[1460]="";

	int asduParsed;

	do
	{
		bytesRecv = recv(m_socket, recvbuf, 1460, 0);

		if(bytesRecv>0)
		{
		    printf("\nServer: bytes received: %ld\n", bytesRecv);
			recIndex = 0;

			asduParsed = parseIec104(recvbuf, bytesRecv);
			cout << "Found "<<  asduParsed << " ASDU" << endl << endl;
			if(asduParsed)
			{
				if(apduArrNewNum > 0)
				{
				  processApduArr(sendbuf, &sendbufLen);
				  bytesSent = send( m_socket, sendbuf, sendbufLen, 0 );								

					if (bytesSent == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(m_socket);
						WSACleanup();
						return 1;
					}
				}
				printf("Bytes sent: %d\n", bytesSent);	
			}
		}
		else if(bytesRecv==0)
			printf("\nConnection closed\n");
		else
		{
			printf("Server: recv() error %ld\n", WSAGetLastError());
			closesocket(m_socket);
			WSACleanup();
			return 1;
		}
	} while(bytesRecv > 0);
		
  // shutdown the connection since we're done
    bytesRecv = shutdown(m_socket, SD_SEND);
    if (bytesRecv == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(m_socket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(m_socket);
    WSACleanup();

	cout << endl << "Press any key to exit";
	getchar();
}

	
typedef enum
{
	PARSE_STATE_INITIAL = 0,
	PARSE_STATE_GET_TYPE = 1,
	PARSE_STATE_FORMAT_I = 2,
	PARSE_STATE_FORMAT_S = 3,
	PARSE_STATE_FORMAT_U = 4

}
ParseState_TypeDef;

#define IEC_FORMAT_U 0x03
#define IEC_FORMAT_S 0x01
#define IEC_FORMAT_I 0x00



int parseIec104(char *recvBuf, int size)
{
	char *ptr = recvBuf;
	int busy = 1;
	int len = 0;
	int ret = 1;
	int apduNum = 0;
	int apduLen=0;
	int recNum, trmNum;
	


	ParseState_TypeDef parseState = PARSE_STATE_INITIAL;

	cout << endl << "Parse iec104 started ..." << endl;

	do
	{
		switch(parseState)
		{
			case PARSE_STATE_INITIAL:
				if (*ptr == 0x68)
				{
					ptr++;
					apduLen = *ptr;				
					ptr++;
					apduNum ++;
					parseState = PARSE_STATE_GET_TYPE;	

					cout << endl << "APDU START found..." << endl;
					cout << "     len: " << apduLen << endl; 
				}
				else
				{
				  ptr++;				  
				  if ((ptr - recvBuf) > size)
				  {
  					busy = 0;				  
				  }			
				}
				break;				

			case PARSE_STATE_GET_TYPE:
				if ((*ptr & 0x01) == IEC_FORMAT_I)
				{
					//format I
					parseState = PARSE_STATE_FORMAT_I;
				}
				else
					if((*ptr & 0x03) == IEC_FORMAT_S)
					{
						//format S					
						parseState = PARSE_STATE_FORMAT_S;

					}
					else
						if((*ptr & 0x3) == IEC_FORMAT_U)
						{
							//format U
							parseState = PARSE_STATE_FORMAT_U;

						}
						else
						{
							cout << "     format unknown! " << endl;
							parseState = PARSE_STATE_INITIAL;
						}
				break;

			case PARSE_STATE_FORMAT_I:
				cout << "     format: I" << endl;
				
				newIFormat(*ptr, apduLen);

				parseState = PARSE_STATE_INITIAL;				
				break;
			case PARSE_STATE_FORMAT_S:
				cout << "     format: S" << endl;				
				
				parseState = PARSE_STATE_INITIAL;
				break;
			case PARSE_STATE_FORMAT_U:
				cout << "     format: U" << endl;
				
				newUFormat(*ptr);

				parseState = PARSE_STATE_INITIAL;
				break;
		}
	}
	while(busy);
	
	cout << "Parsed "<<  (ptr - recvBuf - 1) << " bytes" << endl;

	return apduNum;				
}




void newUFormat(char byte)
{
	APDU_Frame_TypeDef apdu;
	char ret = 0;

	apdu.type=U_Type;
	apdu.byte1 = byte;

	/*if(byte & U_FORMAT_STARTDT_ACT)
	{
		ret |=U_FORMAT_STARTDT_CON;		
	}
	else
		if(byte & U_FORMAT_STOPDT_ACT)
		{
			ret |=U_FORMAT_STOPDT_CON;
		}
		else
			if(byte & U_FORMAT_TESTFR_ACT)
			{
				ret |=U_FORMAT_TESTFR_CON;
			}

	apdu.byte1 = ret;	
	apdu.byte2 = apdu.byte3 =apdu.byte4 = 0;
	*/
	//addApduArr(U_Type, ret, 0, 0, 0);
			addApduArr(&apdu);

}

//void addApduArr(APDU_Type_TypeDef type, char byte1, char byte2, char byte3, char byte4)
void addApduArr(APDU_Frame_TypeDef *apdu)
{

	APDU_Frame_TypeDef *ptr;

	cout << endl << "AddAPDUArr started ..." << endl;

	ptr = (apduArr + apduArrIndexRec);

	*ptr = *apdu;	

	apduArrNewNum ++;

	apduArrIndexRec ++;
	
	if (apduArrIndexRec >= APDU_ARR_MAX_SIZE)
	{
		apduArrIndexRec = 0;
		apduArrOf = 1;
	}
}

#define C_IC_NA_1 100

void processApduArr(char *sendPtr, int *sendLen)
{
	char *ptr = sendPtr;
	*sendLen = 0;
	char ret = 0;

	APDU_Frame_TypeDef *apdu;

	cout << "processAPDUArr started ..." << endl;

	while ((apduArrIndexTrm < apduArrIndexRec) || apduArrOf)
	{
		*ptr = 0x68;
		ptr ++;

		apdu = &apduArr[apduArrIndexTrm];

		switch(apdu->type)
		{
		case U_Type:
			if(apdu->byte1 & U_FORMAT_STARTDT_ACT)
			{
				ret |=U_FORMAT_STARTDT_CON;		
			}
			else
				if(apdu->byte1 & U_FORMAT_STOPDT_ACT)
				{
					ret |=U_FORMAT_STOPDT_CON;
				}
				else
					if(apdu->byte1 & U_FORMAT_TESTFR_ACT)
					{
						ret |=U_FORMAT_TESTFR_CON;
					}

			//������ APDU
			*ptr = 4;
			ptr++;
			*ptr ++  = ret | IEC_FORMAT_U;
			*ptr ++  = 0;
			*ptr ++  = 0;
			*ptr ++  = 0;
			break;

		case I_Type:

			if(apdu->typeId == C_IC_NA_1)
			{
				//������ APDU
				*ptr ++ = 14;
				*ptr ++ = txNum << 1;
				*ptr ++ = txNum >> 7;

				*ptr ++ = rxNum << 1;
				*ptr ++ = rxNum >> 7;

				*ptr ++ = C_IC_NA_1;
				*ptr ++ = 1;
				*ptr =
			}


			break;
		}
		
		apduArrIndexTrm ++;
		if (apduArrIndexTrm >= APDU_ARR_MAX_SIZE)
		{
			apduArrIndexTrm = 0;
			apduArrOf = 0;
		}

		if(apduArrNewNum > 0)
			apduArrNewNum --;
	}

	*sendLen = ptr - sendPtr;
	cout << endl << "processAPDUArr finished:" << *sendLen << " bytes ready to send" << endl;
}

void newIFormat(char *ptrStart, char len)
{
	char *ptr;
	char ret = 0;
	char i;

	int tx = 0;
	int rx = 0;

	APDU_Frame_TypeDef apdu;

	ptr = ptrStart;

	apdu.type=I_Type;

	for(i=0; i<len; i++)
	{
		switch(i)
		{
	  	  case 0:
			  tx = *ptr >> 1;
			  ptr ++;
			  break;
		  case 1:
			  tx |= (*ptr << 7);
			  ptr ++;

			  apdu.txNum = tx;
			  break;
		  case 2:
			  rx = *ptr >> 1;
			  ptr ++;
			  break;
		  case 3:
			  rx |= (*ptr << 7);
			  ptr ++;
			  apdu.rxNum = rx;
			  break;

		  case 4:
			  apdu.typeId = *ptr;
			  ptr ++;
			  break;

		  case 5:
			  apdu.sq = (*ptr & (0x01 << 7)) && 1; //&& 1 ���������� � ����� �������� 1 ��� 0
			  apdu.numIx = *ptr & 0x7F;
			  ptr ++;
			  break;

		  case 6:
			  apdu.causeTx = *ptr & 0x3F;
			  apdu.negative = (*ptr & 0x40) && 1;
			  apdu.test =  (*ptr & 0x80) && 1;
			  ptr ++;
			  break;

		  case 7:
			  apdu.OA = *ptr;
			  ptr++;
			  break;

		  case 8:
			  apdu.addr = *ptr;
			  ptr++;
			  break;

		  case 9:
			  apdu.addr| = (*ptr << 8);
			  ptr++;
			  break;

		  case 10:
			  apdu.ioa[0] = *ptr;
			  ptr++;
			  break;
		  case 11:
			  apdu.ioa[1] = *ptr;
			  ptr++;
			  break;
		  case 12:
			  apdu.ioa[2] = *ptr;
			  ptr++;
			  break;
		  case 13:
			  apdu.qoi = *ptr;
			  break;			
		}
	}
		
	addApduArr(&apdu);
}