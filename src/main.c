#include <stdio.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>

////////////////////////////////////////////////////////////

/// Static Globals

#define FPS_COEF 5 // game fps cap will be 60 * FPS_COEF
#define TIME_COEF 1 // game update tick happens every this number of frames
                    // if both are equal, then game is 60 ticks per second
#define RESET_TIME (3 * 1024) // these ticks and auto-reset if enabled
#define TICKS_TO_FREEDOM 1 // ticks until map opens up

// #define MAP_CIRCULAR_SIZE (1920 / 2) // overridden by the float below for now (temporary)

// #define CELL_COUNT 1000
#define CELL_COUNT 2500
// #define CELL_COUNT 3500
// #define CELL_COUNT 4500
// #define CELL_COUNT 6000

// Stable (short-lived cells as per prototype_build_001)
// DO NOT modify/delete
#define PARTICELL_STARTING_VELOCITY 5.0f
#define CELL_HITPOINTS   (1 * 1 * 1024)
#define CELL_CORPSE_SPAN (1 * 8 * 1024)
#define CELL_CORPSE_HIT_DAMAGE (32)
// #define CELL_HITPOINTS   (4 * 8 * 1024)
// #define CELL_CORPSE_SPAN (2 * 8 * 1024)
// #define CELL_CORPSE_HIT_DAMAGE (32+16)

#ifdef EXPERIMENTAL

// experimental (?)
#define RESET_TIME (1 * 1536) // these ticks and auto-reset if enabled
#define PARTICELL_STARTING_VELOCITY 600.0f
#define CELL_HITPOINTS   (2 * 8 * 1024)
#define CELL_CORPSE_SPAN (4 * 8 * 1024)
#define CELL_CORPSE_HIT_DAMAGE (32+16)

#endif // EXPERIMENTAL

// #define SPAWN_PERIOD (10 * 60) // ambitious TODO, make it spawn more cells every so often

// float MAP_CIRCULAR_SIZE = 600 / 2;
float MAP_CIRCULAR_SIZE = 1920 / 2;

/// Structs

typedef struct {
    bool is_colliding_w_cell;
    bool is_dead;
    bool is_nonexistent;
    float rad;
    int hit_counter;
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

typedef enum {
    PARTY_MODE_INIT = 0,
    PARTY_MODE_FIRE,
    PARTY_MODE_VLUE,
    PARTY_MODE_CLOWN,
} PartyMode;

PartyMode party_mode = 1;

int timer = 0;

/// Functions

char party_color_calculate(char base_color, float bright_factor, char brightness, float rad) {
    switch (party_mode) {
    case PARTY_MODE_INIT:
        return base_color;
    case PARTY_MODE_FIRE:
        return base_color + ((250 - base_color) * brightness * bright_factor / 20); // flame!
    case PARTY_MODE_VLUE:
        return base_color + ((250 - base_color) * brightness / 20); // Vlue Vlask
    case PARTY_MODE_CLOWN:
        return base_color + ((GetRandomValue(0, (250 - base_color))) * rad); // CLOWN !!!!!!!!
    default:
        return base_color;
    }
}

bool draw_cell(FatStruct c) {

    const float distance_to_center = Vector2Length((Vector2){
        .x = c.pos.x - center.x,
        .y = c.pos.y - center.y,
    });
    const float opacity = 0.9f * (MAP_CIRCULAR_SIZE - distance_to_center) / MAP_CIRCULAR_SIZE;

    if (c.as.cell.is_nonexistent) return true;
    if (c.as.cell.is_dead) {
        DrawCircleV(c.pos, c.as.cell.rad, CLITERAL(Color){ 130, 50, 10, opacity * 64 });
        // DrawCircleV(c.pos, c.as.cell.rad, CLITERAL(Color){ 200, 60, 45, 64 }); // Maroon modified
                    // #define GOLD       CLITERAL(Color){ 255, 203, 0, 255 }     // Gold
                    // #define MAROON     CLITERAL(Color){ 190, 33, 55, 255 }     // Maroon
    } else if (c.as.cell.is_colliding_w_cell) {
        // DrawCircleV(c.pos, c.as.cell.rad, GOLD);
                    // #define MAROON     CLITERAL(Color){ 190, 33, 55, 255 }     // Maroon
        const char brightness = GetRandomValue(1, (21 - c.as.cell.rad));
        DrawCircleV(c.pos, c.as.cell.rad, CLITERAL(Color){
            party_color_calculate(190, 0.2f, brightness, c.as.cell.rad),
            party_color_calculate(43, 0.6f, brightness, c.as.cell.rad),
            party_color_calculate(45, 0.0f, brightness, c.as.cell.rad),
            opacity * 255
        });
    } else {
        DrawCircleV(c.pos, c.as.cell.rad, CLITERAL(Color){ 160, 42, 40, opacity * 255 });
                    // #define MAROON     CLITERAL(Color){ 190, 33, 55, 255 }     // Maroon
    }
    return true;
}

bool game_init() {
    // int W = GetScreenWidth();
    // int H = GetScreenHeight();
    int W = 1920;
    int H = 1080;

    memset(&world, 0, sizeof(World));

    for (int i = 0; i < CELL_COUNT; i++) {
        FatStruct *c = &world.cells[i];

        c->pos.x = GetRandomValue(100, W - 100);
        c->pos.y = GetRandomValue(100, H - 100);

        // c->vel.x = GetRandomValue(-5.0f, 5.0f);
        // c->vel.y = GetRandomValue(-5.0f, 5.0f);

        c->vel.x = GetRandomValue(-PARTICELL_STARTING_VELOCITY, PARTICELL_STARTING_VELOCITY);
        c->vel.y = GetRandomValue(-PARTICELL_STARTING_VELOCITY, PARTICELL_STARTING_VELOCITY);

        c->as.cell.rad = GetRandomValue(1, 20);
    }

    return true;
}

bool game_update() {
    // int W = GetScreenWidth();
    // int H = GetScreenHeight();

    for (int i = 0; i < CELL_COUNT; i++) {
        FatStruct *c = &world.cells[i];

        if (c->as.cell.is_nonexistent) continue; // The bigger continue!

        // Movement
        c->pos.x += c->vel.x;
        c->pos.y += c->vel.y;

        // Collision detection w/Walls
#define USE_CIRCULAR_MAP // temporary variable to switch to a circular map
#ifdef USE_CIRCULAR_MAP
        const float map_rad = MAP_CIRCULAR_SIZE;

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

#else // USE_CIRCULAR_MAP
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
#endif // USE_CIRCULAR_MAP

        if (c->as.cell.is_dead) { // The big continue!
            c->as.cell.hit_counter += 1;
            continue;
        }

        c->as.cell.is_colliding_w_cell = false;

        // Collision detection w/Cells
        for (int j = 0; j < CELL_COUNT; j++) {
            FatStruct *c2 = &world.cells[j];

            if (i == j) continue; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!

            if (c2->as.cell.is_nonexistent) continue; // The third big continue!

            // get distance squared
            float dx = c->pos.x - c2->pos.x;
            float dy = c->pos.y - c2->pos.y;

            float dist_squared = (dx * dx) + (dy * dy);

            float rad_sum = c->as.cell.rad + c2->as.cell.rad;
            float rad_sum_squared = rad_sum * rad_sum;

            // check
            if (dist_squared < rad_sum_squared) {
                c->as.cell.is_colliding_w_cell = true;
                c->as.cell.hit_counter += 1;

                if ( ! c->as.cell.is_dead) { // alive still // unnecessary check
                    if (c->as.cell.hit_counter >= CELL_HITPOINTS) {
                        c->as.cell.hit_counter = 0;
                        c->as.cell.is_dead = true;
                    }
                }

                // turned out to be the alive cell's responsibility
                // to "ripe" the dead cell when colliding
                if (c2->as.cell.is_dead) {
                    c2->as.cell.hit_counter += CELL_CORPSE_HIT_DAMAGE;
                    if (c2->as.cell.hit_counter >= CELL_CORPSE_SPAN) {
                        c2->as.cell.is_nonexistent = true;
                    }
                }

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

bool show_debug_info = false; // TODO: consider putting in Globals top part of the file

bool game_render() {
    for (int i = 0; i < CELL_COUNT; i++) {
        draw_cell(world.cells[i]);
    }

    if (show_debug_info) {
        // TODO: render gameplay info like gravity lines etc or whatever

#define MAP_CIRCLE_THICKNESS 3
        DrawRing((Vector2){
            .x = (1920/2), .y = (1080/2)
        }, MAP_CIRCULAR_SIZE, MAP_CIRCULAR_SIZE + MAP_CIRCLE_THICKNESS, 0.0f, 360.0f, 128, CLITERAL(Color){ 0, 0, 0, 255 });
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

    /// Globals Main:

    bool reset_enabled = true;

    int screenX = 1280;
    int screenY = 720;

    /// 2D Starts:

    InitWindow(screenX, screenY, "creaturettes");
    SetTargetFPS(FPS_COEF * 60);
    SetExitKey(KEY_ESCAPE);
    // ToggleFullscreen();
    // WaitTime(1.0);
    // ToggleBorderlessWindowed();

    game_init();
    bool gogo = false;
    bool show_help = false;
    // bool show_debug_info = false; // needs to be global! (this variable was moved outdoors)

    long long cycle_pos = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKBROWN);

        timer += 1;
        if (reset_enabled && (timer > RESET_TIME)) {
            timer = 0;
            game_init();
        }

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_F11) || ( IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT) )) {
            gogo = !gogo;
            ToggleBorderlessWindowed();
        }

        if (IsKeyPressed(KEY_F1)) {
            show_help = !show_help;
        }
        if (gogo && IsKeyPressed(KEY_F5)) {
            game_init();
        }
        if (IsKeyPressed(KEY_F8)) {
            show_debug_info = !show_debug_info;
        }

        if (gogo && IsKeyPressed(KEY_F)) {
            for (int i = 0; i < 1000; i++) {
                game_update();
            }
        }

        if (IsKeyPressed(KEY_R)) {
            reset_enabled = !reset_enabled;
        }

        if (gogo) {
            if      (IsKeyPressed(KEY_ZERO )) party_mode = 0;
            else if (IsKeyPressed(KEY_ONE  )) party_mode = 1;
            else if (IsKeyPressed(KEY_TWO  )) party_mode = 2;
            else if (IsKeyPressed(KEY_THREE)) party_mode = 3;
        }

        // if (gogo && (cycle_pos == (TICKS_TO_FREEDOM * FPS_COEF) || IsKeyPressed(KEY_J))) {
        //     MAP_CIRCULAR_SIZE = 1920 / 2;
        // }

        if (gogo) {
            cycle_pos += 1;

            if ((cycle_pos % TIME_COEF) == 0) game_update(); // GAME !
            game_render();
        } else {
            DrawText(
                "SPACE: Play !\n"
                "==============", 450, 300, 40, GREEN
            );
        }

        if (show_help) {
            // @@ for future use
            DrawText(
                //@@ of the pre-preprocessor
                "F1: Help\n"
                "ESC: Exit\n"
                "SPACE | F11 | Alt+ENTER: Play/Pause\n"
                "F5: Reset Simulation\n"
                "F8: Display Debug Info\n"
                "r: reset on-off\n"
                "f: fast forward 16 seconds (slow)\n"

                , 20, 52, 20, CLITERAL(Color){ 0, 228, 48, (128 + 64 + 32) }
            );
        } else {
            if (gogo) {
                DrawText(
                    "F1: Help"
                    , 20, 52, 20, CLITERAL(Color){ 0, 0, 0, 128 });
            } else {
                DrawText(
                    "F1: Help"
                    , 20, 52, 20, CLITERAL(Color){ 0, 0, 0, 255 });
            }
        }

        char timer_display[32] = {0};
        sprintf(timer_display, "t%3d", timer);

        if (show_debug_info) {
            DrawText(timer_display, 180, 30, 20, CLITERAL(Color){ 0, 126, 100, 255 });

            if (reset_enabled) {
                DrawText("reset_on",  180, 6, 20, CLITERAL(Color){  12, 196, 120, 255 });
            } else {
                DrawText("reset_off", 180, 6, 20, CLITERAL(Color){ 220,  50,  20, 255 });
            }
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
