#pragma once

#include <SDL.h>

using u8 = unsigned char;
using u16 = unsigned short;

class Display 
{
    SDL_Texture* screen;
    SDL_Renderer* renderer;
    bool data[2048];

public:
    Display(SDL_Renderer* renderer);
    ~Display();
    void set(int x, int y, bool value);
    bool get(int x, int y) const;
    void render() const;
};

class Cpu 
{
    u8 memory[4096];
    u8 V[16];
    u16 I;
    u8 DT;
    u8 ST;
    u16 PC;
    u8 SP;
    u16 stack[16];

public:
    Cpu();
    void reset();
    void cycle(Display& display);
};