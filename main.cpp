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
                if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (event.key.keysym.sym == SDLK_LEFT) player.vx = -200;
                if (event.key.keysym.sym == SDLK_RIGHT) player.vx = 200;
                if (event.key.keysym.sym == SDLK_a)
                    player.grab(true, graphics.objects);  // Use graphics.objects
                if (event.key.keysym.sym == SDLK_d)
                    player.grab(false, graphics.objects); // Use graphics.objects
            }

            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_LEFT) if (player.vx < 0) player.vx = 0;
                if (event.key.keysym.sym == SDLK_RIGHT) if (player.vx > 0) player.vx = 0;
                if (event.key.keysym.sym == SDLK_a)
                    player.release(true);
                if (event.key.keysym.sym == SDLK_d)
                    player.release(false);
            }
        }

        // Update all objects
        for (auto& obj : graphics.objects) {
            obj.update(0.016);  // 60 FPS = 0.016 seconds per frame
        }

        // Update player
        player.update();
        player.handlecollision(graphics.objects);

        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
        SDL_RenderClear(graphics.renderer);

        // Render objects first
        for (const auto& obj : graphics.objects) {
            obj.render(graphics.renderer);
        }

        player.render(graphics.renderer);
        SDL_RenderPresent(graphics.renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(graphics.renderer);
    SDL_DestroyWindow(graphics.window);
    SDL_Quit();
    return 0;
}
