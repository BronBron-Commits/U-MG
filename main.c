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
    float angle = atan2f(dir.y, dir.x);

    float bob = sinf(time * 10.0f) * speed * 4.0f;
    float squash = 1.0f - speed * 0.15f;
    float stretch = 1.0f + speed * 0.10f;

    Vector2 drawPos = { pos.x, pos.y + bob };

    DrawEllipse(drawPos.x, drawPos.y, 22 * stretch, 22 * squash, DARKGREEN);

    Vector2 headOffset = { cosf(angle) * 14, sinf(angle) * 14 };
    DrawCircleV(Vector2Add(drawPos, headOffset), 12, GREEN);

    Vector2 nose = { cosf(angle) * 28, sinf(angle) * 28 };
    DrawCircleV(Vector2Add(drawPos, nose), 4, YELLOW);
}

int main(void)
{
    const int screenWidth = 480;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "U-MG Static Light");
    SetTargetFPS(60);

    EnableCursor();
    SetWindowFocused();

    Vector2 player = { screenWidth / 2.0f, screenHeight / 2.0f };
    Vector2 facing = { 1, 0 };

    // 🔆 Static light position (world light)
    Vector2 lightPos = { screenWidth / 2.0f, screenHeight / 3.0f };
    float lightRadius = 180.0f;

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
        ClearBackground(DARKBLUE);

        // --- WORLD ---
        DrawPlayer(player, facing, speed, GetTime());

        // --- LIGHTING PASS ---
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
        EndBlendMode();

        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient(
            (int)lightPos.x,
            (int)lightPos.y,
            lightRadius,
            Fade(YELLOW, 0.9f),
            Fade(BLACK, 0.0f)
        );
        EndBlendMode();

        // --- DEBUG: show light source ---
        DrawCircleLines(lightPos.x, lightPos.y, 6, ORANGE);

        // --- UI ---
        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY, 0.5f));
        DrawCircleV(joy.knob, 25, GRAY);

        DrawText("Static World Light", 20, 20, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
