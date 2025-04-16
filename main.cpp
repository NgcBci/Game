#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "defs.h"
#include "graphics.h"
#include "menupanel.h"
#include "music.h"

using namespace std;

enum GameState {
    MENU,
    PLAYING,
    OPTIONS,
    HOWTOPLAY
};

int SDL_main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return 1;
    }

    Graphics graphics;
    graphics.init();

    // Create character with medium radius (30 pixels = 60x60 total size)
    Character player(graphics.renderer, 300, 100, 30, 10);  // x, y, radius=30, particles=10

    // Create menu panel
    MenuPanel menu(graphics.renderer, 0, 0, 0, 0);  // Position and size are handled internally

    // Add menu items
    menu.addItem("F:\\Game\\startbut.png", []() {
        // This will be handled in the main loop
    });

    menu.addItem("F:\\Game\\optionsbut.png", []() {
        // This will be handled in the main loop
    });

    menu.addItem("F:\\Game\\waybut.png", []() {
        // This will be handled in the main loop
    });

    menu.addItem("F:\\Game\\quitbut.png", []() {
        // This will be handled in the main loop
    });

    // Initialize music
    Music backgroundMusic;
    backgroundMusic.loadMusic("F:\\Game\\sounds\\bgmusic.mp3");
    backgroundMusic.loadSound("F:\\Game\\sounds\\grabbing.mp3");
    backgroundMusic.loadFallSound("F:\\Game\\sounds\\huhu.mp3");
    backgroundMusic.loadApplauseSound("F:\\Game\\sounds\\applause.mp3");
    backgroundMusic.play();

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

                // Handle both keyboard and mouse events for menu state changes
                if ((event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_RETURN) ||
                    (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)) {
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
                            if (!event.key.repeat) {  // Only play sound on initial press, not repeat
                                backgroundMusic.playGrabSound();
                            }
                            break;
                        case SDL_SCANCODE_D:  // Right hand grab
                            player.grab(false, graphics.platforms);
                            if (!event.key.repeat) {  // Only play sound on initial press, not repeat
                                backgroundMusic.playGrabSound();
                            }
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

            // Play falling sound when character is falling freely
            bool isFalling = !player.leftHand.isGrabbingObject && !player.rightHand.isGrabbingObject && player.vy > 0;
            backgroundMusic.playFallSound(isFalling);

            // If character has reached finish line, render congratulations screen and play applause
            if (player.showingCongratulations) {
                graphics.renderCongratulations();
                backgroundMusic.playApplauseSound();
            }
        }
        else if (currentState == OPTIONS) {
            // TODO: Render options screen
            SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
            SDL_Rect optionsRect = {SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 100, 400, 200};
            SDL_RenderFillRect(graphics.renderer, &optionsRect);
        }
        else if (currentState == HOWTOPLAY) {
            // Render how to play screen with guide image
            graphics.renderHowToPlay();
        }

        // Present the frame
        SDL_RenderPresent(graphics.renderer);

        // Small delay to control frame rate
        SDL_Delay(16);  // Approximately 60 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(graphics.renderer);
    SDL_DestroyWindow(graphics.window);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}
