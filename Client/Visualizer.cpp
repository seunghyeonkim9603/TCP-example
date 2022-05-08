#include <Windows.h>
#include <memory>
#include <stdio.h>
#include <vector>

#include "Console.h"
#include "Message.h"
#include "Player.h"
#include "ApplicationManager.h"

#include "Visualizer.h"

static char gBuffer[NUM_ROWS][NUM_COLUMNS];

static void ClearBuffer(void)
{
	memset(gBuffer, ' ', NUM_ROWS * NUM_COLUMNS);

	for (int row = 0; row < NUM_ROWS; ++row)
	{
		gBuffer[row][NUM_COLUMNS - 1] = '\0';
	}
}

static void FlipBuffer(void)
{
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		CSMoveCursor(0, i);
		printf(gBuffer[i]);
	}
}

void Visualize(const std::vector<std::unique_ptr<Player>>& playerList)
{
	ClearBuffer();

	for (auto& player : playerList)
	{
		gBuffer[player->GetY()][player->GetX()] = '*';
	}

	FlipBuffer();
}