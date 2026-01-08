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
   PLAYER
============================= */
void DrawPlayer(Vector2 pos, Vector2 dir, float speed, float time)
{
    float angle = atan2f(dir.y, dir.x);
    float bob = sinf(time * 10.0f) * speed * 4.0f;

    float squash = 1.0f - speed * 0.15f;
    float stretch = 1.0f + speed * 0.10f;

    Vector2 p = { pos.x, pos.y + bob };

    DrawEllipse(p.x, p.y, 22 * stretch, 22 * squash, DARKGREEN);
    DrawCircleV(Vector2Add(p, (Vector2){ cosf(angle)*14, sinf(angle)*14 }), 12, GREEN);
    DrawCircleV(Vector2Add(p, (Vector2){ cosf(angle)*28, sinf(angle)*28 }), 4, YELLOW);
}

/* =============================
   STARS (FAR SKY)
============================= */
typedef struct {
    Vector2 pos;
    float phase;
    float speed;
} Star;

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
   PARALLAX
============================= */
void DrawParallax(float cameraX)
{
    float farX = -cameraX * 0.2f;
    for (int i = -1; i < 12; i++)
        DrawTriangle(
            (Vector2){farX+i*400+200,240},
            (Vector2){farX+i*400,400},
            (Vector2){farX+i*400+400,400},
            DARKPURPLE);

    float midX = -cameraX * 0.4f;
    for (int i = -1; i < 16; i++)
        DrawCircle(midX+i*260,420,160,DARKBLUE);
}

/* =============================
   MAIN
============================= */
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "U-MG Stars Fixed");
    SetTargetFPS(60);

    Vector2 player = {200, GROUND_Y};
    Vector2 facing = {1,0};
    float speed = 0, velY = 0;
    bool grounded = true;
    float cameraX = 0;

    float transitionCenter = WORLD_WIDTH * 0.5f;
    float transitionWidth  = 600.0f;

    float sunX = SCREEN_WIDTH - 80;
    float sunStartY = 80, sunEndY = SCREEN_HEIGHT + 120, sunRadius = 220;

    float moonX = 80;
    float moonStartY = SCREEN_HEIGHT + 120, moonEndY = 100, moonRadius = 160;

    VirtualJoystick joy = {{120,SCREEN_HEIGHT-120},{120,SCREEN_HEIGHT-120},60};
    Vector2 jumpBtn = {SCREEN_WIDTH-120,SCREEN_HEIGHT-120};
    float jumpRadius = 40;

    while (!WindowShouldClose())
    {
        float time = GetTime();
        Vector2 mouse = GetMousePosition();
        speed = 0;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointCircle(mouse, joy.base, joy.radius))
            joy.active = true;

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && joy.active)
        {
            Vector2 d = Vector2Subtract(mouse, joy.base);
            if (Vector2Length(d) > joy.radius)
                d = Vector2Scale(Vector2Normalize(d), joy.radius);
            joy.delta = Vector2Normalize(d);
            joy.knob = Vector2Add(joy.base, d);
            speed = fabsf(joy.delta.x);
            player.x += joy.delta.x * speed * 5.5f;
            facing.x = joy.delta.x >= 0 ? 1 : -1;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            joy.active = false;
            joy.knob = joy.base;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && grounded &&
            CheckCollisionPointCircle(mouse, jumpBtn, jumpRadius))
        {
            velY = JUMP_VELOCITY;
            grounded = false;
        }

        velY += GRAVITY;
        player.y += velY;
        if (player.y >= GROUND_Y)
        {
            player.y = GROUND_Y;
            velY = 0;
            grounded = true;
        }

        player.x = Clamp(player.x, 0, WORLD_WIDTH);
        cameraX = Clamp(player.x - SCREEN_WIDTH*0.4f, 0, WORLD_WIDTH-SCREEN_WIDTH);

        float t = Clamp((player.x - (transitionCenter-transitionWidth*0.5f))/transitionWidth,0,1);
        float ambient = Lerp(DAY_AMBIENT, NIGHT_AMBIENT, t);
        float sunY = Lerp(sunStartY, sunEndY, t);
        float moonY = Lerp(moonStartY, moonEndY, t);

        BeginDrawing();
        ClearBackground(SKYBLUE);

        DrawParallax(cameraX);
        DrawRectangle(-cameraX, GROUND_Y+24, WORLD_WIDTH, 200, DARKBROWN);
        DrawPlayer((Vector2){player.x-cameraX,player.y}, facing, speed, time);

        BeginBlendMode(BLEND_MULTIPLIED);
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,Fade(BLACK,ambient));
        EndBlendMode();

        DrawStars(t, time);

        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient(sunX,sunY,sunRadius,Fade(YELLOW,1-t),Fade(BLACK,0));
        DrawCircleGradient(moonX,moonY,moonRadius,Fade(RAYWHITE,t),Fade(BLACK,0));
        EndBlendMode();

        DrawCircleV(joy.base, joy.radius, Fade(DARKGRAY,0.5f));
        DrawCircleV(joy.knob, 25, GRAY);
        DrawCircleV(jumpBtn, jumpRadius, grounded?Fade(GREEN,0.6f):Fade(GRAY,0.4f));
        DrawText("JUMP", jumpBtn.x-22, jumpBtn.y-8, 16, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
