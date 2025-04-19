#include "level1.h"
#include "SDL_image.h"

Level1::Level1(SDL_Renderer* renderer) : Level(renderer) {
    init();
}

Level1::~Level1() {
    if (background) SDL_DestroyTexture(background);
    if (finishLine) SDL_DestroyTexture(finishLine);
}

void Level1::init() {
    // Load level-specific textures
    background = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\level1_background.png");
    finishLine = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\finish_line.png");

    // Set up level-specific platforms
    platforms.clear();
    // Add platforms for level 1
    platforms.push_back(Platform(100, 400, 200, 20));  // Example platform
    platforms.push_back(Platform(400, 300, 200, 20));  // Example platform
    // Add more platforms as needed

    // Set start and finish positions
    startX = 50;
    startY = 500;
    finishX = 700;
    finishY = 100;

    isCompleted = false;
}

void Level1::update() {
    // Update level-specific logic
    // Check for level completion
    // Update any moving platforms or obstacles
}

void Level1::render(SDL_Renderer* renderer) {
    // Render background
    if (background) {
        SDL_RenderCopy(renderer, background, NULL, NULL);
    }

    // Render platforms
    for (const auto& platform : platforms) {
        platform.render(renderer);
    }

    // Render finish line
    if (finishLine) {
        SDL_Rect finishRect = {finishX, finishY, 50, 50};
        SDL_RenderCopy(renderer, finishLine, NULL, &finishRect);
    }
}

void Level1::handleEvent(SDL_Event& event) {
    // Handle level-specific events
    // For example, special level mechanics or triggers
} 