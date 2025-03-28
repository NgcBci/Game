#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "defs.h"
#include "graphics.h"

using namespace std;

int SDL_main(int argc, char* argv[]) {
    Graphics graphics;
    graphics.init();

    // Fix Character constructor call to use graphics.renderer
    Character player(graphics.renderer, 300, 300, 100, 10);

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_a)
                    player.grab(true, graphics.platforms);
                if (event.key.keysym.sym == SDLK_k)
                    player.grab(false, graphics.platforms);
            }

            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_a)
                    player.release(true);
                if (event.key.keysym.sym == SDLK_k)
                    player.release(false);
            }
        }

        player.update();

        // Clear the renderer
        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
        SDL_RenderClear(graphics.renderer);

        // Render platforms first
        graphics.renderPlatforms();

        // Then render the player (which includes the character and hands)
        player.render(graphics.renderer);

        // Present the frame
        SDL_RenderPresent(graphics.renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(graphics.renderer);
    SDL_DestroyWindow(graphics.window);
    SDL_Quit();
    return 0;
}
