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
#include "level_platforms.h"

using namespace std;

enum GameState {
    MENU,
    CHARACTER_SELECTION,
    LEVEL_SELECTION,
    PLAYING,
    OPTIONS,
    HOWTOPLAY
};

int currentCharacterIndex = 0;  // 0: red, 1: mint, 2: black, 3: sabrina
const char* characterPaths[] = {
    "F:\\Game\\graphic\\red-Photoroom.png",
    "F:\\Game\\graphic\\mint (2)-Photoroom.png",
    "F:\\Game\\graphic\\againblac-Photoroom.png",
    "F:\\Game\\graphic\\allhailsabrina-Photoroom.png"
};
SDL_Rect leftArrowRect, rightArrowRect, characterRect;  // Add characterRect to global variables

// Add these global variables after the character paths
const char* levelPaths[] = {
    "F:\\Game\\graphic\\lv1-Photoroom (1).png",
    "F:\\Game\\graphic\\lv2-Photoroom (1).png",
    "F:\\Game\\graphic\\lv3-Photoroom (1).png",
    "F:\\Game\\graphic\\lv4-Photoroom (1).png",
    "F:\\Game\\graphic\\lv5-Photoroom (1).png"
};
SDL_Rect levelRects[5];  // Array to store level button rectangles

// Add these global variables after the character paths
bool characterUnlocked[] = {true, false, false, false};  // Only red character is unlocked initially
SDL_Texture* lockTexture = nullptr;  // Will store the lock image texture

// Add these global variables after the level paths
bool levelUnlocked[] = {true, false, false, false, false};  // Level 1 unlocked, others locked
SDL_Texture* levelLockTexture = nullptr;  // Will store the level lock image texture
int selectedLevel = 1;  // Currently selected level, starts at 1

// Add these global variables after the other global variables
double cameraOffsetX = 0;
int currentScreenIndex = 0;  // Which screen the player is on (0-based)
const int SCREEN_COUNT = 3;  // Total number of screens in level 2
const int SCREEN_TRANSITION_X = SCREEN_WIDTH - 200;  // Trigger transition when near right edge
Uint32 lastTransitionTime = 0;  // Time of last screen transition
const Uint32 TRANSITION_COOLDOWN = 500;  // Minimum time between transitions (milliseconds)
double playerWorldX = 300;  // Track player's actual world position

int SDL_main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return 1;
    }

    Graphics core;
    core.init();

    // Create character with medium radius (30 pixels = 60x60 total size)
    Character player(core.renderer, 300, 100, 30, 10);  // x, y, radius=30, particles=10

    // Create menu panel
    MenuPanel menu(core.renderer, 0, 0, 0, 0);  // Position and size are handled internally

    // Add menu items
    menu.addItem("F:\\Game\\graphic\\startbut.png", []() {
        // Start game logic
    });

    menu.addItem("F:\\Game\\graphic\\optionsbut.png", []() {
        // Options logic
    });

    menu.addItem("F:\\Game\\graphic\\waybut.png", []() {
        // How to play logic
    });

    menu.addItem("F:\\Game\\graphic\\quitbut.png", []() {
        // Quit game logic
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
    
    // Add delta time tracking for consistent physics
    Uint32 previousTicks = SDL_GetTicks();
    const double dt = 1.0 / 60.0;  // Fixed time step (60 FPS)

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
                            currentState = CHARACTER_SELECTION;
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
            else if (currentState == CHARACTER_SELECTION) {
            if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        currentState = MENU;
                    }
                }
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;
                    
                    // Check if left arrow was clicked
                    if (leftArrowRect.x <= mouseX && mouseX <= leftArrowRect.x + leftArrowRect.w &&
                        leftArrowRect.y <= mouseY && mouseY <= leftArrowRect.y + leftArrowRect.h) {
                        // Move to previous character
                        currentCharacterIndex = (currentCharacterIndex - 1 + 4) % 4;
                    }
                    // Check if right arrow was clicked
                    else if (rightArrowRect.x <= mouseX && mouseX <= rightArrowRect.x + rightArrowRect.w &&
                             rightArrowRect.y <= mouseY && mouseY <= rightArrowRect.y + rightArrowRect.h) {
                        // Move to next character
                        currentCharacterIndex = (currentCharacterIndex + 1) % 4;
                    }
                    // Check if character was clicked and is unlocked
                    else if (characterRect.x <= mouseX && mouseX <= characterRect.x + characterRect.w &&
                             characterRect.y <= mouseY && mouseY <= characterRect.y + characterRect.h &&
                             characterUnlocked[currentCharacterIndex]) {
                        // Move to level selection screen
                        currentState = LEVEL_SELECTION;
                    }
                }

                // Load lock texture if not already loaded
                if (!lockTexture) {
                    lockTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\lock-removebg-preview.png");
                }

                // Render character selection screen
                SDL_Texture* charSelectTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\chooseyourchar-Photoroom.png");
                if (charSelectTexture) {
                    int texWidth, texHeight;
                    SDL_QueryTexture(charSelectTexture, NULL, NULL, &texWidth, &texHeight);
                    
                    // Scale up the dimensions
                    int scaledWidth = texWidth * 1.5;  // 150% of original width
                    int scaledHeight = texHeight * 1.5;  // 150% of original height
                    
                    // Position at middle top of screen
                    SDL_Rect destRect = {
                        (SCREEN_WIDTH - scaledWidth) / 2,  // Center horizontally
                        0,  // At the very top
                        scaledWidth,
                        scaledHeight
                    };
                    
                    SDL_RenderCopy(core.renderer, charSelectTexture, NULL, &destRect);
                    SDL_DestroyTexture(charSelectTexture);
                }

                // Render current character in the middle of the screen
                SDL_Texture* characterTexture = IMG_LoadTexture(core.renderer, characterPaths[currentCharacterIndex]);
                if (characterTexture) {
                    int texWidth, texHeight;
                    SDL_QueryTexture(characterTexture, NULL, NULL, &texWidth, &texHeight);
                    
                    // Scale down the dimensions
                    int scaledWidth = texWidth * 0.7;  // 70% of original width
                    int scaledHeight = texHeight * 0.7;  // 70% of original height
                    
                    // Position at center of screen
                    characterRect = {
                        (SCREEN_WIDTH - scaledWidth) / 2,  // Center horizontally
                        (SCREEN_HEIGHT - scaledHeight) / 2,  // Center vertically
                        scaledWidth,
                        scaledHeight
                    };
                    
                    SDL_RenderCopy(core.renderer, characterTexture, NULL, &characterRect);
                    SDL_DestroyTexture(characterTexture);

                    // Render lock if character is locked
                    if (!characterUnlocked[currentCharacterIndex] && lockTexture) {
                        int lockWidth = scaledWidth * 0.5;  // Lock size relative to character
                        int lockHeight = lockWidth;  // Keep aspect ratio
                        SDL_Rect lockRect = {
                            characterRect.x + (characterRect.w - lockWidth) / 2,
                            characterRect.y + (characterRect.h - lockHeight) / 2,
                            lockWidth,
                            lockHeight
                        };
                        SDL_RenderCopy(core.renderer, lockTexture, NULL, &lockRect);
                    }
                }

                // Render left arrow
                SDL_Texture* leftArrowTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\chontrai-Photoroom.png");
                if (leftArrowTexture) {
                    int arrowWidth, arrowHeight;
                    SDL_QueryTexture(leftArrowTexture, NULL, NULL, &arrowWidth, &arrowHeight);
                    
                    // Scale down the arrow dimensions
                    int scaledArrowWidth = arrowWidth * 0.5;  // 50% of original width
                    int scaledArrowHeight = arrowHeight * 0.5;  // 50% of original height
                    
                    // Position left arrow to the left of the character
                    leftArrowRect = {
                        characterRect.x - scaledArrowWidth - 20,  // 20 pixels gap from character
                        (SCREEN_HEIGHT - scaledArrowHeight) / 2,  // Center vertically
                        scaledArrowWidth,
                        scaledArrowHeight
                    };
                    
                    SDL_RenderCopy(core.renderer, leftArrowTexture, NULL, &leftArrowRect);
                    SDL_DestroyTexture(leftArrowTexture);
                }

                // Render right arrow
                SDL_Texture* rightArrowTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\chonphai-Photoroom.png");
                if (rightArrowTexture) {
                    int arrowWidth, arrowHeight;
                    SDL_QueryTexture(rightArrowTexture, NULL, NULL, &arrowWidth, &arrowHeight);
                    
                    // Scale down the arrow dimensions
                    int scaledArrowWidth = arrowWidth * 0.5;  // 50% of original width
                    int scaledArrowHeight = arrowHeight * 0.5;  // 50% of original height
                    
                    // Position right arrow to the right of the character
                    rightArrowRect = {
                        characterRect.x + characterRect.w + 20,  // 20 pixels gap from character
                        (SCREEN_HEIGHT - scaledArrowHeight) / 2,  // Center vertically
                        scaledArrowWidth,
                        scaledArrowHeight
                    };
                    
                    SDL_RenderCopy(core.renderer, rightArrowTexture, NULL, &rightArrowRect);
                    SDL_DestroyTexture(rightArrowTexture);
                }
            }
            else if (currentState == LEVEL_SELECTION) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        currentState = CHARACTER_SELECTION;
                    }
                }
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    
                    // Check if any level was clicked
                    for (int i = 0; i < 5; i++) {
                        // Only proceed if the level is unlocked
                        if (levelUnlocked[i]) {
                            if (mouseX >= levelRects[i].x && mouseX <= levelRects[i].x + levelRects[i].w &&
                                mouseY >= levelRects[i].y && mouseY <= levelRects[i].y + levelRects[i].h) {
                                selectedLevel = i + 1;
                                // Update platforms for the selected level
                                core.platforms = LevelPlatforms::getPlatformsForLevel(selectedLevel, core.renderer);
                                // Reset player position for the new level
                                player.resetPosition();
                                currentState = PLAYING;
                                break;
                            }
                        }
                    }
                }
            }
            else if (currentState == PLAYING) {
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_A:  // Left hand grab
                            if (selectedLevel == 2) {
                                // Use the camera-aware grab method for Level 2
                                player.grabWithCamera(true, core.platforms, cameraOffsetX);
                            } else {
                                player.grab(true, core.platforms);
                            }
                            if (!event.key.repeat) {  // Only play sound on initial press, not repeat
                                backgroundMusic.playGrabSound();
                            }
                            break;
                        case SDL_SCANCODE_D:  // Right hand grab
                            if (selectedLevel == 2) {
                                // Use the camera-aware grab method for Level 2
                                player.grabWithCamera(false, core.platforms, cameraOffsetX);
                            } else {
                                player.grab(false, core.platforms);
                            }
                            if (!event.key.repeat) {  // Only play sound on initial press, not repeat
                                backgroundMusic.playGrabSound();
                            }
                            break;
                        case SDL_SCANCODE_ESCAPE:  // Return to menu
                            currentState = MENU;
                            break;
                        case SDL_SCANCODE_C:  // Return to level selection after completing level
                            if (player.showingCongratulations) {
                                currentState = LEVEL_SELECTION;
                                player.showingCongratulations = false;  // Reset the congratulations flag
                            }
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
        SDL_SetRenderDrawColor(core.renderer, 245, 245, 220, 255);  // Beige color
        SDL_RenderClear(core.renderer);

        if (currentState == MENU) {
            // Render menu
            menu.render();
        }
        else if (currentState == CHARACTER_SELECTION) {
            // Render character selection screen
            SDL_Texture* charSelectTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\chooseyourchar-Photoroom.png");
            if (charSelectTexture) {
                int texWidth, texHeight;
                SDL_QueryTexture(charSelectTexture, NULL, NULL, &texWidth, &texHeight);
                
                // Scale up the dimensions
                int scaledWidth = texWidth * 1.5;  // 150% of original width
                int scaledHeight = texHeight * 1.5;  // 150% of original height
                
                // Position at middle top of screen
                SDL_Rect destRect = {
                    (SCREEN_WIDTH - scaledWidth) / 2,  // Center horizontally
                    0,  // At the very top
                    scaledWidth,
                    scaledHeight
                };
                
                SDL_RenderCopy(core.renderer, charSelectTexture, NULL, &destRect);
                SDL_DestroyTexture(charSelectTexture);
            }

            // Render current character in the middle of the screen
            SDL_Texture* characterTexture = IMG_LoadTexture(core.renderer, characterPaths[currentCharacterIndex]);
            if (characterTexture) {
                int texWidth, texHeight;
                SDL_QueryTexture(characterTexture, NULL, NULL, &texWidth, &texHeight);
                
                // Scale down the dimensions
                int scaledWidth = texWidth * 0.7;  // 70% of original width
                int scaledHeight = texHeight * 0.7;  // 70% of original height
                
                // Position at center of screen
                characterRect = {
                    (SCREEN_WIDTH - scaledWidth) / 2,  // Center horizontally
                    (SCREEN_HEIGHT - scaledHeight) / 2,  // Center vertically
                    scaledWidth,
                    scaledHeight
                };
                
                SDL_RenderCopy(core.renderer, characterTexture, NULL, &characterRect);
                SDL_DestroyTexture(characterTexture);

                // Render lock if character is locked
                if (!characterUnlocked[currentCharacterIndex] && lockTexture) {
                    int lockWidth = scaledWidth * 0.5;  // Lock size relative to character
                    int lockHeight = lockWidth;  // Keep aspect ratio
                    SDL_Rect lockRect = {
                        characterRect.x + (characterRect.w - lockWidth) / 2,
                        characterRect.y + (characterRect.h - lockHeight) / 2,
                        lockWidth,
                        lockHeight
                    };
                    SDL_RenderCopy(core.renderer, lockTexture, NULL, &lockRect);
                }
            }

            // Render left arrow
            SDL_Texture* leftArrowTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\chontrai-Photoroom.png");
            if (leftArrowTexture) {
                int arrowWidth, arrowHeight;
                SDL_QueryTexture(leftArrowTexture, NULL, NULL, &arrowWidth, &arrowHeight);
                
                // Scale down the arrow dimensions
                int scaledArrowWidth = arrowWidth * 0.5;  // 50% of original width
                int scaledArrowHeight = arrowHeight * 0.5;  // 50% of original height
                
                // Position left arrow to the left of the character
                leftArrowRect = {
                    characterRect.x - scaledArrowWidth - 20,  // 20 pixels gap from character
                    (SCREEN_HEIGHT - scaledArrowHeight) / 2,  // Center vertically
                    scaledArrowWidth,
                    scaledArrowHeight
                };
                
                SDL_RenderCopy(core.renderer, leftArrowTexture, NULL, &leftArrowRect);
                SDL_DestroyTexture(leftArrowTexture);
            }

            // Render right arrow
            SDL_Texture* rightArrowTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\chonphai-Photoroom.png");
            if (rightArrowTexture) {
                int arrowWidth, arrowHeight;
                SDL_QueryTexture(rightArrowTexture, NULL, NULL, &arrowWidth, &arrowHeight);
                
                // Scale down the arrow dimensions
                int scaledArrowWidth = arrowWidth * 0.5;  // 50% of original width
                int scaledArrowHeight = arrowHeight * 0.5;  // 50% of original height
                
                // Position right arrow to the right of the character
                rightArrowRect = {
                    characterRect.x + characterRect.w + 20,  // 20 pixels gap from character
                    (SCREEN_HEIGHT - scaledArrowHeight) / 2,  // Center vertically
                    scaledArrowWidth,
                    scaledArrowHeight
                };
                
                SDL_RenderCopy(core.renderer, rightArrowTexture, NULL, &rightArrowRect);
                SDL_DestroyTexture(rightArrowTexture);
            }
        }
        else if (currentState == LEVEL_SELECTION) {
            // Load level lock texture if not already loaded
            if (!levelLockTexture) {
                levelLockTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\lock-removebg-preview.png");
            }

            // Render level selection background
            SDL_Texture* levelSelectTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\levil-Photoroom.png");
            if (levelSelectTexture) {
                int texWidth, texHeight;
                SDL_QueryTexture(levelSelectTexture, NULL, NULL, &texWidth, &texHeight);
                
                // Scale up the dimensions
                int scaledWidth = texWidth * 1.5;  // 150% of original width
                int scaledHeight = texHeight * 1.5;  // 150% of original height
                
                // Position at middle top of screen
                SDL_Rect destRect = {
                    (SCREEN_WIDTH - scaledWidth) / 2,  // Center horizontally
                    0,  // At the very top
                    scaledWidth,
                    scaledHeight
                };
                
                SDL_RenderCopy(core.renderer, levelSelectTexture, NULL, &destRect);
                SDL_DestroyTexture(levelSelectTexture);
            }

            // Calculate positions for level buttons
            const int BUTTON_WIDTH = 250;
            const int BUTTON_HEIGHT = 250;
            const int HORIZONTAL_SPACING = 100;
            const int VERTICAL_SPACING = 100;
            
            // Calculate total height of both rows
            int totalHeight = (2 * BUTTON_HEIGHT) + VERTICAL_SPACING;
            
            // Center the entire grid vertically
            int firstRowY = (SCREEN_HEIGHT - totalHeight) / 2;
            
            // First row (3 levels)
            int firstRowStartX = (SCREEN_WIDTH - (3 * BUTTON_WIDTH + 2 * HORIZONTAL_SPACING)) / 2;
            
            // Second row (2 levels)
            int secondRowY = firstRowY + BUTTON_HEIGHT + VERTICAL_SPACING;
            int secondRowStartX = (SCREEN_WIDTH - (2 * BUTTON_WIDTH + HORIZONTAL_SPACING)) / 2;

            // Render level buttons
            for (int i = 0; i < 5; i++) {
                SDL_Texture* levelTexture = IMG_LoadTexture(core.renderer, levelPaths[i]);
                if (levelTexture) {
                    SDL_Rect levelRect;
                    
                    if (i < 3) {
                        // First row (3 levels)
                        levelRect = {
                            firstRowStartX + i * (BUTTON_WIDTH + HORIZONTAL_SPACING),
                            firstRowY,
                            BUTTON_WIDTH,
                            BUTTON_HEIGHT
                        };
                    } else {
                        // Second row (2 levels)
                        levelRect = {
                            secondRowStartX + (i - 3) * (BUTTON_WIDTH + HORIZONTAL_SPACING),
                            secondRowY,
                            BUTTON_WIDTH,
                            BUTTON_HEIGHT
                        };
                    }
                    
                    SDL_RenderCopy(core.renderer, levelTexture, NULL, &levelRect);
                    SDL_DestroyTexture(levelTexture);
                    
                    // Store level rectangle for click detection
                    levelRects[i] = levelRect;

                    // Render lock if level is locked
                    if (!levelUnlocked[i] && levelLockTexture) {
                        int lockWidth = BUTTON_WIDTH * 0.5;  // Lock size relative to level button
                        int lockHeight = lockWidth;  // Keep aspect ratio
                        SDL_Rect lockRect = {
                            levelRect.x + (levelRect.w - lockWidth) / 2,
                            levelRect.y + (levelRect.h - lockHeight) / 2,
                            lockWidth,
                            lockHeight
                        };
                        SDL_RenderCopy(core.renderer, levelLockTexture, NULL, &lockRect);
                    }
                }
            }

            // Handle level selection
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                
                // Check if any level was clicked
                for (int i = 0; i < 5; i++) {
                    // Only proceed if the level is unlocked
                    if (levelUnlocked[i]) {
                        if (mouseX >= levelRects[i].x && mouseX <= levelRects[i].x + levelRects[i].w &&
                            mouseY >= levelRects[i].y && mouseY <= levelRects[i].y + levelRects[i].h) {
                            selectedLevel = i + 1;
                            // Update platforms for the selected level
                            core.platforms = LevelPlatforms::getPlatformsForLevel(selectedLevel, core.renderer);
                            // Reset player position for the new level
                            player.resetPosition();
                            currentState = PLAYING;
                            break;
                        }
                    }
                }
            }

            // Handle ESC key to return to character selection
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                currentState = CHARACTER_SELECTION;
            }
        }
        else if (currentState == PLAYING) {
            // Update camera offset based on character position (only for Level 2)
            if (selectedLevel == 2) {
                // Get current time for transition cooldown
                Uint32 currentTime = SDL_GetTicks();
                
                // Check if we need to transition to the next screen
                if (player.x > SCREEN_TRANSITION_X && 
                    currentScreenIndex < SCREEN_COUNT - 1 && 
                    currentTime - lastTransitionTime >= TRANSITION_COOLDOWN) {
                    
                    // Calculate player's position relative to the transition point
                    double relativePos = player.x - SCREEN_TRANSITION_X;
                    
                    // Track grab states before transition
                    bool wasLeftGrabbing = player.leftHand.isGrabbingObject;
                    bool wasRightGrabbing = player.rightHand.isGrabbingObject;
                    
                    // Remember hand positions relative to character
                    double leftHandX = player.leftHand.parti.back().xCurrent - player.x;
                    double leftHandY = player.leftHand.parti.back().yCurrent - player.y;
                    double rightHandX = player.rightHand.parti.back().xCurrent - player.x;
                    double rightHandY = player.rightHand.parti.back().yCurrent - player.y;
                    
                    // Move to the next screen
                    currentScreenIndex++;
                    
                    // Update camera offset for new screen
                    cameraOffsetX = currentScreenIndex * SCREEN_WIDTH;
                    
                    // Position player at the beginning of the new screen
                    // But maintain the same relative position to the platform
                    double oldX = player.x;
                    player.x = 150 + relativePos;
                    
                    // Keep vertical position and velocity
                    
                    // Update hand positions to maintain their relative positions
                    for (size_t i = 0; i < player.leftHand.parti.size(); i++) {
                        // Adjust for new player position
                        double dx = player.x - oldX;
                        
                        player.leftHand.parti[i].xCurrent += dx;
                        player.leftHand.parti[i].xPrevious += dx;
                    }
                    
                    for (size_t i = 0; i < player.rightHand.parti.size(); i++) {
                        // Adjust for new player position
                        double dx = player.x - oldX;
                        
                        player.rightHand.parti[i].xCurrent += dx;
                        player.rightHand.parti[i].xPrevious += dx;
                    }
                    
                    // If hands were grabbing, ensure they remain grabbing at the right positions
                    if (wasLeftGrabbing) {
                        // Update the left hand's grab position on the duplicated platform
                        double newGrabX = player.x + leftHandX;
                        double newGrabY = player.y + leftHandY;
                        player.leftHand.grab(newGrabX, newGrabY);
                    }
                    
                    if (wasRightGrabbing) {
                        // Update the right hand's grab position on the duplicated platform
                        double newGrabX = player.x + rightHandX;
                        double newGrabY = player.y + rightHandY;
                        player.rightHand.grab(newGrabX, newGrabY);
                    }
                    
                    // Record time of this transition
                    lastTransitionTime = currentTime;
                }
                
                // Check if player fell off the screen
                if (player.y > SCREEN_HEIGHT + 100 || 
                    player.x < -100 || 
                    player.x > SCREEN_WIDTH + 100) {
                    // Reset to beginning of first screen
                    currentScreenIndex = 0;
                    cameraOffsetX = 0;
                    player.resetPosition();
                    
                    // Reset transition timer
                    lastTransitionTime = currentTime;
                }
            } else {
                cameraOffsetX = 0;  // No sliding for other levels
                currentScreenIndex = 0;  // Reset screen index for other levels
            }
            
            // Update player physics
        player.update();

            // Handle collisions with appropriate method based on level
            if (selectedLevel == 2) {
                // Adjust platforms for screen positioning in Level 2
                std::vector<Platform> adjustedPlatforms = core.platforms;
                for (auto& platform : adjustedPlatforms) {
                    // If this is a platform that should appear on multiple screens:
                    // For last platform of screen 0 (at x ~= SCREEN_WIDTH - 200)
                    if (currentScreenIndex == 1 && platform.rect.x == SCREEN_WIDTH - 200) {
                        // Create a copy positioned at the start of screen 1
                        platform.rect.x = 150 - static_cast<int>(cameraOffsetX);
                    }
                    // For last platform of screen 1
                    else if (currentScreenIndex == 2 && platform.rect.x == SCREEN_WIDTH * 2 - 200) {
                        // Create a copy positioned at the start of screen 2
                        platform.rect.x = SCREEN_WIDTH * 2 + 150 - static_cast<int>(cameraOffsetX);
                    }
                    else {
                        // Regular platform adjustment
                        platform.rect.x -= static_cast<int>(cameraOffsetX);
                    }
                }
                
                // Special finish line check for Level 2 - only on the last screen
                if (currentScreenIndex == SCREEN_COUNT - 1) {
                    // Get the finish line position directly
                    SDL_Rect finishRect = {SCREEN_WIDTH * 3 - 300, 350, 200, 100};
                    
                    // Adjust for camera offset
                    finishRect.x -= static_cast<int>(cameraOffsetX);
                    
                    // Check if player is touching the finish line
                    if (player.x + player.radius > finishRect.x && 
                        player.x - player.radius < finishRect.x + finishRect.w &&
                        player.y + player.radius > finishRect.y &&
                        player.y - player.radius < finishRect.y + finishRect.h) {
                        
                        player.hasReachedFinish = true;
                        player.showingCongratulations = true;
                        player.vx = 0;
                        player.vy = 0;
                        
                        // Play applause sound
                        backgroundMusic.playApplauseSound();
                    }
                }
                
                // Handle normal collisions with screen-adjusted platforms
                if (!player.showingCongratulations) {
                    player.handlecollision(adjustedPlatforms);
                }
            } else {
                // Special finish line check for Level 1
                SDL_Rect finishRect = {SCREEN_WIDTH - 300, 400, 100, 50}; // Use consistent position from level_platforms.h
                
                // Check if player is touching the finish line
                if (player.x + player.radius > finishRect.x && 
                    player.x - player.radius < finishRect.x + finishRect.w &&
                    player.y + player.radius > finishRect.y &&
                    player.y - player.radius < finishRect.y + finishRect.h) {
                    
                    player.hasReachedFinish = true;
                    player.showingCongratulations = true;
                    player.vx = 0;
                    player.vy = 0;
                    
                    // Play applause sound
                    backgroundMusic.playApplauseSound();
                }
                
                // Standard collision detection for other levels
                if (!player.showingCongratulations) {
                    player.handlecollision(core.platforms);
                }
            }

            // Clear screen
            SDL_SetRenderDrawColor(core.renderer, 245, 245, 220, 255);  // Beige color
            SDL_RenderClear(core.renderer);

            // Render background with camera offset
            SDL_Texture* backgroundTexture = nullptr;
            if (selectedLevel == 2) {
                backgroundTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\level2_background.png");
            } else {
                backgroundTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\level1_background.png");
            }
            
            if (backgroundTexture) {
                SDL_Rect bgRect = {
                    static_cast<int>(-cameraOffsetX),
                    0,
                    SCREEN_WIDTH * (selectedLevel == 2 ? 3 : 1),  // Extra wide background for Level 2
                    SCREEN_HEIGHT
                };
                SDL_RenderCopy(core.renderer, backgroundTexture, NULL, &bgRect);
                SDL_DestroyTexture(backgroundTexture);
            }

            // Render platforms with camera offset
            for (const auto& platform : core.platforms) {
                SDL_Rect platformRect = platform.rect;
                platformRect.x -= static_cast<int>(cameraOffsetX);
                SDL_RenderCopy(core.renderer, platform.texture, NULL, &platformRect);
            }

            // Render player
            player.render(core.renderer);

            // Play falling sound when character is falling freely
            bool isFalling = !player.leftHand.isGrabbingObject && !player.rightHand.isGrabbingObject && player.vy > 0;
            backgroundMusic.playFallSound(isFalling);

            // If character has reached finish line, render congratulations screen and play applause
            if (player.showingCongratulations) {
                core.renderCongratulations();
                
                // Only play applause once - moved to finish line detection
                
                // Unlock the next level
                if (selectedLevel < 5) {  // Only unlock if there is a next level
                    levelUnlocked[selectedLevel] = true;  // Unlock the next level (index is 0-based)
                }
            }
        }
        else if (currentState == OPTIONS) {
            // TODO: Render options screen
            SDL_SetRenderDrawColor(core.renderer, 0, 0, 0, 255);
            SDL_Rect optionsRect = {SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 100, 400, 200};
            SDL_RenderFillRect(core.renderer, &optionsRect);
        }
        else if (currentState == HOWTOPLAY) {
            // Render how to play screen with guide image
            core.renderHowToPlay();
        }

        // Present the frame
        SDL_RenderPresent(core.renderer);

        // Small delay to control frame rate
        SDL_Delay(16);  // Approximately 60 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(core.renderer);
    SDL_DestroyWindow(core.window);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}
