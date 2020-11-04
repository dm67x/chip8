#include "chip8.hpp"
#include <cstring>

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
void opcode_0(Cpu& cpu) {
    u16 opcode = cpu.get_opcode();
    u16 who = opcode_kk(opcode);
    if (who == 0x00E0) {
        cpu.display.clear();
    } else if (who == 0x00EE) {
        // ret
    }
    cpu.PC += 2;
}

void opcode_1(Cpu& cpu) {

}

void opcode_2(Cpu& cpu) {

}

void opcode_3(Cpu& cpu) {

}

void opcode_4(Cpu& cpu) {

}

void opcode_5(Cpu& cpu) {

}

void opcode_6(Cpu& cpu) {

}

void opcode_7(Cpu& cpu) {

}

void opcode_8(Cpu& cpu) {

}

void opcode_9(Cpu& cpu) {

}

void opcode_A(Cpu& cpu) {

}

void opcode_B(Cpu& cpu) {

}

void opcode_C(Cpu& cpu) {

}

void opcode_D(Cpu& cpu) {

}

void opcode_E(Cpu& cpu) {

}

void opcode_F(Cpu& cpu) {

}

void (*Cpu::instructions[16])(Cpu&) = {
    opcode_0, opcode_1, opcode_2, opcode_3, opcode_4, opcode_5, opcode_6,
    opcode_7, opcode_8, opcode_9, opcode_A, opcode_B, opcode_C, opcode_D,
    opcode_E, opcode_F
};

Cpu::Cpu(Display& diplay) : display(display) {
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
}

void Cpu::cycle() {
    u16 opcode = get_opcode();
    u16 type = opcode_type(opcode);
    (*instructions[type])(*this);
}

u16 Cpu::get_opcode() {
    u16 m1 = static_cast<u16>(memory[PC] << 4);
    u16 m2 = static_cast<u16>(memory[PC + 1]);
    return m1 | m2;
}

//=================================[ MA_IN ]=================================//
int main(int argc, char** argv) {
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

    SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
            break;

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