#include <Windows.h>
#include <stdio.h>
#include "Console.h"


void CConsole::cs_Initial(void)
{
	CONSOLE_CURSOR_INFO stConsoleCursor;
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1; // Ŀ�� ũ�� 1

	//-----------------------------------------------------------
	//�ܼ�ȭ�� (���Ĵٵ� �ƿ�ǲ) �ڵ��� ���Ѵ�
	//-----------------------------------------------------------
	m_Console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(m_Console, &stConsoleCursor);

}

void CConsole::cs_MoveCursor(int iPosX, int iPosY)
{
	COORD stCoord;
	stCoord.X = iPosX;
	stCoord.Y = iPosY;
	//-----------------------------------------------------------
	//�ܼ�ȭ�� (���Ĵٵ� �ƿ�ǲ) �ڵ��� ���Ѵ�
	//-----------------------------------------------------------
	SetConsoleCursorPosition(m_Console, stCoord);
}

void CConsole::cs_ClearScreen(void)
{
	unsigned long dw;
	FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', 100 * 100, { 0,0 }, &dw);
}

void CConsole::Buffer_Flip(void)
{
	for (int n = 0; n < SCREEN_HEIGHT; ++n)
	{
		cs_MoveCursor(0, n);
		printf("%s", m_szScreenBuffer[n]);
	}
}
void CConsole::Buffer_Clear(void)
{
	memset(m_szScreenBuffer, ' ', SCREEN_WIDTH * SCREEN_HEIGHT);

	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		m_szScreenBuffer[y][SCREEN_WIDTH - 1] = '\0';
	}

}
void CConsole::Sprite_Draw(int iX, int iY, char chSprite)
{


	//------------------------------------------
	// �Է¹��� ��ǥ�� ��Ŀ� �־, ���ڿ��� ���ۿ�����ִ´�
	//-------------------------------------------

	m_szScreenBuffer[iY][iX] = chSprite;

}

void CConsole::String_Draw(int iY, char* buffer)
{
	if (iY<0 || iY> SCREEN_HEIGHT - 1)
	{
		//printf("Y�� ������ �ʰ��Ͽ����ϴ�");
		return;
	}
	//------------------------------------------
	// �Է¹��� ��ǥ�� ��Ŀ� �־, ���ڿ��� ���ۿ�����ִ´�
	//-------------------------------------------
	memcpy(m_szScreenBuffer+iY, buffer, SCREEN_WIDTH);
}

CConsole::CConsole()
	:
	m_Console(0),
	m_szScreenBuffer{ 0, }
{
	cs_Initial();

}

CConsole::~CConsole()
{
}

CConsole*  CConsole::GetInstance()
{
	static CConsole console;
	return &console;
}
