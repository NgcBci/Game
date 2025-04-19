#pragma once
#include "SDL.h"
#include <vector>
#include "graphics.h"

struct PlatformConfig {
    int x, y, width, height;
};

class LevelPlatforms {
public:
    static std::vector<Platform> getLevel1Platforms(SDL_Renderer* renderer) {
        std::vector<Platform> platforms;
        
        // Main platform
        SDL_Rect rect = {300, 300, 200, 20};
        SDL_Texture* texture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\platform.png");
        platforms.push_back(Platform(rect, texture));
        
        // Square thing at top center
        rect = {(SCREEN_WIDTH - 409) / 2, 0, 409, 307};
        texture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\squarething.png");
        platforms.push_back(Platform(rect, texture));
        
        // Finish line
        rect = {SCREEN_WIDTH - 300, 400, 100, 50};
        texture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\finish.png");
        platforms.push_back(Platform(rect, texture));
        
        return platforms;
    }

    static std::vector<Platform> getLevel2Platforms(SDL_Renderer* renderer) {
        std::vector<Platform> platforms;
        
        // Load textures with correct paths
        SDL_Texture* smallBlackTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\smallblackpf-Photoroom.png");
        SDL_Texture* squareTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\blacksquarepf-Photoroom.png");
        SDL_Texture* finishTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\finish.png");

        // Starting square platform
        SDL_Rect rect = {300, 300, 409, 307};
        platforms.push_back(Platform(rect, squareTexture));

        // Small black platforms in V pattern
        // First V
        rect = {800, 200, 100, 100};    // Top
        platforms.push_back(Platform(rect, smallBlackTexture));
        rect = {1300, 300, 100, 100};   // Middle
        platforms.push_back(Platform(rect, smallBlackTexture));
        
        // Important: This is the screen transition platform (end of screen 1)
        rect = {SCREEN_WIDTH - 200, 400, 100, 100};   // Bottom
        platforms.push_back(Platform(rect, smallBlackTexture));
        
        // Second V - starts with the duplicate platform in new screen
        rect = {SCREEN_WIDTH + 150, 400, 100, 100};   // Same platform duplicated at start of screen 2
        platforms.push_back(Platform(rect, smallBlackTexture));
        
        rect = {SCREEN_WIDTH + 800, 200, 100, 100};   // Top
        platforms.push_back(Platform(rect, smallBlackTexture));
        rect = {SCREEN_WIDTH + 1300, 300, 100, 100};   // Middle
        platforms.push_back(Platform(rect, smallBlackTexture));
        
        // Important: This is the screen transition platform (end of screen 2)
        rect = {SCREEN_WIDTH * 2 - 200, 400, 100, 100};   // Bottom
        platforms.push_back(Platform(rect, smallBlackTexture));
        
        // Third V - starts with the duplicate platform in new screen
        rect = {SCREEN_WIDTH * 2 + 150, 400, 100, 100};   // Same platform duplicated at start of screen 3
        platforms.push_back(Platform(rect, smallBlackTexture));
        
        rect = {SCREEN_WIDTH * 2 + 800, 200, 100, 100};   // Top
        platforms.push_back(Platform(rect, smallBlackTexture));
        rect = {SCREEN_WIDTH * 2 + 1300, 300, 100, 100};   // Middle
        platforms.push_back(Platform(rect, smallBlackTexture));

        // Finish line at the end of the sliding window
        SDL_Rect finishRect = {
            SCREEN_WIDTH * 3 - 300,  // X position - at end of screen 3
            350,                     // Y position - moved slightly lower to be reachable
            200,                     // Width
            100                      // Height
        };
        platforms.push_back(Platform(finishRect, finishTexture));

        return platforms;
    }

    static std::vector<Platform> getLevel3Platforms(SDL_Renderer* renderer) {
        std::vector<Platform> platforms;
        // Add level 3 platforms here
        return platforms;
    }

    static std::vector<Platform> getLevel4Platforms(SDL_Renderer* renderer) {
        std::vector<Platform> platforms;
        // Add level 4 platforms here
        return platforms;
    }

    static std::vector<Platform> getLevel5Platforms(SDL_Renderer* renderer) {
        std::vector<Platform> platforms;
        // Add level 5 platforms here
        return platforms;
    }

    static std::vector<Platform> getPlatformsForLevel(int level, SDL_Renderer* renderer) {
        switch (level) {
            case 1: return getLevel1Platforms(renderer);
            case 2: return getLevel2Platforms(renderer);
            case 3: return getLevel3Platforms(renderer);
            case 4: return getLevel4Platforms(renderer);
            case 5: return getLevel5Platforms(renderer);
            default: return std::vector<Platform>();
        }
    }
}; 