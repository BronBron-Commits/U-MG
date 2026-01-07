#include "raylib.h"
#include "raymath.h"
#include <math.h>

typedef struct {
    Vector2 base;
    Vector2 knob;
    float radius;
    bool active;
    Vector2 delta;
} VirtualJoystick;

void DrawPlayer(Vector2 pos, Vector2 dir, float speed, float time)
{
    // Facing angle
    float angle = atan2f(dir.y, dir.x);

    // Walk animation parameters
    float bob = sinf(time * 10.0f) * speed * 4.0f;
    float squash = 1.0f - speed * 0.15f;
    float stretch = 1.0f + speed * 0.10f;

    // Body radii
    float bodyRadiusX = 22 * stretch;
    float bodyRadiusY = 22 * squash;

    Vector2 drawPos = { pos.x, pos.y + bob };

    // Body (squashed)
    DrawEllipse(drawPos.x, drawPos.y, bodyRadiusX, bodyRadiusY, DARKGREEN);

    // Head offset
    Vector2 headOffset = {
        cosf(angle) * 14,
        sinf(angle) * 14
    };

    Vector2 headPos = Vector2Add(drawPos, headOffset);
    DrawCircleV(headPos, 12, GREEN);

    // Direction marker
    Vector2 nose = {
        cosf(angle) * 28,
        sinf(angle) * 28
    };

    DrawCircleV(Vector2Add(drawPos, nose), 4, YELLOW);
}

int main(void)
{
    const int screenWidth = 480;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "U-MG Walk Squash");
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

    float speed = 0.0f;

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();
        speed = 0.0f;

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

            speed = Clamp(dist / joy.radius, 0.0f, 1.0f);

            Vector2 move = Vector2Scale(joy.delta, speed * 4.0f);
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

        DrawPlayer(player, facing, speed, GetTime());

        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY, 0.5f));
        DrawCircleV(joy.knob, 25, GRAY);

        DrawText("Walk Squash Animation", 20, 20, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
