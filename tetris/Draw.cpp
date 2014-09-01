#include "StdAfx.h"
#include "Draw.h"


Draw::Draw(SDL_Renderer* pass_renderer, int x, int y, int w, int h, Uint32 pass_color)
{
	renderer = pass_renderer;
	color = pass_color;

	surf = NULL;
	surf = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
	if (surf  == NULL) {
		std::cout << "Surface load errer" << std::endl;
	}

	SDL_FillRect(surf, NULL, color);

	text = NULL;
	text = SDL_CreateTextureFromSurface(renderer, surf);
	if (text == NULL) {
		std::cout << "Texture load errer" << std::endl;
	}

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
}


Draw::~Draw(void)
{
	SDL_FreeSurface(surf);
	SDL_DestroyTexture(text);
}

void Draw::Draw_Rect(void)
{
	SDL_RenderCopy(renderer, text, NULL, &rect);
}

void Draw::Setx(int x) 
{
	rect.x = x;
}

void Draw::Sety(int y) 
{
	rect.y = y;
}

int Draw::Getx()
{
	return rect.x;
}

int Draw::Gety()
{
	return rect.y;
}