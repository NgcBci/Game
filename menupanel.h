#ifndef _MENUPANEL__H
#define _MENUPANEL__H

#include <SDL.h>
#include <string>
#include <vector>
#include "defs.h"

struct MenuItem {
    std::string text;
    SDL_Rect rect;
    bool isSelected;
    void (*action)();  // Function pointer for the action to take when selected
};

class MenuPanel {
private:
    SDL_Renderer* renderer;
    std::vector<MenuItem> items;
    int selectedIndex;
    SDL_Color textColor;
    SDL_Color selectedColor;
    SDL_Color backgroundColor;

public:
    MenuPanel(SDL_Renderer* renderer, int x, int y, int width, int height) 
        : renderer(renderer), selectedIndex(0) {
        // Initialize colors
        textColor = {200, 200, 200, 255};        // Light gray
        selectedColor = {255, 255, 255, 255};    // White
        backgroundColor = {0, 0, 0, 255};         // Black
    }

    void addItem(const std::string& text, void (*action)()) {
        MenuItem item;
        item.text = text;
        item.isSelected = false;
        item.action = action;
        
        // Calculate item position
        int itemHeight = 50;
        int padding = 10;
        int yPos = SCREEN_HEIGHT/2 - 100 + (items.size() * (itemHeight + padding));
        
        item.rect = {
            SCREEN_WIDTH/2 - 100,  // Center horizontally
            yPos,
            200,                   // Fixed width
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
    }

    void render() {
        // Draw menu items
        for (const auto& item : items) {
            // Draw item background
            SDL_Color color = item.isSelected ? selectedColor : textColor;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &item.rect);

            // Draw item border
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &item.rect);
        }
    }

    int getSelectedIndex() const {
        return selectedIndex;
    }
};

#endif
