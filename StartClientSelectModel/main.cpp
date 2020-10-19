#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include "Console.h"
#include "SocketLog.h"
#include <locale.h>
#include <WS2tcpip.h>
#include <vector>

#define PORT 3000
#define SERVER_ADDR L"127.0.0.1"
#define BUFFER_SIZE 16



struct Session
{
	int32_t ID;
	int32_t x;
	int32_t y;
};

std::vector <Session> g_VectorSession;


int32_t g_PrevX = 0;
int32_t g_PrevY = 0;
int32_t g_MyID;

char g_Buffer[BUFFER_SIZE];

SOCKET g_Socket;

void KeyProcess();
void Render();
void Network();
void PacketProcess();


int main()
{
	WSAData wsaData;
	setlocale(LC_ALL, "Korean");

	SOCKADDR_IN serverAddr;


	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ERROR_LOG(L"WsaStartUp() Error");
	}

	g_Socket = socket(AF_INET, SOCK_STREAM, 0);
	u_long on = 1;

	int retVal = ioctlsocket(g_Socket, FIONBIO, &on);
	if (retVal == SOCKET_ERROR)
	{
		ERROR_LOG(L"ioctlsocket() error");
	}

	if (g_Socket == INVALID_SOCKET)
	{
		ERROR_LOG(L"socket() Error");
	}
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	InetPton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr.S_un.S_addr);

	if (0 != connect(g_Socket, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		ERROR_LOG(L"connect() error");
	}

	while (true)
	{
		KeyProcess();
		Render();
		Network();
		Sleep(10);
	}

	closesocket(g_Socket);
	WSACleanup();

}

void KeyProcess()
{
	if (g_VectorSession.size() == 0)
	{
		return;
	}

	int searchIndex = -1;

	for (size_t i = 0; i < g_VectorSession.size(); i++)
	{
		if (g_VectorSession[i].ID == g_MyID)
		{
			searchIndex = i;
			g_PrevX = g_VectorSession[i].x;
			g_PrevY = g_VectorSession[i].y;
			break;
		}
	}
	if (searchIndex < 0)
	{
		return;
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		g_VectorSession[searchIndex].x--;
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		g_VectorSession[searchIndex].x+=2;
	}
	if (GetAsyncKeyState(VK_UP))
	{
		g_VectorSession[searchIndex].y--;
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		g_VectorSession[searchIndex].y++;
	}

	if (g_VectorSession[searchIndex].x >= CConsole::SCREEN_WIDTH)
	{
		g_VectorSession[searchIndex].x = CConsole::SCREEN_WIDTH -1;
	}

	if (g_VectorSession[searchIndex].x <0)
	{
		g_VectorSession[searchIndex].x = 0;
	}
	if (g_VectorSession[searchIndex].y > CConsole::SCREEN_HEIGHT-2)
	{
		g_VectorSession[searchIndex].y = CConsole::SCREEN_HEIGHT-2;
	}
	if (g_VectorSession[searchIndex].y<0)
	{
		g_VectorSession[searchIndex].y =0;
	}
	if (g_PrevX != g_VectorSession[searchIndex].x || g_PrevY != g_VectorSession[searchIndex].y)
	{
		//send
		g_PrevX = g_VectorSession[searchIndex].x;
		g_PrevY = g_VectorSession[searchIndex].y;

		int packet[4];
		packet[0] = 3;
		packet[1] = g_VectorSession[searchIndex].ID;
		packet[2] = g_VectorSession[searchIndex].x;
		packet[3] = g_VectorSession[searchIndex].y;

		int ret = send(g_Socket, (char*)packet, 16, 0);
		if (ret < 0)
		{
			ERROR_LOG(L"Send Error");
		}
	}
}

void Render()
{
	CConsole::GetInstance()->Buffer_Clear();
	system("cls");
	for (size_t i = 0; i < g_VectorSession.size(); i++)
	{
		
		CConsole::GetInstance()->Sprite_Draw(g_VectorSession[i].x, g_VectorSession[i].y, '*');
		
	}
	CConsole::GetInstance()->Buffer_Flip();

}

void Network()
{
	fd_set readSet;
	fd_set exceptSet;

	FD_ZERO(&readSet);
	FD_ZERO(&exceptSet);
	FD_SET(g_Socket, &readSet);
	FD_SET(g_Socket, &exceptSet);


	timeval time;
	time.tv_sec = 0;
	time.tv_sec = 0;

	while (true)
	{
		int rtn = select(0, &readSet, NULL, &exceptSet, &time);
		if (rtn <=0)
		{
			return;
		}
		if (FD_ISSET(g_Socket, &exceptSet))
		{
			ERROR_LOG(L"Connect Fail");
		}
		if (FD_ISSET(g_Socket ,&readSet))
		{
			int recvCnt = recv(g_Socket, g_Buffer, BUFFER_SIZE, 0);

			if (recvCnt <= 0)
			{
				ERROR_LOG(L"recv Error");
			}
			else
			{
				PacketProcess();

			}
		}

	}
}

void PacketProcess()
{
	int* header = (int*)g_Buffer;

	switch (*header)
	{
		case 0:
		{
			g_MyID = *((int*)(g_Buffer + 4));
			break;
		}
		case 1:
		{
			int* id = (int*)(g_Buffer + 4);
			int* x = (int*)(g_Buffer + 8);
			int* y = (int*)(g_Buffer + 12);

			Session tempSession = { *id,*x,*y };
			g_VectorSession.push_back(tempSession);
			break;

		}
		case 2:
		{
			int* id = (int*)(g_Buffer + 4);
			auto iter = g_VectorSession.begin();
			for (; iter != g_VectorSession.end(); ++iter)
			{
				if ((*iter).ID == *id)
				{
					g_VectorSession.erase(iter);
					break;
				}
			}
			break;
		}
		case 3:
		{
			int* id = (int*)(g_Buffer + 4);
			int* x = (int*)(g_Buffer + 8);
			int* y = (int*)(g_Buffer + 12);

			auto iter = g_VectorSession.begin();

			for (; iter != g_VectorSession.end(); ++iter)
			{
				if ((*iter).ID == *id)
				{
					(*iter).x = *x;
					(*iter).y = *y;
					break;
				}
			}
			break;

		}
	}
}
