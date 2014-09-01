#pragma once
class SDL_Setup
{
public:
	SDL_Setup(int screen_w, int screen_h);
	~SDL_Setup(void);
	
	SDL_Renderer* Get_Renderer();
	SDL_Event* Get_Main();
	TTF_Font* Get_Font();
	SDL_Window* Get_Window();

	void Start();
	void End();

private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event* mainevent;
	TTF_Font* font;
};

