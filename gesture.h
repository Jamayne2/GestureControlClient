#pragma once

typedef enum {
    GESTURE_NONE = 0,
    GESTURE_FORWARD,
    GESTURE_BACKWARD,
    GESTURE_LEFT,
    GESTURE_RIGHT
} gesture_t;

const char* gesture_name(gesture_t g);
gesture_t detect_gesture(float ax, float ay, float az);
