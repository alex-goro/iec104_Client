#include "stdafx.h"
#include <WinSock2.h>
#include <iostream>

using namespace std;

int parseIec104(char *recvBuf, int size);


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

		/*	do
			{
				parseIec104(recvbuf, bytesRecv, &iecFrame, &recIndex);
	    		
				if(iecFrame.format !=S_FORMAT)
				{
					iec104Process(&iecFrame, &iecFrameSend);
					sendbufLen=0;					
					if(opros==1)
					{
					     iec104MakeSinglePoint(&iecFrameSend); 					     
						 iec104Encode(&iecFrameSend, sendbuf, &sendbufLen);
						 iec104MakeActTerm(&iecFrameSend);
						 iec104Encode(&iecFrameSend, sendbuf+sendbufLen, &sendbufLen);					
						 opros=0;
					}
					else
					{
					  iec104Encode(&iecFrameSend, sendbuf, &sendbufLen);					
					}
					*/
					bytesSent = send( m_socket, sendbuf, sendbufLen, 0 );
					

					if (bytesSent == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(m_socket);
						WSACleanup();
						return 1;
					}
					printf("Bytes sent: %d\n", bytesSent);
			/*	}
			}while(recIndex<bytesRecv);
			*/
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


typedef enum PARSE_STATE
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
				parseState = PARSE_STATE_INITIAL;
				break;
			case PARSE_STATE_FORMAT_S:
				cout << "     format: S" << endl;
				parseState = PARSE_STATE_INITIAL;
				break;
			case PARSE_STATE_FORMAT_U:
				cout << "     format: U" << endl;
				parseState = PARSE_STATE_INITIAL;
				break;
		}
	}
	while(busy);
	
	cout << "Parsed "<<  (ptr - recvBuf - 1) << " bytes" << endl;

	return apduNum;				
}