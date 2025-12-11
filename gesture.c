#include <math.h>
#include "gesture.h"

// -------- Gesture types --------

const char* gesture_name(gesture_t g) {
    switch (g) {
        case GESTURE_FORWARD:  return "FORWARD";
        case GESTURE_BACKWARD: return "BACKWARD";
        case GESTURE_LEFT:     return "LEFT";
        case GESTURE_RIGHT:    return "RIGHT";
        case GESTURE_NONE:
        default:               return "NONE";
    }
}

// Simple gesture detector using X/Y accel only
gesture_t detect_gesture(float ax, float ay, float az_ignored) {
    (void)az_ignored;  // explicitly ignore Z so compiler doesn't warn

    // Thresholds in g (tune these!)
    const float THRESH_FB = 0.25f;   // forward/back
    const float THRESH_LR = 0.25f;   // left/right

    float abs_ax = fabsf(ax);
    float abs_ay = fabsf(ay);

    // Decide which axis has the dominant acceleration

    // Forward / Backward
    if (abs_ay > THRESH_FB && abs_ay >= abs_ax) {
        gesture_t g = (ay > 0) ? GESTURE_FORWARD : GESTURE_BACKWARD;

        // DEBUG: print what caused this
        // printf("[GESTURE FB] ay=%+.3f |ay|=%.3f |ax|=%.3f (TH_FB=%.2f)\n",
        //        ay, abs_ay, abs_ax, THRESH_FB);

        return g;
    }

    // Nothing strong enough
    else {
        return GESTURE_NONE;
    }
}
