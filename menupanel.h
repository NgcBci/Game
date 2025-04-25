#ifndef _MENUPANEL__H
#define _MENUPANEL__H
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <vector>
#include "defs.h"

struct MenuItem {
    SDL_Texture* texture;
    SDL_Rect rect;
    bool isSelected;
    void (*action)();  // Function pointer for the action to take when selected
};

// Add a Slider struct for volume controls, slider trong options
struct Slider {
    SDL_Rect trackRect;     // Rectangle for the slider track
    SDL_Rect knobRect;      // cai nut de keo i
    int value;              // Current value (0-128)
    int minValue, maxValue; // Min and max values
    bool isDragging;        // Whether the slider is being dragged

    Slider() : value(128), minValue(0), maxValue(128), isDragging(false) {}

    void setPosition(int x, int y, int width, int height) {
        trackRect = {x, y, width, height};
        updateKnobPosition();
    }
    // update cai knob nma trong khoang tu min den max
    void setValue(int newValue) {
        value = (newValue < minValue) ? minValue : ((newValue > maxValue) ? maxValue : newValue);
        updateKnobPosition();
    }

    void updateKnobPosition() {
        // Calculate knob position based on value
        float percentage = static_cast<float>(value - minValue) / (maxValue - minValue);
        int knobX = trackRect.x + static_cast<int>(percentage * (trackRect.w - 30)); // 30 is knob width
        knobRect = {knobX, trackRect.y - 10, 30, trackRect.h + 20}; // Make knob slightly larger than track
    }
    // lay event chuot de chinh vi tri cai knob
    bool handleEvent(SDL_Event& event) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = event.button.x;
            int mouseY = event.button.y;

            // Check if mouse is on knob
            if (mouseX >= knobRect.x && mouseX <= knobRect.x + knobRect.w &&
                mouseY >= knobRect.y && mouseY <= knobRect.y + knobRect.h) {
                isDragging = true;
                return true;
            }
            // Check if mouse is on track
            else if (mouseX >= trackRect.x && mouseX <= trackRect.x + trackRect.w &&
                     mouseY >= trackRect.y && mouseY <= trackRect.y + trackRect.h) {
                // Set value based on click position
                float percentage = static_cast<float>(mouseX - trackRect.x) / trackRect.w;
                setValue(static_cast<int>(minValue + percentage * (maxValue - minValue)));
                isDragging = true;
                return true;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            isDragging = false;
        }
        else if (event.type == SDL_MOUSEMOTION && isDragging) {
            int mouseX = event.motion.x;

            // Constrain to track boundaries
            if (mouseX < trackRect.x)
                mouseX = trackRect.x;
            else if (mouseX > trackRect.x + trackRect.w)
                mouseX = trackRect.x + trackRect.w;

            // Update value based on drag position
            float percentage = static_cast<float>(mouseX - trackRect.x) / trackRect.w;
            setValue(static_cast<int>(minValue + percentage * (maxValue - minValue)));
            return true;
        }

        return false;
    }
    // ve may cai can thiet de cho cai slider
    void render(SDL_Renderer* renderer) {
        // Draw track background
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255); // Darker gray

        // Draw filled track
        SDL_Rect filledTrack = trackRect;
        filledTrack.w = static_cast<int>((float)value / maxValue * trackRect.w);

        // Draw empty track
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255); // Light gray
        SDL_RenderFillRect(renderer, &trackRect);

        // Draw filled portion
        SDL_SetRenderDrawColor(renderer, 70, 130, 180, 255); // Steel blue
        SDL_RenderFillRect(renderer, &filledTrack);

        // Draw track border
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderDrawRect(renderer, &trackRect);

        // Draw knob with gradient effect
        // Outer knob (shadow)
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_Rect shadowRect = knobRect;
        shadowRect.x += 2;
        shadowRect.y += 2;
        SDL_RenderFillRect(renderer, &shadowRect);

        // Inner knob (main)
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255); // Light gray
        SDL_RenderFillRect(renderer, &knobRect);

        // Knob highlight (top-left edge for 3D effect)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White

        // Horizontal highlight line
        SDL_RenderDrawLine(renderer,
                          knobRect.x + 2,
                          knobRect.y + 2,
                          knobRect.x + knobRect.w - 2,
                          knobRect.y + 2);

        // Vertical highlight line
        SDL_RenderDrawLine(renderer,
                          knobRect.x + 2,
                          knobRect.y + 2,
                          knobRect.x + 2,
                          knobRect.y + knobRect.h - 2);

        // Knob border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
        SDL_RenderDrawRect(renderer, &knobRect);

        // Draw percentage indicator on knob
        int percentage = static_cast<int>((float)value / maxValue * 100);

        // Draw percentage text on knob (visual indicator)
        if (percentage > 0) {
            // Draw a line on the knob as a visual percentage indicator
            int lineHeight = static_cast<int>((float)knobRect.h * 0.6f);
            int startY = knobRect.y + (knobRect.h - lineHeight) / 2;
            int endY = startY + lineHeight;

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawLine(renderer,
                             knobRect.x + knobRect.w / 2,
                             startY,
                             knobRect.x + knobRect.w / 2,
                             endY);
        }
    }
};

class MenuPanel {
private:
    SDL_Renderer* renderer;
    std::vector<MenuItem> items;
    int selectedIndex;
    SDL_Texture* backgroundTexture;  // Background texture for right side
    int x, y, width, height;  // Add position and size members

    // Character selection variables
    SDL_Texture* lockTexture;
    SDL_Rect leftArrowRect, rightArrowRect, characterRect;

    // Level selection variables
    SDL_Texture* levelLockTexture;
    SDL_Rect levelRects[5];  // Array to store level button rectangles

    // Volume control sliders
    Slider musicVolumeSlider;
    Slider sfxVolumeSlider;
    SDL_Texture* optionsTexture;  // For the options menu title
    SDL_Texture* volumeTexture;   // Volume image
    SDL_Texture* sfxTexture;      // SFX image

public:
    MenuPanel(SDL_Renderer* renderer, int x, int y, int width, int height)
        : renderer(renderer), selectedIndex(0), backgroundTexture(nullptr),
          x(x), y(y), width(width), height(height), lockTexture(nullptr),
          levelLockTexture(nullptr), optionsTexture(nullptr), volumeTexture(nullptr), sfxTexture(nullptr)
    {
        // Load background texture
        backgroundTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\swingngrip.png");

        // Initialize volume sliders with consistent positions
        musicVolumeSlider.setPosition(SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 40, 300, 20);
        sfxVolumeSlider.setPosition(SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 40, 300, 20);

        // Load options title texture
        optionsTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\optionsbut.png");

        // Load volume and sfx textures
        volumeTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\volume.png");
        sfxTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\sfx.png");
    }

    ~MenuPanel() {
        if (backgroundTexture) {
            SDL_DestroyTexture(backgroundTexture);
            backgroundTexture = nullptr;
        }
        if (lockTexture) {
            SDL_DestroyTexture(lockTexture);
            lockTexture = nullptr;
        }
        if (levelLockTexture) {
            SDL_DestroyTexture(levelLockTexture);
            levelLockTexture = nullptr;
        }
        if (optionsTexture) {
            SDL_DestroyTexture(optionsTexture);
            optionsTexture = nullptr;
        }
        if (volumeTexture) {
            SDL_DestroyTexture(volumeTexture);
            volumeTexture = nullptr;
        }
        if (sfxTexture) {
            SDL_DestroyTexture(sfxTexture);
            sfxTexture = nullptr;
        }
        for (auto& item : items) {
            if (item.texture) {
                SDL_DestroyTexture(item.texture);
                item.texture = nullptr;
            }
        }
    }

    // Volume slider getters and setters
    void setMusicVolume(int volume) {
        musicVolumeSlider.setValue(volume);
    }

    void setSfxVolume(int volume) {
        sfxVolumeSlider.setValue(volume);
    }

    int getMusicVolume() const {
        return musicVolumeSlider.value;
    }

    int getSfxVolume() const {
        return sfxVolumeSlider.value;
    }

    // Handle volume slider events
    bool handleVolumeSliders(SDL_Event& event) {
        bool musicChanged = musicVolumeSlider.handleEvent(event);
        bool sfxChanged = sfxVolumeSlider.handleEvent(event);
        return musicChanged || sfxChanged;
    }

    // Render options menu with volume sliders
    void renderOptions() {
        // Draw background
        SDL_SetRenderDrawColor(renderer, 245, 245, 220, 255);  // Beige color
        SDL_RenderClear(renderer);

        // Draw title if texture is loaded
        if (optionsTexture) {
            int texWidth, texHeight;
            SDL_QueryTexture(optionsTexture, NULL, NULL, &texWidth, &texHeight);

            // Calculate scaled dimensions to maintain aspect ratio
            int targetHeight = 120;
            int targetWidth = (int)((float)texWidth * targetHeight / texHeight);

            SDL_Rect titleRect = {
                (SCREEN_WIDTH - targetWidth) / 2,  // Center horizontally
                30,
                targetWidth,
                targetHeight
            };

            SDL_RenderCopy(renderer, optionsTexture, NULL, &titleRect);
        }

        // Update slider positions
        musicVolumeSlider.setPosition(SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 40, 300, 20);
        sfxVolumeSlider.setPosition(SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 40, 300, 20);

        // Render the sliders
        musicVolumeSlider.render(renderer);
        sfxVolumeSlider.render(renderer);

        // Render the volume icon
        if (volumeTexture) {
            int texWidth, texHeight;
            SDL_QueryTexture(volumeTexture, NULL, NULL, &texWidth, &texHeight);

            int iconHeight = 50;
            int iconWidth = (int)((float)texWidth * iconHeight / texHeight);

            SDL_Rect iconRect = {
                musicVolumeSlider.trackRect.x - iconWidth - 20,
                musicVolumeSlider.trackRect.y - (iconHeight - musicVolumeSlider.trackRect.h) / 2,
                iconWidth,
                iconHeight
            };

            SDL_RenderCopy(renderer, volumeTexture, NULL, &iconRect);
        }

        // Render the sfx icon
        if (sfxTexture) {
            int texWidth, texHeight;
            SDL_QueryTexture(sfxTexture, NULL, NULL, &texWidth, &texHeight);

            int iconHeight = 50;
            int iconWidth = (int)((float)texWidth * iconHeight / texHeight);

            SDL_Rect iconRect = {
                sfxVolumeSlider.trackRect.x - iconWidth - 20,
                sfxVolumeSlider.trackRect.y - (iconHeight - sfxVolumeSlider.trackRect.h) / 2,
                iconWidth,
                iconHeight
            };

            SDL_RenderCopy(renderer, sfxTexture, NULL, &iconRect);
        }
    }

    void addItem(const char* imagePath, void (*action)()) {
        MenuItem item;
        item.texture = IMG_LoadTexture(renderer, imagePath);
        if (!item.texture) {
            return;
        }

        item.isSelected = false;
        item.action = action;

        // Get texture dimensions
        int texWidth, texHeight;
        SDL_QueryTexture(item.texture, NULL, NULL, &texWidth, &texHeight);

        // Calculate item position with larger size
        int itemHeight = 150;
        int padding = 30;
        int yPos = SCREEN_HEIGHT/2 - 300 + (items.size() * (itemHeight + padding));

        // Scale width to maintain aspect ratio
        int itemWidth = (int)((float)texWidth * itemHeight / texHeight);

        item.rect = {
            SCREEN_WIDTH/6 - itemWidth/2,
            yPos,
            itemWidth,
            itemHeight
        };

        items.push_back(item);

        // Select first item by default
        if (items.size() == 1) {
            items[0].isSelected = true;
        }
    }

    void handleEvent(SDL_Event& event) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                // Get mouse position
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                // Check if click is within any button's rectangle
                for (size_t i = 0; i < items.size(); i++) {
                    if (mouseX >= items[i].rect.x &&
                        mouseX <= items[i].rect.x + items[i].rect.w &&
                        mouseY >= items[i].rect.y &&
                        mouseY <= items[i].rect.y + items[i].rect.h) {

                        // Update selection
                        items[selectedIndex].isSelected = false;
                        selectedIndex = i;
                        items[selectedIndex].isSelected = true;

                        // Execute the button's action
                        if (items[i].action) {
                            items[i].action();
                        }
                        break;
                    }
                }
            }
        }
    }
// ve cai hinh nen con mau do do
    void render() {
        // Draw background on right side
        if (backgroundTexture) {
            // Get original texture dimensions
            int texWidth, texHeight;
            SDL_QueryTexture(backgroundTexture, NULL, NULL, &texWidth, &texHeight);

            // Calculate destination rectangle for right side
            SDL_Rect destRect = {
                SCREEN_WIDTH * 7/20,  // Start at 7/20 of screen width (moved right from 3/10)
                0,                    // Start at top
                SCREEN_WIDTH * 2/3,   // Take up 2/3 of screen width
                SCREEN_HEIGHT         // Full height
            };

            // Calculate source rectangle to maintain aspect ratio
            float aspectRatio = (float)texWidth / texHeight;
            float destAspectRatio = (float)destRect.w / destRect.h;

            if (aspectRatio > destAspectRatio) {
                // Image is wider than destination, adjust height
                destRect.h = (int)(destRect.w / aspectRatio);
                destRect.y = (SCREEN_HEIGHT - destRect.h) / 2;  // Center vertically
            } else {
                // Image is taller than destination, adjust width
                destRect.w = (int)(destRect.h * aspectRatio);
                // Keep the x position we set, don't override it
            }

            // Use the full source image
            SDL_Rect srcRect = {0, 0, texWidth, texHeight};

            SDL_RenderCopy(renderer, backgroundTexture, &srcRect, &destRect);
        }

        // Draw menu items (buttons) on left side
        for (const auto& item : items) {
            // Draw button texture
            SDL_RenderCopy(renderer, item.texture, NULL, &item.rect);
        }
    }

    int getSelectedIndex() const {
        return selectedIndex;
    }

    // New method for rendering character selection
    void renderCharacterSelection(int currentCharacterIndex, const char* characterPaths[], bool characterUnlocked[]) {
        // Load lock texture if not already loaded
        if (!lockTexture) {
            lockTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\lock-removebg-preview.png");
        }

        // Render character selection screen
        SDL_Texture* charSelectTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\chooseyourchar-Photoroom.png");
        if (charSelectTexture) {
            int texWidth, texHeight;
            SDL_QueryTexture(charSelectTexture, NULL, NULL, &texWidth, &texHeight);

            // Scale up the dimensions
            int scaledWidth = texWidth * 1.5;  // 150% of original width
            int scaledHeight = texHeight * 1.5;  // 150% of original height
// nay cho to hinh len vi ve be
            // Position at middle top of screen
            SDL_Rect destRect = {
                (SCREEN_WIDTH - scaledWidth) / 2,  // Center horizontally
                0,  // At the very top
                scaledWidth,
                scaledHeight
            };

            SDL_RenderCopy(renderer, charSelectTexture, NULL, &destRect);
            SDL_DestroyTexture(charSelectTexture);
        }

        // Render current character in the middle of the screen
        SDL_Texture* characterTexture = IMG_LoadTexture(renderer, characterPaths[currentCharacterIndex]);
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

            SDL_RenderCopy(renderer, characterTexture, NULL, &characterRect);
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
                SDL_RenderCopy(renderer, lockTexture, NULL, &lockRect);
            }
        }

        // Render left arrow
        SDL_Texture* leftArrowTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\chontrai-Photoroom.png");
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

            SDL_RenderCopy(renderer, leftArrowTexture, NULL, &leftArrowRect);
            SDL_DestroyTexture(leftArrowTexture);
        }

        // Render right arrow
        SDL_Texture* rightArrowTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\chonphai-Photoroom.png");
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

            SDL_RenderCopy(renderer, rightArrowTexture, NULL, &rightArrowRect);
            SDL_DestroyTexture(rightArrowTexture);
        }
    }

    // handle event chuot trong main
    SDL_Rect getLeftArrowRect() const { return leftArrowRect; }
    SDL_Rect getRightArrowRect() const { return rightArrowRect; }
    SDL_Rect getCharacterRect() const { return characterRect; }

    // New method for rendering level selection
    void renderLevelSelection(const char* levelPaths[], bool levelUnlocked[]) {
        // Load level lock texture if not already loaded
        if (!levelLockTexture) {
            levelLockTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\lock-removebg-preview.png");
        }

        // Render level selection background
        SDL_Texture* levelSelectTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\levil-Photoroom.png");
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

            SDL_RenderCopy(renderer, levelSelectTexture, NULL, &destRect);
            SDL_DestroyTexture(levelSelectTexture);
        }

        // Calculate positions for level buttons with dynamic sizing

        // Pre-load all level textures to get their dimensions
        SDL_Texture* levelTextures[5];
        int actualWidths[5];
        int actualHeights[5];
        const int BASE_BUTTON_WIDTH = 200;  // Base width, will be adjusted to actual image size
        const int BASE_BUTTON_HEIGHT = 200; // Base height, will be adjusted to actual image size
        const int HORIZONTAL_SPACING = 100;
        const int VERTICAL_SPACING = 100;
        // Load all textures and get dimensions
        for (int i = 0; i < 5; i++) {
            levelTextures[i] = IMG_LoadTexture(renderer, levelPaths[i]);
            if (levelTextures[i]) {
                SDL_QueryTexture(levelTextures[i], NULL, NULL, &actualWidths[i], &actualHeights[i]);
            }
        }

        // Calculate max button size in each row for consistent layout
        int firstRowMaxHeight = 0;
        int secondRowMaxHeight = 0;

        for (int i = 0; i < 5; i++) {
            if (levelTextures[i] && actualHeights[i] > firstRowMaxHeight)
                firstRowMaxHeight = actualHeights[i];
        }

        // Ensure minimum height
        if (firstRowMaxHeight < BASE_BUTTON_HEIGHT) firstRowMaxHeight = BASE_BUTTON_HEIGHT;
        if (secondRowMaxHeight < BASE_BUTTON_HEIGHT) secondRowMaxHeight = BASE_BUTTON_HEIGHT;

        // Calculate total height of both rows
        int totalHeight = firstRowMaxHeight + secondRowMaxHeight + VERTICAL_SPACING;

        // Center the entire grid vertically
        int firstRowY = (SCREEN_HEIGHT - totalHeight) / 2;
        int secondRowY = firstRowY + firstRowMaxHeight + VERTICAL_SPACING;

        // Calculate row widths
        int firstRowWidth = 0;
        for (int i = 0; i < 3; i++) {
            firstRowWidth += actualWidths[i];
        }
        firstRowWidth += 2 * HORIZONTAL_SPACING; // Add spacing between buttons

        int secondRowWidth = actualWidths[3] + actualWidths[4] + HORIZONTAL_SPACING;

        // Calculate starting X for each row
        int firstRowStartX = (SCREEN_WIDTH - firstRowWidth) / 2;
        int secondRowStartX = (SCREEN_WIDTH - secondRowWidth) / 2;

        // Render level buttons
        int currentFirstRowX = firstRowStartX;
        int currentSecondRowX = secondRowStartX;

        for (int i = 0; i < 5; i++) {
            if (levelTextures[i]) {
                SDL_Rect levelRect;

                if (i < 3) {
                    // First row (3 levels)
                    levelRect = {
                        currentFirstRowX,
                        firstRowY,
                        actualWidths[i],
                        actualHeights[i]
                    };
                    currentFirstRowX += actualWidths[i] + HORIZONTAL_SPACING;
                } else {
                    // Second row (2 levels)
                    levelRect = {
                        currentSecondRowX,
                        secondRowY,
                        actualWidths[i],
                        actualHeights[i]
                    };
                    currentSecondRowX += actualWidths[i] + HORIZONTAL_SPACING;
                }

                SDL_RenderCopy(renderer, levelTextures[i], NULL, &levelRect);

                // Store level rectangle for click detection - USE THE EXACT SAME RECTANGLE
                levelRects[i] = levelRect;

                // Render lock if level is locked
                if (!levelUnlocked[i] && levelLockTexture) {
                    int lockWidth = actualWidths[i] * 0.5;  // Lock size relative to level button
                    int lockHeight = lockWidth;  // Keep aspect ratio
                    SDL_Rect lockRect = {
                        levelRect.x + (levelRect.w - lockWidth) / 2,
                        levelRect.y + (levelRect.h - lockHeight) / 2,
                        lockWidth,
                        lockHeight
                    };
                    SDL_RenderCopy(renderer, levelLockTexture, NULL, &lockRect);
                }
            }
        }

        // Clean up textures
        for (int i = 0; i < 5; i++) {
            if (levelTextures[i]) {
                SDL_DestroyTexture(levelTextures[i]);
            }
        }
    }

    // Handle level selection events, returns selectedLevel if a level was clicked, 0 otherwise
    int handleLevelSelectionEvent(SDL_Event& event, bool levelUnlocked[]) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Check if any level was clicked
            for (int i = 0; i < 5; i++) {
                // Only proceed if the level is unlocked
                if (levelUnlocked[i]) {
                    // Create a larger hit box for all levels
                    SDL_Rect hitBox = levelRects[i];
                    // Expand the hit box by 20 pixels in each direction
                    hitBox.x -= 20;
                    hitBox.y -= 20;
                    hitBox.w += 40;
                    hitBox.h += 40;

                    if (mouseX >= hitBox.x && mouseX <= hitBox.x + hitBox.w &&
                        mouseY >= hitBox.y && mouseY <= hitBox.y + hitBox.h) {
                        // Return the level index + 1 (1-based indexing)
                        return i + 1;
                    }
                }
            }
        }

        return 0; // No level was clicked
    }

    // Get level rects for external access if needed
    const SDL_Rect* getLevelRects() const {
        return levelRects;
    }
};

#endif
