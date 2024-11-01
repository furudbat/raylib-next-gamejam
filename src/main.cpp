/*******************************************************************************************
 *
 *   raylib gamejam template
 *
 *   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
 *
 *   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include <raylib.h>

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS // Force custom modal dialogs usage
#include <emscripten/emscripten.h> // Emscripten library - LLVM to JavaScript compiler
#endif

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <memory>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#if !defined(PLATFORM_WEB)
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif
#else
#define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#include "constants.h"
#include "game.h"
#include "types.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

/// @NOTE: game needs to be global for emscripten, see UpdateDrawFrame (no parameter passing)
static std::unique_ptr<GameContext> g_gameContext{nullptr};
void UpdateGameLogic();
void UpdateDrawFrame(); // Update and Draw one frame

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int /*argc*/, char** /*argv*/)
{
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
    SetTraceLogLevel(LOG_DEBUG);
#else
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE); // Disable raylib trace log messages
#endif
#endif
#else
    SetTraceLogLevel(LOG_NONE);
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    // SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(ScreenWidth, ScreenHeight, "NeuroCircuit - raylib NEXT gamejam 2024");
    // InitAudioDevice();

    /// @NOTE: use unique_ptr, init game context AFTER init window to avoid some init. fiasco ... (problems with font loading...) ???
    g_gameContext = std::make_unique<GameContext>();

    //g_gameContext->font = LoadFontEx("resources/MonaspaceArgon-ExtraBold.otf", 32, 0, 250);
    g_gameContext->font = GetFontDefault();

    g_gameContext->logoTexture = LoadTexture("resources/logo.png");
    g_gameContext->instruction1Texture = LoadTexture("resources/instruction1.png");
    g_gameContext->instruction2Texture = LoadTexture("resources/instruction2.png");
    g_gameContext->tilesetTexture = LoadTexture("resources/tileset.png");
    g_gameContext->characterSpriteSheetTexture = LoadTexture("resources/character.png");
    g_gameContext->iconsSpriteSheetTexture = LoadTexture("resources/icons.png");
    g_gameContext->iconsControlSpriteSheetTexture = LoadTexture("resources/icons-control.png");
    g_gameContext->endTexture = LoadTexture("resources/end.png");


    constexpr int FPS = 60;
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, FPS, 1);
#else
    SetTargetFPS(FPS); // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------

    UnloadFont(g_gameContext->font);
    UnloadTexture(g_gameContext->logoTexture);
    UnloadTexture(g_gameContext->instruction1Texture);
    UnloadTexture(g_gameContext->instruction2Texture);
    UnloadTexture(g_gameContext->tilesetTexture);
    UnloadTexture(g_gameContext->characterSpriteSheetTexture);
    UnloadTexture(g_gameContext->iconsSpriteSheetTexture);
    UnloadTexture(g_gameContext->iconsControlSpriteSheetTexture);
    UnloadTexture(g_gameContext->endTexture);

    // CloseAudioDevice();
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return EXIT_SUCCESS;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
void UpdateGameLogic()
{
    assert(g_gameContext != nullptr);
    using fsec = std::chrono::duration<float>;
    g_gameContext->delta = std::chrono::duration_cast<std::chrono::milliseconds>(fsec{GetFrameTime()});
    switch (g_gameContext->state)
    {
        case GameState::Start: UpdateStartScene(*g_gameContext); break;
        case GameState::NodesMain: UpdateMainSceneNodes(*g_gameContext); break;
        case GameState::CharacterMain: UpdateMainSceneMap(*g_gameContext); break;
        case GameState::End: UpdateEndScene(*g_gameContext); break;
    }
    if (g_gameContext->state == GameState::NodesMain || g_gameContext->state == GameState::CharacterMain)
    {
        // help icon
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(Help1IconArea, g_gameContext->mouse))
        {
            g_gameContext->showHelp1 = !g_gameContext->showHelp1;
        }
        // guide line icon
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(Help2IconArea, g_gameContext->mouse))
        {
            g_gameContext->showHelp2 = !g_gameContext->showHelp2;
            g_gameContext->manuelHelp = true;
        }
    }

    // dev tools (debug)
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
    if (IsKeyPressed(KEY_F3))
    {
        NextLevel(*g_gameContext);
    }
#endif
#endif
}

// Update and draw frame
void UpdateDrawFrame()
{
    assert(g_gameContext != nullptr);
    auto mousePos = GetMousePosition();
    g_gameContext->mouse = {.x = mousePos.x, .y = mousePos.y, .width = 8, .height = 8};

    // Update
    UpdateGameLogic();

    // Draw
    //----------------------------------------------------------------------------------
    // Render to screen (main framebuffer)
    BeginDrawing();
    ClearBackground(BackgroundColor);

    // borders
    DrawRectangleLinesEx(ConnectorArea, WindowBorderLineThick, BorderColor);
    DrawRectangleLinesEx(LevelArea, WindowBorderLineThick, BorderColor);

    // render start scene
    if (g_gameContext->state == GameState::Start)
    {
        RenderStartScene(*g_gameContext);
    }
    // render main game
    else if (g_gameContext->state == GameState::NodesMain || g_gameContext->state == GameState::CharacterMain)
    {
        RenderMainScene(*g_gameContext);
    }
    // render end scene
    else if (g_gameContext->state == GameState::End)
    {
        RenderEndScene(*g_gameContext);
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
}
