#include <stdio.h>
#include <stdlib.h>

// #!/usr/bin/env -S tcc -run

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

/// Static Globals

#define CELL_COUNT 2500
#define FPS_COEF 2
#define TIME_COEF 2
#define MAP_CIRCULAR_SIZE (1920 / 2)

/// Structs

typedef struct {
    float rad;
    bool is_colliding_w_cell;
} Cell;

typedef struct {
} Bush;

typedef struct {
} Tree;

typedef struct {
} Animal;

/// Fat struct

typedef enum {
    K_INIT = 0,
    K_CELL,
    K_BUSH,
    K_TREE,
    K_ANIMAL,
} Kind;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Kind kind;
    union {
        Cell cell;
        Bush bush;
        Tree tree;
        Animal animal;
    } as;
} FatStruct;

typedef struct {
    FatStruct cells[CELL_COUNT];
} World;

/// Dynamic Globals

World world = {0};
Vector2 center = {
    .x = 1920 / 2,
    .y = 1080 / 2,
};

/// Functions

bool draw_cell(FatStruct c) {
    if (c.as.cell.is_colliding_w_cell) {
        DrawCircleV(c.pos, c.as.cell.rad, GOLD);
    } else {
        DrawCircleV(c.pos, c.as.cell.rad, MAROON);
    }
    return true;
}

bool game_init() {
    // int W = GetScreenWidth();
    // int H = GetScreenHeight();
    int W = 1920;
    int H = 1080;
    for (int i = 0; i < CELL_COUNT; i++) {
        FatStruct *c = &world.cells[i];

        c->pos.x = GetRandomValue(100, W - 100);
        c->pos.y = GetRandomValue(100, H - 100);

        c->vel.x = GetRandomValue(-5.0f, 5.0f);
        c->vel.y = GetRandomValue(-5.0f, 5.0f);

        c->as.cell.rad = GetRandomValue(1, 20);
    }

    return true;
}

bool game_update() {
    // int W = GetScreenWidth();
    // int H = GetScreenHeight();

    for (int i = 0; i < CELL_COUNT; i++) {
        FatStruct *c = &world.cells[i];

        // Movement
        c->pos.x += c->vel.x;
        c->pos.y += c->vel.y;

        // Collision detection w/Walls
#define PEDO
#ifdef PEDO
        const float map_rad = 1920 / 2;

        // if (Vector2Distance(c->pos, center) + c->rad > map_rad) {
        //     // set position to the map circular border minus radius, making it touch the border
        //     // set velocity to a mirror using the normal of the circular border pointing to our c->pos
        // }

        // AI
        // Vector from Center to Ball
        float dx = c->pos.x - center.x;
        float dy = c->pos.y - center.y;

        // Distance from center
        float dist = sqrtf((dx*dx) + (dy*dy));

        if (dist + c->as.cell.rad > map_rad) {
            // 1. Calculate Normal (Direction from Center to Ball)
            float nx = dx / dist;
            float ny = dy / dist;

            // 2. Position Correction (Clamp to edge)
            // The ball is too far out. Pull it back so it just touches the rim.
            float touchDist = map_rad - c->as.cell.rad;
            c->pos.x = center.x + (nx * touchDist);
            c->pos.y = center.y + (ny * touchDist);

            // 3. Velocity Reflection (Bounce off the curved wall)
            // Calculate velocity along normal (Radial Velocity)
            float vDotN = (c->vel.x * nx) + (c->vel.y * ny);

            // Only bounce if moving OUTWARD
            if (vDotN > 0) {
                // Apply bounce impulse (-2.0 is perfectly elastic)
                float bounce = -2.0f * vDotN;
                c->vel.x += bounce * nx;
                c->vel.y += bounce * ny;

                // Add a little friction/damping here if you want:
                // c->vel.x *= 0.9f;
            }
        }

#else
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
#endif // PEDO
        c->as.cell.is_colliding_w_cell = false;

        // Collision detection w/Cells
        for (int j = 0; j < CELL_COUNT; j++) {
            FatStruct *c2 = &world.cells[j];

            if (i == j) continue; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!

            // get distance squared
            float dx = c->pos.x - c2->pos.x;
            float dy = c->pos.y - c2->pos.y;

            float dist_squared = (dx * dx) + (dy * dy);

            float rad_sum = c->as.cell.rad + c2->as.cell.rad;
            float rad_sum_squared = rad_sum * rad_sum;

            // check
            if (dist_squared < rad_sum_squared) {
                c->as.cell.is_colliding_w_cell = true;

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
    }

    return true;
}

bool game_render() {
    for (int i = 0; i < CELL_COUNT; i++) {
        draw_cell(world.cells[i]);
    }
    return true;
}

int main(void) {
    printf("Hello, World\n");

#ifdef RELEASE
    printf("Release!\n");
#else
    printf("No Release!\n");
#endif // RELEASE

    int screenX = 1280;
    int screenY = 720;
    InitWindow(screenX, screenY, "creaturettes");
    SetTargetFPS(FPS_COEF * 60);
    SetExitKey(KEY_ESCAPE);
    // ToggleFullscreen();
    // WaitTime(1.0);
    // ToggleBorderlessWindowed();

    game_init();
    bool gogo = false;
    bool show_help = false;
    bool show_debug_info = false;

    int cycle_pos = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKBROWN);

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_F11) || ( IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT) )) {
            gogo = !gogo;
            ToggleBorderlessWindowed();
        }

        if (IsKeyPressed(KEY_F1)) {
            show_help = !show_help;
        }
        if (IsKeyPressed(KEY_F8)) {
            show_debug_info = !show_debug_info;
        }

        if (IsKeyPressed(KEY_F)) {
            for (int i = 0; i < 1000; i++) {
                game_update();
            }
        }

        if (gogo) {
            cycle_pos += 1;
            cycle_pos %= TIME_COEF;

            if (cycle_pos == 0) game_update(); // GAME !
            game_render();
        }

        if (show_help) {
            DrawText(
                "F1: Help\n"
                "F8: display Debug Info\n"

                "=========", 30, 60, 20, BLACK
            );
        } else {
            DrawText("F1: Help", 30, 60, 20, BLACK);
        }

        if (show_debug_info) {
            // TODO: render gameplay info like gravity lines etc or whatever
            DrawText("DEBUGINFOPLACEHOLDER", 120, 60, 20, BLACK);
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
