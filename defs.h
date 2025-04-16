#ifndef _DEFS__H
#define _DEFS__H

const int SCREEN_WIDTH = 2100;
const int SCREEN_HEIGHT = 1200;
const char* WINDOW_TITLE = "Hello World!";
float t = 0.01;
int jakobsenit = 20;
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

#endif // _DEFS__H
