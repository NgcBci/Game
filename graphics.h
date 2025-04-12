#ifndef _GRAPHICS__H
#define _GRAPHICS__H
#include <SDL.h>
#include <SDL_image.h>
#include "defs.h"
#include <vector>
#include <cmath>
#include <cstdlib> // For rand()

using std::vector;

// Forward declaration
struct GameObject;

struct Particle {
    double xCurrent;
    double yCurrent;
    double xPrevious;
    double yPrevious;
    bool checkmovement;
};

struct GameObject {
    double x, y, width, height;
    SDL_Texture* texture;
    bool isGrabbable;

    GameObject(SDL_Renderer* renderer, double startX, double startY, double w, double h, bool grabbable = true)
        : x(startX), y(startY), width(w), height(h), isGrabbable(grabbable), texture(nullptr)
    {
        const char* texturePath = "F:\\Game\\platform.png";

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
    SDL_Texture* congratTexture;  // Add texture for congratulations screen

    void logErrorAndExit(const char* msg, const char* error)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "%s: %s", msg, error);
        SDL_Quit();
    }

    void init() {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
            logErrorAndExit("SDL_Init", SDL_GetError());

        // Initialize SDL_image
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
            SDL_Quit();
            return;
        }

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
        congratTexture = IMG_LoadTexture(renderer, "F:\\Game\\congrat.png");
        if (!congratTexture) {
            printf("Failed to load congratulations texture: %s\n", IMG_GetError());
            printf("Attempted to load from: F:\\Game\\congrat.png\n");
        } else {
            printf("Successfully loaded congratulations texture\n");
        }

        // Load platform texture
        SDL_Texture* platformTexture = IMG_LoadTexture(renderer, "F:\\Game\\platform.png");
        if (platformTexture) {
            // Add single platform
            SDL_Rect platformRect = {300, 300, 200, 20};
            platforms.push_back(Platform(platformRect, platformTexture));
        }

        // Load square thing texture
        SDL_Texture* squareTexture = IMG_LoadTexture(renderer, "F:\\Game\\squarething.png");
        if (squareTexture) {
            // Add square thing at the very top of the screen
            SDL_Rect squareRect = {
                SCREEN_WIDTH/2 - 200,  // Center horizontally (screen width/2 - half of square width)
                0,                     // Position at the very top of the screen
                409,                   // Width
                307                    // Height
            };
            platforms.push_back(Platform(squareRect, squareTexture));
        }

        // Add finish line on the right side
        SDL_Texture* finishTexture = IMG_LoadTexture(renderer, "F:\\Game\\finish.png");
        if (finishTexture) {
            SDL_Rect finishRect = {
                SCREEN_WIDTH - 300,  // Moved more to the left
                400,                 // Same y position
                200,                 // Width
                100                  // Height
            };
            platforms.push_back(Platform(finishRect, finishTexture));
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

    void renderCongratulations() {
        if (congratTexture) {
            // Center the congratulations image on screen but make it smaller
            int w, h;
            SDL_QueryTexture(congratTexture, NULL, NULL, &w, &h);
            
            // Scale down the image to half size
            w = w / 2;
            h = h / 2;
            
            SDL_Rect destRect = {
                SCREEN_WIDTH/2 - w/2,  // Center horizontally
                SCREEN_HEIGHT/2 - h/2, // Center vertically
                w,                     // Scaled width
                h                      // Scaled height
            };
            SDL_RenderCopy(renderer, congratTexture, NULL, &destRect);
        }
    }

    ~Graphics() {
        if (congratTexture) {
            SDL_DestroyTexture(congratTexture);
        }
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
    SDL_Texture* handTexture;
    SDL_Texture* grabTexture;  // New texture for grabbing state
    bool isLeftHand;

    ropehand(SDL_Renderer* renderer, double x1, double x2, double y1, double y2, int numberofparticles, bool isLeft)
        : isLeftHand(isLeft) {
        maxLength = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));  // Calculate maximum length
        currentLength = maxLength;

        // Load release hand texture based on hand type
        const char* releasePath = isLeft ? "F:\\Game\\lefthandrelease.png" : "F:\\Game\\righthandrelease.png";
        SDL_Surface* surface = IMG_Load(releasePath);
        if (surface) {
            handTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        } else {
            printf("Failed to load release hand texture: %s\n", IMG_GetError());
            handTexture = nullptr;
        }

        // Load grab hand texture based on hand type
        const char* grabPath = isLeft ? "F:\\Game\\grableft.png" : "F:\\Game\\grabright.png";
        surface = IMG_Load(grabPath);
        if (surface) {
            grabTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        } else {
            printf("Failed to load grab hand texture: %s\n", IMG_GetError());
            grabTexture = nullptr;
        }

        // If grab texture failed to load, use release texture as fallback
        if (!grabTexture && handTexture) {
            grabTexture = handTexture;
        }

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

    ~ropehand() {
        if (handTexture) {
            SDL_DestroyTexture(handTexture);
        }
        if (grabTexture) {
            SDL_DestroyTexture(grabTexture);
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
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

        // Draw hand at the end of the rope
        SDL_Texture* currentTexture = isGrabbingObject ? grabTexture : handTexture;
        if (currentTexture) {
            int handWidth = 40;
            int handHeight = 40;

            // Calculate the angle of the rope
            double dx = parti.back().xCurrent - parti[parti.size()-2].xCurrent;
            double dy = parti.back().yCurrent - parti[parti.size()-2].yCurrent;
            double angle = atan2(dy, dx) * 180.0 / M_PI;

            // Calculate hand position based on rope angle
            double handX = parti.back().xCurrent;
            double handY = parti.back().yCurrent + 10; // Move hand down to connect with rope

            // Create destination rectangle
            SDL_Rect handRect = {
                static_cast<int>(handX - handWidth/2),
                static_cast<int>(handY - handHeight/2),
                handWidth,
                handHeight
            };

            // Calculate center point for rotation
            SDL_Point center = {handWidth/2, handHeight/2};

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

    void grab(double grabx, double graby) {
        // Update the last particle's position to the grab point
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

    // Physics constants
    const double dt = 0.016;             // Time step
    const double GRAVITY = 170.0;        // Reduced gravity for better jumping arcs
    const double DRAG = 0.99;            // Minimal drag to preserve momentum
    const double MAX_SPEED = 1200.0;     // Increased for better jumping between platforms
    const double HAND_FORCE = 800.0;     // Force for climbing
    const double FLAP_LIFT = 150.0;      // Body lift for climbing
    const double ROPE_LENGTH = 120.0;    // Fixed rope length for consistent climbing
    const double ROPE_STIFFNESS = 150.0; // Spring force for rope
    const double HAND_MASS = 1.0;        // Hand mass
    const double BODY_MASS = 3.0;        // Body mass
    const double SWING_BOOST = 1.5;      // Momentum boost when swinging aggressively
    const double JUMP_BOOST = 1.4;       // Increased velocity boost for better tossing
    const double FORWARD_TOSS = 300.0;   // Forward momentum when releasing
    const double UPWARD_BOOST = 80.0;    // Reduced upward boost when releasing

    // Track swing state and movement
    double lastSwingAngle = 0;
    double maxSwingSpeed = 0;
    bool isSwingingRight = false;
    double swingEnergy = 0;
    double facingDirection = 1.0; // 1.0 for right, -1.0 for left

    Character(SDL_Renderer* renderer, double startX, double startY, double r, int handParticles)
        : x(startX), y(startY - 500), vx(0), vy(0), radius(r),  // Start 500 pixels above the platform
          // Initialize hands at the edges of the character with medium length ropes (35 pixels)
          leftHand(renderer, startX - r, startX - r - 35, startY - 500, startY - 500 - 35, handParticles, true),
          rightHand(renderer, startX + r, startX + r + 35, startY - 500, startY - 500 - 35, handParticles, false),
          hasReachedFinish(false),
          showingCongratulations(false)  // Initialize new flag
    {
        const char* texturePath = "F:\\Game\\character.png";

        // Try to load the image first
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

        if (!texture) {
        } else {
        }
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
        }

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

        // Apply horizontal force to free hand for directional swing control
        if (inputX != 0) {
            handVelocityX += inputX * HAND_FORCE * 0.6 * dt;
        }

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
        double dx = x - pivotX;
        double dy = y - pivotY;
        double distance = sqrt(dx*dx + dy*dy);

        if (distance < 0.0001) return; // Avoid division by zero

        // Calculate tangent direction (perpendicular to radius)
        double tangentX = -dy / distance;
        double tangentY = dx / distance;

        // Project current velocity onto tangent
        double tangentialSpeed = vx * tangentX + vy * tangentY;

        // Apply gravity as a tangential force - increased for better swinging
        double gravityAngle = atan2(dy, dx);
        double gravityTangentialComponent = GRAVITY * 1.2 * cos(gravityAngle); // Increased gravity effect for better swing
        tangentialSpeed += gravityTangentialComponent * dt;

        // Normalize input direction for body control
        double inputLength = sqrt(inputX*inputX + inputY*inputY);
        if (inputLength > 0) {
            inputX /= inputLength;
            inputY /= inputLength;

            // Apply input to tangential speed - for body swing control
            double controlForce = 700.0; // Increased force for better swinging
            double swingControl = (inputX * tangentX + inputY * tangentY) * controlForce * dt;
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

        // Check if character has reached the finish line
        if (!hasReachedFinish) {
            // Check if character is inside the finish line area
            if (x >= SCREEN_WIDTH - 300 && x <= SCREEN_WIDTH - 100 &&  // Between left and right edges
                y >= 400 && y <= 500) {  // Between top and bottom edges
                hasReachedFinish = true;
                showingCongratulations = true;
                // Stop character movement when reaching finish
                vx = 0;
                vy = 0;
                return;  // Skip rest of update when showing congratulations
            }
        }

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

            // Normal collision handling for all platforms
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

                // Check if this is the finish line platform first
                if (platform.rect.x == SCREEN_WIDTH - 300) {
                    hasReachedFinish = true;
                    showingCongratulations = true;
                    vx = 0;
                    vy = 0;
                    return;  // Skip normal collision handling for finish line
                }

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

void renderPlatform(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect) {
    // Apply jitter effect
    int jitterX = rand() % 3 - 1; // Random offset between -1 and 1
    int jitterY = rand() % 3 - 1; // Random offset between -1 and 1

    // Apply wobble effect
    double wobble = sin(SDL_GetTicks() / 100.0) * 5; // Wobble angle

    // Adjust rectangle for jitter
    SDL_Rect jitteredRect = {
        rect.x + jitterX,
        rect.y + jitterY,
        rect.w,
        rect.h
    };

    // Calculate center point for rotation
    SDL_Point center = {rect.w / 2, rect.h / 2};

    // Render with rotation and wobble
    SDL_RenderCopyEx(renderer, texture, NULL, &jitteredRect, wobble, &center, SDL_FLIP_NONE);
}

#endif
