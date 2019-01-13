#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <SDL2/SDL.h>

bool mouse_hover(int x, int y, int w, int h)
{
	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);
	
	if(mouse_x > x && mouse_x < x + w && mouse_y > y && mouse_y < y + h) return true;
	return false;
}

#endif // MOUSE_HANDLER_H