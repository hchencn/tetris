#include "StdAfx.h"
#include "SDL_Setup.h"

SDL_Setup::SDL_Setup(int pass_w, int pass_h)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	font = NULL;
	font = TTF_OpenFont("font/Cassia Regular.ttf", 30);
	if(font == NULL) 
		std::cout << "Font load errer" << std::endl;

	window = NULL;
	window = SDL_CreateWindow("Tetris Game", 100, 100, pass_w, pass_h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if(window == NULL) 
		std::cout << "Window load error" << std::endl;

	renderer = NULL;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == NULL) 
		std::cout << "Renderer load error" << std::endl;

	mainevent = new SDL_Event();
}


SDL_Setup::~SDL_Setup(void)
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	TTF_CloseFont(font);
	delete mainevent;	
}

SDL_Renderer* SDL_Setup::Get_Renderer()
{
	return renderer;
}

SDL_Event* SDL_Setup::Get_Main()
{
	return mainevent;	
}

TTF_Font* SDL_Setup::Get_Font()
{
	return font;
}
SDL_Window* SDL_Setup::Get_Window()
{
	return window;
}

void SDL_Setup::Start()
{
	SDL_PollEvent(mainevent);
	SDL_RenderClear(renderer);
}

void SDL_Setup::End()
{
	SDL_RenderPresent(renderer);
}
