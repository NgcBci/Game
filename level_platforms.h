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
        
        // Load textures with correct paths
        SDL_Texture* roundPlatformTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\roundpf-Photoroom.png");
        SDL_Texture* pollTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\poll.png");
        SDL_Texture* spikesTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\spikes-Photoroom.png");
        SDL_Texture* finishTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\finish.png");

        // Create platforms for Level 3 - using 3 screens like Level 2
        // SCREEN 1
        
        // Starting round platform - positioned to match character spawn at (200, 450)
        SDL_Rect rect = {200, 500, 100, 100};
        platforms.push_back(Platform(rect, roundPlatformTexture));
        
        // Poll platforms with more spacing - SIGNIFICANTLY INCREASED spacing
        rect = {450, 200, 50, 200};  // Moved further left
        platforms.push_back(Platform(rect, pollTexture));
        
        rect = {950, 350, 50, 200};  // Moved further right - more distance between polls
        platforms.push_back(Platform(rect, pollTexture));
        
        // Add spikes on the ground - LOWERED position and moved
        rect = {700, 600, 100, 50};  // Kept in the middle but lowered
        platforms.push_back(Platform(rect, spikesTexture, true));
        
        // Screen transition platform (end of screen 1)
        rect = {SCREEN_WIDTH - 200, 450, 100, 100};
        platforms.push_back(Platform(rect, roundPlatformTexture));

        // SCREEN 2
        
        // Duplicated platform at start of screen 2
        rect = {SCREEN_WIDTH + 150, 450, 100, 100};
        platforms.push_back(Platform(rect, roundPlatformTexture));
        
        // More polls and round platforms with MUCH GREATER spacing
        rect = {SCREEN_WIDTH + 400, 250, 50, 200};  // First poll
        platforms.push_back(Platform(rect, pollTexture));
        
        rect = {SCREEN_WIDTH + 800, 200, 100, 100};  // Middle round platform - moved further right
        platforms.push_back(Platform(rect, roundPlatformTexture));
        
        rect = {SCREEN_WIDTH + 1150, 300, 50, 200};  // Last poll - moved further right
        platforms.push_back(Platform(rect, pollTexture));
        
        // More ground spikes - wider spacing
        rect = {SCREEN_WIDTH + 550, 600, 100, 50};  // First spike
        platforms.push_back(Platform(rect, spikesTexture, true));
        
        rect = {SCREEN_WIDTH + 950, 600, 100, 50};  // Second spike - moved further away
        platforms.push_back(Platform(rect, spikesTexture, true));
        
        // Screen transition platform (end of screen 2)
        rect = {SCREEN_WIDTH * 2 - 200, 450, 100, 100};
        platforms.push_back(Platform(rect, roundPlatformTexture));

        // SCREEN 3
        
        // Duplicated platform at start of screen 3
        rect = {SCREEN_WIDTH * 2 + 150, 450, 100, 100};
        platforms.push_back(Platform(rect, roundPlatformTexture));
        
        // Final section with mix of all platform types - MUCH WIDER spacing
        rect = {SCREEN_WIDTH * 2 + 400, 250, 100, 100};  // First round platform
        platforms.push_back(Platform(rect, roundPlatformTexture));
        
        rect = {SCREEN_WIDTH * 2 + 650, 200, 50, 200};  // First poll - more distant from previous
        platforms.push_back(Platform(rect, pollTexture));
        
        rect = {SCREEN_WIDTH * 2 + 1000, 300, 100, 100};  // Middle round platform - moved further apart
        platforms.push_back(Platform(rect, roundPlatformTexture));
        
        rect = {SCREEN_WIDTH * 2 + 1250, 250, 50, 200};  // Last poll - greater distance
        platforms.push_back(Platform(rect, pollTexture));
        
        // More ground spikes near finish with much greater spacing
        rect = {SCREEN_WIDTH * 2 + 750, 600, 100, 50};  // First spike
        platforms.push_back(Platform(rect, spikesTexture, true));
        
        rect = {SCREEN_WIDTH * 2 + 1150, 600, 100, 50};  // Second spike - more separation
        platforms.push_back(Platform(rect, spikesTexture, true));

        // Finish line at the end of the sliding window
        SDL_Rect finishRect = {
            SCREEN_WIDTH * 3 - 300,  // X position - at end of screen 3
            350,                     // Y position - same as Level 2
            200,                     // Width
            100                      // Height
        };
        platforms.push_back(Platform(finishRect, finishTexture));
        
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