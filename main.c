#include "raylib.h"
#include "raymath.h"
#include <math.h>

/* =============================
   CONFIG
============================= */
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 800
#define WORLD_WIDTH   4000.0f
#define GROUND_Y      520.0f

/* =============================
   VIRTUAL JOYSTICK
============================= */
typedef struct {
    Vector2 base;
    Vector2 knob;
    float radius;
    bool active;
    Vector2 delta;
} VirtualJoystick;

/* =============================
   PLAYER RENDERING
============================= */
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

/* =============================
   PARALLAX BACKGROUND
============================= */
void DrawParallax(float cameraX)
{
    float farX = -cameraX * 0.2f;
    for (int i = -1; i < 12; i++)
    {
        DrawTriangle(
            (Vector2){ farX + i * 400 + 200, 240 },
            (Vector2){ farX + i * 400,       400 },
            (Vector2){ farX + i * 400 + 400, 400 },
            DARKPURPLE
        );
    }

    float midX = -cameraX * 0.4f;
    for (int i = -1; i < 16; i++)
    {
        DrawCircle(midX + i * 260, 420, 160, DARKBLUE);
    }
}

/* =============================
   MAIN
============================= */
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "U-MG Side Scroller + Sun");
    SetTargetFPS(60);

    EnableCursor();
    SetWindowFocused();

    Vector2 player = { 200.0f, GROUND_Y };
    Vector2 facing = { 1, 0 };
    float speed = 0.0f;
    float cameraX = 0.0f;

    /* --- Sun (screen-space) --- */
    Vector2 sunPos = { SCREEN_WIDTH - 80.0f, 80.0f };
    float sunRadius = 220.0f;

    VirtualJoystick joy = {
        .base   = { 120, SCREEN_HEIGHT - 120 },
        .knob   = { 120, SCREEN_HEIGHT - 120 },
        .radius = 60,
        .active = false,
        .delta  = { 0, 0 }
    };

    while (!WindowShouldClose())
    {
        float time = GetTime();
        Vector2 mouse = GetMousePosition();
        speed = 0.0f;

        /* INPUT */
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

            speed = Clamp(fabsf(joy.delta.x), 0.0f, 1.0f);
            player.x += joy.delta.x * speed * 7f;
            facing = (Vector2){ joy.delta.x >= 0 ? 1 : -1, 0 };
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            joy.active = false;
            joy.knob = joy.base;
            joy.delta = (Vector2){0, 0};
        }

        player.x = Clamp(player.x, 0.0f, WORLD_WIDTH);

        cameraX = player.x - SCREEN_WIDTH * 0.4f;
        cameraX = Clamp(cameraX, 0.0f, WORLD_WIDTH - SCREEN_WIDTH);

        /* DRAW */
        BeginDrawing();
        ClearBackground(SKYBLUE);

        DrawParallax(cameraX);

        DrawRectangle(-cameraX, GROUND_Y + 24, WORLD_WIDTH, 200, DARKBROWN);

        Vector2 screenPlayer = { player.x - cameraX, player.y };
        DrawPlayer(screenPlayer, facing, speed, time);

        /* --- SUN LIGHTING (SCREEN SPACE) --- */
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.65f));
        EndBlendMode();

        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient(
            (int)sunPos.x,
            (int)sunPos.y,
            sunRadius,
            Fade(YELLOW, 0.8f),
            Fade(BLACK, 0.0f)
        );
        EndBlendMode();

        DrawCircleLines(sunPos.x, sunPos.y, 10, ORANGE);

        /* UI */
        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY, 0.5f));
        DrawCircleV(joy.knob, 25, GRAY);
        DrawText("Sun Lighting (Screen Space)", 20, 20, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
