#pragma once
#include "level.h"

class Level1 : public Level {
public:
    Level1(SDL_Renderer* renderer);
    ~Level1();

    void init() override;
    void update() override;
    void render(SDL_Renderer* renderer) override;
    void handleEvent(SDL_Event& event) override;

private:
    // Level 1 specific properties
    SDL_Texture* background;
    SDL_Texture* finishLine;
    // Add any other level-specific properties here
}; 