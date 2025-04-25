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
    HOWTOPLAY,
    CONGRATULATIONS
};

int currentCharacterIndex = 0;  // 0: red, 1: mint, 2: black, 3: sabrina
// Character images shown in the menu
const char* characterMenuPaths[] = {
    "F:\\Game\\graphic\\red-Photoroom.png",
    "F:\\Game\\graphic\\mint (2)-Photoroom.png",
    "F:\\Game\\graphic\\againblac-Photoroom.png",
    "F:\\Game\\graphic\\allhailsabrina-Photoroom.png"
};
// Actual character images used during gameplay
const char* characterGamePaths[] = {
    "F:\\Game\\graphic\\character.png",
    "F:\\Game\\graphic\\mintchar-Photoroom.png",
    "F:\\Game\\graphic\\blackchar-Photoroom.png",
    "F:\\Game\\graphic\\3735783d-bb0e-422b-870a-5176cba98eaf-Photoroom.png"
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
bool levelUnlocked[] = {true, false, false, false, false};  // Only level 1 is unlocked initially
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

// Add global variables for the spike walls at the top of the file with other globals
MovingObject* spikeWall = nullptr;
bool isSpikewallActive = false;

// Add finish line tracking flags
SDL_Rect level1FinishRect = {SCREEN_WIDTH - 300, 400, 100, 50};
SDL_Rect level2FinishRect = {SCREEN_WIDTH * 3 - 300, 350, 200, 100};
SDL_Rect level3FinishRect = {SCREEN_WIDTH * 3 - 300, 350, 200, 100};
SDL_Rect level4FinishRect = {SCREEN_WIDTH * 3 - 300, 350, 200, 100};
SDL_Rect level5FinishRect = {SCREEN_WIDTH - 300, 400, 100, 50}; // Finish rect for level 5
bool finishLineEnabled = true;

// Back button variables
SDL_Texture* backButtonTexture = nullptr;
SDL_Rect backButtonRect = {20, 20, 60, 60}; // Position in top-left corner with size 60x60

// Check new character prompt variables
SDL_Texture* checkNewCharTexture = nullptr;
SDL_Rect checkNewCharRect = {0, 0, 0, 0}; // Will be set when loaded
bool showingNewCharPrompt = false;
bool hasUnlockedNewChar = false;
// Variables to save camera position when showing prompt
int savedScreenIndex = 0;
double savedCameraOffsetX = 0;

int SDL_main(int argc, char* argv[]) {
    Graphics core;
    core.init();

    // Create character with medium radius (30 pixels = 60x60 total size)
    Character player(core.renderer, 300, 100, 30, 10);  // x, y, radius=30, particles=10

    // Set the initial character texture to the currently selected character
    player.setTexture(core.renderer, characterGamePaths[currentCharacterIndex]);

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

    // Load back button texture
    backButtonTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\back (2).png");
    if (!backButtonTexture) {
        // SDL_Log("Failed to load back button texture: %s", IMG_GetError());
    }

    // Load check new character texture
    checkNewCharTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\checknewchar.png");
        // Get texture dimensions and center it on screen
        int texWidth, texHeight;
        SDL_QueryTexture(checkNewCharTexture, NULL, NULL, &texWidth, &texHeight);
        checkNewCharRect = {
            (SCREEN_WIDTH - texWidth) / 2,
            (SCREEN_HEIGHT - texHeight) / 2,
            texWidth,
            texHeight
        };
    // Initialize level-specific objects
    if (spikeWall == nullptr) {
        // Create the spike wall - full screen height
        SDL_Rect spikeRect = {-200, 0, 200, SCREEN_HEIGHT}; // Full screen height
        SDL_Texture* spikeTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\spikewall.png");
        spikeWall = new MovingObject(spikeRect, spikeTexture, 1.0, 0);
    }

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

                // Handle mouse clicks for menu state changes
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
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

                    // Check if back button was clicked
                    if (mouseX >= backButtonRect.x && mouseX <= backButtonRect.x + backButtonRect.w &&
                        mouseY >= backButtonRect.y && mouseY <= backButtonRect.y + backButtonRect.h) {
                        currentState = MENU;
                        continue;  // Skip the rest of the event handling
                    }

                    // Get rectangle references from menu
                    SDL_Rect leftArrowRect = menu.getLeftArrowRect();
                    SDL_Rect rightArrowRect = menu.getRightArrowRect();
                    SDL_Rect characterRect = menu.getCharacterRect();

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
                        // Update the player character texture based on selection
                        player.setTexture(core.renderer, characterGamePaths[currentCharacterIndex]);

                        // Move to level selection screen
                        currentState = LEVEL_SELECTION;
                    }
                }

                // Load lock texture if not already loaded
                if (!lockTexture) {
                    lockTexture = IMG_LoadTexture(core.renderer, "F:\\Game\\graphic\\lock-removebg-preview.png");
                }
            }
            else if (currentState == LEVEL_SELECTION) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        currentState = CHARACTER_SELECTION;
                        // Ensure character texture is properly set when returning
                        player.setTexture(core.renderer, characterGamePaths[currentCharacterIndex]);
                    }
                }

                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Check if back button was clicked
                    if (mouseX >= backButtonRect.x && mouseX <= backButtonRect.x + backButtonRect.w &&
                        mouseY >= backButtonRect.y && mouseY <= backButtonRect.y + backButtonRect.h) {
                        currentState = CHARACTER_SELECTION;
                        continue;  // Skip the rest of the event handling
                    }
                }

                // Use the menu's level selection handler
                int clickedLevel = menu.handleLevelSelectionEvent(event, levelUnlocked);
                if (clickedLevel > 0) {
                    selectedLevel = clickedLevel;
                    // Update platforms for the selected level
                    core.platforms = LevelPlatforms::getPlatformsForLevel(selectedLevel, core.renderer);
                    // Reset player position for the new level
                    player.resetPosition();

                    // Ensure character texture is set to the current selection
                    player.setTexture(core.renderer, characterGamePaths[currentCharacterIndex]);

                    // Reset screen tracking
                    currentScreenIndex = 0;
                    cameraOffsetX = 0;

                    // Reset finish line state
                    finishLineEnabled = true;
                    player.hasReachedFinish = false;
                    player.showingCongratulations = false;
                    showingNewCharPrompt = false;
                    hasUnlockedNewChar = false;

                    // Reset applause sound flag so it can play again
                    backgroundMusic.resetApplause();

                    // If selecting Level 3, reset spike walls
                    if (selectedLevel == 3) {
                        if (spikeWall) {
                            spikeWall->reset();
                            // Set a slower speed specifically for Level 3
                            spikeWall->velocityX = 1.0;
                        }
                        isSpikewallActive = true;
                    } else {
                        isSpikewallActive = false;
                    }

                    currentState = PLAYING;
                }
            }
            else if (currentState == PLAYING) {
                if (event.type == SDL_KEYDOWN) {
                    // If showing the character prompt, only handle Y/N keys
                    if (showingNewCharPrompt) {
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_Y:  // Yes - go to character selection
                                showingNewCharPrompt = false;
                                hasUnlockedNewChar = false;
                                currentState = CHARACTER_SELECTION;
                                player.showingCongratulations = false;
                                finishLineEnabled = true;
                                // No need to restore camera position since leaving the level
                                break;

                            case SDL_SCANCODE_N:  // No - go to level selection
                                showingNewCharPrompt = false;
                                hasUnlockedNewChar = false;
                                currentState = LEVEL_SELECTION;
                                player.showingCongratulations = false;
                                finishLineEnabled = true;
                                // No need to restore camera position since leaving the level
                                break;

                            default:
                                // Ignore other keys when showing prompt
                                break;
                        }
                    } else {
                        // Handle normal gameplay keys
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_A:  // Left hand grab
                                if (selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4) {
                                    // Use the camera-aware grab method for levels with camera system
                                    player.grabWithCamera(true, core.platforms, cameraOffsetX);
                                } else {
                                    player.grab(true, core.platforms);
                                }
                                if (!event.key.repeat) {  // Only play sound on initial press, not repeat
                                    backgroundMusic.playGrabSound();
                                }
                                break;
                            case SDL_SCANCODE_D:  // Right hand grab
                                if (selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4) {
                                    // Use the camera-aware grab method for levels with camera system
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
                                // Reset finish line for next play
                                finishLineEnabled = true;
                                break;
                            case SDL_SCANCODE_C:  // Return to level selection after completing level
                                if (player.showingCongratulations) {
                                    // Only show the character unlock prompt for levels 1-3
                                    if (hasUnlockedNewChar && selectedLevel >= 1 && selectedLevel <= 3) {
                                        savedScreenIndex = currentScreenIndex;
                                        savedCameraOffsetX = cameraOffsetX;
                                        showingNewCharPrompt = true;
                                    } else {
                                        // Either no new character unlocked or it's level 4 or 5
                                        // SDL_Log("No new character prompt for level %d, going to level selection", selectedLevel);
                                        // No new character unlocked, proceed to level selection
                                        currentState = LEVEL_SELECTION;
                                        player.showingCongratulations = false;  // Reset the congratulations flag
                                        // Reset finish line for next play
                                        finishLineEnabled = true;
                                    }
                                }
                                break;
                            case SDL_SCANCODE_Q:  // Return to menu when 'aced' screen is shown
                                if (player.showingCongratulations) {
                                    currentState = MENU;
                                    player.showingCongratulations = false;  // Reset the congratulations flag
                                    // Reset finish line for next play
                                    finishLineEnabled = true;
                                }
                                break;
                        }
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
                else if (event.type == SDL_MOUSEBUTTONDOWN && !player.showingCongratulations) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Check if back button was clicked
                    if (mouseX >= backButtonRect.x && mouseX <= backButtonRect.x + backButtonRect.w &&
                        mouseY >= backButtonRect.y && mouseY <= backButtonRect.y + backButtonRect.h) {
                        currentState = LEVEL_SELECTION;
                        // Reset finish line for next play
                        finishLineEnabled = true;
                        continue;  // Skip the rest of the event handling
                    }
                }
                // Add mouse click handling for the Y/N buttons in the new character prompt
                else if (event.type == SDL_MOUSEBUTTONDOWN && showingNewCharPrompt) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Define Y/N button areas (approximating based on image)
                    // Assuming the Y button is on the left side of the prompt
                    SDL_Rect yesButtonRect = {
                        checkNewCharRect.x + checkNewCharRect.w / 4 - 40,
                        checkNewCharRect.y + checkNewCharRect.h * 3/4,
                        80, 40
                    };

                    // Assuming the N button is on the right side of the prompt
                    SDL_Rect noButtonRect = {
                        checkNewCharRect.x + checkNewCharRect.w * 3/4 - 40,
                        checkNewCharRect.y + checkNewCharRect.h * 3/4,
                        80, 40
                    };

                    // Check if Yes button was clicked
                    if (mouseX >= yesButtonRect.x && mouseX <= yesButtonRect.x + yesButtonRect.w &&
                        mouseY >= yesButtonRect.y && mouseY <= yesButtonRect.y + yesButtonRect.h) {
                        // Go to character selection
                        showingNewCharPrompt = false;
                        hasUnlockedNewChar = false;
                        currentState = CHARACTER_SELECTION;
                        player.showingCongratulations = false;
                        finishLineEnabled = true;
                        // No need to restore camera position since leaving the level
                    }
                    // Check if No button was clicked
                    else if (mouseX >= noButtonRect.x && mouseX <= noButtonRect.x + noButtonRect.w &&
                             mouseY >= noButtonRect.y && mouseY <= noButtonRect.y + noButtonRect.h) {
                        // Go to level selection
                        showingNewCharPrompt = false;
                        hasUnlockedNewChar = false;
                        currentState = LEVEL_SELECTION;
                        player.showingCongratulations = false;
                        finishLineEnabled = true;
                        // No need to restore camera position since leaving the level
                    }
                }
            }
            else if (currentState == OPTIONS || currentState == HOWTOPLAY) {
                if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    currentState = MENU;
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Check if back button was clicked
                    if (mouseX >= backButtonRect.x && mouseX <= backButtonRect.x + backButtonRect.w &&
                        mouseY >= backButtonRect.y && mouseY <= backButtonRect.y + backButtonRect.h) {
                        currentState = MENU;
                        continue;
                    }
                }

                // Handle volume slider events if in OPTIONS state
                if (currentState == OPTIONS) {
                    if (menu.handleVolumeSliders(event)) {
                        backgroundMusic.setMusicVolume(menu.getMusicVolume());
                        backgroundMusic.setSfxVolume(menu.getSfxVolume());

                        // Store volume settings in a file
                        FILE* volumeFile = fopen("F:\\Game\\volume_settings.dat", "wb");
                        if (volumeFile) {
                            int musicVol = menu.getMusicVolume();
                            int sfxVol = menu.getSfxVolume();
                            fwrite(&musicVol, sizeof(int), 1, volumeFile);
                            fwrite(&sfxVol, sizeof(int), 1, volumeFile);
                            fclose(volumeFile);
                        }
                    }
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
            // Use the new method from MenuPanel to render character selection
            menu.renderCharacterSelection(currentCharacterIndex, characterMenuPaths, characterUnlocked);

            // Render back button
            if (backButtonTexture) {
                SDL_RenderCopy(core.renderer, backButtonTexture, NULL, &backButtonRect);
            }
        }
        else if (currentState == LEVEL_SELECTION) {
            // Use the new method from MenuPanel to render level selection
            menu.renderLevelSelection(levelPaths, levelUnlocked);

            // Render back button
            if (backButtonTexture) {
                SDL_RenderCopy(core.renderer, backButtonTexture, NULL, &backButtonRect);
            }
        }
        else if (currentState == PLAYING) {
            // Update camera offset based on character position (for Level 2, 3, and 4)
            if ((selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4) && !showingNewCharPrompt) {
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

                    // Ensure currentScreenIndex is within valid range
                    if (currentScreenIndex >= SCREEN_COUNT) {
                        currentScreenIndex = SCREEN_COUNT - 1;
                    }

                    // Update camera offset for new screen
                    cameraOffsetX = currentScreenIndex * SCREEN_WIDTH;

                    // Position player at the beginning of the new screen
                    // But maintain the same relative position to the platform
                    double oldX = player.x;
                    player.x = 150 + relativePos; // nhay sang man hinh tiep

                    // Keep vertical position and velocity
// ng nhay thi tay cung nhay
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

                    // If this is Level 3, also move the spike wall to the next screen
                    if (selectedLevel == 3 && spikeWall) {
                        // Reset spike wall to the left of the new screen
                        spikeWall->rect.x = -200 + (currentScreenIndex * SCREEN_WIDTH);
                        // Ensure we maintain the slower velocity
                        spikeWall->velocityX = 1.0;
                    }
                }

                // Check if player fell off the screen
                if (player.y > SCREEN_HEIGHT + 100 ||
                    player.x < -100 ||
                    player.x > SCREEN_WIDTH + 100) {
                    // Reset to beginning of first screen
                    currentScreenIndex = 0;
                    cameraOffsetX = 0;
                    player.resetPosition();

                    // If level 3, reset the spike wall to the first screen
                    if (selectedLevel == 3 && spikeWall) {
                        spikeWall->reset();  // This resets to original start position (-200, 0)
                        // Make sure velocity stays at the slower speed
                        spikeWall->velocityX = 1.0;
                    }

                    // Reset transition timer
                    lastTransitionTime = currentTime;
                }
            } else if (showingNewCharPrompt && (selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4)) {
                // Restore saved camera position for rendering while showing the prompt
                currentScreenIndex = savedScreenIndex;
                cameraOffsetX = savedCameraOffsetX;
            } else {
                cameraOffsetX = 0;  // No sliding for other levels
                currentScreenIndex = 0;  // Reset screen index for other levels
            }

            // Handle moving platform in Level 4 - only if not showing new character prompt
            if (selectedLevel == 4 && !showingNewCharPrompt) {
                // First, save current positions of platforms before updating
                std::vector<int> oldPositions;
                for (const auto& platform : core.platforms) {
                    if (platform.isMoving) {
                        oldPositions.push_back(platform.rect.x);
                    }
                }

                // Update all moving platforms
                int platformIndex = 0;
                for (auto& platform : core.platforms) {
                    if (platform.isMoving) {
                        // Store the old position before updating
                        int oldX = oldPositions[platformIndex++];

                        // Update the platform position
                        platform.update(1.0f);

                        // Calculate the actual change in platform position
                        int deltaX = platform.rect.x - oldX;

                        // Check if platform is in current screen's range
                        int worldX = platform.rect.x;
                        int screenStartX = currentScreenIndex * SCREEN_WIDTH;
                        int screenEndX = (currentScreenIndex + 1) * SCREEN_WIDTH;

                        // Only process platforms that are either:
                        // 1. Visible in the current screen, or
                        // 2. Being grabbed by the player
                        bool isInCurrentScreen = (worldX >= screenStartX - platform.rect.w &&
                                                  worldX <= screenEndX);

                        // If platform is on current screen or being grabbed, check for hand interaction
                        if (isInCurrentScreen ||
                            (player.leftHand.isGrabbingObject && player.leftHand.grabbedPlatformIndex == (&platform - &core.platforms[0])) ||
                            (player.rightHand.isGrabbingObject && player.rightHand.grabbedPlatformIndex == (&platform - &core.platforms[0]))) {

                            // Left hand check
                            if (player.leftHand.isGrabbingObject) {
                                double handX = player.leftHand.parti.back().xCurrent + cameraOffsetX; // Convert to world coordinates
                                double handY = player.leftHand.parti.back().yCurrent;

                                if (handX >= oldX && handX <= oldX + platform.rect.w &&
                                    handY >= platform.rect.y && handY <= platform.rect.y + platform.rect.h) {
                                    // Hand is on this platform - move all particles of the hand
                                    for (auto& particle : player.leftHand.parti) {
                                        particle.xCurrent += deltaX;
                                        particle.xPrevious += deltaX;
                                    }
                                    // Move character too
                                    player.x += deltaX;
                                }
                            }

                            // Right hand check
                            if (player.rightHand.isGrabbingObject) {
                                double handX = player.rightHand.parti.back().xCurrent + cameraOffsetX; // Convert to world coordinates
                                double handY = player.rightHand.parti.back().yCurrent;

                                if (handX >= oldX && handX <= oldX + platform.rect.w &&
                                    handY >= platform.rect.y && handY <= platform.rect.y + platform.rect.h) {
                                    // Hand is on this platform - move all particles of the hand
                                    for (auto& particle : player.rightHand.parti) {
                                        particle.xCurrent += deltaX;
                                        particle.xPrevious += deltaX;
                                    }
                                    // Move character too if not already moved by left hand
                                    if (!player.leftHand.isGrabbingObject) {
                                        player.x += deltaX;
                                    }
                                }
                            }
                        }
                    }
                }

                // Provide current platforms to character for moving platform handling
                player.setPlatformsReference(core.platforms);
            }

            // Update player physics - only if not showing congratulations or new character prompt
            if (!player.showingCongratulations && !showingNewCharPrompt) {
                player.update();
            }

            // Handle collisions with appropriate method based on level - only if not showing congratulations or new character prompt
            if (!player.showingCongratulations && !showingNewCharPrompt) {
                if (selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4) {
                    // Adjust platforms for screen positioning in Level 2/3/4
                    std::vector<Platform> adjustedPlatforms = core.platforms;
                    for (auto& platform : adjustedPlatforms) {
                        // If this is a platform that should appear on multiple screens:
                        // For last platform of screen 0 (at x ~= SCREEN_WIDTH - 200)
                        if ((selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4) && currentScreenIndex == 1 && platform.rect.x == SCREEN_WIDTH - 200) {
                            // Create a copy positioned at the start of screen 1
                            platform.rect.x = 150 - static_cast<int>(cameraOffsetX);
                        }
                        // For last platform of screen 1
                        else if ((selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4) && currentScreenIndex == 2 && platform.rect.x == SCREEN_WIDTH * 2 - 200) {
                            // Create a copy positioned at the start of screen 2
                            platform.rect.x = SCREEN_WIDTH * 2 + 150 - static_cast<int>(cameraOffsetX);
                        }
                        else {
                            // Regular platform adjustment
                            platform.rect.x -= static_cast<int>(cameraOffsetX);
                        }
                    }

                    // Special logic for Level 3 - moving spike wall
                    if (selectedLevel == 3 && !showingNewCharPrompt) {
                        // Activate the spike wall if not already active
                        if (!isSpikewallActive) {
                            spikeWall->reset();
                            isSpikewallActive = true;
                        }

                        // Update spike wall position
                        spikeWall->update();

                        // Check if the spike wall has gone off the right side of the current screen
                        if (spikeWall->rect.x > SCREEN_WIDTH * (currentScreenIndex + 1)) {
                            // Reset to the left side of the current screen
                            spikeWall->rect.x = -200 + (currentScreenIndex * SCREEN_WIDTH);
                        }

                        // Check collision with spike wall
                        SDL_Rect adjustedSpikeRect = spikeWall->rect;
                        adjustedSpikeRect.x -= static_cast<int>(cameraOffsetX);

                        if (player.x + player.radius > adjustedSpikeRect.x &&
                            player.x - player.radius < adjustedSpikeRect.x + adjustedSpikeRect.w &&
                            player.y + player.radius > adjustedSpikeRect.y &&
                            player.y - player.radius < adjustedSpikeRect.y + adjustedSpikeRect.h) {
                            // Collision with spike wall - always reset to beginning of first screen
                            currentScreenIndex = 0;  // Reset to first screen
                            cameraOffsetX = 0;      // Reset camera offset
                            player.resetPosition(); // Reset player position
                            spikeWall->reset();     // Reset spike wall to first screen
                            // Make sure velocity stays at the slower speed
                            spikeWall->velocityX = 1.0;
                        }
                    } else {
                        // Deactivate spike wall for other levels
                        isSpikewallActive = false;
                    }

                    // Special finish line check for Level 2, 3, 4 & 5 - only on the last screen
                    if (finishLineEnabled && (selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4 || selectedLevel == 5) && currentScreenIndex == SCREEN_COUNT - 1) {
                        // Use the appropriate finish line rectangle for this level
                        SDL_Rect finishRect;
                        if (selectedLevel == 2) {
                            finishRect = level2FinishRect;
                        } else if (selectedLevel == 3) {
                            finishRect = level3FinishRect;
                        } else if (selectedLevel == 4) {
                            finishRect = level4FinishRect;
                        } else { // Level 5
                            finishRect = level5FinishRect;
                        }

                        // Adjust for camera offset
                        SDL_Rect adjustedFinishRect = finishRect;
                        adjustedFinishRect.x -= static_cast<int>(cameraOffsetX);

                        // Check if player is touching the finish line
                        if (player.x + player.radius > adjustedFinishRect.x &&
                            player.x - player.radius < adjustedFinishRect.x + adjustedFinishRect.w &&
                            player.y + player.radius > adjustedFinishRect.y &&
                            player.y - player.radius < adjustedFinishRect.y + adjustedFinishRect.h) {

                            player.hasReachedFinish = true;
                            player.showingCongratulations = true;
                            player.vx = 0;
                            player.vy = 0;

                            // Disable finish line detection to prevent re-triggering
                            finishLineEnabled = false;

                            // Play applause sound
                            backgroundMusic.playApplauseSound();
                        }
                    }

                    // Handle normal collisions with screen-adjusted platforms
                    player.handlecollision(adjustedPlatforms);
                } else if (selectedLevel == 1) {
                    // Special finish line check for Level 1 only
                    if (finishLineEnabled) {
                        SDL_Rect finishRect = level1FinishRect; // Use the global finish line rect for Level 1

                        // Check if player is touching the finish line
                        if (player.x + player.radius > finishRect.x &&
                            player.x - player.radius < finishRect.x + finishRect.w &&
                            player.y + player.radius > finishRect.y &&
                            player.y - player.radius < finishRect.y + finishRect.h) {

                            player.hasReachedFinish = true;
                            player.showingCongratulations = true;
                            player.vx = 0;
                            player.vy = 0;

                            // Disable finish line detection to prevent re-triggering
                            finishLineEnabled = false;

                            // Play applause sound
                            backgroundMusic.playApplauseSound();
                        }
                    }

                    // Standard collision detection for Level 1
                    player.handlecollision(core.platforms);
                } else {
                    // For any other levels without special handling
                    // Standard collision detection
                    player.handlecollision(core.platforms);
                }
            }

            // Clear screen
            SDL_SetRenderDrawColor(core.renderer, 245, 245, 220, 255);  // Beige color
            SDL_RenderClear(core.renderer);

            // Render background with camera offset
            SDL_Texture* backgroundTexture = nullptr;
            const char* backgroundPath = "F:\\Game\\graphic\\level1_background.png";

            if (selectedLevel == 2) {
                backgroundPath = "F:\\Game\\graphic\\level2_background.png";
            }
            // All other levels use level1 background

            backgroundTexture = IMG_LoadTexture(core.renderer, backgroundPath);

            if (backgroundTexture) {
                SDL_Rect bgRect = {
                    static_cast<int>(-cameraOffsetX),
                    0,
                    SCREEN_WIDTH * ((selectedLevel == 2 || selectedLevel == 3 || selectedLevel == 4) ? 3 : 1),  // Extra wide background for multi-screen levels
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

            // Special handling for Level 5 interactive platform
            if (selectedLevel == 5 && !showingNewCharPrompt) {
                // Count how many buttons are activated
                int activatedButtonCount = 0;
                int buttonIndex1 = -1;
                int buttonIndex2 = -1;
                int pollIndex = -1;

                // Find the interactive platforms and the poll platform
                for (int i = 0; i < core.platforms.size(); i++) {
                    const auto& platform = core.platforms[i];
                    if (platform.isInteractive) {
                        if (buttonIndex1 == -1) {
                            buttonIndex1 = i;
                        } else {
                            buttonIndex2 = i;
                        }

                        if (platform.activated) {
                            activatedButtonCount++;
                        }
                    }

                    // Find the poll platform (assuming it's the one closest to SCREEN_WIDTH - 350)
                    if (abs(platform.rect.x - (SCREEN_WIDTH - 350)) < 10 && platform.rect.h == SCREEN_HEIGHT) {
                        pollIndex = i;
                    }
                }

                // Check if both buttons have been activated
                if (activatedButtonCount == 2 && pollIndex != -1) {
                    // Move the poll off-screen to make it "disappear"
                    core.platforms[pollIndex].rect.x = -1000;
                }

                // Handle activation of individual buttons
                for (auto& platform : core.platforms) {
                    if (platform.isInteractive && !platform.activated) {
                        // Check if player is touching the interactive platform
                        SDL_Rect platformRect = platform.rect;

                        // Check for any overlap with player
                        if (player.x + player.radius > platformRect.x &&
                            player.x - player.radius < platformRect.x + platformRect.w &&
                            player.y + player.radius > platformRect.y &&
                            player.y - player.radius < platformRect.y + platformRect.h) {

                            // Activate the platform (swap textures) immediately
                            platform.activate();
                            // Play a sound for feedback
                            backgroundMusic.playApplauseSound();
                        }
                    }
                }

                // Add finish line detection specific for Level 5
                if (finishLineEnabled) {
                    // Only check finish line if the poll is removed (both buttons activated)
                    if (activatedButtonCount == 2) {
                        SDL_Rect finishRect = level5FinishRect;

                        // Check if player is touching the finish line
                        if (player.x + player.radius > finishRect.x &&
                            player.x - player.radius < finishRect.x + finishRect.w &&
                            player.y + player.radius > finishRect.y &&
                            player.y - player.radius < finishRect.y + finishRect.h) {

                            // Player reached the finish line
                            player.hasReachedFinish = true;
                            player.showingCongratulations = true;
                            player.vx = 0;
                            player.vy = 0;

                            // Disable finish line detection to prevent re-triggering
                            finishLineEnabled = false;

                            // Force reset and play applause sound regardless of previous state
                            backgroundMusic.resetApplause();
                            backgroundMusic.playApplauseSound();
                        }
                    }
                }
            }

            // Render the spike wall for Level 3
            if (selectedLevel == 3 && isSpikewallActive && spikeWall && !showingNewCharPrompt) {
                SDL_Rect adjustedSpikeRect = spikeWall->rect;
                adjustedSpikeRect.x -= static_cast<int>(cameraOffsetX);
                SDL_RenderCopy(core.renderer, spikeWall->texture, NULL, &adjustedSpikeRect);
            }

            // Render player
            player.render(core.renderer);

            // Play falling sound when character is falling freely - but not during prompt
            if (!showingNewCharPrompt) {
                bool isFalling = !player.leftHand.isGrabbingObject && !player.rightHand.isGrabbingObject && player.vy > 0;
                backgroundMusic.playFallSound(isFalling);
            }

            // If character has reached finish line, render "aced" screen and play applause
            if (player.showingCongratulations && !showingNewCharPrompt) {
                // Choose which congratulations screen to show based on level
                if (selectedLevel == 5) {
                    // Use the "aced" image for level 5
                    core.renderAced();
                } else {
                    // Use the regular congratulations screen for levels 1-4
                    core.renderCongratulations();
                }

                // Only play applause once - moved to finish line detection

                // Unlock the next level
                if (selectedLevel < 5) {  // Only unlock if there is a next level
                    levelUnlocked[selectedLevel] = true;  // Unlock the next level (index is 0-based)
                }

                // Unlock characters based on level completion
                if (selectedLevel == 1 && !characterUnlocked[1]) {
                    // Unlock mint character after completing level 1
                    // SDL_Log("Unlocking mint character from level 1");
                    characterUnlocked[1] = true;
                    hasUnlockedNewChar = true;
                }
                else if (selectedLevel == 2 && !characterUnlocked[2]) {
                    // Unlock black character after completing level 2
                    // SDL_Log("Unlocking black character from level 2");
                    characterUnlocked[2] = true;
                    hasUnlockedNewChar = true;
                }
                else if (selectedLevel == 3 && !characterUnlocked[3]) {
                    // Unlock sabrina character after completing level 3
                    // SDL_Log("Unlocking sabrina character from level 3");
                    characterUnlocked[3] = true;
                    hasUnlockedNewChar = true;
                }
            }

            // Render back button (only if not in congratulations screen and not showing prompt)
            if (backButtonTexture && !player.showingCongratulations && !showingNewCharPrompt) {
                SDL_RenderCopy(core.renderer, backButtonTexture, NULL, &backButtonRect);
            }
        }
        else if (currentState == OPTIONS) {
            // Initialize slider values with current volumes
            static bool volumeInitialized = false;
            if (!volumeInitialized) {
                FILE* volumeFile = fopen("F:\\Game\\volume_settings.dat", "rb");
                if (volumeFile) {
                    int musicVol, sfxVol;
                    if (fread(&musicVol, sizeof(int), 1, volumeFile) == 1 &&
                        fread(&sfxVol, sizeof(int), 1, volumeFile) == 1) {
                        menu.setMusicVolume(musicVol);
                        menu.setSfxVolume(sfxVol);
                        backgroundMusic.setMusicVolume(musicVol);
                        backgroundMusic.setSfxVolume(sfxVol);
                    }
                    fclose(volumeFile);
                } else {
                    menu.setMusicVolume(backgroundMusic.getMusicVolume());
                    menu.setSfxVolume(backgroundMusic.getSfxVolume());
                }
                volumeInitialized = true;
            }

            menu.renderOptions();

            if (backButtonTexture) {
                SDL_RenderCopy(core.renderer, backButtonTexture, NULL, &backButtonRect);
            }
        }
        else if (currentState == HOWTOPLAY) {
            // Render how to play screen with guide image
            core.renderHowToPlay();

            // Render back button
            if (backButtonTexture) {
                SDL_RenderCopy(core.renderer, backButtonTexture, NULL, &backButtonRect);
            }
        }

        // Render the new character prompt at the end, on top of everything else
        if (showingNewCharPrompt && checkNewCharTexture) {
            // SDL_Log("Rendering new character prompt at the end");
            SDL_RenderCopy(core.renderer, checkNewCharTexture, NULL, &checkNewCharRect);
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

    // Cleanup spike wall
    if (spikeWall) {
        SDL_DestroyTexture(spikeWall->texture);
        delete spikeWall;
        spikeWall = nullptr;
    }

    // Cleanup back button texture
    if (backButtonTexture) {
        SDL_DestroyTexture(backButtonTexture);
        backButtonTexture = nullptr;
    }

    // Cleanup check new character texture
    if (checkNewCharTexture) {
        SDL_DestroyTexture(checkNewCharTexture);
        checkNewCharTexture = nullptr;
    }

    return 0;
}
