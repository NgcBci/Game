#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "defs.h"
#include "graphics.h"
#include "menupanel.h"

using namespace std;

enum GameState {
    MENU,
    PLAYING,
    OPTIONS,
    HOWTOPLAY
};

int SDL_main(int argc, char* argv[]) {
    Graphics graphics;
    graphics.init();

    // Create character with medium radius (30 pixels = 60x60 total size)
    Character player(graphics.renderer, 300, 100, 30, 10);  // x, y, radius=30, particles=10

    // Create menu panel
    MenuPanel menu(graphics.renderer, 0, 0, 0, 0);  // Position and size are handled internally

    // Add menu items
    menu.addItem("Start Game", []() {
        // This will be handled in the main loop
    });

    menu.addItem("Options", []() {
        // This will be handled in the main loop
    });

    menu.addItem("How to Play", []() {
        // This will be handled in the main loop
    });

    menu.addItem("Quit Game", []() {
        // This will be handled in the main loop
    });

    bool running = true;
    GameState currentState = MENU;
    SDL_Event event;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            
            if (currentState == MENU) {
                menu.handleEvent(event);
                
                if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                    switch (menu.getSelectedIndex()) {
                        case 0:  // Start Game
                            currentState = PLAYING;
                            break;
                        case 1:  // Options
                            currentState = OPTIONS;
                            break;
                        case 2:  // How to Play
                            currentState = HOWTOPLAY;
                            break;
                        case 3:  // Quit Game
                            running = false;
                            break;
                    }
                }
            }
            else if (currentState == PLAYING) {
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_A:  // Left hand grab
                            player.grab(true, graphics.platforms);
                            break;
                        case SDL_SCANCODE_D:  // Right hand grab
                            player.grab(false, graphics.platforms);
                            break;
                        case SDL_SCANCODE_ESCAPE:  // Return to menu
                            currentState = MENU;
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
            else if (currentState == OPTIONS || currentState == HOWTOPLAY) {
                if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    currentState = MENU;
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(graphics.renderer, 245, 245, 220, 255);
        SDL_RenderClear(graphics.renderer);

        if (currentState == MENU) {
            // Render menu
            menu.render();
        }
        else if (currentState == PLAYING) {
            // Update and render game
            player.update();
            player.handlecollision(graphics.platforms);
            graphics.renderPlatforms();
            player.render(graphics.renderer);
        }
        else if (currentState == OPTIONS) {
            // TODO: Render options screen
            SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
            SDL_Rect optionsRect = {SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 100, 400, 200};
            SDL_RenderFillRect(graphics.renderer, &optionsRect);
        }
        else if (currentState == HOWTOPLAY) {
            // TODO: Render how to play screen
            SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
            SDL_Rect howToPlayRect = {SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 100, 400, 200};
            SDL_RenderFillRect(graphics.renderer, &howToPlayRect);
        }

        // Present the frame
        SDL_RenderPresent(graphics.renderer);

        // Small delay to control frame rate
        SDL_Delay(16);  // Approximately 60 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(graphics.renderer);
    SDL_DestroyWindow(graphics.window);
    SDL_Quit();
    return 0;
}
