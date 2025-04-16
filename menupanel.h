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

public:
    MenuPanel(SDL_Renderer* renderer) : renderer(renderer), selectedIndex(0), backgroundTexture(nullptr) {
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
};

#endif
