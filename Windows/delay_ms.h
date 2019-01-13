#ifndef DELAY_MS_H
#define DELAY_MS_H

#ifdef LINUX
	#include <unistd.h>
#endif // LINUX

#ifdef WINDOWS
	#include <windows.h>
#endif // WINDOWS

void delay_ms(int sleepMs)
{
#ifdef LINUX
    usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif // LINUX

#ifdef WINDOWS
    Sleep(sleepMs);
#endif // WINDOWS
}

#endif // DELAY_MS_H