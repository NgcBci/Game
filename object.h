#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED
#ifndef _GRAPHICS__H
#define _GRAPHICS__H
#include <SDL.h>
#include <SDL_image.h>
#include "defs.h"
#include <vector>
#include <cmath>
#include <cstdlib>

struct GameObject {
    double x, y, width, height;
    SDL_Texture* texture;
    bool isGrabbable;

    GameObject(SDL_Renderer* renderer, double startX, double startY, double w, double h, bool grabbable = true)
        : x(startX), y(startY), width(w), height(h), isGrabbable(grabbable), texture(nullptr)
    {
        const char* texturePath = "F:\\Game\\graphic\\platform.png";

        SDL_Surface* surface = IMG_Load(texturePath);
        if (surface) {
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }

    void update(double dt) {
        // No update needed for static platforms
    }

    void render(SDL_Renderer* renderer) const {
        if (!texture) {
            // Draw a colored rectangle as fallback
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Red color
            SDL_Rect destRect = {
                static_cast<int>(x),
                static_cast<int>(y),
                static_cast<int>(width),
                static_cast<int>(height)
            };
            SDL_RenderFillRect(renderer, &destRect);
            return;
        }
        SDL_Rect destRect = {
            static_cast<int>(x),
            static_cast<int>(y),
            static_cast<int>(width),
            static_cast<int>(height)
        };
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
    }

    ~GameObject() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
};


#endif // OBJECT_H_INCLUDED
