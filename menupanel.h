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

public:
    MenuPanel(SDL_Renderer* renderer, int x, int y, int width, int height)
        : renderer(renderer), selectedIndex(0), backgroundTexture(nullptr),
          x(x), y(y), width(width), height(height), lockTexture(nullptr), levelLockTexture(nullptr)  // Initialize position and size
    {
        // Load background texture
        backgroundTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\swingngrip.png");
        if (!backgroundTexture) {
            printf("Failed to load background texture: %s\n", IMG_GetError());
        }
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
        for (auto& item : items) {
            if (item.texture) {
                SDL_DestroyTexture(item.texture);
                item.texture = nullptr;
            }
        }
    }

    void addItem(const char* imagePath, void (*action)()) {
        MenuItem item;
        item.texture = IMG_LoadTexture(renderer, imagePath);
        if (!item.texture) {
            printf("Failed to load button texture: %s\n", IMG_GetError());
            return;
        }

        item.isSelected = false;
        item.action = action;

        // Get texture dimensions
        int texWidth, texHeight;
        SDL_QueryTexture(item.texture, NULL, NULL, &texWidth, &texHeight);

        // Calculate item position with larger size
        int itemHeight = 150;  // Increased from 100 to 150
        int padding = 30;      // Increased from 20 to 30
        int yPos = SCREEN_HEIGHT/2 - 300 + (items.size() * (itemHeight + padding));  // Adjusted starting position

        // Scale width to maintain aspect ratio
        int itemWidth = (int)((float)texWidth * itemHeight / texHeight);

        item.rect = {
            SCREEN_WIDTH/6 - itemWidth/2,  // Center in left third
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
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_UP:
                    if (selectedIndex > 0) {
                        items[selectedIndex].isSelected = false;
                        selectedIndex--;
                        items[selectedIndex].isSelected = true;
                    }
                    break;

                case SDL_SCANCODE_DOWN:
                    if (selectedIndex < items.size() - 1) {
                        items[selectedIndex].isSelected = false;
                        selectedIndex++;
                        items[selectedIndex].isSelected = true;
                    }
                    break;

                case SDL_SCANCODE_RETURN:
                    if (items[selectedIndex].action) {
                        items[selectedIndex].action();
                    }
                    break;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
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
            if (item.isSelected) {
                // Draw selection highlight
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                SDL_Rect highlightRect = {
                    item.rect.x - 10,
                    item.rect.y - 10,
                    item.rect.w + 20,
                    item.rect.h + 20
                };
                SDL_RenderFillRect(renderer, &highlightRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }

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

    // Add getters for the arrow and character rectangles
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
        const int BASE_BUTTON_WIDTH = 200;  // Base width, will be adjusted to actual image size
        const int BASE_BUTTON_HEIGHT = 200; // Base height, will be adjusted to actual image size
        const int HORIZONTAL_SPACING = 100;
        const int VERTICAL_SPACING = 100;

        // Pre-load all level textures to get their dimensions
        SDL_Texture* levelTextures[5];
        int actualWidths[5];
        int actualHeights[5];
        
        // Load all textures and get dimensions
        for (int i = 0; i < 5; i++) {
            levelTextures[i] = IMG_LoadTexture(renderer, levelPaths[i]);
            if (levelTextures[i]) {
                SDL_QueryTexture(levelTextures[i], NULL, NULL, &actualWidths[i], &actualHeights[i]);
            } else {
                // Default sizes if loading fails
                actualWidths[i] = BASE_BUTTON_WIDTH;
                actualHeights[i] = BASE_BUTTON_HEIGHT;
            }
        }
        
        // Calculate max button size in each row for consistent layout
        int firstRowMaxHeight = 0;
        int secondRowMaxHeight = 0;
        
        for (int i = 0; i < 3; i++) {
            if (levelTextures[i] && actualHeights[i] > firstRowMaxHeight)
                firstRowMaxHeight = actualHeights[i];
        }
        
        for (int i = 3; i < 5; i++) {
            if (levelTextures[i] && actualHeights[i] > secondRowMaxHeight)
                secondRowMaxHeight = actualHeights[i];
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

                SDL_RenderFillRect(renderer, &highlightRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }

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

    // Add getters for the arrow and character rectangles
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
        const int BASE_BUTTON_WIDTH = 200;  // Base width, will be adjusted to actual image size
        const int BASE_BUTTON_HEIGHT = 200; // Base height, will be adjusted to actual image size
        const int HORIZONTAL_SPACING = 100;
        const int VERTICAL_SPACING = 100;

        // Pre-load all level textures to get their dimensions
        SDL_Texture* levelTextures[5];
        int actualWidths[5];
        int actualHeights[5];
        
        // Load all textures and get dimensions
        for (int i = 0; i < 5; i++) {
            levelTextures[i] = IMG_LoadTexture(renderer, levelPaths[i]);
            if (levelTextures[i]) {
                SDL_QueryTexture(levelTextures[i], NULL, NULL, &actualWidths[i], &actualHeights[i]);
            } else {
                // Default sizes if loading fails
                actualWidths[i] = BASE_BUTTON_WIDTH;
                actualHeights[i] = BASE_BUTTON_HEIGHT;
            }
        }
        
        // Calculate max button size in each row for consistent layout
        int firstRowMaxHeight = 0;
        int secondRowMaxHeight = 0;
        
        for (int i = 0; i < 3; i++) {
            if (levelTextures[i] && actualHeights[i] > firstRowMaxHeight)
                firstRowMaxHeight = actualHeights[i];
        }
        
        for (int i = 3; i < 5; i++) {
            if (levelTextures[i] && actualHeights[i] > secondRowMaxHeight)
                secondRowMaxHeight = actualHeights[i];
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
