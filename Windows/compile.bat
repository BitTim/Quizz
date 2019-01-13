@echo off
echo Compiling main.cpp
g++ main.cpp -IC:\Dev\SDL2-mingw32-32bit\include -LC:\Dev\SDL2-mingw32-32bit\lib -w -Wl,-subsystem,windows -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer -o main.exe
echo Compiled to main.exe!
pause