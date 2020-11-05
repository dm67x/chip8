#pragma once

#include <SDL.h>
#include <string>

using u8 = unsigned char;
using u16 = unsigned short;

#define opcode_type(opcode) (opcode & 0xF000) >> 12
#define opcode_x(opcode) (opcode & 0x0F00) >> 8
#define opcode_y(opcode) (opcode & 0x00F0) >> 4
#define opcode_n(opcode) opcode & 0x000F
#define opcode_addr(opcode) opcode & 0x0FFF
#define opcode_kk(opcode) opcode & 0x00FF

enum class Flags {
    NONE,
    CARRY,
    NOT_BORROW,
    COLLISION
};

class Display 
{
    SDL_Texture* screen;
    SDL_Renderer* renderer;
    bool data[2048];

public:
    Display(SDL_Renderer* renderer);
    void dispose();
    void set(int x, int y, bool value);
    bool get(int x, int y) const;
    void clear();
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
    Display& display;
    Flags flag;

private:
    static void (*instructions[16])(Cpu&, u16);

private:
    u16 get_opcode();
    friend void opcode_0(Cpu& cpu, u16 opcode);
    friend void opcode_1(Cpu& cpu, u16 opcode);
    friend void opcode_2(Cpu& cpu, u16 opcode);
    friend void opcode_3(Cpu& cpu, u16 opcode);
    friend void opcode_4(Cpu& cpu, u16 opcode);
    friend void opcode_5(Cpu& cpu, u16 opcode);
    friend void opcode_6(Cpu& cpu, u16 opcode);
    friend void opcode_7(Cpu& cpu, u16 opcode);
    friend void opcode_8(Cpu& cpu, u16 opcode);
    friend void opcode_9(Cpu& cpu, u16 opcode);
    friend void opcode_A(Cpu& cpu, u16 opcode);
    friend void opcode_B(Cpu& cpu, u16 opcode);
    friend void opcode_C(Cpu& cpu, u16 opcode);
    friend void opcode_D(Cpu& cpu, u16 opcode);
    friend void opcode_E(Cpu& cpu, u16 opcode);
    friend void opcode_F(Cpu& cpu, u16 opcode);

public:
    Cpu(Display& display);
    void reset();
    void cycle();
    void open(const std::string& filename);
};