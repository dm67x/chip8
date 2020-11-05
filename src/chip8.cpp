#include "chip8.hpp"
#include <cstring>
#include <cassert>
#include <iostream>
#include <fstream>
#include <random>

#define FREQUENCY 60 // Hz
#define CYCLE_MS (1.0 / FREQUENCY) * 1000

#define unvalid_instr() {\
    std::cerr << "unvalid instruction" << std::endl;\
    std::exit(EXIT_FAILURE); }

//=================================[DISPLAY]=================================//
Display::Display(SDL_Renderer* renderer) {
    this->renderer = renderer;
    screen = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_TARGET, 
        64, 32);

    if (!screen) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't create texture: %s", 
            SDL_GetError());
        return;
    }

    clear();
}

void Display::dispose() {
    SDL_DestroyTexture(screen);
}

void Display::set(int x, int y, bool value) {
    data[y * 64 + x] = value;
}

bool Display::get(int x, int y) const {
    return data[y * 64 + x];
}

void Display::clear() {
    for (int i = 0; i < 2048; i++) {
        data[i] = false;
    }
}

void Display::render() const {
    SDL_SetRenderTarget(renderer, screen);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            SDL_Rect r;
            r.x = x;
            r.y = y;
            r.w = 1;
            r.h = 1;

            if (data[y * 64 + x]) {
                SDL_RenderDrawRect(renderer, &r);
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderCopy(renderer, screen, nullptr, nullptr);
}

//=================================[  CPU  ]=================================//
void opcode_0(Cpu& cpu, u16 opcode) {
    u16 who = opcode_kk(opcode);
    switch (who) {
        case 0x00E0:
            cpu.display.clear();
            break;

        case 0x00EE:
            assert(cpu.SP > 0);
            cpu.PC = cpu.stack[--cpu.SP];
            break;

        default: 
            unvalid_instr();
            break;
    }
    cpu.PC += 2;
}

void opcode_1(Cpu& cpu, u16 opcode) {
    cpu.PC = opcode_addr(opcode);
}

void opcode_2(Cpu& cpu, u16 opcode) {
    assert(cpu.SP < 16);
    cpu.stack[cpu.SP++] = cpu.PC;
    cpu.PC = opcode_addr(opcode);
}

void opcode_3(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    u8 kk = opcode_kk(opcode);
    assert(x >= 0 && x < 16);
    cpu.PC += (cpu.V[x] == kk) ? 4 : 2;
}

void opcode_4(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    u8 kk = opcode_kk(opcode);
    assert(x >= 0 && x < 16);
    cpu.PC += (cpu.V[x] != kk) ? 4 : 2;
}

void opcode_5(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    u16 y = opcode_y(opcode);
    u8 n = opcode_n(opcode);
    if (n != 0) {
        unvalid_instr();
        return;
    }
    assert(x >= 0 && x < 16);
    assert(y >= 0 && y < 16);
    cpu.PC += (cpu.V[x] == cpu.V[y]) ? 4 : 2;
}

void opcode_6(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    u8 kk = opcode_kk(opcode);
    assert(x >= 0 && x < 16);
    cpu.V[x] = kk;
    cpu.PC += 2;
}

void opcode_7(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    u8 kk = opcode_kk(opcode);
    assert(x >= 0 && x < 16);
    cpu.V[x] = opcode_kk((cpu.V[x] + kk));
    cpu.PC += 2;
}

void opcode_8(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    u16 y = opcode_y(opcode);
    u8 n = opcode_n(opcode);
    assert(x >= 0 && x < 16);
    assert(y >= 0 && y < 16);

    if (n == 0) {
        cpu.V[x] = cpu.V[y];
    } else if (n == 1) {
        cpu.V[x] |= cpu.V[y];
    } else if (n == 2) {
        cpu.V[x] &= cpu.V[y];
    } else if (n == 3) {
        cpu.V[x] ^= cpu.V[y];
    } else if (n == 4) {
        u16 value = cpu.V[x] + cpu.V[y];
        cpu.V[15] = (value > 255) ? 1 : 0;
        cpu.V[x] = opcode_kk(value);
        cpu.flag = Flags::CARRY;
    } else if (n == 5) {
        cpu.V[15] = cpu.V[x] > cpu.V[y] ? 1 : 0;
        cpu.V[x] -= cpu.V[y];
        cpu.flag = Flags::NOT_BORROW;
    } else if (n == 6) {
        cpu.V[15] = cpu.V[x] & 0x01;
        cpu.V[x] >>= 1;
    } else if (n == 7) {
        cpu.V[15] = cpu.V[y] > cpu.V[x] ? 1 : 0;
        cpu.V[x] = cpu.V[y] - cpu.V[x];
        cpu.flag = Flags::NOT_BORROW;
    } else if (n == 14) {
        cpu.V[15] = (cpu.V[x] & 0x80) >> 7;
        cpu.V[x] <<= 1;
    } else {
        unvalid_instr();
    }

    cpu.PC += 2;
}

void opcode_9(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    u16 y = opcode_y(opcode);
    assert(x >= 0 && x < 16);
    assert(y >= 0 && y < 16);
    u8 n = opcode_n(opcode);
    if (n != 0) {
        unvalid_instr();
        return;
    }
    cpu.PC += (cpu.V[x] != cpu.V[y]) ? 4 : 2;
}

void opcode_A(Cpu& cpu, u16 opcode) {
    u16 addr = opcode_addr(opcode);
    cpu.I = addr;
    cpu.PC += 2;
}

void opcode_B(Cpu& cpu, u16 opcode) {
    u16 addr = opcode_addr(opcode);
    cpu.PC = addr + cpu.V[0];
}

void opcode_C(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    assert(x >= 0 && x < 16);
    u8 kk = opcode_kk(opcode);
    std::random_device rd;
    std::default_random_engine dre(rd());
    std::uniform_int_distribution<u8> dist(0, 255);
    u8 value = dist(dre);
    cpu.V[x] = value & kk;
    cpu.PC += 2;
}

void opcode_D(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    assert(x >= 0 && x < 16);
    u16 y = opcode_y(opcode);
    assert(y >= 0 && y < 16);
    u8 n = opcode_n(opcode);

    cpu.V[15] = 0;
    for (u8 yline = 0; yline < n; yline++) {
        u8 pixel = cpu.memory[cpu.I + yline];
        for (u8 xline = 0; xline < 8; xline++) {
            if ((pixel & (0x80 >> xline)) != 0) {
                int xx = cpu.V[x] + xline;
                int yy = cpu.V[y] + yline;
                bool p = cpu.display.get(xx, yy);
                if (p) {
                    cpu.V[15] = 1;
                }
                cpu.display.set(xx, yy, p ^ 1);
            }
        }
    }

    cpu.flag = Flags::COLLISION;
    cpu.PC += 2;
}

void opcode_E(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    assert(x >= 0 && x < 16);
    u16 type = opcode_kk(opcode);
    switch (type) {
        case 0x009E:
            cpu.PC += cpu.keys[cpu.V[x]] ? 2 : 0;
            break;

        case 0x00A1:
            cpu.PC += cpu.keys[cpu.V[x]] ? 0 : 2;
            break;

        default: 
            unvalid_instr(); 
            break;
    }
    cpu.PC += 2;
}

void opcode_F(Cpu& cpu, u16 opcode) {
    u16 x = opcode_x(opcode);
    assert(x >= 0 && x < 16);
    u16 type = opcode_kk(opcode);
    switch (type) {
        case 0x0007:
            cpu.V[x] = cpu.DT;
            cpu.PC += 2;
            break;

        case 0x000A:
            for (u8 i = 0; i < 16; i++) {
                if (cpu.keys[i]) {
                    cpu.keys[i] = false;
                    cpu.V[x] = i;
                    cpu.PC += 2;
                }
            }
            break;

        case 0x0015:
            cpu.DT = cpu.V[x];
            cpu.PC += 2;
            break;

        case 0x0018:
            cpu.ST = cpu.V[x];
            cpu.PC += 2;
            break;

        case 0x001E:
            cpu.I += cpu.V[x];
            cpu.PC += 2;
            break;

        case 0x0029:
            // TODO: sprite location
            cpu.PC += 2;
            break;

        case 0x0033:
            cpu.memory[cpu.I] = cpu.V[x] / 100;
            cpu.memory[cpu.I + 1] = (cpu.V[x] % 100) / 10;
            cpu.memory[cpu.I + 2] = cpu.V[x] % 10;
            cpu.PC += 2;
            break;

        case 0x0055:
            for (u16 i = 0; i <= x; i++) {
                cpu.memory[cpu.I + i] = cpu.V[i];
            }
            cpu.PC += 2;
            break;

        case 0x0065:
            for (u16 i = 0; i <= x; i++) {
                cpu.V[i] = cpu.memory[cpu.I + i];
            }
            cpu.PC += 2;
            break;

        default:
            unvalid_instr();
            break;
    }
}

void (*Cpu::instructions[16])(Cpu&, u16) = {
    opcode_0, opcode_1, opcode_2, opcode_3, opcode_4, opcode_5, opcode_6,
    opcode_7, opcode_8, opcode_9, opcode_A, opcode_B, opcode_C, opcode_D,
    opcode_E, opcode_F
};

Cpu::Cpu(Display& display) : display(display) {
    reset();
}

void Cpu::reset() {
    std::memset(memory, 0, sizeof(memory));
    std::memset(V, 0, sizeof(V));
    I = 0x0000;
    DT = 0x00;
    ST = 0x00;
    PC = 0x0200;
    SP = 0x00;
    std::memset(stack, 0, sizeof(stack));
    flag = Flags::NONE;
    std::memset(keys, false, sizeof(keys));
    time = SDL_GetTicks();
}

void Cpu::cycle() {
    Uint32 current_time = SDL_GetTicks();
    Uint32 elapsed_time = current_time - time;
    if (elapsed_time >= CYCLE_MS) {
        if (DT > 0) {
            DT--;
        }

        u16 opcode = get_opcode();
        u16 type = opcode_type(opcode);
        (*instructions[type])(*this, opcode);
        time = current_time;
    }
}

u16 Cpu::get_opcode() {
    u16 m1 = static_cast<u16>(memory[PC] << 8);
    u16 m2 = static_cast<u16>(memory[PC + 1]);
    return m1 | m2;
}

void Cpu::open(const std::string& filename) {
    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open()) {
        std::cerr << "cannot open file: " << filename << std::endl;
        std::exit(EXIT_FAILURE);
    }

    file.seekg(0, file.end);
    std::streampos size = file.tellg();
    file.seekg(0, file.beg);
    file.read((char*)&memory[PC], size);
    file.close();
}

void Cpu::set_keydown(int key) {
    if (key >= 0) {
        keys[key] = true;
    }
}

void Cpu::set_keyup(int key) {
    if (key >= 0) {
        keys[key] = false;
    }
}

int map_keys(int scancode) {
    switch (scancode) {
        case SDL_SCANCODE_0: return 0;
        case SDL_SCANCODE_1: return 1;
        case SDL_SCANCODE_2: return 2;
        case SDL_SCANCODE_3: return 3;
        case SDL_SCANCODE_4: return 4;
        case SDL_SCANCODE_5: return 5;
        case SDL_SCANCODE_6: return 6;
        case SDL_SCANCODE_7: return 7;
        case SDL_SCANCODE_8: return 8;
        case SDL_SCANCODE_9: return 9;
        case SDL_SCANCODE_A: return 10;
        case SDL_SCANCODE_B: return 11;
        case SDL_SCANCODE_C: return 12;
        case SDL_SCANCODE_D: return 13;
        case SDL_SCANCODE_E: return 14; 
        case SDL_SCANCODE_F: return 15;
        default: return -1;
    };
}

//=================================[ MA_IN ]=================================//
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " program" << std::endl;
        return EXIT_FAILURE;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't initialize SDL: %s", 
            SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Chip8", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED,
        1280, 900, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't create SDL window: %s", 
            SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't create SDL renderer: %s", 
            SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    Display display(renderer);
    Cpu cpu(display);
    cpu.open(argv[1]);

    SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
            break;
        else if (event.type == SDL_KEYDOWN) {
            cpu.set_keydown(map_keys(event.key.keysym.scancode));
        } else if (event.type == SDL_KEYUP) {
            cpu.set_keyup(map_keys(event.key.keysym.scancode));
        }

        cpu.cycle();

        SDL_RenderClear(renderer);
        display.render();
        SDL_RenderPresent(renderer);
    }

    display.dispose();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}