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
   MAIN
============================= */
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "U-MG Side Scroller");
    SetTargetFPS(60);

    EnableCursor();
    SetWindowFocused();

    /* --- Player world state --- */
    Vector2 player = { 200.0f, GROUND_Y };
    Vector2 facing = { 1, 0 };
    float speed = 0.0f;

    /* --- Camera --- */
    float cameraX = 0.0f;

    /* --- Moving world light --- */
    Vector2 lightAnchor = { 900.0f, 260.0f };
    float lightRadius = 180.0f;
    float lightAmplitude = 160.0f;
    float lightSpeed = 1.2f;

    /* --- Virtual Joystick --- */
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

        /* =============================
           INPUT
        ============================= */
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

            /* --- SIDE SCROLLER MOVEMENT (X ONLY) --- */
            player.x += joy.delta.x * speed * 4.0f;
            facing = (Vector2){ joy.delta.x >= 0 ? 1 : -1, 0 };
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            joy.active = false;
            joy.knob = joy.base;
            joy.delta = (Vector2){0, 0};
        }

        /* =============================
           CAMERA
        ============================= */
        cameraX = player.x - SCREEN_WIDTH * 0.4f;
        cameraX = Clamp(cameraX, 0.0f, WORLD_WIDTH - SCREEN_WIDTH);

        /* =============================
           MOVING LIGHT (WORLD SPACE)
        ============================= */
        Vector2 lightPos = {
            lightAnchor.x + sinf(time * lightSpeed) * lightAmplitude,
            lightAnchor.y + cosf(time * lightSpeed * 0.6f) * (lightAmplitude * 0.3f)
        };

        /* =============================
           DRAW
        ============================= */
        BeginDrawing();
        ClearBackground(DARKBLUE);

        /* --- Ground --- */
        DrawRectangle(-cameraX, GROUND_Y + 24, WORLD_WIDTH, 200, DARKBROWN);

        /* --- Player (world → screen) --- */
        Vector2 screenPlayer = {
            player.x - cameraX,
            player.y
        };
        DrawPlayer(screenPlayer, facing, speed, time);

        /* --- Lighting Pass --- */
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.75f));
        EndBlendMode();

        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient(
            (int)(lightPos.x - cameraX),
            (int)lightPos.y,
            lightRadius,
            Fade(YELLOW, 0.9f),
            Fade(BLACK, 0.0f)
        );
        EndBlendMode();

        /* --- Debug light source --- */
        DrawCircleLines(lightPos.x - cameraX, lightPos.y, 6, ORANGE);

        /* --- UI --- */
        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY, 0.5f));
        DrawCircleV(joy.knob, 25, GRAY);
        DrawText("Side Scroller Prototype", 20, 20, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
