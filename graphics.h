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
};

struct GameObject {
    double x, y, width, height;
    SDL_Texture* texture;
    bool isGrabbable;

    GameObject(SDL_Renderer* renderer, double startX, double startY, double w, double h, bool grabbable = true)
        : x(startX), y(startY), width(w), height(h), isGrabbable(grabbable)
    {
        const char* texturePath = "F:\\Game\\platform.png";
        SDL_Surface* surface = IMG_Load(texturePath);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    void update(double dt) {
    }

    void render(SDL_Renderer* renderer) const {
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

    Platform(SDL_Rect r, SDL_Texture* t) : rect(r), texture(t) {}
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
            SDL_Rect platformRect = {300, 300, 400, 150};
            platforms.push_back(Platform(platformRect, platformTexture));
        }
    }

    void renderPlatforms() {
        for (const auto& platform : platforms) {
            SDL_RenderCopy(renderer, platform.texture, NULL, &platform.rect);
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

    void handlecollision(const std::vector<GameObject>& object) {
        for (size_t i = 0; i < parti.size(); i++) {
            Particle &p1 = parti[i];
            for (const auto &obj : object) {
                double overlapX = 0;
                double overlapY = 0;

                if (p1.xCurrent > obj.x &&
                    p1.xCurrent < obj.x + obj.width &&
                    p1.yCurrent > obj.y &&
                    p1.yCurrent < obj.y + obj.height) {

                    if (p1.xCurrent - obj.x < obj.x + obj.width - p1.xCurrent)
                        overlapX = -(p1.xCurrent - obj.x);  // Need negative for correct push direction
                    else
                        overlapX = obj.x + obj.width - p1.xCurrent;

                    // Calculate Y overlap
                    if (p1.yCurrent - obj.y < obj.y + obj.height - p1.yCurrent)
                        overlapY = -(p1.yCurrent - obj.y);  // Need negative for correct push direction
                    else
                        overlapY = obj.y + obj.height - p1.yCurrent;

                    // Apply the smaller overlap
                    if (abs(overlapX) < abs(overlapY))
                        p1.xCurrent += overlapX;
                    else
                        p1.yCurrent += overlapY;
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

    const double GRAVITY = 981.0;
    const double DRAG = 0.98;
    const double MAX_SPEED = 400.0;
    const double HAND_FORCE = 500.0;

    Character(SDL_Renderer* renderer, double startX, double startY, double r, int handParticles)
        : x(startX), y(startY - 500), vx(0), vy(0), radius(r),  // Start 500 pixels above the platform
          leftHand(startX - r, startX - r - 50, startY - 500, startY - 500, handParticles),
          rightHand(startX + r, startX + r + 50, startY - 500, startY - 500, handParticles)
    {
        const char* texturePath = "F:\\Game\\character.png";
        std::cout << "Attempting to load character texture from: " << texturePath << std::endl;

        // Try to load the image first
        SDL_Surface* surface = IMG_Load(texturePath);
        if (!surface) {
            std::cout << "Failed to load character image: " << IMG_GetError() << std::endl;
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

    ~Character() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }

    void update() {
        const double dt = 0.016;

        // Apply gravity only when not grabbing
        if (!leftHand.isGrabbingObject && !rightHand.isGrabbingObject) {
            vy += GRAVITY * dt;
            vy = std::min(vy, MAX_SPEED);
        }
        else {
            // When grabbing, stop falling
            vy = 0;
        }

        // Apply drag to horizontal movement
        vx *= DRAG;

        // Update position
        x += vx * dt;
        y += vy * dt;

        // Keep hands attached
        leftHand.attachtothebody(x, y, radius, true);
        rightHand.attachtothebody(x, y, radius, false);

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
            double overlapX = 0;
            double overlapY = 0;
            if (checkCollision(platform.rect, overlapX, overlapY)) {
                if (abs(overlapX) < abs(overlapY)) {
                    x += overlapX;
                    vx = 0;
                }
                else {
                    y += overlapY;
                    vy = 0;
                }
            }
        }
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
