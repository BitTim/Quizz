//Quizz v1.0.0 Windows
//Copyright (c) BitTim 2019
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "SDL_Extend.h"
#include "question_handler.h"
#include "mouse_handler.h"
#include "variables.h"
#include "delay_ms.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event event;

TTF_Font *font;
TTF_Font *link_font;
SDL_Color text_color = {255, 255, 255};
SDL_Color green_color = {53, 214, 56};
SDL_Color red_color = {214, 53, 56};

Mix_Music *bgm;
Mix_Chunk *click;
Mix_Chunk *hover;
Mix_Chunk *correct_sound;
Mix_Chunk *wrong_sound;

int tile_color[3] = {36, 36, 36};
int tile_color_menu[3] = {21, 49, 89};

const Uint8 *key_state = SDL_GetKeyboardState(NULL);
unsigned int last_time = 0, current_time;
int unused;

bool quit = false;
bool menu = true;
bool sleep = false;

bool escape_down = false;
bool left_mouse_down = false;

int num_questions = 0;
int curr_question = 0;
int amount_selected_questions = 1;
int menu_type = 1;

int answer = 0;
int prev_answer = -1;
int corr_answer = 0;

int screen = 0;

int progress_height = 5;
int progress_width = screen_width - 40;

bool trig_end = false;
bool first_trig_end = false;
bool music = true;

std::vector<int> answer_history;
std::vector<int> question_history;
int correct = 0;
int wrong = 0;

int curr_question_id = 0;
int corr_rate;

std::fstream config_file;
std::string str_buffer;

int get_new_id();

void re_init_screen()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	window = SDL_CreateWindow("Quizz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);
}

void init()
{
	re_init_screen();
	Mix_Init(MIX_INIT_MP3);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	font = TTF_OpenFont("data/fonts/font.ttf", 50);
	if(!font) std::cout << "Error loading font" << std::endl;

	num_questions = load_question_files();
	if(num_questions == -1) std::cout << "Error loading questions" << std::endl;

	bgm = Mix_LoadMUS("data/sound/bgm.mp3");
	click = Mix_LoadWAV("data/sound/click.wav");
	hover = Mix_LoadWAV("data/sound/hover.wav");
	correct_sound = Mix_LoadWAV("data/sound/correct.wav");
	wrong_sound = Mix_LoadWAV("data/sound/wrong.wav");

	Mix_PlayMusic(bgm, -1);

	config_file.open("data/config.cfg", std::fstream::in);

	std::getline(config_file, str_buffer);
  	screen_width = std::stoi(str_buffer);

  	std::getline(config_file, str_buffer);
  	screen_height = std::stoi(str_buffer);

  	config_file.close();

	get_new_id();
}

void end_screen()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	TTF_Quit();
	SDL_Quit();
}

void end()
{
	Mix_HaltMusic();

	Mix_FreeMusic(bgm);
	Mix_FreeChunk(click);
	Mix_FreeChunk(hover);

	TTF_CloseFont(font);
	Mix_CloseAudio();

	Mix_Quit();
	end_screen();
}

int get_new_id()
{
	if(curr_question < amount_selected_questions)
	{
		int n_secs = 0;
		int rnd = 0;
		bool repeat = true;
		curr_question++;

		while(repeat)
		{
			n_secs++;
			repeat = false;

			rnd = rand() % num_questions;

			for(int i = 0; i < question_history.size(); i++)
			{
				if(question_history[i] == rnd) repeat = true;
			}

			if(n_secs >= 10000)
			{
				return -1;
			}

			delay_ms(1);
		}

		curr_question_id = rnd;
		return 0;
	}

	if(curr_question >= amount_selected_questions)
	{
		first_trig_end = true;
		return 0;
	}

	return -1;
}

void reset()
{
	curr_question = 1;
	menu_type = 2;
	menu = true;

	correct = 0;
	wrong = 0;

	answer_history.clear();
	question_history.clear();
}

int check_answer()
{
	if(!menu && !trig_end)
	{
		if(corr_answer == 0 || corr_answer > 4)
		{
			printf("Error loading correct answer\n");
			return -1;
		}

		if(answer == corr_answer) 
		{
			answer = 0;
			answer_history[curr_question - 1] = 1;
			correct++;

			Mix_PlayChannel(-1, correct_sound, 0);

			if(get_new_id() == -1) return -1;
			return 1;
		}

		answer = 0;
		answer_history[curr_question - 1] = 2;
		wrong++;

		Mix_PlayChannel(-1, wrong_sound, 0);

		if(get_new_id() == -1) return -1;
		return 2;
	}
	else if(trig_end)
	{
		switch(answer)
		{
			case 3:
				trig_end = false;
				reset();
				break;

			case 4:
				quit = true;
				Mix_PlayChannel(-1, click, 0);
				break;
		}
	}
	else if(menu && !trig_end)
	{
		if((answer == 1 || answer == 3) && menu_type == 4)
		{
			config_file.open("data/config.cfg", std::fstream::out);

			if(answer == 1)
			{
				config_file << "1366" << std::endl << "768" << std::endl;
				screen_width = 1366;
				screen_height = 768;
			}

			if(answer == 3)
			{
				config_file << "1280" << std::endl << "720" << std::endl;
				screen_width = 1280;
				screen_height = 720;
			}
			
			config_file.close();

			end_screen();
			re_init_screen();
		}
		else
		{
			switch(answer)
			{
				case 1:
					if(menu_type == 1)
					{
						reset();
						menu_type = 2;
						amount_selected_questions--;
					}

					if(menu_type == 2) if(amount_selected_questions < num_questions) amount_selected_questions++;
					if(menu_type == 5)
					{
						menu = false;
						delay_ms(200);
					}
					break;

				case 2:
					if(menu_type == 1) menu_type = 3;
					if(menu_type == 2) if(amount_selected_questions > 1) amount_selected_questions--;
					if(menu_type == 4)
					{
						music ? Mix_HaltMusic() : Mix_PlayMusic(bgm, -1);
						music = !music;
					}
					if(menu_type == 5) reset();
					break;

				case 3:
					if(menu_type == 1) menu_type = 4;
					if(menu_type == 2)
					{
						for(int i = 0; i < amount_selected_questions; i++)
						{
							answer_history.push_back(0);
							question_history.push_back(-1);
						}

						question_history[0] = curr_question_id;
						menu = false;
						delay_ms(200);
					}
					break;

				case 4:
					if(menu_type == 1)
					{
						quit = true;
						Mix_PlayChannel(-1, click, 0);
					}
					if(menu_type == 2 || menu_type == 3 || menu_type == 4 || menu_type == 5) menu_type = 1;
					break;
			}
		}
	}
}

void draw_screen()
{
	if(!menu && !trig_end)
	{
		float progress_segemnt_width;

		SDL_SetRenderDrawColor(renderer, 63, 63, 63, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, tile_color[0], tile_color[1], tile_color[2], 255);

		//Question Box
		SDL_RenderDrawBox(renderer, 20, 20, screen_width - 40, screen_height / 2 - 40);

		//Answer Boxes
		answer == 1 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color[0], tile_color[1], tile_color[2], 255);
		SDL_RenderDrawBox(renderer, 20, screen_height / 2 + 20, screen_width / 2 - 30, screen_height / 4 - 30);

		answer == 2 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color[0], tile_color[1], tile_color[2], 255);
		SDL_RenderDrawBox(renderer, screen_width / 2 + 10, screen_height / 2 + 20, screen_width / 2 - 30, screen_height / 4 - 30);

		answer == 3 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color[0], tile_color[1], tile_color[2], 255);
		SDL_RenderDrawBox(renderer, 20, screen_height / 2  + screen_height / 4 + 10, screen_width / 2 - 30, screen_height / 4 - 30);

		answer == 4 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color[0], tile_color[1], tile_color[2], 255);
		SDL_RenderDrawBox(renderer, screen_width / 2 + 10, screen_height / 2 + screen_height / 4 + 10, screen_width / 2 - 30, screen_height / 4 - 30);

		//Progress bar
		for(int i = 0; i < amount_selected_questions; i++)
		{
			if(answer_history[i] == 0)
			{
				SDL_SetRenderDrawColor(renderer, 49, 49, 49, 255);
			}
			else if(answer_history[i] == 1)
			{
				SDL_SetRenderDrawColor(renderer, 53, 214, 56, 255);
			}
			else if(answer_history[i] == 2)
			{
				SDL_SetRenderDrawColor(renderer, 214, 53, 56, 255);
			}

			if(i == amount_selected_questions - 1) SDL_RenderDrawBox(renderer, 20 + (i * (int)(progress_width / amount_selected_questions)), screen_height / 2 - 20, (int)(progress_width / amount_selected_questions), progress_height);
			else SDL_RenderDrawBox(renderer, 20 + (i * (int)(progress_width / amount_selected_questions)), screen_height / 2 - 20, (int)(progress_width / amount_selected_questions), progress_height);
		}

		//Text
		if(screen == 0)
		{
			print_question(renderer, curr_question_id, &corr_answer, font, text_color);
			sleep = false;
		}
		else if(screen == 1)
		{
			TTF_Print(renderer, "Korrekt!", &unused, &unused, 30, 30, screen_width - 40, font, green_color);
			screen = 0;
			sleep = true;
		}
		else if(screen == 2)
		{
			TTF_Print(renderer, "Leider Falsch!", &unused, &unused, 30, 30, screen_width - 40, font, red_color);
			screen = 0;
			sleep = true;
		}
	}
	else if(trig_end && !menu)
	{
		int text_height = 0, text_width = 0, text_width_2 = 0, text_height_2 = 0;
		SDL_SetRenderDrawColor(renderer, 39, 103, 196, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, tile_color_menu[0], tile_color_menu[1], tile_color_menu[2], 255);

		//Question Box
		SDL_RenderDrawBox(renderer, 20, 20, screen_width - 40, screen_height / 2 + screen_height / 4 - 40);

		//Answer Boxes
		answer == 3 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color_menu[0], tile_color_menu[1], tile_color_menu[2], 255);
		SDL_RenderDrawBox(renderer, 20, screen_height / 2  + screen_height / 4 + 10, screen_width / 2 - 30, screen_height / 4 - 30);

		answer == 4 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color_menu[0], tile_color_menu[1], tile_color_menu[2], 255);
		SDL_RenderDrawBox(renderer, screen_width / 2 + 10, screen_height / 2 + screen_height / 4 + 10, screen_width / 2 - 30, screen_height / 4 - 30);

		//Text
		sprintf(buffer, "%d ", amount_selected_questions);
		TTF_Print(renderer, "Sie haben ", &text_width, &unused, 30, 30, screen_width - 40, font, text_color);
		TTF_Print(renderer, buffer, &text_width_2, &unused, 30 + text_width, 30, screen_width - 40, font, text_color);
		TTF_Print(renderer, "Fragen Beantwortet", &unused, &text_height, 30 + text_width + text_width_2, 30, screen_width - 40, font, text_color);

		sprintf(buffer, "%d ", correct);
		TTF_Print(renderer, "Davon haben sie ", &text_width, &unused, 30, 30 + text_height, screen_width - 40, font, text_color);
		TTF_Print(renderer, buffer, &text_width_2, &unused, 30 + text_width, 30 + text_height, screen_width - 40, font, green_color);
		TTF_Print(renderer, "Richtig", &unused, &text_height_2, 30 + text_width + text_width_2, 30 + text_height, screen_width - 40, font, green_color);

		text_height += text_height_2;

		sprintf(buffer, "%d ", wrong);
		TTF_Print(renderer, "und ", &text_width, &unused, 30, 30 + text_height, screen_width - 40, font, text_color);
		TTF_Print(renderer, buffer, &text_width_2, &unused, 30 + text_width, 30 + text_height, screen_width - 40, font, red_color);
		text_width += text_width_2;
		TTF_Print(renderer, "Falsch", &text_width_2, &text_height_2, 30 + text_width, 30 + text_height, screen_width - 40, font, red_color);
		TTF_Print(renderer, " beantwortet", &unused, &unused, 30 + text_width + text_width_2, 30 + text_height, screen_width - 40, font, text_color);

		corr_rate = (int)((float)(100.0f * correct / amount_selected_questions));
		text_height += text_height_2;

		sprintf(buffer, "%d", corr_rate);
		strcat(buffer, "\% ");
		TTF_Print(renderer, "Sie haben ", &text_width, &unused, 30, 30 + text_height, screen_width - 40, font, text_color);
		TTF_Print(renderer, buffer, &text_width_2, &unused, 30 + text_width, 30 + text_height, screen_width - 40, font, green_color);
		TTF_Print(renderer, "der Fragen", &unused, &text_height_2, 30 + text_width + text_width_2, 30 + text_height, screen_width - 40, font, text_color);
		TTF_Print(renderer, "richtig beantwortet", &unused, &unused, 30, 30 + text_height + text_height_2, screen_width - 40, font, text_color);

		TTF_Print(renderer, "Nochmal", &unused, &unused, 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
		TTF_Print(renderer, "Beenden", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);

		//Pie Chart
		SDL_SetRenderDrawColor(renderer, 53, 214, 56, 255);
		SDL_RenderDrawCircle(renderer, screen_width / 2 + screen_width / 4, (screen_height / 2 + screen_height / 4) / 2, screen_width / 4 - 150, 0, 360 * 100 * corr_rate / (100 * 100));
		
		SDL_SetRenderDrawColor(renderer, 214, 53, 56, 255);
		SDL_RenderDrawCircle(renderer, screen_width / 2 + screen_width / 4, (screen_height / 2 + screen_height / 4) / 2, screen_width / 4 - 150, 360 * 100 * corr_rate / (100 * 100), 360);
	}
	else if(menu && !trig_end)
	{
		SDL_SetRenderDrawColor(renderer, 39, 103, 196, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, tile_color_menu[0], tile_color_menu[1], tile_color_menu[2], 255);

		//Question Box
		SDL_RenderDrawBox(renderer, 20, 20, screen_width - 40, screen_height / 2 - 40);

		//Answer Boxes
		answer == 1 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color_menu[0], tile_color_menu[1], tile_color_menu[2], 255);
		SDL_RenderDrawBox(renderer, 20, screen_height / 2 + 20, screen_width / 2 - 30, screen_height / 4 - 30);

		answer == 2 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color_menu[0], tile_color_menu[1], tile_color_menu[2], 255);
		SDL_RenderDrawBox(renderer, screen_width / 2 + 10, screen_height / 2 + 20, screen_width / 2 - 30, screen_height / 4 - 30);

		answer == 3 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color_menu[0], tile_color_menu[1], tile_color_menu[2], 255);
		SDL_RenderDrawBox(renderer, 20, screen_height / 2  + screen_height / 4 + 10, screen_width / 2 - 30, screen_height / 4 - 30);

		answer == 4 ? SDL_SetRenderDrawColor(renderer, 168, 45, 98, 255) : SDL_SetRenderDrawColor(renderer, tile_color_menu[0], tile_color_menu[1], tile_color_menu[2], 255);
		SDL_RenderDrawBox(renderer, screen_width / 2 + 10, screen_height / 2 + screen_height / 4 + 10, screen_width / 2 - 30, screen_height / 4 - 30);

		//Text
		switch(menu_type)
		{
			case 1:
				TTF_Print(renderer, "Willkommen zu Quizz v1.0.0!", &unused, &unused, 30, 30, screen_width - 40, font, text_color);

				TTF_Print(renderer, "Spielen", &unused, &unused, 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Credits", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Einstellungen", &unused, &unused, 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Beenden", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				break;

			case 2:
			{
				int text_height = 0, text_width = 0;

				TTF_Print(renderer, "Wie viele Fragen wollen Sie beantworten?", &unused, &text_height, 30, 30, screen_width - 40, font, text_color);

				sprintf(buffer, "%d", num_questions);
				TTF_Print(renderer, "Verfügbar: ", &text_width, &unused, 30, 30 + text_height + 10, screen_width - 40, font, text_color);
				TTF_Print(renderer, buffer, &unused, &unused, 30 + text_width, 30 + text_height + 10, screen_width - 40, font, text_color);

				sprintf(buffer, "%d", amount_selected_questions);
				TTF_Print(renderer, "Ausgewählt: ", &text_width, &unused, screen_width / 2, 30 + text_height + 10, screen_width / 2 - 20, font, text_color);
				TTF_Print(renderer, buffer, &unused, &unused, screen_width / 2 + text_width, 30 + text_height + 10, screen_width / 2 - 20, font, text_color);


				TTF_Print(renderer, "Mehr", &unused, &unused, 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Weniger", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Start", &unused, &unused, 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Zurück", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				break;
			}

			case 3:
			{
				int text_height = 0, text_height_2 = 0;
				TTF_Print(renderer, "Code, GUI und Musik: BitTim", &unused, &text_height, 30, 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "SFX: \"Multimedia click 3\", \"Multimedia click 1\" und \"Game error tone 5\" von \"https://www.zapsplat.com/\"", &unused, &text_height_2, 30, 30 + text_height, screen_width - 40, font, text_color);

				text_height += text_height_2;
				TTF_Print(renderer, "Quizz, Copyright (c) BitTim 2019", &unused, &unused, 30, 30 + text_height + 75, screen_width - 100, font, red_color);



				TTF_Print(renderer, " ", &unused, &unused, 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, " ", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, " ", &unused, &unused, 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Zurück", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				break;
			}

			case 4:
			{
				int text_height = 0, text_width = 0, text_height_2 = 0;
				TTF_Print(renderer, "Bitte Wälen sie eine Bildschirmauflösung / Einstellung", &unused, &text_height, 30, 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Aktuell: ", &text_width, &text_height_2, 30, 30 + text_height, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Das Programm kann möglicherweise nach dem Wechseln der Auflösung ein paar Sekunden nicht reagieren", &unused, &unused, 30, 30 + text_height + text_height_2 + 80, screen_width - 40, font, red_color);

				sprintf(buffer, "%dx%d", screen_width, screen_height);
				TTF_Print(renderer, buffer, &unused, &unused, 30 + text_width, 30 + text_height, screen_width - 40, font, red_color);

				TTF_Print(renderer, "1366x768", &unused, &unused, 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Musik", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "1280x720", &unused, &unused, 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Zurück", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				break;
			}

			case 5:
			{
				int text_height = 0, text_width = 0, text_height_2 = 0;
				TTF_Print(renderer, "Pausiert", &unused, &text_height, 30, 30, screen_width - 40, font, text_color);

				TTF_Print(renderer, "Weiter", &unused, &unused, 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Neustarten", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + 30, screen_width - 40, font, text_color);
				TTF_Print(renderer, " ", &unused, &unused, 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				TTF_Print(renderer, "Menü", &unused, &unused, screen_width / 2 + 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, text_color);
				break;
			}
		}
	}
}

void update()
{
	if(answer != prev_answer)
	{
		if(answer != 0) Mix_PlayChannel(-1, hover, 0);
		draw_screen();
	}

	prev_answer = answer;

	if(first_trig_end)
	{
		trig_end = true;
		first_trig_end = false;
	}

	if(mouse_hover(20, screen_height / 2 + 20, screen_width / 2 - 30, screen_height / 4 - 30)) answer = 1;
	else if(mouse_hover(screen_width / 2 + 10, screen_height / 2 + 20, screen_width / 2 - 30, screen_height / 4 - 30)) answer = 2;
	else if(mouse_hover(20, screen_height / 2  + screen_height / 4 + 10, screen_width / 2 - 30, screen_height / 4 - 30)) answer = 3;
	else if(mouse_hover(screen_width / 2 + 10, screen_height / 2 + screen_height / 4 + 10, screen_width / 2 - 30, screen_height / 4 - 30)) answer = 4;
	else answer = 0;

	SDL_RenderPresent(renderer);

	if(sleep)
	{
		sleep = false;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int main(int argc, char* argv[])
{
	init();

	while(!quit)
	{
		current_time = SDL_GetTicks();
 		if(current_time > last_time + 5)
 		{
 			last_time = current_time;
 			SDL_PollEvent(&event);

			switch(event.type)
			{
				case SDL_QUIT:
					quit = true;
					break;	

				case SDL_MOUSEBUTTONDOWN:
					switch(event.button.button)
					{
						case SDL_BUTTON_LEFT:
							if(!left_mouse_down)
							{
								left_mouse_down = true;

								if(answer != 0)
								{
									Mix_PlayChannel(-1, click, 0);
									screen = check_answer();

									if(screen == -1)
									{
										end();
										return -1;
									}
								}
							}

							left_mouse_down = true;
							break;
					}
					break;

				case SDL_MOUSEBUTTONUP:
					switch(event.button.button)
					{
						case SDL_BUTTON_LEFT:
							left_mouse_down = false;
							break;
					}
					break;
			}

			if(key_state[SDL_SCANCODE_ESCAPE] == 1 && !escape_down)
			{
				escape_down = true;
				Mix_PlayChannel(-1, click, 0);
				menu = !menu;
				menu_type = 5;
			}

			if(key_state[SDL_SCANCODE_ESCAPE] == 0) escape_down = false;

			update();
		}
	}

	end();

	return 0;
}
