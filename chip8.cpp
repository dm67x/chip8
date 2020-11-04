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

    for (int i = 0; i < 2048; i++) {
        data[i] = false;
    }
}

Display::~Display() {
    SDL_DestroyTexture(screen);
}

void Display::set(int x, int y, bool value) {
    data[y * 64 + x] = value;
}

bool Display::get(int x, int y) const {
    return data[y * 64 + x];
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
Cpu::Cpu() {
    reset();
}

void Cpu::reset() {
    u8 memory[4096];
    u8 V[16];
    u16 I;
    u8 DT;
    u8 ST;
    u16 PC;
    u8 SP;
    u16 stack[16];

    std::memset(memory, 0x00, 4096);
    std::memset(V, 0x00, 16);
    I = 0x0000;
    DT = 0x00;
    ST = 0x00;
    PC = 0x0200;
    SP = 0x00;
    std::memset(stack, 0x0000, 16);
}

void Cpu::cycle(Display& display) {

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

    Display* display = new Display(renderer);
    Cpu cpu;

    SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
            break;

        cpu.cycle(*display);

        SDL_RenderClear(renderer);
        display->render();
        SDL_RenderPresent(renderer);
    }

    delete display;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}