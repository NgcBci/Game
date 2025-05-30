#ifndef _GRAPHICS__H
#define _GRAPHICS__H
#include <SDL.h>
#include <SDL_image.h>
#include "defs.h"
#include <vector>
#include <cmath>
#include <cstdlib> // For rand()
#include <cstring> // For strstr()
#include <SDL_mixer.h>

using std::vector;

struct Particle {
    double xCurrent;
    double yCurrent;
    double xPrevious;
    double yPrevious;
    bool checkmovement;
};

struct Platform {
    SDL_Rect rect;
    SDL_Texture* texture;
    double visualHeight;  // Actual visual height of the platform texture
    bool isSpike;        // New flag to identify if this platform is a spike

    // Moving platform properties
    bool isMoving;
    float startX, endX;
    float speed;
    bool movingForward;
    bool movesVertically;  // New property to indicate vertical movement

    // Interactive platform properties
    bool isInteractive;
    SDL_Texture* alternateTexture;
    bool activated;
// constructor for basic, non moving platforms
    Platform(SDL_Rect r, SDL_Texture* t, bool spike = false)
        : rect(r), texture(t), isSpike(spike), isMoving(false),
          startX(0), endX(0), speed(0), movingForward(true), movesVertically(false),
          isInteractive(false), alternateTexture(nullptr), activated(false) {
        // Get the actual texture dimensions
        int w, h;
        SDL_QueryTexture(t, NULL, NULL, &w, &h);
        visualHeight = h;
    }

    // Constructor for moving platforms
    Platform(SDL_Rect r, SDL_Texture* t, float startPos, float endPos, float moveSpeed, bool spike = false)
        : rect(r), texture(t), isSpike(spike), isMoving(true),
          startX(startPos), endX(endPos), speed(moveSpeed), movingForward(true), movesVertically(false),
          isInteractive(false), alternateTexture(nullptr), activated(false) {
        // Get the actual texture dimensions
        int w, h;
        SDL_QueryTexture(t, NULL, NULL, &w, &h);
        visualHeight = h;
        rect.x = static_cast<int>(startX); // Initialize position
    }

    // Constructor for interactive platforms
    Platform(SDL_Rect r, SDL_Texture* t, SDL_Texture* altTexture)
        : rect(r), texture(t), isSpike(false), isMoving(false),
          startX(0), endX(0), speed(0), movingForward(true), movesVertically(false),
          isInteractive(true), alternateTexture(altTexture), activated(false) {
        // Get the actual texture dimensions
        int w, h;
        SDL_QueryTexture(t, NULL, NULL, &w, &h);
        visualHeight = h;
    }

    void update(float deltaTime = 1.0f) {
        if (!isMoving) return;
            if (movingForward) {
                rect.x += static_cast<int>(speed * deltaTime);
                if (rect.x >= endX) {
                    rect.x = static_cast<int>(endX);
                    movingForward = false;
                }
            } else {
                rect.x -= static_cast<int>(speed * deltaTime);
                if (rect.x <= startX) {
                    rect.x = static_cast<int>(startX);
                    movingForward = true;
                }
            }
        }
        // when the platform reaches endX or startX it reverses

    // Activate the interactive platform, cnay cho lv5 cho cai nut ay
    void activate() {
        if (isInteractive && !activated) {
            activated = true;
            // Swap textures
            SDL_Texture* temp = texture;
            texture = alternateTexture;
            alternateTexture = temp;
        }
    }
};

// Add struct for moving objects like the spike wall
struct MovingObject {
    SDL_Rect rect;
    SDL_Texture* texture;
    float velocityX;
    float velocityY;
    float startX;
    float startY;

    MovingObject(SDL_Rect r, SDL_Texture* t, float vx = 0, float vy = 0) :
        rect(r), texture(t), velocityX(vx), velocityY(vy), startX(r.x), startY(r.y) {}

    void update() {
        rect.x += velocityX;
        rect.y += velocityY;
    }

    void reset() {
        rect.x = startX;
        rect.y = startY;
    }
};

struct Graphics {
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture* characterTexture;
    SDL_Texture* handTexture;
    SDL_Texture* congratulationsTexture;
    SDL_Texture* guideTexture;  // Add guide texture
    SDL_Texture* acedTexture;   // Add aced texture
    std::vector<Platform> platforms;

    Graphics() : window(nullptr), renderer(nullptr), characterTexture(nullptr),
                handTexture(nullptr), congratulationsTexture(nullptr), guideTexture(nullptr),
                acedTexture(nullptr) {}

    void logErrorAndExit(const char* msg, const char* error)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "%s: %s", msg, error);
        SDL_Quit();
    }

    void init() {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
            logErrorAndExit("SDL_Init", SDL_GetError());

        // Initialize SDL_mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            logErrorAndExit("Mix_OpenAudio", Mix_GetError());
        }

        // Initialize SDL_image
        int imgFlags = IMG_INIT_PNG;

        window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if (window == nullptr) logErrorAndExit("CreateWindow", SDL_GetError());
        SDL_SetWindowResizable(window, SDL_TRUE);
        SDL_MaximizeWindow(window);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                              SDL_RENDERER_PRESENTVSYNC);

        if (renderer == nullptr) logErrorAndExit("CreateRenderer", SDL_GetError());

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Load congratulations texture
        congratulationsTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\congrat.png");
        // Load guide image
        guideTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\guidefinalroi.png");
        if (guideTexture == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                          SDL_LOG_PRIORITY_ERROR,
                          "Could not load guide image! SDL_Error: %s", SDL_GetError());
        }

        // Load aced image
        acedTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\aced-Photoroom.png");
        if (acedTexture == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                          SDL_LOG_PRIORITY_ERROR,
                          "Could not load aced image! SDL_Error: %s", SDL_GetError());
        }
    }

    void renderCongratulations() {
        if (congratulationsTexture) {
            // Center the congratulations image on screen but make it smaller
            int w, h;
            SDL_QueryTexture(congratulationsTexture, NULL, NULL, &w, &h);

            // Scale down the image to half size
            w = w / 2;
            h = h / 2;

            SDL_Rect destRect = {
                SCREEN_WIDTH/2 - w/2,  // Center horizontally
                SCREEN_HEIGHT/2 - h/2, // Center vertically
                w,                     // Scaled width
                h                      // Scaled height
            };
            SDL_RenderCopy(renderer, congratulationsTexture, NULL, &destRect);
        }
    }

    void renderHowToPlay() {
        if (guideTexture == nullptr) {
            guideTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\guidefinalroi.png");
            if (guideTexture == nullptr) {
                logErrorAndExit("Could not load guide image", IMG_GetError());
            }
        }

        int guideWidth, guideHeight;
        SDL_QueryTexture(guideTexture, NULL, NULL, &guideWidth, &guideHeight);

        // Center the guide image on screen
        SDL_Rect guideRect = {
            (SCREEN_WIDTH - guideWidth) / 2,
            (SCREEN_HEIGHT - guideHeight) / 2 + 25, // Decreased from 30 to 25 to move it up 5 more pixels
            guideWidth,
            guideHeight
        };

        SDL_RenderCopy(renderer, guideTexture, NULL, &guideRect);
    }

    void renderAced() {
        if (!acedTexture) return;

        // Get texture dimensions
        int texWidth, texHeight;
        SDL_QueryTexture(acedTexture, NULL, NULL, &texWidth, &texHeight);

        // Calculate smaller size (70% of original)
        int newWidth = static_cast<int>(texWidth * 0.7);
        int newHeight = static_cast<int>(texHeight * 0.7);

        // Calculate position to center on screen
        SDL_Rect destRect = {
            (SCREEN_WIDTH - newWidth) / 2,
            (SCREEN_HEIGHT - newHeight) / 2,
            newWidth,
            newHeight
        };

        // Render the texture
        SDL_RenderCopy(renderer, acedTexture, NULL, &destRect);
    }

    ~Graphics() {
        if (window != nullptr) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        if (renderer != nullptr) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if (characterTexture != nullptr) {
            SDL_DestroyTexture(characterTexture);
            characterTexture = nullptr;
        }
        if (handTexture != nullptr) {
            SDL_DestroyTexture(handTexture);
            handTexture = nullptr;
        }
        if (congratulationsTexture != nullptr) {
            SDL_DestroyTexture(congratulationsTexture);
            congratulationsTexture = nullptr;
        }
        if (guideTexture != nullptr) {
            SDL_DestroyTexture(guideTexture);
            guideTexture = nullptr;
        }
        if (acedTexture != nullptr) {
            SDL_DestroyTexture(acedTexture);
            acedTexture = nullptr;
        }

        // Cleanup platforms
        for (auto& platform : platforms) {
            if (platform.texture) {
                SDL_DestroyTexture(platform.texture);
            }
        }
    }
};

class ropehand {
public:
    bool isGrabbingObject;
    static const int jakobsenit = 10;  // Reduced for more stable physics
    vector<Particle> parti;
    double maxLength;
    double currentLength;
    SDL_Texture* handTexture;
    SDL_Texture* grabTexture;  // New texture for grabbing state
    bool isLeftHand;
    // Add color property for the rope
    SDL_Color ropeColor;

    // Keep platform tracking for debugging purposes
    int grabbedPlatformIndex;  // Index of the grabbed platform in platforms vector

    ropehand(SDL_Renderer* renderer, double x1, double x2, double y1, double y2, int numberofparticles, bool isLeft)
        : isGrabbingObject(false), isLeftHand(isLeft), grabbedPlatformIndex(-1) {
        maxLength = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));  // Calculate maximum length
        currentLength = maxLength;

        // Default rope color is red
        ropeColor = {255, 0, 0, 255};  // Red color (R,G,B,A)

        // Load release hand texture based on hand type
        const char* releasePath = isLeft ? "F:\\Game\\graphic\\lefthandrelease.png" : "F:\\Game\\graphic\\righthandrelease.png";
        SDL_Surface* surface = IMG_Load(releasePath);
        if (surface) {
            handTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
        // Load grab hand texture based on hand type
        const char* grabPath = isLeft ? "F:\\Game\\graphic\\grableft.png" : "F:\\Game\\graphic\\grabright.png";
        surface = IMG_Load(grabPath);
        if (surface) {
            grabTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
        //cong thuc li: position = (1 - t) * start + t * end(cong thuc noi suy tuyen tinh de suy ra vi tri cua tung particle)
        for (int i = 0; i < numberofparticles; i++) {
            double weightforlerp = (double)i/(numberofparticles - 1);
            double x = (1 - weightforlerp)*x1 + weightforlerp*x2;
            double y = (1 - weightforlerp)*y1 + weightforlerp*y2;
            Particle pn;
            pn.xCurrent = x;
            pn.yCurrent = y;
            pn.xPrevious = x;
            pn.yPrevious = y;
            pn.checkmovement = (i == 0);  // First particle is fixed to body
            parti.push_back(pn);
        } // vdu parti 5 hat thi co cac vi tri la 0, 0,25, 0,5...
        int segments = numberofparticles - 1;
        desireddistance = maxLength/segments; // dam bao cac hat cach nhau 1 khoang nhat dinh
    }

    ~ropehand() {
        if (handTexture) {
            SDL_DestroyTexture(handTexture);
            handTexture = nullptr;
        }
        if (grabTexture) {
            SDL_DestroyTexture(grabTexture);
            grabTexture = nullptr;
        }
    }

    void render(SDL_Renderer* renderer) {
        // Use the rope color for drawing the rope
        SDL_SetRenderDrawColor(renderer, ropeColor.r, ropeColor.g, ropeColor.b, ropeColor.a);

        for (size_t i = 0; i < parti.size() - 1; i++) {
            // Draw multiple lines to create a thicker rope
            for (int offset = -2; offset <= 2; offset++) {
                SDL_RenderDrawLine(renderer,
                    static_cast<int>(parti[i].xCurrent),
                    static_cast<int>(parti[i].yCurrent + offset),
                    static_cast<int>(parti[i + 1].xCurrent),
                    static_cast<int>(parti[i + 1].yCurrent + offset));
            }
            for (int offset = -2; offset <= 2; offset++) {
                SDL_RenderDrawLine(renderer,
                    static_cast<int>(parti[i].xCurrent + offset),
                    static_cast<int>(parti[i].yCurrent),
                    static_cast<int>(parti[i + 1].xCurrent + offset),
                    static_cast<int>(parti[i + 1].yCurrent));
            }
        }

        // Draw hand at the end of the rope, check xem co grab ko, grab thi load anh grab
        SDL_Texture* currentTexture = isGrabbingObject ? grabTexture : handTexture;
        if (currentTexture) {
            int handWidth = 40;
            int handHeight = 40;

            // tính hướng dây đoạn cuối, 2 cái parti cuối ấy, xoay tay theo hướng dây, tn2(dx,y) là góc giữa trục ox và vector dxx dy
            double dx = parti.back().xCurrent - parti[parti.size()-2].xCurrent;
            double dy = parti.back().yCurrent - parti[parti.size()-2].yCurrent;
            double angle = atan2(dy, dx) * 180.0 / M_PI;
// vdu dx = 1, dy = 0 ko xoay, dx =0, dy = -1 xoay -90
            // Calculate hand position based on rope angle
            double handX = parti.back().xCurrent;
            double handY = parti.back().yCurrent + 10; // Move hand down to connect with rope
// dịch 1 tí tại lệch với hình vẽ
            // Create destination rectangle
            SDL_Rect handRect = {
                static_cast<int>(handX - handWidth/2),
                static_cast<int>(handY - handHeight/2),
                handWidth,
                handHeight
            };

            // Calculate center point for rotation
            SDL_Point center = {handWidth/2, handHeight/2};
// hàm để xoay ảnh
            // Render with rotation
            SDL_RenderCopyEx(renderer, currentTexture, NULL, &handRect, angle, &center, SDL_FLIP_NONE);
        }
    }

    void step() {
    verletintergraion();
    enforceConstraint();
    }

    void attachtothebody(double bodyx, double bodyy, double bodywidth, bool islefthand) {
        // Update the first particle (attachment point)
        if (islefthand) {
            parti[0].xCurrent = bodyx - (bodywidth/2);
        } else {
            parti[0].xCurrent = bodyx + (bodywidth/2);
        }
    parti[0].yCurrent = bodyy;
    parti[0].xPrevious = parti[0].xCurrent;
    parti[0].yPrevious = parti[0].yCurrent;

        // If not grabbing, keep the hand within max length
        if (!isGrabbingObject) {
            double dx = parti.back().xCurrent - parti[0].xCurrent;
            double dy = parti.back().yCurrent - parti[0].yCurrent;
            double length = sqrt(dx*dx + dy*dy);

            if (length > maxLength) {
                double ratio = maxLength / length;
                parti.back().xCurrent = parti[0].xCurrent + dx * ratio;
                parti.back().yCurrent = parti[0].yCurrent + dy * ratio;
            }
        }
    }

    // Standard grab method for non-moving objects
    void grab(double grabx, double graby) {
        // Update the last particle's position to the grab point
        parti.back().xCurrent = grabx;
        parti.back().yCurrent = graby;
        parti.back().checkmovement = true;
        isGrabbingObject = true;

        // Reset platform tracking when doing a standard grab
        grabbedPlatformIndex = -1;
    }

    void release() {
         parti.back().checkmovement = false;
        isGrabbingObject = false;
        grabbedPlatformIndex = -1;
    }

    void handlecollision(const std::vector<Platform>& platforms) {
        const double BOUNCE = 0.1; // Reduced bounce factor

        // Check each particle in the rope except the fixed ones
        for (auto& particle : parti) {
            // Skip fixed particles (first and possibly last particle)
            if (particle.checkmovement) continue;

            for (const auto& platform : platforms) {
                // Get current velocity
                double velX = particle.xCurrent - particle.xPrevious;
                double velY = particle.yCurrent - particle.yPrevious;

                // Adjust velocity for moving platforms
                if (platform.isMoving) {
                    // Add platform's movement to relative velocity
                    float platformVelocity = platform.speed * (platform.movingForward ? 1.0f : -1.0f);
                    velX -= platformVelocity * 0.1f; // Same scale as used in main.cpp
                }

                // Check current position and predicted next position
                double nextX = particle.xCurrent + velX;
                double nextY = particle.yCurrent + velY;

                // Line segment intersection test between current and next position
                bool collision = false;

                // Check top edge of platform
                if (velY > 0 && // Moving downward
                    particle.yCurrent <= platform.rect.y && nextY >= platform.rect.y &&
                    particle.xCurrent + velX >= platform.rect.x &&
                    particle.xCurrent + velX <= platform.rect.x + platform.rect.w) {
                    // Collision with top of platform
                    particle.yCurrent = platform.rect.y;
                    particle.yPrevious = particle.yCurrent - velY * BOUNCE;
                    collision = true;
                }

                // Check bottom edge of platform
                else if (velY < 0 && // Moving upward
                    particle.yCurrent >= platform.rect.y + platform.rect.h &&
                    nextY <= platform.rect.y + platform.rect.h &&
                    particle.xCurrent + velX >= platform.rect.x &&
                    particle.xCurrent + velX <= platform.rect.x + platform.rect.w) {
                    // Collision with bottom of platform
                    particle.yCurrent = platform.rect.y + platform.rect.h;
                    particle.yPrevious = particle.yCurrent - velY * BOUNCE;
                    collision = true;
                }

                // Check left edge of platform
                if (!collision && velX > 0 && // Moving right
                    particle.xCurrent <= platform.rect.x && nextX >= platform.rect.x &&
                    particle.yCurrent + velY >= platform.rect.y &&
                    particle.yCurrent + velY <= platform.rect.y + platform.rect.h) {
                    // Collision with left of platform
                    particle.xCurrent = platform.rect.x;
                    particle.xPrevious = particle.xCurrent - velX * BOUNCE;
                }

                // Check right edge of platform
                else if (!collision && velX < 0 && // Moving left
                    particle.xCurrent >= platform.rect.x + platform.rect.w &&
                    nextX <= platform.rect.x + platform.rect.w &&
                    particle.yCurrent + velY >= platform.rect.y &&
                    particle.yCurrent + velY <= platform.rect.y + platform.rect.h) {
                    // Collision with right of platform
                    particle.xCurrent = platform.rect.x + platform.rect.w;
                    particle.xPrevious = particle.xCurrent - velX * BOUNCE;
                }
            }
        }
    }

private:
    const double dt = 0.016;
    double desireddistance;

    void verletintergraion() {
        double damping = 0.99;  // Increased damping for better momentum preservation
    for (Particle &pn : parti) {
            if (pn.checkmovement) continue;

        double prevX = pn.xCurrent;
        double prevY = pn.yCurrent;

        double velocityX = (pn.xCurrent - pn.xPrevious) * damping;
        double velocityY = (pn.yCurrent - pn.yPrevious) * damping;

            // Apply less gravity to rope particles for smoother rope movement
            velocityY += 5.0 * dt; // Reduced from 9.81 for rope particles

        pn.xCurrent += velocityX;
        pn.yCurrent += velocityY;

        pn.xPrevious = prevX;
        pn.yPrevious = prevY;
    }
}

    void enforceConstraint() {
        const int iterations = isGrabbingObject ? 20 : jakobsenit;  // More iterations when grabbing
        for (int i = 0; i < iterations; i++) {
            for (size_t j = 1; j < parti.size(); j++) {
                Particle &p1 = parti[j-1];
                Particle &p2 = parti[j];
                double distance = sqrt(pow(p1.xCurrent - p2.xCurrent, 2) + pow(p1.yCurrent - p2.yCurrent, 2));
                double distanceError = distance - desireddistance;

                double xDifference = p2.xCurrent - p1.xCurrent;
                double yDifference = p2.yCurrent - p1.yCurrent;

                // Avoid division by zero
                double totalDifference = sqrt(pow(xDifference, 2) + pow(yDifference, 2));
                if (totalDifference < 0.0001) continue;

                double xDirection = xDifference / totalDifference;
                double yDirection = yDifference / totalDifference;

                double correction = isGrabbingObject ? 1.0 : 0.5;  // Full correction when grabbing
// p1 fixed , p2 ko fixed thi p2 dich ve p1
                if (p1.checkmovement && !p2.checkmovement) {
                    p2.xCurrent -= xDirection * distanceError * correction;
                    p2.yCurrent -= yDirection * distanceError * correction;
                }
                else if (p2.checkmovement && !p1.checkmovement) {
                    p1.xCurrent += xDirection * distanceError * correction;
                    p1.yCurrent += yDirection * distanceError * correction;
                }
                else if (!p1.checkmovement && !p2.checkmovement) {
                    p2.xCurrent -= xDirection * distanceError * correction * 0.5;
                    p2.yCurrent -= yDirection * distanceError * correction * 0.5;
                    p1.xCurrent += xDirection * distanceError * correction * 0.5;
                    p1.yCurrent += yDirection * distanceError * correction * 0.5;
                }
            }
        }
    }
};

class Character {
public:
    double x, y;
    double vx, vy;
    double radius;
    ropehand leftHand;
    ropehand rightHand;
    SDL_Texture *texture;
    bool hasReachedFinish;
    bool showingCongratulations;  // New flag for congratulations state
    // Track swing state and movement
    double lastSwingAngle = 0;
    double maxSwingSpeed = 0;
    bool isSwingingRight = false;
    double swingEnergy = 0;
    double facingDirection = 1.0; // 1.0 for right, -1.0 for left

    // Add a function to set the character texture at runtime
    void setTexture(SDL_Renderer* renderer, const char* texturePath) {
        // Free old texture if it exists
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }

        // Try to load the new image
        SDL_Surface* originalSurface = IMG_Load(texturePath);
        if (!originalSurface) {
            return;
        }

        // Create a new surface at the desired size (2*radius x 2*radius)
        SDL_Surface* scaledSurface = SDL_CreateRGBSurface(0, radius * 2, radius * 2, 32,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

        if (!scaledSurface) {
            SDL_FreeSurface(originalSurface);
            return;
        }

        // Scale the image to match the collision size
        SDL_BlitScaled(originalSurface, NULL, scaledSurface, NULL);

        // Create texture from the scaled surface
        texture = SDL_CreateTextureFromSurface(renderer, scaledSurface);

        // Clean up surfaces
        SDL_FreeSurface(originalSurface);
        SDL_FreeSurface(scaledSurface);

        // Free existing hand textures first to prevent memory leaks
        if (leftHand.handTexture) {
            SDL_DestroyTexture(leftHand.handTexture);
            leftHand.handTexture = nullptr;
        }
        if (leftHand.grabTexture) {
            SDL_DestroyTexture(leftHand.grabTexture);
            leftHand.grabTexture = nullptr;
        }
        if (rightHand.handTexture) {
            SDL_DestroyTexture(rightHand.handTexture);
            rightHand.handTexture = nullptr;
        }
        if (rightHand.grabTexture) {
            SDL_DestroyTexture(rightHand.grabTexture);
            rightHand.grabTexture = nullptr;
        }

        // Check if this is the mint character
        if (strstr(texturePath, "mintchar") != nullptr) {
            // Load mint-specific hand textures
            leftHand.handTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\mintleftrelease-Photoroom - Copy.png");
            leftHand.grabTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\mintrightgrab-Photoroom.png");
            rightHand.handTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\mintleftrelease-Photoroom.png");
            rightHand.grabTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\mintrightgrab-Photoroom - Copy.png");

            // Set mint color for ropes (a light mint/teal color)
            leftHand.ropeColor = {0, 200, 150, 255};  // Mint/teal color for mint character
            rightHand.ropeColor = {0, 200, 150, 255};
        }
        // Check if this is the black character
        else if (strstr(texturePath, "blackchar") != nullptr) {
            // Load black character-specific hand textures
            rightHand.handTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\blackreleaseleft-Photoroom.png");
            leftHand.handTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\blackreleaseleft-Photoroom - Copy.png");
            rightHand.grabTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\blacklefthand-Photoroom - Copy - Copy.png");
            leftHand.grabTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\blacklefthand-Photoroom - Copy.png");

            // Set black color for ropes
            SDL_Color blackColor = {30, 30, 30, 255};  // Dark black color with a bit of visibility
            leftHand.ropeColor = blackColor;
            rightHand.ropeColor = blackColor;
        }
        // Check if this is the Sabrina character
        else if (strstr(texturePath, "3735783d") != nullptr) {
            // Load Sabrina-specific hand textures
            rightHand.handTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\sabrinaleftrelease-removebg-preview.png");
            leftHand.handTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\sabrinaleftrelease-removebg-preview - Copy.png");
            leftHand.grabTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\sabrinaleftgrab-removebg-preview - Copy.png");
            rightHand.grabTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\sabrinaleftgrab-removebg-preview.png");

            // Set muted yellow color for ropes
            SDL_Color mutedYellow = {220, 180, 50, 255};  // Muted yellow color
            leftHand.ropeColor = mutedYellow;
            rightHand.ropeColor = mutedYellow;
        }
        else {
            // Load default hand textures for other characters
            leftHand.handTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\lefthandrelease.png");
            leftHand.grabTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\grableft.png");
            rightHand.handTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\righthandrelease.png");
            rightHand.grabTexture = IMG_LoadTexture(renderer, "F:\\Game\\graphic\\grabright.png");

            // Reset to default red color for ropes
            leftHand.ropeColor = {255, 0, 0, 255};  // Red color
            rightHand.ropeColor = {255, 0, 0, 255};
        }
    }

    // External variables from main.cpp that we need to modify
    static int& getCurrentScreenIndex() {
        extern int currentScreenIndex;
        return currentScreenIndex;
    }

    static double& getCameraOffsetX() {
        extern double cameraOffsetX;
        return cameraOffsetX;
    }

    // Add platforms reference for moving platform interaction
    std::vector<Platform> currentPlatforms;

    Character(SDL_Renderer* renderer, double startX, double startY, double r, int handParticles)
        : x(startX), y(startY - 500), vx(0), vy(0), radius(r),  // Start 500 pixels above the platform
          // Initialize hands at the edges of the character with medium length ropes (35 pixels)
          leftHand(renderer, startX - r, startX - r - 35, startY - 500, startY - 500 - 35, handParticles, true),
          rightHand(renderer, startX + r, startX + r + 35, startY - 500, startY - 500 - 35, handParticles, false),
          hasReachedFinish(false),
          showingCongratulations(false)  // Initialize new flag
    {
        const char* texturePath = "F:\\Game\\graphic\\character.png";
        SDL_Surface* originalSurface = IMG_Load(texturePath);
        // Create a new surface at the desired size (2*radius x 2*radius)
        SDL_Surface* scaledSurface = SDL_CreateRGBSurface(0, radius * 2, radius * 2, 32,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        // Scale the image to match the collision size
        SDL_BlitScaled(originalSurface, NULL, scaledSurface, NULL);

        // Create texture from the scaled surface
        texture = SDL_CreateTextureFromSurface(renderer, scaledSurface);

        // Clean up surfaces
        SDL_FreeSurface(originalSurface);
        SDL_FreeSurface(scaledSurface);
    }

    ~Character() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }

    void applySwingForces(const Uint8* keystate) {
        // Handle case when both hands are grabbing
        if (leftHand.isGrabbingObject && rightHand.isGrabbingObject) {
            // Get both grab points
            double leftX = leftHand.parti.back().xCurrent;
            double leftY = leftHand.parti.back().yCurrent;
            double rightX = rightHand.parti.back().xCurrent;
            double rightY = rightHand.parti.back().yCurrent;

            // Calculate midpoint between hands
            double midX = (leftX + rightX) / 2.0;
            double midY = (leftY + rightY) / 2.0;

            // Calculate distance between hands
            double handDistance = sqrt(pow(rightX - leftX, 2) + pow(rightY - leftY, 2));

            // Maximum allowed distance between character and midpoint
            double maxDistance = handDistance * 0.5; // Character can't be more than half the hand distance away

            // Calculate current distance from midpoint
            double currentDistance = sqrt(pow(x - midX, 2) + pow(y - midY, 2));

            // If character is too far from midpoint, pull it back
            if (currentDistance > maxDistance) {
                double ratio = maxDistance / currentDistance;
                x = midX + (x - midX) * ratio;
                y = midY + (y - midY) * ratio;
            }

            // Apply gravity
            vy += GRAVITY * 0.7 * dt;
            return;
        } // dam bao nhan vat ko qua midpoint

        // Original single-hand grabbing code
        if (!(leftHand.isGrabbingObject ^ rightHand.isGrabbingObject)) {
            // In free fall, just apply gravity
            vy += GRAVITY * dt;
            return;
        }

        // Get the pivot point (the grabbing hand's position)
        double pivotX, pivotY;
        bool isRightHandGrabbing = rightHand.isGrabbingObject;
        ropehand& freeHand = isRightHandGrabbing ? leftHand : rightHand;

        if (isRightHandGrabbing) {
            pivotX = rightHand.parti.back().xCurrent;
            pivotY = rightHand.parti.back().yCurrent;
        } else {
            pivotX = leftHand.parti.back().xCurrent;
            pivotY = leftHand.parti.back().yCurrent;
        }

        // Get input direction
        double inputX = 0, inputY = 0;
        if (keystate[SDL_SCANCODE_UP]) inputY -= 1;
        if (keystate[SDL_SCANCODE_DOWN]) inputY += 1;
        if (keystate[SDL_SCANCODE_LEFT]) inputX -= 1;
        if (keystate[SDL_SCANCODE_RIGHT]) inputX += 1;

        // For the free hand - let natural rope physics handle it most of the time,
        // but apply an upward force on UP key specifically
        double& handX = freeHand.parti.back().xCurrent;
        double& handY = freeHand.parti.back().yCurrent;
        double& handVX = freeHand.parti.back().xPrevious;  // Using previous position to store velocity
        double& handVY = freeHand.parti.back().yPrevious;

        // Calculate current hand velocity
        double handVelocityX = handX - handVX;
        double handVelocityY = handY - handVY;

        // Apply gentler upward force when UP key is pressed - special case
        if (inputY < 0) {
            // Apply upward force to the free hand - more gradual
            handVelocityY += inputY * HAND_FORCE * 0.8 * dt; // Reduced to 80% for more visibility

            // Add small lift to body for air climbing (reduced)
            vy -= FLAP_LIFT * 0.7 * dt;
        }
        //lên thì nâng ng lên cho tay free

        // Apply horizontal force to free hand for directional swing control
        if (inputX != 0) {
            handVelocityX += inputX * HAND_FORCE * 0.6 * dt;
        }
        // dua vao phim mui ten ma tang hay giam velo

        // Apply minimal damping to preserve momentum
        handVelocityX *= 0.98; // Slightly more damping for smoother motion
        handVelocityY *= 0.98;

        // Update hand position with gentler movement
        handX = handVX + handVelocityX;
        handY = handVY + handVelocityY;

        // Store velocity for next frame
        handVX = handX - handVelocityX;
        handVY = handY - handVelocityY;

        // Calculate tangential velocity (velocity along the swing arc)
        double dx = x - pivotX; // x dis tu Pivot den C
        double dy = y - pivotY;
        double distance = sqrt(dx*dx + dy*dy);

        if (distance < 0.0001) return; // Avoid division by zero

        // Calculate tangent direction (perpendicular to radius)
        double tangentX = -dy / distance;
        double tangentY = dx / distance; // de lay vecto chuan hoa cua vecto vuong goc

        // Project current velocity onto tangent
        double tangentialSpeed = vx * tangentX + vy * tangentY; // how fast ủr going from side to side

        // Apply gravity as a tangential force - increased for better swinging
        double gravityAngle = atan2(dy, dx);
        double gravityTangentialComponent = GRAVITY * 1.2 * cos(gravityAngle); // Increased gravity effect for better swing, goc tu pivot den body
        tangentialSpeed += gravityTangentialComponent * dt;

        // Normalize input direction for body control
        double inputLength = sqrt(inputX*inputX + inputY*inputY);
        if (inputLength > 0) {
            inputX /= inputLength;
            inputY /= inputLength;

            // Apply input to tangential speed - for body swing control
            double controlForce = 700.0; // Increased force for better swinging
            double swingControl = (inputX * tangentX + inputY * tangentY) * controlForce * dt; // dot product cua input va cai goc quay
            tangentialSpeed += swingControl;

            // Apply slight upward boost when pressing Up, to make higher jumps possible
            if (inputY < 0) {
                // Apply reduced upward component to the body when pressing up
                vy -= 40.0 * dt; // Reduced from 80.0
            }

            // Add extra horizontal momentum when pressing left/right
            // This helps create more of a tossing motion
            if (inputX != 0) {
                // Add extra horizontal momentum in the direction of input
                vx += inputX * 50.0 * dt;
            }
        }

        // Apply a boost at the bottom of the swing (when gravity is pulling downward)
        // This creates a more natural swinging motion and builds momentum
        if (dy > 0 && tangentialSpeed != 0) {
            double boostMultiplier = std::min(1.0, std::abs(dy) / (ROPE_LENGTH * 0.75));
            double boostForce = 250.0 * boostMultiplier; // Increased boost for better jumps
            tangentialSpeed += (tangentialSpeed > 0 ? boostForce : -boostForce) * dt;

            // Add slight upward impulse at bottom of swing for better arc (reduced)
            if (std::abs(dx) < ROPE_LENGTH * 0.5) {
                vy -= 10.0 * boostMultiplier * dt; // Reduced from 20.0
            }
        }

        // Update position based on tangential velocity
        x += tangentX * tangentialSpeed * dt;
        y += tangentY * tangentialSpeed * dt;

        // Maintain rope length constraint
        dx = x - pivotX;
        dy = y - pivotY;
        distance = sqrt(dx*dx + dy*dy);

        if (distance > ROPE_LENGTH) {
            double ratio = ROPE_LENGTH / distance;
            x = pivotX + dx * ratio;
            y = pivotY + dy * ratio;
        }

        // Store velocity for when we release
        vx = tangentX * tangentialSpeed;
        vy = tangentY * tangentialSpeed;

        // Very minimal damping to preserve momentum for jumping
        vx *= 0.999;
        vy *= 0.999;
    }

    void update() {
        // Get keyboard state for swing controls
        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        // Remove hardcoded finish line detection - this is now handled in main.cpp

        // If showing congratulations, only check for C or Q keys
        if (showingCongratulations) {
            if (keystate[SDL_SCANCODE_C]) {
                // Reset for next level
                hasReachedFinish = false;
                showingCongratulations = false;
                x = 300;  // Reset position
                y = 100;
                vx = 0;
                vy = 0;
                leftHand.release();
                rightHand.release();

                // Reset hand positions relative to character position
                for (size_t i = 0; i < leftHand.parti.size(); i++) {
                    double t = static_cast<double>(i) / (leftHand.parti.size() - 1);
                    leftHand.parti[i].xCurrent = x - radius - (35 * t);  // Spread particles left
                    leftHand.parti[i].yCurrent = y;
                    leftHand.parti[i].xPrevious = leftHand.parti[i].xCurrent;
                    leftHand.parti[i].yPrevious = leftHand.parti[i].yCurrent;
                }

                for (size_t i = 0; i < rightHand.parti.size(); i++) {
                    double t = static_cast<double>(i) / (rightHand.parti.size() - 1);
                    rightHand.parti[i].xCurrent = x + radius + (35 * t);  // Spread particles right
                    rightHand.parti[i].yCurrent = y;
                    rightHand.parti[i].xPrevious = rightHand.parti[i].xCurrent;
                    rightHand.parti[i].yPrevious = rightHand.parti[i].yCurrent;
                }
            }
            else if (keystate[SDL_SCANCODE_Q]) {
                SDL_Event quitEvent;
                quitEvent.type = SDL_QUIT;
                SDL_PushEvent(&quitEvent);
            }
            return;
        }

        // Check if character is out of bounds and respawn if needed
        if (y > SCREEN_HEIGHT + 500 || y < -500 ||
            x > SCREEN_WIDTH + 500 || x < -500) {
            // Reset position to initial position
            x = 300;  // Initial x position
            y = 100;  // Initial y position
            vx = 0;
            vy = 0;

            // Reset hands
            leftHand.release();
            rightHand.release();

            // Reset hand positions relative to character position
            for (size_t i = 0; i < leftHand.parti.size(); i++) {
                double t = static_cast<double>(i) / (leftHand.parti.size() - 1);
                leftHand.parti[i].xCurrent = x - radius - (35 * t);  // Spread particles left
                leftHand.parti[i].yCurrent = y;
                leftHand.parti[i].xPrevious = leftHand.parti[i].xCurrent;
                leftHand.parti[i].yPrevious = leftHand.parti[i].yCurrent;
            }

            for (size_t i = 0; i < rightHand.parti.size(); i++) {
                double t = static_cast<double>(i) / (rightHand.parti.size() - 1);
                rightHand.parti[i].xCurrent = x + radius + (35 * t);  // Spread particles right
                rightHand.parti[i].yCurrent = y;
                rightHand.parti[i].xPrevious = rightHand.parti[i].xCurrent;
                rightHand.parti[i].yPrevious = rightHand.parti[i].yCurrent;
            }

            // Reset swing tracking
            maxSwingSpeed = 0;
            swingEnergy = 0;
            return;
        }
        // Update movement direction based on input
        if (keystate[SDL_SCANCODE_LEFT]) facingDirection = -1.0;
        if (keystate[SDL_SCANCODE_RIGHT]) facingDirection = 1.0;

        // Update both hands
        leftHand.step();
        rightHand.step();

        // Apply swing forces before gravity
        applySwingForces(keystate);

        // Always apply gravity, but less when swinging to preserve momentum
        if (leftHand.isGrabbingObject || rightHand.isGrabbingObject) {
            // Reduced gravity when holding on to something
            vy += GRAVITY * 0.7 * dt;
        } else {
            // Full gravity in free fall
            vy += GRAVITY * dt;
        }

        // Track swing energy for better jumps
        if (leftHand.isGrabbingObject || rightHand.isGrabbingObject) {
            // Current swing speed
            double swingSpeed = sqrt(vx*vx + vy*vy);
            if (swingSpeed > maxSwingSpeed) {
                maxSwingSpeed = swingSpeed;
            }

            // Update swing energy based on current velocity
            swingEnergy = 0.8 * swingEnergy + 0.2 * swingSpeed;

            // Update facing direction based on movement if significant
            if (abs(vx) > 50.0) {
                facingDirection = (vx > 0) ? 1.0 : -1.0;
            }
        } else {
            // Gradually reduce swing energy when not holding
            swingEnergy *= 0.95;
            maxSwingSpeed = 0;
        }

        // Limit fall speed but allow higher horizontal speed
        if (vy > MAX_SPEED) vy = MAX_SPEED;
        if (abs(vx) > MAX_SPEED * 1.2) vx = (vx > 0) ? MAX_SPEED * 1.2 : -MAX_SPEED * 1.2;

        // Apply less drag when swinging for better momentum preservation
        if (leftHand.isGrabbingObject || rightHand.isGrabbingObject) {
            vx *= 0.995; // Very minimal drag during swing
        } else {
            vx *= DRAG; // Normal drag in free fall
        }

        // Update position
        x += vx * dt;
        y += vy * dt;

        // Keep hands attached at the edges of the character
        leftHand.attachtothebody(x - radius, y, radius * 0.2, true);
        rightHand.attachtothebody(x + radius, y, radius * 0.2, false);
    }

    // Add a reference to platforms for the update method to use
    void setPlatformsReference(const std::vector<Platform>& platforms) {
        currentPlatforms = platforms;
    }

    void grab(bool isLeft, const std::vector<Platform>& platforms) {
        double handX, handY;
        if (isLeft) {
            handX = leftHand.parti.back().xCurrent;
            handY = leftHand.parti.back().yCurrent;
        } else {
            handX = rightHand.parti.back().xCurrent;
            handY = rightHand.parti.back().yCurrent;
        }

        for (const auto& platform : platforms) {
            if (handX >= platform.rect.x && handX <= platform.rect.x + platform.rect.w &&
                handY >= platform.rect.y && handY <= platform.rect.y + platform.rect.h) {
                if (isLeft) {
                    // Set fixed grab point exactly at current position
                    leftHand.parti.back().xCurrent = handX;
                    leftHand.parti.back().yCurrent = handY;
                    leftHand.parti.back().xPrevious = handX;
                    leftHand.parti.back().yPrevious = handY;
                    leftHand.parti.back().checkmovement = true;
                    leftHand.isGrabbingObject = true;

                    // If it's a moving platform, store position information
                    if (platform.isMoving) {
                        leftHand.grabbedPlatformIndex = -1;  // We'll handle movement directly in main.cpp
                        for (int i = 0; i < platforms.size(); i++) {
                            if (&platform == &platforms[i]) {
                                leftHand.grabbedPlatformIndex = i;
                                break;
                            }
                        }
                    }
                } else {
                    // Set fixed grab point exactly at current position
                    rightHand.parti.back().xCurrent = handX;
                    rightHand.parti.back().yCurrent = handY;
                    rightHand.parti.back().xPrevious = handX;
                    rightHand.parti.back().yPrevious = handY;
                    rightHand.parti.back().checkmovement = true;
                    rightHand.isGrabbingObject = true;

                    // If it's a moving platform, store position information
                    if (platform.isMoving) {
                        rightHand.grabbedPlatformIndex = -1;  // We'll handle movement directly in main.cpp
                        for (int i = 0; i < platforms.size(); i++) {
                            if (&platform == &platforms[i]) {
                                rightHand.grabbedPlatformIndex = i;
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    // New method that can handle camera offset
    void grabWithCamera(bool isLeft, const std::vector<Platform>& platforms, double cameraOffsetX) {
        double handX, handY;
        if (isLeft) {
            handX = leftHand.parti.back().xCurrent;
            handY = leftHand.parti.back().yCurrent;
        } else {
            handX = rightHand.parti.back().xCurrent;
            handY = rightHand.parti.back().yCurrent;
        }

        // Add camera offset to hand position for the check
        handX += cameraOffsetX;

        for (const auto& platform : platforms) {
            if (handX >= platform.rect.x && handX <= platform.rect.x + platform.rect.w &&
                handY >= platform.rect.y && handY <= platform.rect.y + platform.rect.h) {

                // Adjust grab position back to screen space
                double screenSpaceHandX = handX - cameraOffsetX;

                if (isLeft) {
                    // Set fixed grab point exactly at current position
                    leftHand.parti.back().xCurrent = screenSpaceHandX;
                    leftHand.parti.back().yCurrent = handY;
                    leftHand.parti.back().xPrevious = screenSpaceHandX;
                    leftHand.parti.back().yPrevious = handY;
                    leftHand.parti.back().checkmovement = true;
                    leftHand.isGrabbingObject = true;

                    // If it's a moving platform, store position information
                    if (platform.isMoving) {
                        leftHand.grabbedPlatformIndex = -1;  // We'll handle movement directly in main.cpp
                        for (int i = 0; i < platforms.size(); i++) {
                            if (&platform == &platforms[i]) {
                                leftHand.grabbedPlatformIndex = i;
                                break;
                            }
                        }
                    }
                } else {
                    // Set fixed grab point exactly at current position
                    rightHand.parti.back().xCurrent = screenSpaceHandX;
                    rightHand.parti.back().yCurrent = handY;
                    rightHand.parti.back().xPrevious = screenSpaceHandX;
                    rightHand.parti.back().yPrevious = handY;
                    rightHand.parti.back().checkmovement = true;
                    rightHand.isGrabbingObject = true;

                    // If it's a moving platform, store position information
                    if (platform.isMoving) {
                        rightHand.grabbedPlatformIndex = -1;  // We'll handle movement directly in main.cpp
                        for (int i = 0; i < platforms.size(); i++) {
                            if (&platform == &platforms[i]) {
                                rightHand.grabbedPlatformIndex = i;
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    void release(bool isLeft) {
        double releaseBoost = JUMP_BOOST;

        // Calculate boost based on swing energy and direction
        if (swingEnergy > 100) {
            // Additional boost based on accumulated swing energy
            releaseBoost += 0.3 * (swingEnergy / 300.0);

            // Cap the boost at a reasonable value
            if (releaseBoost > 2.0) releaseBoost = 2.0;
        }

        // Use the tracked facing direction for forward toss
        double forwardDir = facingDirection;

        // Apply the boost to horizontal velocity - enhanced forward boost
        vx *= releaseBoost;

        // Add extra forward momentum (tossing effect)
        vx += forwardDir * FORWARD_TOSS;

        // Apply the boost to vertical velocity with a reduced upward component
        vy *= releaseBoost * 0.85; // Reduced vertical boost multiplier

        // Add a smaller upward impulse to help reach the next object
        // but less if we're going forward fast (more horizontal trajectory)
        double upwardBoost = UPWARD_BOOST;
        double horizontalFactor = std::min(1.0, std::abs(vx) / 500.0); // How much we're moving horizontally
        upwardBoost *= (1.0 - 0.5 * horizontalFactor); // Reduce upward boost more when moving fast horizontally

        vy -= upwardBoost; // Negative is upward

        // Reset swing tracking after releasing
        maxSwingSpeed = 0;

        // Release the hand
        if (isLeft) leftHand.release();
        else rightHand.release();
    }

    void handlecollision(const std::vector<Platform>& platforms, int screenIndex = 0) {
        for (const auto& platform : platforms) {
            SDL_Rect collisionRect = platform.rect;

            // Get platform corners and edges
            double platformLeft = collisionRect.x;
            double platformRight = collisionRect.x + collisionRect.w;
            double platformTop = collisionRect.y;
            double platformBottom = collisionRect.y + collisionRect.h;

            // Find the closest point on the platform to the circle center
            double closestX = std::max(platformLeft, std::min(x, platformRight));
            double closestY = std::max(platformTop, std::min(y, platformBottom));

            // Calculate distance between closest point and circle center
            double distanceX = x - closestX;
            double distanceY = y - closestY;
            double distanceSquared = distanceX * distanceX + distanceY * distanceY;

            // Check if circle collides with platform
            if (distanceSquared < radius * radius) {
                // Check if this is a spike platform
                if (platform.isSpike) {
                    // Hit a spike, reset player position and go to first screen
                    getCurrentScreenIndex() = 0;  // Reset to first screen
                    getCameraOffsetX() = 0;       // Reset camera offset
                    resetPosition();              // Reset player position
                    return; // Exit collision check after respawning
                }

                // Determine finish line based on level (handled in main.cpp)

                // Calculate overlap amounts
                double overlapLeft = x + radius - platformLeft;
                double overlapRight = platformRight - (x - radius);
                double overlapTop = y + radius - platformTop;
                double overlapBottom = platformBottom - (y - radius);

                // Find smallest overlap
                double overlapX = (overlapLeft < overlapRight) ? -overlapLeft : overlapRight;
                double overlapY = (overlapTop < overlapBottom) ? -overlapTop : overlapBottom;

                // Resolve collision based on smallest overlap
                if (abs(overlapX) < abs(overlapY)) {
                    // Horizontal collision
                    x += overlapX;
                    vx = 0;
                } else {
                    // Vertical collision
                    y += overlapY;
                    if (overlapY < 0) { // If hitting from above
                        vy = 0;  // Just stop, no bounce
                        y = platformTop - radius;  // Place exactly on top
                    } else {  // If hitting from below
                        vy = 0;  // Stop upward movement
                    }
                }
            }
        }

        // Also handle rope collisions
        leftHand.handlecollision(platforms);
        rightHand.handlecollision(platforms);
    }

    void render(SDL_Renderer* renderer) {
        // First render the character
        SDL_Rect destrec = {
            static_cast<int>(x-radius),
            static_cast<int>(y - radius),
            static_cast<int>(radius * 2),
            static_cast<int>(radius * 2),
        };
        SDL_RenderCopy(renderer, texture, NULL, &destrec);

        // Then render the ropes on top
        leftHand.render(renderer);
        rightHand.render(renderer);
    }

    void resetPosition() {
        x = 300;  // Initial x position
        y = 100;  // Initial y position
        vx = 0;
        vy = 0;

        // Reset hands
        leftHand.release();
        rightHand.release();

        // Reset hand positions relative to character position
        for (size_t i = 0; i < leftHand.parti.size(); i++) {
            double t = static_cast<double>(i) / (leftHand.parti.size() - 1);
            leftHand.parti[i].xCurrent = x - radius - (35 * t);  // Spread particles left
            leftHand.parti[i].yCurrent = y;
            leftHand.parti[i].xPrevious = leftHand.parti[i].xCurrent;
            leftHand.parti[i].yPrevious = leftHand.parti[i].yCurrent;
        }

        for (size_t i = 0; i < rightHand.parti.size(); i++) {
            double t = static_cast<double>(i) / (rightHand.parti.size() - 1);
            rightHand.parti[i].xCurrent = x + radius + (35 * t);  // Spread particles right
            rightHand.parti[i].yCurrent = y;
            rightHand.parti[i].xPrevious = rightHand.parti[i].xCurrent;
            rightHand.parti[i].yPrevious = rightHand.parti[i].yCurrent;
        }

        // Reset swing tracking
        maxSwingSpeed = 0;
        swingEnergy = 0;
    }

private:
};

#endif
