#pragma once
#include "StdAfx.h"
#include "SDL_Setup.h"
#include "Draw.h"
#include <time.h>
#include <fstream>
#include <vector>

const int WIDTH = 12;	//num of blocks in a row
const int HEIGHT = 24;	//num of blocks in a cloumn
const int BLOCK_SIZE = 30;	//block size(in pixel)
const int FPS = 30;	

class Game
{
public:
	Game(int screen_w, int screen_h);
	~Game(void);

	void Init_Game(int num_blocks, int blocks[8][4][4], int game_table[HEIGHT][WIDTH], int falling_block[4][4], int &block_x, int &block_y, int &next_block, bool &isfour, 
		int &normal_speed, int &current_speed, int &points, int &erase_lines);
	void Rotate(int falling_block[4][4], bool isfour);
	void Fill_Blocks(int blocks[8][4][4], int &num);
	void Set_Currentblock(int blocks[8][4][4], int falling_block[4][4], int idx);
	bool Collision(int falling_block[4][4], int pos_x, int pos_y, int game_table[HEIGHT][WIDTH]);
	void Display_Toplist(TTF_Font* font);
	bool Add_Toplist(TTF_Font* font,int points);
	void Write_Text(TTF_Font* font, int x, int y, const char* text, int r, int g, int b);
	void Write_Text(TTF_Font* font, int x, int y, int num, int r, int g, int b);
	void Render_IMG(SDL_Texture* image, int rect_x, int rect_y, int rect_w, int rect_h, int crop_x, int crop_y, int crop_w, int crop_h);
	int Check_Fulllines(int game_table[HEIGHT][WIDTH]);
	void Move(void);
	void Excute(void);
	void Show_Screen(void);
	void Free(void);
	void Start(void);

private:
	int screen_w;
	int screen_h;
	int delay;
	bool quit; 

	int num_blocks;	
	int blocks[8][4][4];	//all of the blocks
	int game_table[HEIGHT][WIDTH];	//store all the blocks already down
	int falling_block[4][4];	//current falling block
	int block_x;	//the init position of the falling block
	int block_y;
	bool isfour;	//4x4, or 3x3

	Uint32 lastmove;	//how often should the block fall down one
	Uint32 lastkey;	//how often should react the block, when the key is pressed (without release)
	Uint32 start;	//the start time, to limit FPS

	bool direction[2];	//direction[0]=left, direction[1]=right
	int points;
	int top_num;
	int normal_speed;	//blocks fall down every 0.5 seconds
	int current_speed;	
	int holding_speed;	//move blocks every 150ms when left/right key is pressed
	int quick_movingspeed;

	int next_block;	
	int erase_lines;	//how much lines we already cleared
	int move_step;

	bool mousepointing;	//mouse point to toplist
	int mouse_x;	//the location of the mouse cursor
	int	mouse_y;	

	SDL_Texture* image;

	Mix_Chunk* effect;
	Mix_Music* music;

	SDL_Setup* sdl_setup;

	std::vector<Draw*> menu;
};

