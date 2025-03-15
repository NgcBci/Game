#ifndef _GRAPHICS__H
#define _GRAPHICS__H
#include <SDL.h>
#include <SDL_image.h>
#include "defs.h"

struct Particle {
    double xCurrent;
    double yCurrent;
    double xPrevious;
    double yPrevious;
    bool checkmovement;
};

// linear interpolation is used to figure out the position of each particle with general formula being P = (1-t)P1 + tP2 => each particle is divided equally on the rope
class ropehand {
public:
    ropehand(double x1, double x2, double y1, double y2, int numberofparticles) {
        for (int i = 0; i < numberofparticles;i++) {
            double weightforlerp= (double)i/(numberofparticles - 1);
            double x = (1 - weightforlerp)*x1 + weightforlerp*x2;
            double y = (1 - weightforlerp)*y1 + weightforlerp*y2;
            // now we know the postion of each particle
            Particle pn;
            pn.xCurrent = x;
            pn.yCurrent = y;
            pn.xPrevious = x;
            pn.yPrevious = y;
            // p0 is fixed to mouse's position
            pn.checkmovement = (i == 0);
            parti.push_back(pn);
        }
        int segments = numberofparticles - 1;
        double ropelength = sqrt(pow(x1 - x2,2) + pow(y1 -y2,2));
        desireddistance = ropelength/segments;
    }

    void step() {
    verletintergraion();
    enforceConstraint();
    }
    void followthemouse(double movementx, double movementy) {
    parti[0].xCurrent = movementx;
    parti[0].yCurrent = movementy;
    }
    void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (size_t i = 0; i < parti.size() - 1; i++) {
        SDL_RenderDrawLine(renderer, parti[i].xCurrent, parti[i].yCurrent, parti[i + 1].xCurrent, parti[i + 1].yCurrent);
    }
}

private:
    //function for updating rope position
    void verletintergraion() {
    float damping = 0.98;  // Reduce damping for more natural movement
    for (Particle &pn : parti) {
        if (pn.checkmovement) continue;  // Skip fixed particles

        // Store previous position correctly
        double prevX = pn.xCurrent;
        double prevY = pn.yCurrent;

        // Compute velocity (current - previous) with damping
        double velocityX = (pn.xCurrent - pn.xPrevious) * damping;
        double velocityY = (pn.yCurrent - pn.yPrevious) * damping;

        // Apply gravity (positive direction for SDL's coordinate system)
        velocityY += 9.81 * t;

        // Update position using velocity
        pn.xCurrent += velocityX;
        pn.yCurrent += velocityY;

        // Update previous position for next frame
        pn.xPrevious = prevX;
        pn.yPrevious = prevY;
    }
}

    //function for simulating constraint of rope
    void enforceConstraint() {
        for (int i = 0;i < jakobsenit;i++) {
            for (size_t j = 1;j < parti.size();j++) {
                Particle &p1 = parti[j-1];
                Particle &p2 = parti[j];
                float distance = sqrt(pow(p1.xCurrent - p2.xCurrent, 2) + pow(p1.yCurrent - p2.yCurrent, 2));
                float distanceError = distance - desireddistance;

                 // The direction in which particles should be pulled or pushed
                float xDifference = p2.xCurrent - p1.xCurrent;
                float yDifference = p2.yCurrent - p1.yCurrent;

                // We need to make it a unit vector
                // This will allow us to easily scale the impact we have
                float xDirection = xDifference / sqrt(pow(xDifference, 2) + pow(yDifference, 2));
                float yDirection = yDifference / sqrt(pow(xDifference, 2) + pow(yDifference, 2));

                // Finally, we can update particles' positions
                // We need to remember that fixed particles should stay in place
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
    std::vector<Particle> parti;

    float desireddistance;
};


struct Graphics {
    SDL_Renderer *renderer;
	SDL_Window *window;
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
    }

};

#endif //
