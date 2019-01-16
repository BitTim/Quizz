#ifndef QUESTION_HANDLER_H
#define QUESTION_HANDLER_H

#include <dirent.h>
#include <string.h>
#include <vector>
#include <fstream>

#include "SDL_Extend.h"
#include "variables.h"

std::vector<std::string> files;
std::vector<std::string> categories;
int num_files = 0;
int num_cfiles = 0;
char buffer[100];

int load_category_files()
{
	struct dirent *directory;
	DIR *dir = opendir("data/questions/.");

	if (dir == NULL)
	{
	    printf("Could not open current directory\n");
	    return -1;
	}

	while ((directory = readdir(dir)) != NULL)
	{
	    if(directory->d_name[0] != '.')
	    {
	    	printf("Found Category: %s\n", directory->d_name);
	    	categories.push_back(directory->d_name);
	    	num_cfiles++;
	    }
	}

	printf("Found %d Categories\n", num_cfiles);

	return num_cfiles;
}

int load_question_files(int category)
{
	files.clear();
	struct dirent *directory;

	strcpy(buffer, "data/questions/");
	strcat(buffer, categories[category].c_str());
	strcat(buffer, "/.");

	printf("Selected Category: %s\n", buffer);

	DIR *dir = opendir(buffer);

	if (dir == NULL)
	{
	    printf("Could not open current directory\n");
	    return -1;
	}

	while ((directory = readdir(dir)) != NULL)
	{
	    if(directory->d_name[0] != '.')
	    {
	    	printf("Found Question: %s\n", directory->d_name);
	    	files.push_back(directory->d_name);
	    	num_files++;
	    }
	}

	if(num_files == 1)
	{
		printf("\nFound %d File\n", num_files);
	}
	else
	{
		printf("\nFound %d Files\n", num_files);
	}

	closedir(dir);
	return files.size();
}

int print_question(SDL_Renderer *renderer, int id, int category, int *corr_answer, TTF_Font *font, SDL_Color color)
{
	int text_width, text_height;
	std::string str_buffer;
	std::ifstream question_file;

	std::string question;
	std::string answers[4];

	strcpy(buffer, "data/questions/");
	strcat(buffer, categories[category].c_str());
	strcat(buffer, "/");
	strcat(buffer, files[id].c_str());

	question_file.open(buffer);
	if (question_file.is_open())
  	{
  		std::getline(question_file, question);

  		for(int i = 0; i < 4; i++)
  		{
  			std::getline(question_file, answers[i]);
  		}

  		std::getline(question_file, str_buffer);
  		*corr_answer = std::stoi(str_buffer);
  	}
  	else
  	{
  		std::cout << "Unable to open file";
  		return -1;
  	}

  	//Question
	TTF_Print(renderer, question.c_str(), &text_width, &text_height, 30, 30, screen_width - 40, font, color);

	//Answers
	TTF_Print(renderer, answers[0].c_str(), &text_width, &text_height, 30, screen_height / 2 + 30, screen_width - 40, font, color);
	TTF_Print(renderer, answers[1].c_str(), &text_width, &text_height, screen_width / 2 + 30, screen_height / 2 + 30, screen_width - 40, font, color);
	TTF_Print(renderer, answers[2].c_str(), &text_width, &text_height, 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, color);
	TTF_Print(renderer, answers[3].c_str(), &text_width, &text_height, screen_width / 2 + 30, screen_height / 2 + screen_height / 4 + 10, screen_width - 40, font, color);

	question_file.close();
	return 0;
}

#endif // QUESTION_HANDLER_H
