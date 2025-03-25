#ifndef _GRAPaHICS__H
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
    double vx, vy;  // Velocity
    double mass;    // Mass for physics
    bool isGrabbable;
    bool isFalling; // Flag to indicate if platform should fall
    SDL_Texture* texture;

    GameObject(SDL_Renderer* renderer, double startX, double startY, double w, double h, bool grabbable = true, bool falling = false)
        : x(startX), y(startY), width(w), height(h), vx(0), vy(0), mass(1.0), isGrabbable(grabbable), isFalling(falling)
    {
        texture = IMG_LoadTexture(renderer, "F:\\Game\\platform.jpg");
        if (!texture) std::cout << "Failed to load object texture";
    }

    void update(double dt) {
        if (isFalling) {
            vy += 981.0 * dt;  // Apply gravity
            y += vy * dt;      // Update position
            x += vx * dt;      // Allow horizontal movement too
        }
    }

    void render(SDL_Renderer* renderer) const {
        SDL_Rect destRect = {
            static_cast<int>(x - width/2),
            static_cast<int>(y - height/2),
            static_cast<int>(width),
            static_cast<int>(height)
        };
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
    }

    ~GameObject() {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
};

struct Graphics {
    SDL_Renderer *renderer;
	SDL_Window *window;
    std::vector<GameObject> objects;

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

        // Add some grabbable objects
        objects.push_back(GameObject(renderer, 400, 500, 200, 200, true));  // Main platform
        objects.push_back(GameObject(renderer, 600, 400, 100, 20, true));  // Grabbable platform
        objects.push_back(GameObject(renderer, 200, 300, 100, 20, true));  // Another grabbable platform
    }

    SDL_Texture *loadTexture(const char *filename)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO,
                       "Loading %s", filename);
        SDL_Texture *texture = IMG_LoadTexture(renderer, filename);
        if (texture == NULL)
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                  SDL_LOG_PRIORITY_ERROR, "Load texture %s", IMG_GetError());

        return texture;
    }

    void renderTexture(SDL_Texture *texture, int x, int y)
    {
        SDL_Rect dest;

        dest.x = x;
        dest.y = y;
        SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);

        SDL_RenderCopy(renderer, texture, NULL, &dest);
    }
    //not gonna use this just for sample code so i can figure how to attach the hands to the body
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
        for (int i = 0; i < jakobsenit; i++) {
            for (size_t j = 1; j < parti.size(); j++) {
                Particle &p1 = parti[j-1];
                Particle &p2 = parti[j];
                double distance = sqrt(pow(p1.xCurrent - p2.xCurrent, 2) + pow(p1.yCurrent - p2.yCurrent, 2));
                double distanceError = distance - desireddistance;

                double xDifference = p2.xCurrent - p1.xCurrent;
                double yDifference = p2.yCurrent - p1.yCurrent;

                double xDirection = xDifference / sqrt(pow(xDifference, 2) + pow(yDifference, 2));
                double yDirection = yDifference / sqrt(pow(xDifference, 2) + pow(yDifference, 2));

                if (p1.checkmovement && !p2.checkmovement) {
                    p2.xCurrent -= xDirection * distanceError;
                    p2.yCurrent -= yDirection * distanceError;
                }
                else if (p2.checkmovement && !p1.checkmovement) {
                    p1.xCurrent += xDirection * distanceError;
                    p1.yCurrent += yDirection * distanceError;
                }
                else if (!p1.checkmovement && !p2.checkmovement) {
                    p2.xCurrent -= 0.5 * xDirection * distanceError;
                    p2.yCurrent -= 0.5 * yDirection * distanceError;
                    p1.xCurrent += 0.5 * xDirection * distanceError;
                    p1.yCurrent += 0.5 * yDirection * distanceError;
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
        : x(startX), y(-100), vx(0), vy(0), radius(r),
          leftHand(startX - r, startX - r - 50, startY, startY, handParticles),
          rightHand(startX + r, startX + r + 50, startY, startY, handParticles)
    {
        texture = IMG_LoadTexture(renderer, "F:\\Game\\character.jpg");
        if (!texture) cout << "Failed";
    }

    ~Character() {
        if (texture) SDL_DestroyTexture(texture);
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

    void grab(bool isLeft, const std::vector<GameObject>& objects) {
        double handX, handY;
        if (isLeft) {
            handX = leftHand.parti.back().xCurrent;
            handY = leftHand.parti.back().yCurrent;
        } else {
            handX = rightHand.parti.back().xCurrent;
            handY = rightHand.parti.back().yCurrent;
        }

        for (const auto& obj : objects) {
            if (obj.isGrabbable && handX >= obj.x && handX <= obj.x + obj.width &&
                handY >= obj.y && handY <= obj.y + obj.height) {
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

    bool checkCollision(const GameObject& obj, double& overlapX, double& overlapY) {
        double left = x - radius;
        double right = x + radius;
        double top = y - radius;
        double bottom = y + radius;
        if (right > obj.x && left < obj.x + obj.width &&
            bottom > obj.y && top < obj.y + obj.height) {
            if (right - obj.x < obj.x + radius - left) overlapX = -(right - obj.x);
            else overlapX = obj.x + radius - left;
            if (bottom - obj.y < obj.y + radius - top) overlapY = -(bottom - obj.y);
            else overlapY = obj.y + radius - top;
            return true;
        }
        return false;
    }

    void handlecollision(const std::vector<GameObject>& objects) {
        for (const auto& obj : objects) {
            double overlapX = 0;
            double overlapY = 0;
            if (checkCollision(obj, overlapX, overlapY)) {
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
