#include "raylib.h"
#include "raymath.h"   // <-- REQUIRED for Vector2 math
#include <math.h>

typedef struct {
    Vector2 base;
    Vector2 knob;
    float radius;
    bool active;
} VirtualJoystick;

int main(void)
{
    const int screenWidth = 480;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Virtual Joystick Demo");
    SetTargetFPS(60);

    Vector2 player = { screenWidth / 2.0f, screenHeight / 2.0f };

    VirtualJoystick joy = {
        .base = { 100, screenHeight - 120 },
        .knob = { 100, screenHeight - 120 },
        .radius = 60,
        .active = false
    };

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointCircle(mouse, joy.base, joy.radius))
                joy.active = true;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && joy.active)
        {
            Vector2 delta = Vector2Subtract(mouse, joy.base);
            float dist = Vector2Length(delta);

            if (dist > joy.radius)
                delta = Vector2Scale(Vector2Normalize(delta), joy.radius);

            joy.knob = Vector2Add(joy.base, delta);

            Vector2 move = Vector2Scale(delta, 0.08f);
            player = Vector2Add(player, move);
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            joy.active = false;
            joy.knob = joy.base;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawCircleV(player, 20, GREEN);

        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY, 0.5f));
        DrawCircleV(joy.knob, 25, GRAY);

        DrawText("Virtual Joystick (Touch/Mouse)", 20, 20, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
