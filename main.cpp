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

    // Create character with medium radius (30 pixels = 60x60 total size)
    Character player(graphics.renderer, 300, 100, 30, 10);  // x, y, radius=30, particles=10

    bool running = true;
    SDL_Event event;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_A:  // Left hand grab
                        player.grab(true, graphics.platforms);
                        break;
                    case SDL_SCANCODE_D:  // Right hand grab
                        player.grab(false, graphics.platforms);
                        break;
                }
            }
            else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_A:  // Release left hand
                        player.release(true);
                        break;
                    case SDL_SCANCODE_D:  // Release right hand
                        player.release(false);
                        break;
                }
            }
        }

        // Update character (includes swing forces based on left/right arrow keys)
        player.update();
        
        // Handle collisions with platforms
        player.handlecollision(graphics.platforms);

        // Clear screen
        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
        SDL_RenderClear(graphics.renderer);

        // Render platforms first
        graphics.renderPlatforms();
        
        // Render character on top
        player.render(graphics.renderer);

        // Present the frame
        SDL_RenderPresent(graphics.renderer);

        // Small delay to control frame rate
        SDL_Delay(16);  // Approximately 60 FPS
    }

    SDL_DestroyRenderer(graphics.renderer);
    SDL_DestroyWindow(graphics.window);
    SDL_Quit();
    return 0;
}
