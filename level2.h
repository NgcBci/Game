class Level2 : public Level {
private:
    SDL_Texture* backgroundTexture;
    SDL_Texture* finishLineTexture;
    double cameraOffsetX;  // Add camera offset for sliding screen
    const int SCROLL_THRESHOLD = SCREEN_WIDTH / 3;  // Start scrolling when character is 1/3 across screen

public:
    Level2() : Level(), backgroundTexture(nullptr), finishLineTexture(nullptr), cameraOffsetX(0) {}

    void init(SDL_Renderer* renderer) override {
        // Load textures
        backgroundTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\level2_background.png");
        finishLineTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\finish.png");
        
        // Initialize platforms
        platforms = getLevel2Platforms(renderer);
        
        // Set start and finish positions
        startX = 300;
        startY = 200;  // Start above the square platform
        finishX = 2000;
        finishY = 300;
    }

    void update(Character& character, double dt) override {
        // Update camera offset based on character position
        if (character.x > SCROLL_THRESHOLD) {
            cameraOffsetX = character.x - SCROLL_THRESHOLD;
        } else {
            cameraOffsetX = 0;
        }

        // Check for spike collisions
        for (const auto& platform : platforms) {
            if (platform.texture == IMG_LoadTexture(renderer, "F:\\Game\\graphic\\spikes-Photoroom.png")) {
                if (character.x + character.radius > platform.rect.x &&
                    character.x - character.radius < platform.rect.x + platform.rect.w &&
                    character.y + character.radius > platform.rect.y &&
                    character.y - character.radius < platform.rect.y + platform.rect.h) {
                    // Respawn character if touching spikes
                    character.resetPosition();
                }
            }
        }

        // Check for finish line collision
        if (character.x + character.radius > finishX &&
            character.x - character.radius < finishX + 200 &&
            character.y + character.radius > finishY &&
            character.y - character.radius < finishY + 100) {
            isComplete = true;
        }
    }

    void render(SDL_Renderer* renderer) override {
        // Render background with camera offset
        SDL_Rect bgRect = {
            static_cast<int>(-cameraOffsetX),
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        };
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect);

        // Render platforms with camera offset
        for (const auto& platform : platforms) {
            SDL_Rect platformRect = platform.rect;
            platformRect.x -= static_cast<int>(cameraOffsetX);
            SDL_RenderCopy(renderer, platform.texture, NULL, &platformRect);
        }

        // Render finish line with camera offset
        SDL_Rect finishRect = {
            static_cast<int>(finishX - cameraOffsetX),
            static_cast<int>(finishY),
            200,
            100
        };
        SDL_RenderCopy(renderer, finishLineTexture, NULL, &finishRect);
    }

    ~Level2() {
        if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
        if (finishLineTexture) SDL_DestroyTexture(finishLineTexture);
    }
}; 