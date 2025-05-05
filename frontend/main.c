#include "raylib/raylib.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "raylib/clay_renderer_raylib.c"

#include <stdio.h>

#include "mongoose/mongoose.h"


const float ScreenWidth = 1000.f;
const float ScreenHeight = 1000.f;

const int FONT_MONO = 0; 

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("Clay Error: %.*s\n", errorData.errorText.length, errorData.errorText.chars);
}


int main(void) {
    Clay_Raylib_Initialize((int)ScreenWidth, (int)ScreenHeight, "test", FLAG_WINDOW_RESIZABLE);
    
    uint64_t clayMemorySize = Clay_MinMemorySize();
    void* memory = malloc(clayMemorySize);
    if (!memory) {
        printf("Failed to allocate memory for Clay\n");
        CloseWindow();
        return 1;
    }
    
    Clay_Arena memoryArena = {
        .memory = memory,
        .capacity = clayMemorySize,
    };
    
    Clay_Dimensions dimensions = {
        .width = ScreenWidth,
        .height = ScreenHeight
    };

    Clay_ErrorHandler errorHandler = {
        .errorHandlerFunction = HandleClayErrors,
        .userData = NULL
    };

    Clay_Initialize(memoryArena, dimensions, errorHandler);
    Font fonts[1] = {0}; 

    if (FileExists("resources/RobotoMono-Medium.ttf")) {
        fonts[FONT_MONO] = LoadFontEx("resources/RobotoMono-Medium.ttf", 48, 0, 400);
        SetTextureFilter(fonts[FONT_MONO].texture, TEXTURE_FILTER_BILINEAR);
    } else {
        printf("Warning: Font file not found, using default font\n");
        fonts[FONT_MONO] = GetFontDefault(); 
    }
    
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
    
    SetTargetFPS(120);
    EnableEventWaiting();

    while (!WindowShouldClose()) {
        dimensions.width = GetScreenWidth();
        dimensions.height = GetScreenHeight();
        Clay_SetLayoutDimensions(dimensions);
        
        Clay_Vector2 mousePos = { GetMouseX(), GetMouseY() };
        Clay_SetPointerState(mousePos, IsMouseButtonDown(MOUSE_LEFT_BUTTON));
        
        Clay_BeginLayout();
        
        CLAY({
            .layout = { .padding = { .left = mousePos.x, .top = mousePos.y } } 
        }) {
            
        }
        Clay_RenderCommandArray renderCommands = Clay_EndLayout();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        Clay_Raylib_Render(renderCommands, fonts);
        EndDrawing();
    }

    free(memoryArena.memory);
    CloseWindow();
    
    return 0;
}