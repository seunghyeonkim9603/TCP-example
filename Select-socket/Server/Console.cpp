#include <stdio.h>
#include <windows.h>

#include "Console.h"

HANDLE  hConsole;

void CSInitial(void)
{
	CONSOLE_CURSOR_INFO consoleCursor;

	consoleCursor.bVisible = FALSE;
	consoleCursor.dwSize = 1;

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &consoleCursor);
}

void CSMoveCursor(int x, int y)
{
	COORD pos = { x, y };

	SetConsoleCursorPosition(hConsole, pos);
}

void CSClearScreen(void)
{
	COORD coord;
	DWORD dw;

	coord.X = 0;
	coord.Y = 0;
	FillConsoleOutputCharacter(hConsole, ' ', 100 * 100, coord, &dw);
}