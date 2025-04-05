#ifndef _GRAPHICS__H
#define _GRAPHICS__H
#include <SDL.h>
#include <SDL_image.h>
#include "defs.h"
#include <vector>
#include <iostream>
#include <cmath>

using std::cout;
using std::vector;

// Forward declaration
struct GameObject;

struct Particle {
    double xCurrent;
    double yCurrent;
    double xPrevious;
    double yPrevious;
    bool checkmovement;
    double xOld;
    double yOld;
};

struct GameObject {
    double x, y, width, height;
    SDL_Texture* texture;
    bool isGrabbable;

    GameObject(SDL_Renderer* renderer, double startX, double startY, double w, double h, bool grabbable = true)
        : x(startX), y(startY), width(w), height(h), isGrabbable(grabbable)
    {
        const char* texturePath = "F:\\Game\\platform.png";
        std::cout << "Attempting to load platform texture from: " << texturePath << std::endl;

        // Try to load the image first
        SDL_Surface* surface = IMG_Load(texturePath);
        if (!surface) {
            std::cout << "Failed to load platform image: " << IMG_GetError() << std::endl;
        } else {
            std::cout << "Successfully loaded surface. Size: " << surface->w << "x" << surface->h << std::endl;

            // Try to create texture from surface
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (!texture) {
                std::cout << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
            } else {
                std::cout << "Successfully created texture from surface" << std::endl;
            }

            SDL_FreeSurface(surface);
        }
    }

    void update(double dt) {
        // No update needed for static platforms
    }

    void render(SDL_Renderer* renderer) const {
        if (!texture) {
            std::cout << "Warning: Trying to render with null texture" << std::endl;
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
        if (SDL_RenderCopy(renderer, texture, NULL, &destRect) != 0) {
            std::cout << "Failed to render platform: " << SDL_GetError() << std::endl;
        }
    }

    ~GameObject() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
};

struct Platform {
    SDL_Rect rect;
    SDL_Texture* texture;
    double visualHeight;  // Actual visual height of the platform texture

    Platform(SDL_Rect r, SDL_Texture* t) : rect(r), texture(t) {
        // Get the actual texture dimensions
        int w, h;
        SDL_QueryTexture(t, NULL, NULL, &w, &h);
        visualHeight = h;
    }
};

struct Graphics {
    SDL_Renderer *renderer;
    SDL_Window *window;
    std::vector<Platform> platforms;

    void logErrorAndExit(const char* msg, const char* error)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "%s: %s", msg, error);
        SDL_Quit();
    }

    void init() {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
            logErrorAndExit("SDL_Init", SDL_GetError());

        window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if (window == nullptr) logErrorAndExit("CreateWindow", SDL_GetError());
        SDL_SetWindowResizable(window, SDL_TRUE);
        SDL_MaximizeWindow(window);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                              SDL_RENDERER_PRESENTVSYNC);

        if (renderer == nullptr) logErrorAndExit("CreateRenderer", SDL_GetError());

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Load platform texture
        SDL_Texture* platformTexture = IMG_LoadTexture(renderer, "F:\\Game\\platform.png");
        if (platformTexture) {
            // Add single platform
            SDL_Rect platformRect = {300, 300, 200, 20};
            platforms.push_back(Platform(platformRect, platformTexture));
            std::cout << "Created platform at position: (300, 300) with size: 200x20" << std::endl;
        }
    }

    void renderPlatforms() {
        for (const auto& platform : platforms) {
            // Calculate the visual height based on the texture's aspect ratio
            int w, h;
            SDL_QueryTexture(platform.texture, NULL, NULL, &w, &h);
            double aspectRatio = static_cast<double>(h) / w;
            int visualHeight = static_cast<int>(platform.rect.w * aspectRatio);

            SDL_Rect destRect = {
                platform.rect.x,
                platform.rect.y,
                platform.rect.w,
                visualHeight
            };
            SDL_RenderCopy(renderer, platform.texture, NULL, &destRect);
        }
    }

    ~Graphics() {
        for (auto& platform : platforms) {
            if (platform.texture) {
                SDL_DestroyTexture(platform.texture);
            }
        }
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
    }
};

class ropehand {
public:
    bool isGrabbingObject;
    static const int jakobsenit = 10;  // Reduced for more stable physics
    vector<Particle> parti;
    double maxLength;
    double currentLength;

    ropehand(double x1, double x2, double y1, double y2, int numberofparticles) {
        maxLength = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));  // Calculate maximum length
        currentLength = maxLength;

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
            isGrabbingObject = false;
        }
        int segments = numberofparticles - 1;
        desireddistance = maxLength/segments;
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

    void grab(double grabx, double graby) {
        parti.back().xCurrent = grabx;
        parti.back().yCurrent = graby;
        parti.back().checkmovement = true;
        isGrabbingObject = true;
    }

    void release() {
         parti.back().checkmovement = false;
        isGrabbingObject = false;
    }

    void handlecollision(const std::vector<Platform>& platforms) {
        const double BOUNCE = 0.1; // Reduced bounce factor
        
        // Check each particle in the rope
        for (auto& particle : parti) {
            for (const auto& platform : platforms) {
                // Get current velocity
                double velX = particle.xCurrent - particle.xPrevious;
                double velY = particle.yCurrent - particle.yPrevious;
                
                // Check current position and predicted next position
                double nextX = particle.xCurrent + velX;
                double nextY = particle.yCurrent + velY;
                
                // Line segment intersection test between current and next position
                bool collision = false;
                double intersectX = 0, intersectY = 0;
                
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

    void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (size_t i = 0; i < parti.size() - 1; i++) {
            SDL_RenderDrawLine(renderer,
                static_cast<int>(parti[i].xCurrent),
                static_cast<int>(parti[i].yCurrent),
                static_cast<int>(parti[i + 1].xCurrent),
                static_cast<int>(parti[i + 1].yCurrent));
        }
    }

    void pull(double forceX, double forceY) {
        if (isGrabbingObject) {
            // Apply force to the last particle (hand)
            parti.back().xCurrent += forceX;
            parti.back().yCurrent += forceY;
    }
}

private:
    const double dt = 0.016;
    double desireddistance;

    void verletintergraion() {
        double damping = 0.98;  // Increased damping for more stable movement
    for (Particle &pn : parti) {
            if (pn.checkmovement) continue;

        double prevX = pn.xCurrent;
        double prevY = pn.yCurrent;

        double velocityX = (pn.xCurrent - pn.xPrevious) * damping;
        double velocityY = (pn.yCurrent - pn.yPrevious) * damping;

            velocityY += 9.81 * dt;

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

    const double GRAVITY = 500.0;  // Reduced from 981.0
    const double DRAG = 0.95;      // Increased drag from 0.98 to slow horizontal movement
    const double MAX_SPEED = 300.0; // Reduced from 400.0
    const double HAND_FORCE = 300.0; // Reduced from 500.0
    const double SWING_FORCE = 150.0;  // Reduced swing force for more controlled movement

    Character(SDL_Renderer* renderer, double startX, double startY, double r, int handParticles)
        : x(startX), y(startY - 500), vx(0), vy(0), radius(r),  // Start 500 pixels above the platform
          // Initialize hands at the edges of the character with slightly longer ropes (25 pixels instead of 20)
          leftHand(startX - r, startX - r - 25, startY - 500, startY - 500 - 25, handParticles),
          rightHand(startX + r, startX + r + 25, startY - 500, startY - 500 - 25, handParticles)
    {
        const char* texturePath = "F:\\Game\\character.png";
        std::cout << "Attempting to load character texture from: " << texturePath << std::endl;

        // Try to load the image first
        SDL_Surface* originalSurface = IMG_Load(texturePath);
        if (!originalSurface) {
            std::cout << "Failed to load character image: " << IMG_GetError() << std::endl;
            return;
        }

        // Create a new surface at the desired size (2*radius x 2*radius)
        SDL_Surface* scaledSurface = SDL_CreateRGBSurface(0, radius * 2, radius * 2, 32,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

        if (!scaledSurface) {
            std::cout << "Failed to create scaled surface: " << SDL_GetError() << std::endl;
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

        if (!texture) {
            std::cout << "Failed to create texture: " << SDL_GetError() << std::endl;
        } else {
            std::cout << "Successfully created character texture at size: " << (radius * 2) << "x" << (radius * 2) << std::endl;
        }
    }

    ~Character() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }

    void applySwingForces(const Uint8* keystate) {
        // Only apply swing if exactly one hand is grabbing
        if (!(leftHand.isGrabbingObject ^ rightHand.isGrabbingObject)) return;

        // Get the pivot point (the grabbing hand's position)
        double pivotX, pivotY;
        bool isRightHandGrabbing = rightHand.isGrabbingObject;

        if (isRightHandGrabbing) {
            pivotX = rightHand.parti.back().xCurrent;
            pivotY = rightHand.parti.back().yCurrent;
        } else {
            pivotX = leftHand.parti.back().xCurrent;
            pivotY = leftHand.parti.back().yCurrent;
        }

        // Check direction controls
        bool swingLeft = keystate[SDL_SCANCODE_LEFT];
        bool swingRight = keystate[SDL_SCANCODE_RIGHT];
        bool reachUp = keystate[SDL_SCANCODE_UP];
        bool reachDown = keystate[SDL_SCANCODE_DOWN];

        // Calculate current angle from pivot to body
        double dx = x - pivotX;
        double dy = y - pivotY;
        static double currentAngle = atan2(dy, dx);
        
        const double ROTATION_SPEED = 4.0; // Speed of rotation
        const double dt = 0.016;

        // Update angle based on input
        if (reachUp) {
            currentAngle += ROTATION_SPEED * dt * (isRightHandGrabbing ? -1 : 1);
        } else if (reachDown) {
            currentAngle -= ROTATION_SPEED * dt * (isRightHandGrabbing ? -1 : 1);
        }

        // Keep angle in range [0, 2π]
        while (currentAngle > 2 * M_PI) currentAngle -= 2 * M_PI;
        while (currentAngle < 0) currentAngle += 2 * M_PI;

        // Calculate rope length
        double ropeLength = radius * 2.0;

        // Move body based on angle
        x = pivotX + cos(currentAngle) * ropeLength;
        y = pivotY + sin(currentAngle) * ropeLength;

        // Apply swing force to velocity only when actively swinging
        if (swingLeft || swingRight) {
            double swingForce = 150.0;
            if (swingLeft) vx -= swingForce * dt;
            if (swingRight) vx += swingForce * dt;
        }
    }

    void update() {
        const double dt = 0.016;

        // Get keyboard state for swing controls
        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        // Apply swing forces before gravity
        applySwingForces(keystate);

        // Always apply gravity
        vy += GRAVITY * dt;

        // Limit fall speed
        vy = std::min(vy, MAX_SPEED);

        // Apply drag to horizontal movement
        vx *= DRAG;

        // Update position
        x += vx * dt;
        y += vy * dt;

        // Keep hands attached at the edges of the character
        leftHand.attachtothebody(x - radius, y, radius * 0.2, true);
        rightHand.attachtothebody(x + radius, y, radius * 0.2, false);

        // Update hands physics
        leftHand.step();
        rightHand.step();
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
                    leftHand.grab(handX, handY);
                } else {
                    rightHand.grab(handX, handY);
                }
                break;
            }
        }
    }

    void release(bool isLeft) {
        if (isLeft) leftHand.release();
        else rightHand.release();
    }

    void pullHand(bool isLeft, double forceX, double forceY) {
        if (isLeft) {
            leftHand.pull(forceX, forceY);
        } else {
            rightHand.pull(forceX, forceY);
        }
    }

    bool checkCollision(const SDL_Rect& rect, double& overlapX, double& overlapY) {
        double left = x - radius;
        double right = x + radius;
        double top = y - radius;
        double bottom = y + radius;
        if (right > rect.x && left < rect.x + rect.w &&
            bottom > rect.y && top < rect.y + rect.h) {
            if (right - rect.x < rect.x + radius - left) overlapX = -(right - rect.x);
            else overlapX = rect.x + radius - left;
            if (bottom - rect.y < rect.y + radius - top) overlapY = -(bottom - rect.y);
            else overlapY = rect.y + radius - top;
            return true;
        }
        return false;
    }

    void handlecollision(const std::vector<Platform>& platforms) {
        for (const auto& platform : platforms) {
            SDL_Rect collisionRect = platform.rect;

            // Check collision between character and platform
            double characterLeft = x - radius;
            double characterRight = x + radius;
            double characterTop = y - radius;
            double characterBottom = y + radius;

            double platformLeft = collisionRect.x;
            double platformRight = collisionRect.x + collisionRect.w;
            double platformTop = collisionRect.y;
            double platformBottom = collisionRect.y + collisionRect.h;

            // Check if there is any overlap
            if (characterRight > platformLeft && characterLeft < platformRight &&
                characterBottom > platformTop && characterTop < platformBottom) {

                // Calculate overlap amounts
                double overlapLeft = characterRight - platformLeft;
                double overlapRight = platformRight - characterLeft;
                double overlapTop = characterBottom - platformTop;
                double overlapBottom = platformBottom - characterTop;

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
};

#endif
