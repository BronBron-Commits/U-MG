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

#define GRAVITY       0.6f
#define JUMP_VELOCITY -12.0f

#define DAY_AMBIENT   0.40f
#define NIGHT_AMBIENT 0.75f

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
   PLAYER (WITH NOSE)
============================= */
void DrawPlayer(Vector2 pos, Vector2 dir, float speed, float time)
{
    float angle = atan2f(dir.y, dir.x);
    float bob = sinf(time * 10.0f) * speed * 4.0f;

    float squash = 1.0f - speed * 0.15f;
    float stretch = 1.0f + speed * 0.10f;

    Vector2 p = { pos.x, pos.y + bob };

    DrawEllipse(p.x, p.y, 22 * stretch, 22 * squash, DARKGREEN);

    Vector2 headOffset = { cosf(angle) * 14, sinf(angle) * 14 };
    DrawCircleV(Vector2Add(p, headOffset), 12, GREEN);

    Vector2 noseOffset = { cosf(angle) * 28, sinf(angle) * 28 };
    DrawCircleV(Vector2Add(p, noseOffset), 4, YELLOW);
}

/* =============================
   STARS (TWINKLE)
============================= */
typedef struct { Vector2 pos; float phase, speed; } Star;

#define STAR_COUNT 18
Star stars[STAR_COUNT] = {
    {{40,60},0,1.2},{{120,90},1.1,0.9},{{200,50},2.3,1.4},
    {{280,110},0.7,1.0},{{360,70},2.9,0.8},{{430,100},1.6,1.3},
    {{90,160},2.1,0.7},{{170,140},0.4,1.1},{{260,180},1.8,0.9},
    {{350,150},2.6,1.2},{{420,200},0.9,0.8},
    {{60,240},1.5,1.0},{{140,260},2.8,0.7},{{220,230},0.2,1.4},
    {{310,270},1.9,0.9},{{390,250},0.6,1.1},{{450,300},2.4,0.8},
    {{300,60},1.3,1.0}
};

void DrawStars(float nightT, float time)
{
    if (nightT <= 0.01f) return;

    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < STAR_COUNT; i++)
    {
        float twinkle = 0.6f + 0.4f * sinf(time * stars[i].speed + stars[i].phase);
        float alpha = nightT * twinkle;
        DrawCircleV(stars[i].pos, 2, Fade(RAYWHITE, alpha));
    }
    EndBlendMode();
}

/* =============================
   BIRDS (DAY ONLY)
============================= */
typedef struct { float x, y, speed, phase; } Bird;

#define BIRD_COUNT 6
Bird birds[BIRD_COUNT] = {
    { -60, 120, 0.9f, 0.0f },
    { -220, 160, 0.7f, 1.2f },
    { -140,  95, 1.1f, 2.1f },
    { -360, 140, 0.8f, 0.6f },
    { -520, 110, 1.0f, 2.7f },
    { -680, 150, 0.75f, 1.8f }
};

static void UpdateBirds(float time)
{
    for (int i = 0; i < BIRD_COUNT; i++)
    {
        // Small vertical bob so they don't look like static V's
        float bob = sinf(time * 1.2f + birds[i].phase) * 0.3f;

        birds[i].x += birds[i].speed;
        birds[i].y += bob;

        if (birds[i].x > SCREEN_WIDTH + 60)
            birds[i].x = -80;
    }
}

static void DrawBirds(float dayT, float time)
{
    if (dayT <= 0.01f) return;

    // Slight fade in/out with day factor; darker than UI, not pure black
    Color birdColor = Fade(BLACK, dayT * 0.8f);

    for (int i = 0; i < BIRD_COUNT; i++)
    {
        float flap = 1.0f + 1.2f * sinf(time * 6.0f + birds[i].phase);
        float x = birds[i].x;
        float y = birds[i].y;

        // Simple "V" bird with animated wing height
        DrawLine((int)x, (int)y, (int)(x + 6), (int)(y + flap), birdColor);
        DrawLine((int)(x + 6), (int)(y + flap), (int)(x + 12), (int)y, birdColor);
    }
}

/* =============================
   PARALLAX
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
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "U-MG Birds (Day) + Sky");
    SetTargetFPS(60);

    EnableCursor();
    SetWindowFocused();

    /* Player */
    Vector2 player = { 200.0f, GROUND_Y };
    Vector2 facing = { 1, 0 };
    float moveSpeed = 5.5f;
    float speed = 0.0f;
    float velocityY = 0.0f;
    bool grounded = true;

    float cameraX = 0.0f;

    /* Day/Night zone */
    float transitionCenter = WORLD_WIDTH * 0.5f;
    float transitionWidth  = 600.0f;

    /* Sun */
    float sunX = SCREEN_WIDTH - 80.0f;
    float sunStartY = 80.0f;
    float sunEndY   = SCREEN_HEIGHT + 120.0f;
    float sunRadius = 220.0f;

    /* Moon */
    float moonX = 80.0f;
    float moonStartY = SCREEN_HEIGHT + 120.0f;
    float moonEndY   = 100.0f;
    float moonRadius = 160.0f;

    /* Controls */
    VirtualJoystick joy = {
        .base   = { 120, SCREEN_HEIGHT - 120 },
        .knob   = { 120, SCREEN_HEIGHT - 120 },
        .radius = 60,
        .active = false,
        .delta  = { 0, 0 }
    };

    Vector2 jumpBtn = { SCREEN_WIDTH - 120.0f, SCREEN_HEIGHT - 120.0f };
    float jumpRadius = 40.0f;

    while (!WindowShouldClose())
    {
        float time = GetTime();
        Vector2 mouse = GetMousePosition();
        speed = 0.0f;

        /* =============================
           INPUT — MOVE (JOYSTICK)
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
            player.x += joy.delta.x * speed * moveSpeed;

            facing = (Vector2){ joy.delta.x >= 0 ? 1 : -1, 0 };
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            joy.active = false;
            joy.knob = joy.base;
            joy.delta = (Vector2){0, 0};
        }

        /* =============================
           INPUT — JUMP BUTTON
        ============================= */
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (grounded && CheckCollisionPointCircle(mouse, jumpBtn, jumpRadius))
            {
                velocityY = JUMP_VELOCITY;
                grounded = false;
            }
        }

        /* =============================
           PHYSICS
        ============================= */
        velocityY += GRAVITY;
        player.y += velocityY;

        if (player.y >= GROUND_Y)
        {
            player.y = GROUND_Y;
            velocityY = 0.0f;
            grounded = true;
        }

        player.x = Clamp(player.x, 0.0f, WORLD_WIDTH);

        /* =============================
           CAMERA
        ============================= */
        cameraX = player.x - SCREEN_WIDTH * 0.4f;
        cameraX = Clamp(cameraX, 0.0f, WORLD_WIDTH - SCREEN_WIDTH);

        /* =============================
           DAY → NIGHT BLEND
        ============================= */
        float t = (player.x - (transitionCenter - transitionWidth * 0.5f)) / transitionWidth;
        t = Clamp(t, 0.0f, 1.0f);

        float dayT = 1.0f - t;
        float nightT = t;

        float ambient = Lerp(DAY_AMBIENT, NIGHT_AMBIENT, nightT);

        float sunY = Lerp(sunStartY, sunEndY, nightT);
        float moonY = Lerp(moonStartY, moonEndY, nightT);

        /* Update birds (always), draw only in day */
        UpdateBirds(time);

        /* =============================
           DRAW
        ============================= */
        BeginDrawing();
        ClearBackground(SKYBLUE);

        DrawParallax(cameraX);

        // Birds are part of the daytime sky; drawn before ambient so dusk naturally darkens them.
        DrawBirds(dayT, time);

        DrawRectangle(-cameraX, GROUND_Y + 24, WORLD_WIDTH, 200, DARKBROWN);

        Vector2 screenPlayer = { player.x - cameraX, player.y };
        DrawPlayer(screenPlayer, facing, speed, time);

        // Ambient darkening pass
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, ambient));
        EndBlendMode();

        // Stars are drawn after darkening so they don't get crushed
        DrawStars(nightT, time);

        // Sun & moon glow
        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient((int)sunX, (int)sunY, sunRadius, Fade(YELLOW, dayT), Fade(BLACK, 0.0f));
        DrawCircleGradient((int)moonX, (int)moonY, moonRadius, Fade(RAYWHITE, nightT), Fade(BLACK, 0.0f));
        EndBlendMode();

        // UI
        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY, 0.5f));
        DrawCircleV(joy.knob, 25, GRAY);

        DrawCircleV(jumpBtn, jumpRadius, grounded ? Fade(GREEN, 0.6f) : Fade(GRAY, 0.4f));
        DrawText("JUMP", (int)jumpBtn.x - 22, (int)jumpBtn.y - 8, 16, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
