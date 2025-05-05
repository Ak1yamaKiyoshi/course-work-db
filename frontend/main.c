#include "raylib/raylib.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "raylib/clay_renderer_raylib.c"

#include <stdio.h>


const float ScreenWidth = 1000.f;
const float ScreenHeight = 1000.f;

int main(void) {
    Clay_Raylib_Initialize((int)ScreenWidth, (int)ScreenHeight, "test", FLAG_WINDOW_RESIZABLE);
    
    uint64_t clayMemorySize = Clay_MinMemorySize();
    Clay_Arena memoryArena = {
        .memory = malloc(clayMemorySize),
        .capacity = clayMemorySize,
    };


    Clay_Dimensions dimensions = {
        .width = ScreenWidth,
        .height = ScreenHeight
    };

    Clay_ErrorHandler errorHandler;

    Clay_Initialize(memoryArena, dimensions, errorHandler);
    SetTargetFPS(24);

    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(YELLOW);
        // render 
        EndDrawing();
    }


} 