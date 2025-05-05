#include "raylib/raylib.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "raylib/clay_renderer_raylib.c"
#include "src/utils/fetch.c"
// include "src/utils/strings.c"
#include <stdio.h>

#include "cJSON.h"
#include "mongoose/mongoose.h"


const float ScreenWidth = 1000.f;
const float ScreenHeight = 1000.f;

const int FONT_MONO = 0; 

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("Clay Error: %.*s\n", errorData.errorText.length, errorData.errorText.chars);
}


struct CRUDState {
    char categories[32][64]; // 32 categories, max 64 chars
    int categories_amount;
    char http_responce_buffer[4096];
};

void apiHandleCategoriesResponse(const char* data, size_t length, void* user_data) {
    struct CRUDState *state = (struct CRUDState*)user_data;
    cJSON *root = cJSON_Parse(data);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error parsing JSON before: %s\n", error_ptr);
        }
        return;
    }

    int array_size = cJSON_GetArraySize(root);
    state->categories_amount = array_size;
    for (int i = 0; i < array_size; i++) {
        cJSON *item = cJSON_GetArrayItem(root, i);

        if (cJSON_IsString(item) && item->valuestring != NULL) {
            strncpy(state->categories[i], item->valuestring, 63);
            state->categories[i][63] = '\0'; 
        }
    }
}



int main(void) {
    struct CRUDState state = {
        .categories = {},
        .categories_amount = 0,
        .http_responce_buffer = {0}
    };


    // Mongoose init 
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    
    mg_fetch_async(&mgr, "http://127.0.0.1:8000/api/categories", 
        apiHandleCategoriesResponse, &state);

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
            .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM } 
        }) {
            for (int i = 0; i < state.categories_amount; i++) {
                Clay_String category_clay_string = {
                    .isStaticallyAllocated = false,
                    .length = strlen(state.categories[i]),
                    .chars = state.categories[i]
                };

                CLAY_TEXT(
                    category_clay_string,
                    CLAY_TEXT_CONFIG({
                        .fontId = FONT_MONO,
                        .fontSize = 52,
                        .textColor = (Clay_Color){ 20, 50, 20, 255 }
                    })
                );
            }
        }

        
        Clay_RenderCommandArray renderCommands = Clay_EndLayout();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        Clay_Raylib_Render(renderCommands, fonts);
        EndDrawing();
    
        mg_mgr_poll(&mgr, 10);
    }

    free(memoryArena.memory);
    CloseWindow();
    
    return 0;
}