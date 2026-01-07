#include "raylib.h"
#include "raymath.h"

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

    EnableCursor();              // REQUIRED on X11
    SetWindowFocused();          // FORCE focus

    Vector2 player = { screenWidth / 2.0f, screenHeight / 2.0f };

    VirtualJoystick joy = {
        .base = { 120, screenHeight - 120 },
        .knob = { 120, screenHeight - 120 },
        .radius = 60,
        .active = false
    };

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();
        bool down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointCircle(mouse, joy.base, joy.radius))
                joy.active = true;
        }

        if (down && joy.active)
        {
            Vector2 delta = Vector2Subtract(mouse, joy.base);
            float dist = Vector2Length(delta);

            if (dist > joy.radius)
                delta = Vector2Scale(Vector2Normalize(delta), joy.radius);

            joy.knob = Vector2Add(joy.base, delta);

            Vector2 move = Vector2Scale(delta, 0.1f);
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

        // DEBUG INPUT VISUALIZATION (temporary)
        DrawCircleV(mouse, 6, RED);
        DrawText(down ? "MOUSE DOWN" : "MOUSE UP", 20, 50, 20, YELLOW);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
