
#include "Game.h"

Game::Game(int pass_w, int pass_h)
{
	screen_w = pass_w;
	screen_h = pass_h;

	sdl_setup = new SDL_Setup(screen_w, screen_h);

	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
	music = Mix_LoadMUS("music/bg.wav");
	effect = Mix_LoadWAV("music/hit.wav");
	Mix_PlayMusic(music, -1);

	delay = 100;
	srand(time(0));

	top_num = 10;
	num_blocks = 0;
	holding_speed = 400;	
	quick_movingspeed = 30;
	move_step = 0;

	quit = false;
	mousepointing = false;	
	direction[0] = false;
	direction[1] = false;

	lastmove = SDL_GetTicks();	
	lastkey = SDL_GetTicks();	

	image = NULL;
	image = IMG_LoadTexture(sdl_setup->Get_Renderer(), "pic/blocks.bmp");
	if (image == NULL) {
		std::cout << "image load errer" << std::endl;
	}

	Fill_Blocks(blocks, num_blocks);
	Init_Game(num_blocks, blocks, game_table, falling_block, block_x, block_y, next_block, isfour, normal_speed, current_speed, points, erase_lines);		
}


Game::~Game(void)
{
	SDL_DestroyTexture(image);

	Mix_FreeChunk(effect);
	Mix_FreeMusic(music);
	Mix_CloseAudio();

	delete sdl_setup;
}

void Game::Rotate(int falling_block[4][4], bool isfour)	//we rotate a 4x4 (or 3x3 if h) matrix to the left
{
	char tmp[4][4];
	if(isfour)	//	4x4
	{
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
				tmp[3-j][i] = falling_block[i][j]; //just swap the rows and coloumns to a tmp matrix (draw it, if you don't understand)
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
				falling_block[i][j] = tmp[i][j];	//and copy back to the original
	}
	else	//	3x3
	{
		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 3; j++)
				tmp[2-j][i] = falling_block[i][j];	//do the same, just different datas
		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 3; j++)
				falling_block[i][j] = tmp[i][j];
		for(int i = 0; i < 4; i++)
		{
			falling_block[3][i] = 0;	//in this case, we need to make 0's to the bottom row or most right coloumn
			falling_block[i][3] = 0;
		}
	}
}

void Game::Set_Currentblock(int blocks[8][4][4], int falling_block[4][4], int idx)
{
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
			falling_block[i][j] = blocks[idx][i][j];
}

void Game::Fill_Blocks(int blocks[8][4][4], int &num)	//fill all of the tetris blocks from file
{
	std::ifstream in("data/tetris.dat");	//read from this file
	int tmp[16];		
	while(!in.eof())	
	{
		for(int i = 0; i < 16; i++)
		{
			in >> tmp[i];	//read the 16 integers
		}
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
				blocks[num][i][j] = tmp[i*4+j];
		num++;	
	}
	num--;	//end of the file
}

bool Game::Collision(int falling_block[4][4], int pos_x, int pos_y, int game_table[HEIGHT][WIDTH])
{
	//if the block is out of the screen in the right
	if(pos_x > 8)
	{
		for(int j = 0; j < 4; j++)
		{
			for(int i = 3; i >= WIDTH - pos_x; i--)
			{
				//we go through every 4 line, and check the element in the end, which is out of the screen
				//if it's 0, than it's nothing, but as soon, as we find something, which is not zero, then one part of the block is
				//out of the screen, so we return with 1
				if(falling_block[j][i] != 0)
					return true;
			}
		}
	}
	
	if(pos_x < 0)	//if our block is out of the screen in the left, we do almost the same thing, just in the oposite direction
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < -pos_x; j++)
			{
				if(falling_block[i][j] != 0)
					return true;
			}
		}
	}
	//now check, if it is down, the principle is the same, just in this case, we start from the bottom of the block
	if(pos_y >= 20)
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 3; j > HEIGHT - pos_y -1; j--)
			{
				if(falling_block[j][i] != 0)
					return true;
			}	
		}
	}
	//now check, if we collided with another block in the table, the principle is similar, we go from tho bottom of the block, check, if the block
	//has some part there, if it has, check, if the table has some part there, if so, we return with 1, because there was a collision
	//probably, you can just go through this matrix in a normal way, but it may be a little bit more efficient (1,2 ns :)
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			if(pos_y + i < HEIGHT && pos_x + j < WIDTH)
				if(game_table[pos_y+i][pos_x+j] != 0 && falling_block[i][j] != 0)
					return true;
	return false;	//if nothing has returned, return with false
}

int Game::Check_Fulllines(int game_table[HEIGHT][WIDTH])	
{
	//self-explanotary
	bool isfull;
	for(int i = HEIGHT - 1; i >= 0; i--)
	{
		isfull = true;
		for(int j = 0; j < WIDTH; j++)
		{
			if(game_table[i][j] == 0)
			{
				isfull = false;
				break;
			}
		}
		if(isfull)
			return i;
	}
	return -1;
}

void Game::Init_Game(int num_blocks, int blocks[8][4][4], int game_table[HEIGHT][WIDTH], int falling_block[4][4], int &block_x, int &block_y, int &next_block, bool &isfour, 
	int &normal_speed, int &current_speed, int &points, int &erase_lines)
{
	block_x = 4;	//the starting coordinate of our falling block is 4,0
	block_y = 0;

	int cur_idx = rand() % (num_blocks + 1);	//we pick  random block
	isfour = (cur_idx <= 2);	//the 1st and 2nd block is 4x4 all others are 3x3
	Set_Currentblock(blocks, falling_block, cur_idx);		//set it to current block

	for(int i = 0;i < HEIGHT; i++)
		for(int j = 0; j < WIDTH ; j++)
			game_table[i][j] = 0;	//clear
	normal_speed = 500;	//half a second between every time, the block moves down one
	points = 0;
	erase_lines = 0;
	next_block = rand() % (num_blocks + 1);	//we pick a random next block
	current_speed = normal_speed;	//our current speed is the same as our normal speed (if we press the down arrow, the speed will increase, while														//the normalspeed remains the same)
}

void Game::Move(void)
{
	start = SDL_GetTicks();
	switch(sdl_setup->Get_Main()->type)
	{
		case SDL_KEYDOWN:
			switch(sdl_setup->Get_Main()->key.keysym.sym)
			{
				case SDLK_ESCAPE:	//if escape is pressed, quit
					quit = true;
					break;
				case SDLK_w:	//if up arrow is pressed
					Rotate(falling_block, isfour);	//rotate the block
					if(Collision(falling_block, block_x, block_y, game_table))	//and check if there is a collision
						for(int i = 0; i < 3; i++)
							Rotate(falling_block, isfour);	//if there was a collision, rotate back (rotate 4 times, is like if you haven't done anything)
					break;
				case SDLK_a:
					direction[0] = true;
					break;
				case SDLK_d:
					direction[1] = true;
					break;
				case SDLK_s:
					current_speed = 20;	//if down key is pressed, speed up a little bit
					break;
			}
			break;

		case SDL_KEYUP:
			switch(sdl_setup->Get_Main()->key.keysym.sym)
			{
				case SDLK_a:
					direction[0] = false;
					move_step = 0;
					break;
				case SDLK_d:
					direction[1] = false;
					move_step = 0;
					break;
				case SDLK_s:
					current_speed = normal_speed;	//if you released the down arrow, set back the speed
					break;
			}
			break;

		case SDL_QUIT:
			quit = true;
			break;

		case SDL_MOUSEMOTION:
			//if we moved the mouse
			mouse_x = sdl_setup->Get_Main()->motion.x;	//get the coordinates
			mouse_y = sdl_setup->Get_Main()->motion.y;
			//if we are pointing to the square, which contain the toplist text
			if(mouse_x > BLOCK_SIZE*WIDTH + 50 && mouse_x < BLOCK_SIZE*WIDTH + 200 && mouse_y > 250 + BLOCK_SIZE*4 && mouse_y < 300 + BLOCK_SIZE*4)
				mousepointing = true;	//make this boolean true
			else
				mousepointing = false;	//else false
			break;

		case SDL_MOUSEBUTTONDOWN: 
			//if we hit the mousebutton 
			mouse_x = sdl_setup->Get_Main()->button.x;
			mouse_y = sdl_setup->Get_Main()->button.y;
			//on the toplist text
			if(mouse_x > BLOCK_SIZE*WIDTH + 50 && mouse_x < BLOCK_SIZE*WIDTH + 200 && mouse_y > 250 + BLOCK_SIZE*4 && mouse_y < 300 + BLOCK_SIZE*4)
				Display_Toplist(sdl_setup->Get_Font());	//display it
			break;
	}
}

void Game::Excute(void)
{
	if(Collision(falling_block, block_x, block_y, game_table) && block_y == 0)
	{
		if(Add_Toplist(sdl_setup->Get_Font(), points))	//restart the game
			Init_Game(num_blocks, blocks, game_table, falling_block, block_x, block_y, next_block, isfour, normal_speed, current_speed, points, erase_lines);
		else
			quit = true;	
	}

	//if we exceeded the time, move it down, and set back the time
	if(SDL_GetTicks() - lastmove > current_speed)
	{
		block_y++;
		lastmove = SDL_GetTicks();
	}
	//left move
	if((direction[0] && move_step >= 2 && SDL_GetTicks() - lastkey > quick_movingspeed) || (direction[0] && move_step == 0) 
		|| (direction[0] && move_step == 1 && SDL_GetTicks() - lastkey > holding_speed))
	{
		block_x--;	//move
		move_step++;
		lastkey = SDL_GetTicks();	
		if(Collision(falling_block, block_x, block_y, game_table) == 1)	
			block_x++;			//move back
	}
	//right move
	else if((direction[1] && move_step >= 2 && SDL_GetTicks() - lastkey > quick_movingspeed) || (direction[1] && move_step == 0) 
		|| (direction[1] && move_step == 1 && SDL_GetTicks() - lastkey > holding_speed))	
	{
		block_x++;
		move_step++;
		lastkey=SDL_GetTicks();
		if(Collision(falling_block, block_x, block_y, game_table) == 1)
			block_x--;		
	}
}

void Game::Show_Screen(void)
{
	for(int i = 0; i < HEIGHT; i++)	//render out the table
	{
		for(int j = 0; j < WIDTH; j++)
		{
			if(!game_table[i][j])	
				continue;
			else
				Render_IMG(image, j*BLOCK_SIZE, i*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, (game_table[i][j] - 1)*BLOCK_SIZE, 0, BLOCK_SIZE, BLOCK_SIZE);
		}
	}
	//render the falling block
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(falling_block[i][j])
				Render_IMG(image, block_x*BLOCK_SIZE + j*BLOCK_SIZE, block_y*BLOCK_SIZE + i*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 
					(falling_block[i][j] - 1)*BLOCK_SIZE, 0, BLOCK_SIZE, BLOCK_SIZE);
		}
	}
	//move down to check the collision
	block_y++;
	if(Collision(falling_block, block_x, block_y, game_table))
	{
		Mix_PlayChannel(-1, effect, 0);
		block_y--;	//move back
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
			{
				if(falling_block[i][j] != 0)
					game_table[block_y + i][block_x + j] = falling_block[i][j];	
			}
			block_y = 0;	//generate a new block
			block_x = 4;
			Set_Currentblock(blocks, falling_block, next_block);
			isfour = (next_block <= 2);
			next_block = rand()%(num_blocks + 1);
	}
	else
		block_y--;	//move back

	//this while loop will go as long as we have full lines
	while(1)
	{
		int k = Check_Fulllines(game_table);
		if(k == -1)
			break;
		erase_lines++;
		//if we have full lines
		for(int i = k; i > 0; i--)
			for(int j = 0; j < WIDTH; j++)
				game_table[i][j] = game_table[i-1][j];	//move everything above it a line down
		for(int i = 0; i < HEIGHT; i++)
			game_table[0][i] = 0;	//out 0 the most top line
		points += (550 - normal_speed)/10;
		if((points%50) == 0 && normal_speed >= 50)	
			normal_speed -= 50;
	}

	//render the menu
	menu.push_back(new Draw(sdl_setup->Get_Renderer(), BLOCK_SIZE*WIDTH, 0, 450, screen_h, 0x323232));					
	menu.push_back(new Draw(sdl_setup->Get_Renderer(), BLOCK_SIZE*WIDTH + 50, 50, 4*BLOCK_SIZE, 4*BLOCK_SIZE, 0x646464));

	if(!mousepointing)	//not pointing to the toplist square
		menu.push_back(new Draw(sdl_setup->Get_Renderer(), BLOCK_SIZE*WIDTH + 50, 250 + BLOCK_SIZE*4, 150, 50, 0x000000));
	else
		menu.push_back(new Draw(sdl_setup->Get_Renderer(), BLOCK_SIZE*WIDTH + 50, 250 + BLOCK_SIZE*4, 150, 50, 0xff0000));
	for(int i = 0;i < menu.size(); i++)
		menu[i]->Draw_Rect();

	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(blocks[next_block][i][j])
				Render_IMG(image, (BLOCK_SIZE*WIDTH)+j*BLOCK_SIZE + 50, 50 + i*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 
					(blocks[next_block][i][j] - 1)*BLOCK_SIZE, 0, BLOCK_SIZE, BLOCK_SIZE);				
		}
	}
	//draw all of the menutext 
	Write_Text(sdl_setup->Get_Font(), BLOCK_SIZE*WIDTH + 50, 10, "NEXT BLOCK", 255, 255, 255);
	Write_Text(sdl_setup->Get_Font(), BLOCK_SIZE*WIDTH + 50, 100 + BLOCK_SIZE*4, "POINTS: ", 255, 255, 255);
	Write_Text(sdl_setup->Get_Font(), BLOCK_SIZE*WIDTH + 350, 100 + BLOCK_SIZE*4, points, 255, 255, 255);
	Write_Text(sdl_setup->Get_Font(), BLOCK_SIZE*WIDTH + 50, 150 + BLOCK_SIZE*4, "CURRENT SPEED:", 255, 255, 255);
	Write_Text(sdl_setup->Get_Font(), BLOCK_SIZE*WIDTH + 350, 150 + BLOCK_SIZE*4, (500 - normal_speed)/50, 255, 255, 255);
	Write_Text(sdl_setup->Get_Font(), BLOCK_SIZE*WIDTH + 50, 200 + BLOCK_SIZE*4, "CLEARED LINES: ", 255, 255, 255);
	Write_Text(sdl_setup->Get_Font(), BLOCK_SIZE*WIDTH + 350, 200 + BLOCK_SIZE*4, erase_lines, 255, 255, 255);
	Write_Text(sdl_setup->Get_Font(), BLOCK_SIZE*WIDTH + 50, 250 + BLOCK_SIZE*4, "TOPLIST", 255, 255, 255);	//and write the toplist text	
}

void Game::Render_IMG(SDL_Texture* image, int rect_x, int rect_y, int rect_w, int rect_h, int crop_x, int crop_y, int crop_w, int crop_h)
{
	SDL_Rect rec1= {rect_x, rect_y, rect_w, rect_h};
	SDL_Rect rec2= {crop_x, crop_y, crop_w, crop_h};
	SDL_RenderCopy(sdl_setup->Get_Renderer(), image, &rec2, &rec1);
}

void Game::Write_Text(TTF_Font* font, int x, int y, const char* text, int r, int g, int b)
{
	SDL_Color color = {r, g, b};
	SDL_Surface* surf = TTF_RenderText_Blended(sdl_setup->Get_Font(), text, color);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl_setup->Get_Renderer(), surf);
	;
	int text_w = 0;
	int text_h = 0;
	SDL_QueryTexture(tex, NULL, NULL, &text_w, &text_h);

	SDL_Rect rec = {x, y, text_w, text_h};
	SDL_RenderCopy(sdl_setup->Get_Renderer(), tex, NULL, &rec);
	SDL_FreeSurface(surf);
	SDL_DestroyTexture(tex);
}

void Game::Write_Text(TTF_Font* font, int x, int y, int num, int r, int g, int b)
{
	char text[200];
	sprintf(text, "%d", num);

	SDL_Color color = {r, g, b};
	SDL_Surface* surf = TTF_RenderText_Blended(sdl_setup->Get_Font(), text, color);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl_setup->Get_Renderer(), surf);
	;
	int text_w = 0;
	int text_h = 0;
	SDL_QueryTexture(tex, NULL, NULL, &text_w, &text_h);

	SDL_Rect rec = {x, y, text_w, text_h};
	SDL_RenderCopy(sdl_setup->Get_Renderer(), tex, NULL, &rec);
	SDL_FreeSurface(surf);
	SDL_DestroyTexture(tex);
}

void Game::Display_Toplist(TTF_Font* font)	//we display the toplist
{
	std::ifstream in("data/toplist.dat");
	char name[10][100];
	int point[10];
	int num_p = 0;

	while(1)	//read in from the toplist.dat file
	{
		in >> name[num_p] >> point[num_p];
		if(in.eof())
			break;
		if(name[num_p] != '\0')
			num_p++;
		if(num_p == 10)
			break;
	}

	SDL_Event event;

	Draw* toplist = new Draw(sdl_setup->Get_Renderer(), 10, 10, screen_w - 20, screen_h - 20, 0x000000);
	while(1)
	{		
		sdl_setup->Start();

		toplist->Draw_Rect();
		for(int i = 0; i < num_p; i++)
		{
			//write out the name and point row by row
			Write_Text(font, 40, 20 + i * 40, name[i], 255, 255, 255);
			Write_Text(font, 500, 20 + i * 40, point[i], 255, 255, 255);
		}

		Write_Text(font, 40, screen_h - 40, "PRESS P TO BACK TO THE GAME", 255, 255, 255);
		sdl_setup->End();
		SDL_WaitEvent(sdl_setup->Get_Main());
		if(sdl_setup->Get_Main()->type == SDL_KEYDOWN && sdl_setup->Get_Main()->key.keysym.sym == SDLK_p)	//if escape is pressed, return to the game
		{	
			delete toplist;
			return;
		}
	}
}

bool Game::Add_Toplist(TTF_Font* font,int points)
{
	std::ifstream in("data/toplist.dat");	//open the toplist
	char name[10][100];	
	int point[10];			
	int num_p = 0;	
	top_num = 10;
	std::string tmpname;	//the player's name

	while(!in.eof())
	{
		in >> name[num_p] >> point[num_p];
		if(in.eof())
		{
			top_num = num_p;
			break;
		}
		if(point[num_p] < points)
		{
			if(top_num > num_p)
				top_num = num_p;
		}
		if(name[num_p] != '\0')
			num_p++;
		if(num_p == 10)
			break;
	}

	SDL_Event subevent;

	if(top_num < 10)	
	{
		//add the player into toplist
		bool isquit = false;	
		bool shift = false;			
		bool ready = false;		

		Draw* toplist = new Draw(sdl_setup->Get_Renderer(), 0, (BLOCK_SIZE*HEIGHT)/2 - 50, screen_w, 50, 0xff0000);

		while(!isquit && !ready)
		{
			sdl_setup->Start();

			toplist->Draw_Rect();
			Write_Text(font, 50, (BLOCK_SIZE*HEIGHT)/2 - 50,"CONGRATULATION, YOU GET INTO THE TOPLIST", 255, 255, 255);	//write out a text
			Write_Text(font, 50, (BLOCK_SIZE*HEIGHT)/2, tmpname.c_str(), 255, 255, 255);	//and the characters, which the user already entered

			sdl_setup->End();	//show

			SDL_WaitEvent(&subevent);	
			if(subevent.type == SDL_KEYDOWN)
			{
				if(subevent.key.keysym.sym == SDLK_ESCAPE)	
				{
					isquit = true;	//exit
					break;
				}
				else if(subevent.key.keysym.sym == SDLK_RETURN && tmpname.size()!=0)	
				{
					ready = true;	//we are ready
					break;
				}
				else if((subevent.key.keysym.sym >= SDLK_a && subevent.key.keysym.sym <= SDLK_z))	//if key a to z
				{
					if(shift)	//if we pressed shift
						tmpname += subevent.key.keysym.sym - 32;	//ASCII code
					else
						tmpname += subevent.key.keysym.sym;	
						
				}
				else if(subevent.key.keysym.sym >= SDLK_0 && subevent.key.keysym.sym <= SDLK_9)	//if key 1 to 9
				{
					tmpname += subevent.key.keysym.sym;	
				}
				else if(subevent.key.keysym.sym == SDLK_RSHIFT || subevent.key.keysym.sym == SDLK_LSHIFT)	
				{
					shift = true;
				}
				else if(subevent.key.keysym.sym == SDLK_BACKSPACE && tmpname.size() != 0)	
				{
					tmpname.erase(tmpname.end() - 1);	//delete the last character
				}
			}
			else if(subevent.type == SDL_KEYUP)
			{
				if(subevent.key.keysym.sym == SDLK_RSHIFT || subevent.key.keysym.sym == SDLK_LSHIFT)	//if we released shift
				{
					shift = false;
				}			
			}
		}
		delete toplist;

		if(ready)
		{
			strcpy(name[num_p], tmpname.c_str());	//add the name at the end of the array
			point[num_p] = points;	

			std::ofstream out("data/toplist.dat");

			for(int i = 0; i < num_p; i++)
			{
				if(i == top_num)
					out << name[num_p] << " " << point[num_p] << "\n";
				out << name[i] << " " << point[i] << "\n";
			}
			if(top_num == num_p)
				out << name[num_p] << " " << point[num_p] << "\n";
			out.flush();	
			//do we want to play again?

			Draw* toplist = new Draw(sdl_setup->Get_Renderer(), 0, (BLOCK_SIZE*HEIGHT)/2 - 50, screen_w, 50, 0xff0000);

			while(1)
			{
				sdl_setup->Start();	

				toplist->Draw_Rect();
				Write_Text(font, 50, (BLOCK_SIZE*HEIGHT)/2 - 50, "AGAIN? (y/n)", 255, 255, 255);	

				sdl_setup->End();	//show

				SDL_WaitEvent(&subevent);
				if(subevent.type == SDL_KEYDOWN && subevent.key.keysym.sym == SDLK_y)	
				{
					delete toplist;
					return true;
				}
				if(subevent.type == SDL_KEYDOWN && (subevent.key.keysym.sym == SDLK_n || subevent.key.keysym.sym==SDLK_ESCAPE))	
				{
					delete toplist;
					return 0;
				}
			}
		}
	}
	else
	{
		Draw* toplist = new Draw(sdl_setup->Get_Renderer(), 0, (BLOCK_SIZE*HEIGHT)/2 - 50, screen_w, 50, 0xff0000);
	
		while(1)
		{
			sdl_setup->Start();	

			toplist->Draw_Rect();
			Write_Text(font, 50, (BLOCK_SIZE*HEIGHT)/2 - 50, "YOU LOST, AGAIN? (y/n)", 255, 255, 255);

			sdl_setup->End();	//show

			SDL_WaitEvent(&subevent);
			if(subevent.type == SDL_KEYDOWN && subevent.key.keysym.sym == SDLK_y)	
			{
				delete toplist;
				return true;
			}
			if(subevent.type == SDL_KEYDOWN && (subevent.key.keysym.sym == SDLK_n || subevent.key.keysym.sym==SDLK_ESCAPE))	
			{
				delete toplist;
				return 0;
			}
		}
	}
}

void Game::Free(void)
{
	for(int i = 0;i < menu.size(); i++)
		delete menu[i];
	menu.clear();
}

void Game::Start(void)
{
	while(!quit) 
	{	
		sdl_setup->Start();

		Move();
		Excute();
		Show_Screen();

		if(1000.0/FPS > SDL_GetTicks() - start)	
			SDL_Delay(1000.0/FPS - (SDL_GetTicks() - start));	//regulate the FPS

		sdl_setup->End();
		Free();
	}
}