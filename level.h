#pragma once
#include "SDL.h"
#include <vector>
#include "graphics.h"

class Level {
public:
    Level(SDL_Renderer* renderer);
    virtual ~Level();

    virtual void init() = 0;  // Pure virtual function - must be implemented by each level
    virtual void update() = 0;
    virtual void render(SDL_Renderer* renderer) = 0;
    virtual void handleEvent(SDL_Event& event) = 0;

    // Common properties for all levels
    std::vector<Platform> platforms;
    int startX, startY;  // Starting position for the character
    int finishX, finishY;  // Finish line position
    bool isCompleted;

protected:
    SDL_Renderer* renderer;
}; 