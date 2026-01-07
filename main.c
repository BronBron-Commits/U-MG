#include "raylib.h"
#include "raymath.h"

typedef struct {
    Vector2 base;
    Vector2 knob;
    float radius;
    bool active;
    Vector2 delta;   // normalized direction
} VirtualJoystick;

void DrawPlayer(Vector2 pos, Vector2 dir)
{
    float angle = atan2f(dir.y, dir.x);

    // Body
    DrawCircleV(pos, 22, DARKGREEN);

    // Head offset in facing direction
    Vector2 headOffset = {
        cosf(angle) * 14,
        sinf(angle) * 14
    };

    DrawCircleV(Vector2Add(pos, headOffset), 12, GREEN);

    // Direction marker ("nose")
    Vector2 nose = {
        cosf(angle) * 28,
        sinf(angle) * 28
    };

    DrawCircleV(Vector2Add(pos, nose), 4, YELLOW);
}

int main(void)
{
    const int screenWidth = 480;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "U-MG v0.1.0");
    SetTargetFPS(60);

    EnableCursor();
    SetWindowFocused();

    Vector2 player = { screenWidth / 2.0f, screenHeight / 2.0f };
    Vector2 facing = { 1, 0 };

    VirtualJoystick joy = {
        .base = { 120, screenHeight - 120 },
        .knob = { 120, screenHeight - 120 },
        .radius = 60,
        .active = false,
        .delta = { 0, 0 }
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

            joy.delta = Vector2Normalize(delta);
            facing = joy.delta;

            Vector2 move = Vector2Scale(joy.delta, 3.5f);
            player = Vector2Add(player, move);
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            joy.active = false;
            joy.knob = joy.base;
            joy.delta = (Vector2){0, 0};
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawPlayer(player, facing);

        // Joystick
        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY, 0.5f));
        DrawCircleV(joy.knob, 25, GRAY);

        DrawText("Procedural Player Sprite", 20, 20, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
