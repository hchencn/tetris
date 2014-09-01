#pragma once
class Draw
{
public:
	Draw(SDL_Renderer* pass_renderer, int x, int y, int w, int h, Uint32 color);
	~Draw(void);
	void Draw_Rect();

	void Setx(int x);
	void Sety(int y);
	int Getx(void);
	int Gety(void);

private: 
	Uint32 color;

	SDL_Surface* surf;
	SDL_Texture* text;
	SDL_Rect rect;
	SDL_Renderer* renderer;
};

