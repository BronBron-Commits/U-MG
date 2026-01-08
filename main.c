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
   STARS (SCREEN SPACE)
============================= */
void DrawStars(float alpha)
{
    if (alpha <= 0.01f) return;

    Color starColor = Fade(RAYWHITE, alpha);

    static Vector2 stars[] = {
        { 40, 60 }, { 120, 90 }, { 200, 50 }, { 280, 110 },
        { 360, 70 }, { 430, 100 }, { 90, 160 }, { 170, 140 },
        { 260, 180 }, { 350, 150 }, { 420, 200 },
        { 60, 240 }, { 140, 260 }, { 220, 230 }, { 310, 270 },
        { 390, 250 }, { 450, 300 }
    };

    for (int i = 0; i < (int)(sizeof(stars)/sizeof(stars[0])); i++)
    {
        DrawCircleV(stars[i], 2, starColor);
    }
}

/* =============================
   MAIN
============================= */
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "U-MG Sun, Moon & Stars");
    SetTargetFPS(60);

    EnableCursor();
    SetWindowFocused();

    /* --- Player --- */
    Vector2 player = { 200.0f, GROUND_Y };
    Vector2 facing = { 1, 0 };
    float speed = 0.0f;
    float velocityY = 0.0f;
    bool grounded = true;

    float cameraX = 0.0f;

    /* --- Day/Night zone --- */
    float transitionCenter = WORLD_WIDTH * 0.5f;
    float transitionWidth  = 600.0f;

    /* --- Sun --- */
    float sunX = SCREEN_WIDTH - 80.0f;
    float sunStartY = 80.0f;
    float sunEndY   = SCREEN_HEIGHT + 120.0f;
    float sunRadius = 220.0f;

    /* --- Moon --- */
    float moonX = 80.0f;
    float moonStartY = SCREEN_HEIGHT + 120.0f;
    float moonEndY   = 100.0f;
    float moonRadius = 160.0f;

    /* --- Controls --- */
    VirtualJoystick joy = {
        .base   = { 120, SCREEN_HEIGHT - 120 },
        .knob   = { 120, SCREEN_HEIGHT - 120 },
        .radius = 60
    };

    Vector2 jumpButtonPos = { SCREEN_WIDTH - 120.0f, SCREEN_HEIGHT - 120.0f };
    float jumpButtonRadius = 40.0f;

    while (!WindowShouldClose())
    {
        float time = GetTime();
        Vector2 mouse = GetMousePosition();
        speed = 0.0f;

        /* MOVE */
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointCircle(mouse, joy.base, joy.radius))
            joy.active = true;

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && joy.active)
        {
            Vector2 delta = Vector2Subtract(mouse, joy.base);
            float dist = Vector2Length(delta);
            if (dist > joy.radius)
                delta = Vector2Scale(Vector2Normalize(delta), joy.radius);

            joy.delta = Vector2Normalize(delta);
            joy.knob = Vector2Add(joy.base, delta);

            speed = fabsf(joy.delta.x);
            player.x += joy.delta.x * speed * 5.5f;
            facing = (Vector2){ joy.delta.x >= 0 ? 1 : -1, 0 };
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            joy.active = false;
            joy.knob = joy.base;
        }

        /* JUMP */
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            grounded &&
            CheckCollisionPointCircle(mouse, jumpButtonPos, jumpButtonRadius))
        {
            velocityY = JUMP_VELOCITY;
            grounded = false;
        }

        /* PHYSICS */
        velocityY += GRAVITY;
        player.y += velocityY;
        if (player.y >= GROUND_Y)
        {
            player.y = GROUND_Y;
            velocityY = 0.0f;
            grounded = true;
        }

        player.x = Clamp(player.x, 0.0f, WORLD_WIDTH);

        /* CAMERA */
        cameraX = player.x - SCREEN_WIDTH * 0.4f;
        cameraX = Clamp(cameraX, 0.0f, WORLD_WIDTH - SCREEN_WIDTH);

        /* DAY → NIGHT */
        float t = (player.x - (transitionCenter - transitionWidth * 0.5f))
                  / transitionWidth;
        t = Clamp(t, 0.0f, 1.0f);

        float ambient = Lerp(DAY_AMBIENT, NIGHT_AMBIENT, t);

        float sunY = Lerp(sunStartY, sunEndY, t);
        float sunAlpha = 1.0f - t;

        float moonY = Lerp(moonStartY, moonEndY, t);
        float moonAlpha = t;

        /* DRAW */
        BeginDrawing();
        ClearBackground(SKYBLUE);

        DrawParallax(cameraX);
        DrawRectangle(-cameraX, GROUND_Y + 24, WORLD_WIDTH, 200, DARKBROWN);

        DrawPlayer((Vector2){ player.x - cameraX, player.y }, facing, speed, time);

        /* Ambient darkening */
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, ambient));
        EndBlendMode();

        /* Sun & Moon */
        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient(
            (int)sunX, (int)sunY, sunRadius,
            Fade(YELLOW, sunAlpha),
            Fade(BLACK, 0.0f)
        );

        DrawCircleGradient(
            (int)moonX, (int)moonY, moonRadius,
            Fade(RAYWHITE, moonAlpha),
            Fade(BLACK, 0.0f)
        );
        EndBlendMode();

        /* Stars (after lighting) */
        DrawStars(t);

        /* UI */
        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY, 0.5f));
        DrawCircleV(joy.knob, 25, GRAY);

        DrawCircleV(jumpButtonPos, jumpButtonRadius,
                    grounded ? Fade(GREEN, 0.6f) : Fade(GRAY, 0.4f));
        DrawText("JUMP", jumpButtonPos.x - 22, jumpButtonPos.y - 8, 16, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
