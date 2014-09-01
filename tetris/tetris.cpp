#include "stdafx.h"
#include "Game.h"

int main(int argc, char* argv[])
{
	Game newgame(BLOCK_SIZE * WIDTH + 450, BLOCK_SIZE * HEIGHT);
	newgame.Start();
	return 0;
}

