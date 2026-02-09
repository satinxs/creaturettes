#include <stdio.h>
#include <stdlib.h>

// // #!/usr/bin/env -S tcc -run

// #include <stdio.h>

// // celularam

// // typedef struct Vector2 {
// //     float x;
// //     float y;
// // } Vector2;

// int main(int argc, char *argv[]) {
//     printf("Hello, World!\n");
//     return 0;
// }

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include <raylib.h>
#include <raymath.h>

// bool sh(char *str, ...) {
//     static Cmd cmd = {0};

//     cmd.count = 0;
//     cmd_append(&cmd, str);
//     return cmd_run_sync(cmd);
// }

//////////////////////////////////////////////////////////
/// FUN BEGINS HERE !
//////////////////////////////////////////////////////////

#define CELL_COUNT 1000

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float rad;
} Cell;

typedef struct {
    Cell cells[CELL_COUNT];
} CellGroup;

bool draw_cell(Cell c) {
    DrawCircleV(c.pos, c.rad, GREEN);
    return true;
}

CellGroup world = {0};

bool game_init() {
    for (int i = 0; i < CELL_COUNT; i++) {
        world.cells[i].pos.x = GetRandomValue(100, GetScreenWidth() - 100);
        world.cells[i].pos.y = GetRandomValue(100, GetScreenHeight() - 100);

        world.cells[i].vel.x = GetRandomValue(-5.0f, 5.0f);
        world.cells[i].vel.y = GetRandomValue(-5.0f, 5.0f);

        world.cells[i].rad = GetRandomValue(1, 20);
    }

    return true;
}

//////////////////////////////////////////////////////////
/// Game Update Loop!
//////////////////////////////////////////////////////////

bool game_update() {
    int W = GetScreenWidth();
    int H = GetScreenHeight();

    for (int i = 0; i < CELL_COUNT; i++) {
        Cell *c = &world.cells[i];

        // Movement
        c->pos.x += c->vel.x;
        c->pos.y += c->vel.y;

        // Collision detection w/Walls
        if (c->pos.x - c->rad <= 0) {
            c->pos.x = c->rad;
            c->vel.x = -c->vel.x;
        }

        if (c->pos.x + c->rad >= W) {
            c->pos.x = W - c->rad;
            c->vel.x = -c->vel.x;
        }

        if (c->pos.y - c->rad <= 0) {
            c->pos.y = c->rad;
            c->vel.y = -c->vel.y;
        }

        if (c->pos.y + c->rad >= H) {
            c->pos.y = H - c->rad;
            c->vel.y = -c->vel.y;
        }

        // Collision detection w/Cells
        for (int j = 0; j < CELL_COUNT; j++) {
            Cell *c2 = &world.cells[j];

            // get distance squared
            float dx = c->pos.x - c2->pos.x;
            float dy = c->pos.y - c2->pos.y;

            float dist_squared = (dx * dx) + (dy * dy);

            float rad_sum = c->rad + c2->rad;
            float rad_sum_squared = rad_sum * rad_sum;

            // check
            if (dist_squared < rad_sum_squared) {

                float dist = sqrtf(dist_squared);
                if (dist == 0.0f) dist = 0.01f; // Avoid div by zero

                // Normal Vector (Direction of collision)
                float nx = dx / dist;
                float ny = dy / dist;

                // A. Position Correction (Stop them from overlapping)
                float overlap = rad_sum - dist;
                // float push = overlap * 0.5f; // change for weird effects
                // float push = overlap * 1.0f;
                float push = overlap * 0.8f;

                // Push apart along normal
                c->pos.x -= nx * push;
                c->pos.y -= ny * push;
                c2->pos.x += nx * push;
                c2->pos.y += ny * push;

                // B. Velocity Reflection (Bounce)
                // Relative Velocity
                float rvx = c->vel.x - c2->vel.x;
                float rvy = c->vel.y - c2->vel.y;

                // Velocity along normal
                float vel_norm = (rvx * nx) + (rvy * ny);

                // If moving away, don't resolve
                if (vel_norm > 0) continue;

                // Impulse (1.0 = restitution/bounciness)
                float impulse = -(1.0f + 1.0f) * vel_norm;
                impulse /= 2.0f; // Equal mass assumption

                // SOFTENER:
                // reduce the impulse to make it a soft sim
                impulse *= 0.02f;

                // Apply Impulse
                float ix = impulse * nx;
                float iy = impulse * ny;

                c->vel.x += ix;
                c->vel.y += iy;
                c2->vel.x -= ix;
                c2->vel.y -= iy;


                // // --- SOLVER --- AI

                // float dist = sqrtf(dist_squared);
                // if (dist == 0.0f) dist = 0.01f; // Avoid div by zero

                // // Normal Vector (Direction of collision)
                // float nx = dx / dist;
                // float ny = dy / dist;

                // // A. Position Correction (Stop them from overlapping)
                // float overlap = rad_sum - dist;
                // // float push = overlap * 0.5f; // change for weird effects
                // // float push = overlap * 1.0f;
                // float push = overlap * 0.8f;

                // // Push apart along normal
                // c->pos.x -= nx * push;
                // c->pos.y -= ny * push;
                // c2->pos.x += nx * push;
                // c2->pos.y += ny * push;

                // // B. Velocity Reflection (Bounce)
                // // Relative Velocity
                // float rvx = c->vel.x - c2->vel.x;
                // float rvy = c->vel.y - c2->vel.y;

                // // Velocity along normal
                // float vel_norm = (rvx * nx) + (rvy * ny);

                // // If moving away, don't resolve
                // if (vel_norm > 0) continue;

                // // Impulse (1.0 = restitution/bounciness)
                // float impulse = -(1.0f + 1.0f) * vel_norm;
                // impulse /= 2.0f; // Equal mass assumption

                // // Apply Impulse
                // float ix = impulse * nx;
                // float iy = impulse * ny;

                // c->vel.x += ix;
                // c->vel.y += iy;
                // c2->vel.x -= ix;
                // c2->vel.y -= iy;
            }
        }

        // Draw
        draw_cell(*c);
    }

    return true;
}

// int main(int argc, char **argv) {
int main() {
    printf("Hello, World\n");

    Cmd cmd = {0};
    #define C(...) cmd_append(&cmd, __VA_ARGS__);
    #define RUN if (!cmd_run_sync_and_reset(&cmd)) return 1;

    // C("ls", "-lA"); // test pwsh or bash as original callers. difference?
    // RUN

#ifdef RELEASE
    printf("Release!\n");
#else
    printf("No Release!\n");
#endif // RELEASE

    _putenv_s("HOME", "foo");
    const char *home = getenv("HOME");
    printf("Here's your $HOME: %s\n", home);

    C("bash", "./bashscript.sh"); RUN

    // InitWindow(1920, 1080, "Raylib Template");
    int screenX = 1280;
    int screenY = 720;
    InitWindow(screenX, screenY, "rl");
    SetTargetFPS(60);
    SetExitKey(KEY_ESCAPE);
    // ToggleFullscreen();
    // WaitTime(1.0);
    // ToggleBorderlessWindowed();

    game_init();
    bool gogo = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (IsKeyPressed(KEY_F11) || ( IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT) )) {
            gogo = !gogo;
            ToggleBorderlessWindowed();
        }

        if (gogo) {
            game_update(); // GAME !
        }

        DrawText("creaturettes", 30, 30, 20, GREEN);
        DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}

// // 1. Initialization (Load Once)
// // Make a white circle PNG with a transparent background.
// Texture2D myTexture = LoadTexture("resources/circle.png");

// // 2. The Draw Call (Inside Loop)
// // Source: The full image (0,0 to width,height). change these later for animation.
// Rectangle sourceRec = { 0.0f, 0.0f, (float)myTexture.width, (float)myTexture.height };

// // Dest: Where to draw on screen. X, Y, Width, Height.
// // This allows you to scale the texture independently of the source image size.
// Rectangle destRec = {
//     myPosition.x,
//     myPosition.y,
//     sourceRec.width * 2.0f,  // Example: Scale 2x
//     sourceRec.height * 2.0f
// };

// // Origin: The pivot point relative to top-left of destRec.
// // Setting this to half width/height makes the texture rotate around its CENTER.
// Vector2 origin = { destRec.width / 2.0f, destRec.height / 2.0f };

// // Rotation: In degrees
// float rotation = 0.0f;

// // Color: Tinting. WHITE means "No tint, use original texture colors".
// // If your texture is white, setting this to RED makes it red.
// DrawTexturePro(myTexture, sourceRec, destRec, origin, rotation, WHITE);

// // 3. Cleanup (End of Program)
// UnloadTexture(myTexture);
