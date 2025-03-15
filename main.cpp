#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "defs.h"
#include "graphics.h"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600


using namespace std;



int main(int arg, char* argv[]) {
    Graphics graphics;
    graphics.init();
    ropehand rope(400, 400, 100, 300, 10);
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEMOTION) {
                rope.followthemouse(event.motion.x, event.motion.y);
            }
        }
        rope.step();
        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
        SDL_RenderClear(graphics.renderer);
        rope.render(graphics.renderer);
        SDL_RenderPresent(graphics.renderer);
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(graphics.renderer);
    SDL_DestroyWindow(graphics.window);
    SDL_Quit();
    return 0;
}
