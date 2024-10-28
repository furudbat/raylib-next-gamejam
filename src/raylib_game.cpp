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
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <string_view>
#include <array>
#include <chrono>
#include <cassert>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#if defined(PLATFORM_WEB)
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
inline constexpr std::array<Color, 8> ColorPalette {
    Color{39, 41, 70, 255},
    Color{237, 160, 49, 255},
    Color{231, 255, 238, 255},
    Color{96, 34, 34, 255},
    Color{103, 160, 160, 255},
    Color{129, 15, 166, 255},
    Color{237, 35, 61, 255},
    RAYWHITE,
};
inline constexpr Rectangle ConnectorArea {
    0, 0, 400, 450,
};
inline constexpr Rectangle LevelArea {
    401, 0, 399, 450,
};

inline constexpr std::chrono::milliseconds StartCooldown {3800};
enum class GameState
{
    Start,
    Main,
    End,
};

enum class ConnectorType
{
    DISABLED,
    Action,
    Key,
};
enum class ConnectorAction : uint8_t
{
    NONE,
    MovementLeft,
    MovementRight,
    MovementUp,
    MovementDown,
    Jump,
};
enum class ConnectorKey : int
{
    NONE = KEY_NULL,
    W = KEY_W,
    A = KEY_A,
    S = KEY_S,
    D = KEY_D,
    Space = KEY_SPACE,
};

inline constexpr int MaxNodeConnections = 2;
inline constexpr int MaxIndirectConnections = 2;
struct ConnectorNode {
    int index{-1};
    Vector2 position{0, 0};
    ConnectorAction action{ConnectorAction::NONE};
    ConnectorKey key{ConnectorKey::NONE};
    ConnectorType type{ConnectorType::DISABLED};
    int connected_counter{0};
    bool is_selected{false};
    std::array<int, MaxNodeConnections> direct_connections{};

    // computed
    std::array<ConnectorNode*, MaxIndirectConnections> connected_nodes{};
    std::vector<ConnectorAction> connected_actions{};

    ConnectorNode()
    {
        direct_connections.fill(-1);
        connected_nodes.fill(nullptr);
        connected_actions.reserve(MaxIndirectConnections);
    }
};
struct GameContext
{
    std::array<ConnectorNode, 10> nodes;
    std::chrono::milliseconds delta{std::chrono::milliseconds::zero()};
    int score{0};
    GameState state {GameState::Start};
    std::chrono::milliseconds start_cooldown {StartCooldown};
    std::chrono::milliseconds timer {std::chrono::milliseconds::zero()};
    std::chrono::milliseconds start_time{std::chrono::milliseconds::zero()};

    void clearNodes ()
    {
        assert(nodes.size() <= std::numeric_limits<int>::max());
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            nodes[i] = {};
            nodes[i].index = i;
        }
    }
};
constexpr void setActionNode(ConnectorNode& node, Vector2 pos, ConnectorAction action)
{
    node.position = pos;
    node.action = action;
    node.key = ConnectorKey::NONE;
    node.type = ConnectorType::Action;
    node.is_selected = false;
}
constexpr void setKeyNode(ConnectorNode& node, Vector2 pos, ConnectorKey key)
{
    node.position = pos;
    node.action = ConnectorAction::NONE;
    node.key = key;
    node.type = ConnectorType::Key;
    node.is_selected = false;
}
constexpr void clearKeyNode(ConnectorNode& node)
{
    //node.index = -1;
    node.position = {0, 0};
    node.action = ConnectorAction::NONE;
    node.key = ConnectorKey::NONE;
    node.type = ConnectorType::DISABLED;
    node.is_selected = false;
}

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
inline constexpr int ScreenWidth = 800;
inline constexpr int ScreenHeight = 450;

/// @NOTE: game needs to be static for emscripten, see UpdateDrawFrame (no parameter passing)
static GameContext game_context;

// TODO: Define global variables here, recommended to make them static

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateGameLogic();
static void UpdateDrawFrame();      // Update and Draw one frame

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
    SetTraceLogLevel(LOG_DEBUG);
#else
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif
#endif
#else
    SetTraceLogLevel(LOG_NONE);
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(ScreenWidth, ScreenHeight, "raylib gamejam template");
    
    // TODO: Load resources / Initialize variables at this point

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
static void UpdateStart()
{
    using fsec = std::chrono::duration<float>;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || game_context.start_cooldown <= std::chrono::milliseconds::zero()) {
        game_context.start_cooldown = std::chrono::milliseconds::zero();
        game_context.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(fsec{GetTime()});
        game_context.state = GameState::Main;

        game_context.clearNodes();
        setActionNode(game_context.nodes[0], {60, 44}, ConnectorAction::MovementRight);
        setKeyNode(game_context.nodes[1], {204, 70}, ConnectorKey::D);
        setActionNode(game_context.nodes[2], {68, 386}, ConnectorAction::MovementUp);
        setKeyNode(game_context.nodes[3], {238, 254}, ConnectorKey::W);

        return;
    }
    game_context.start_cooldown -= game_context.delta;
}

static void linkNodes(int node_selected1, int node_selected2);
[[nodiscard]] static bool validConnection(int node_selected1, int node_selected2);
static void unlinkNode(ConnectorNode& node);
static void updateCountConnectedNode(ConnectorNode& node);
static void updateNodeConnections(ConnectorNode& node);
static void UpdateMain()
{
    // end goal
    using fsec = std::chrono::duration<float>;
    const auto end_time = std::chrono::duration_cast<std::chrono::milliseconds>(fsec{GetTime()});
    game_context.timer = end_time - game_context.start_time;

    // Connector Area
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const auto mouse = GetMousePosition();
        for (auto& node : game_context.nodes)
        {
            if (node.type != ConnectorType::DISABLED)
            {
                if(CheckCollisionCircleRec(node.position, 16, {mouse.x, mouse.y, 8, 8}))
                {
                    node.is_selected = true;
                }
            }
        }
        int node_selected1 = -1;
        int node_selected2 = -1;
        for (size_t i = 0; i < game_context.nodes.size(); ++i)
        {
            if (game_context.nodes[i].is_selected)
            {
                node_selected1 = i;
                break;
            }
        }
        for (size_t i = 0; i < game_context.nodes.size(); ++i)
        {
            if (i != node_selected1 && game_context.nodes[i].is_selected)
            {
                node_selected2 = i;
                break;
            }
        }
        if (node_selected1 != -1 && node_selected2 != -1)
        {
            linkNodes(node_selected1, node_selected2);
            game_context.nodes[node_selected1].is_selected = false;
            game_context.nodes[node_selected2].is_selected = false;
        }
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        const auto mouse = GetMousePosition();
        for (size_t i = 0; i < game_context.nodes.size(); ++i)
        {
            auto& node = game_context.nodes[i];
            if (node.type != ConnectorType::DISABLED)
            {
                if(CheckCollisionCircleRec(node.position, 16, {mouse.x, mouse.y, 8, 8}))
                {
                    unlinkNode(node);
                }
            }
        }
    }
}

void linkNodes(int node_selected1, int node_selected2)
{
    /// @TODO: validate connection
    if (validConnection(node_selected1, node_selected2))
    {
        for (auto& connected_node : game_context.nodes[node_selected1].direct_connections)
        {
            if (connected_node == -1)
            {
                connected_node = node_selected2;
                break;
            }
        }
        for (auto& connected_node : game_context.nodes[node_selected2].direct_connections)
        {
            if (connected_node == -1)
            {
                connected_node = node_selected1;
                break;;
            }
        }
        /// @TODO: optimize count with `game_context.nodes[node_selected1].connected_counter++` ???
        updateCountConnectedNode(game_context.nodes[node_selected1]);
        updateCountConnectedNode(game_context.nodes[node_selected2]);
    }
}
bool validConnection(int node_selected1, int node_selected2)
{
    if (node_selected1 != -1 && node_selected2 != -1)
    {
        if (game_context.nodes[node_selected1].type != game_context.nodes[node_selected2].type ||
            (game_context.nodes[node_selected1].type == ConnectorType::Action && game_context.nodes[node_selected2].type == ConnectorType::Action))
        {
            return true;
        }
    }
    return false;
}
void unlinkNode(ConnectorNode& node)
{
    // unlink inner connections
    for (auto& connected_node_index : node.direct_connections)
    {
        if (connected_node_index != -1)
        {
            for (auto& sibling_connected_node_index : game_context.nodes[connected_node_index].direct_connections)
            {
                if (node.index == sibling_connected_node_index)
                {
                    sibling_connected_node_index = -1;
                }
            }
            node.direct_connections.fill(-1);
        }
    }
    // check for all other nodes
    for (size_t j = 0; j < game_context.nodes.size(); ++j)
    {
        auto& other_node = game_context.nodes[j];
        if (node.index != j && other_node.type != ConnectorType::DISABLED)
        {
            // unlink outer connections
            for (auto& connected_node_index : other_node.direct_connections)
            {
                if (connected_node_index != -1)
                {
                    for (auto& sibling_connected_node_index : game_context.nodes[connected_node_index].direct_connections)
                    {
                        if (node.index == sibling_connected_node_index)
                        {
                            sibling_connected_node_index = -1;
                        }
                    }
                    if (connected_node_index == node.index)
                    {
                        connected_node_index = -1;
                    }
                }
            }
        }
    }
    updateCountConnectedNode(node);
}
void updateCountConnectedNode(ConnectorNode& node)
{
    node.connected_counter = 0;
    for (size_t i = 0; i < game_context.nodes.size(); ++i)
    {
        if (i != node.index)
        {
            for (const auto& connected_node : game_context.nodes[i].direct_connections)
            {
                if (connected_node == node.index)
                {
                    node.connected_counter++;
                }
            }
        }
    }
    updateNodeConnections(node);
}
void updateNodeConnections(ConnectorNode& node)
{
    const auto getFreeConnectedNode = [&]() -> ConnectorNode**
    {
        for (auto& connection : node.connected_nodes)
        {
            if (connection == nullptr)
            {
                return &connection;
            }
        }
        return nullptr;
    };
    const auto addConnection = [&](int root_node_index, int search_node_index, const std::array<int, MaxNodeConnections>& direct_connections)
    {
        if (search_node_index != -1)
        {
            for (const auto& direct_connected_node_index : direct_connections)
            {
                if (direct_connected_node_index == search_node_index && direct_connected_node_index != root_node_index)
                {
                    if (auto** connection = getFreeConnectedNode(); connection != nullptr)
                    {
                        *connection = &game_context.nodes[direct_connected_node_index];
                    }
                }
            }
        }
    };

    node.connected_nodes.fill(nullptr);
    for (size_t i = 0; i < game_context.nodes.size(); ++i)
    {
        if (node.type != ConnectorType::DISABLED && node.index != i)
        {
            // check direct connect with the other node
            addConnection(node.index, node.index, game_context.nodes[i].direct_connections);
            /// @TODO: use recursion, for going deeper in the graph
            for (auto connected_node_index : node.direct_connections)
            {
                if (node.index != connected_node_index && connected_node_index != -1)
                {
                    addConnection(node.index, connected_node_index, game_context.nodes[connected_node_index].direct_connections);
                    /*
                    for (auto inner_connected_node_index_1 : game_context.nodes[connected_node_index].connections)
                    {
                        if (i != inner_connected_node_index_1 && inner_connected_node_index_1 != -1)
                        {
                            addConnection(connected_node_index, game_context.nodes[inner_connected_node_index_1].connections);
                            for (auto inner_connected_node_index_2 : game_context.nodes[inner_connected_node_index_1].connections)
                            {
                                if (i != inner_connected_node_index_2 && inner_connected_node_index_2 != -1)
                                {
                                    addConnection(inner_connected_node_index_1, game_context.nodes[inner_connected_node_index_2].connections);
                                }
                            }
                        }
                    }
                    */
                }
            }
        }
    }

    // update actions
    node.connected_actions.clear();
    for(const auto& connected_node : node.connected_nodes)
    {
        if (connected_node != nullptr && connected_node->index != node.index)
        {
            if (connected_node->type == ConnectorType::Action)
            {
                node.connected_actions.push_back(connected_node->action);
            }
        }
    }

    // debug
    {
        TraceLog(LOG_DEBUG, "node %d -> ", node.index);
        for(const auto& connection : node.connected_nodes)
        {
            TraceLog(LOG_DEBUG, " %d", connection);
        }
    }
}

static void UpdateEnd()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        // restart
        game_context = {};
        game_context.state = GameState::Start;
        return;
    }
}
void UpdateGameLogic() {
    using fsec = std::chrono::duration<float>;
    game_context.delta = std::chrono::duration_cast<std::chrono::milliseconds>(fsec{GetFrameTime()});
    switch (game_context.state)
    {
        case GameState::Start:
            UpdateStart();
            break;
        case GameState::Main:
            UpdateMain();
            break;
        case GameState::End:
            UpdateEnd();
            break;
    }
}

void RenderNode(const ConnectorNode& node)
{
    // render Connection
    for (auto direct_connected_node_index : node.direct_connections)
    {
        if (direct_connected_node_index != -1)
        {
            const auto& sibling_connected = game_context.nodes[direct_connected_node_index];
            DrawLineV(node.position, sibling_connected.position, ColorPalette[5]);
        }
    }

    // render Node
    constexpr int FontSize = 12;
    const char* innerTextAction = [&]()
    {
        return TextFormat("%i", node.index);
        switch (node.action)
        {
        case ConnectorAction::NONE:
            return "";
        case ConnectorAction::MovementLeft:
            return "L";
        case ConnectorAction::MovementRight:
            return "L";
        case ConnectorAction::MovementUp:
            return "L";
        case ConnectorAction::MovementDown:
            return "L";
        case ConnectorAction::Jump:
            return "L";
        }
        return "";
    }();
    const auto innerTextActionSize = MeasureTextEx(GetFontDefault(), innerTextAction, FontSize, FontSize/10);

    const char* innerTextKey = [&]()
    {
        return TextFormat("%i", node.index);
        switch (node.key)
        {
        case ConnectorKey::NONE:
            return "";
        case ConnectorKey::W:
            return "W";
        case ConnectorKey::A:
            return "A";
        case ConnectorKey::S:
            return "S";
        case ConnectorKey::D:
            return "D";
        case ConnectorKey::Space:
            return "_";
        }
        return "";
    }();
    const auto innerTextKeySize = MeasureTextEx(GetFontDefault(), innerTextKey, FontSize, FontSize/2);

    switch (node.type)
    {
    case ConnectorType::DISABLED:
        return;
    case ConnectorType::Action:
        if (node.is_selected)
        {
            DrawPoly(node.position, 6, 16, 0, ColorPalette[1]);
        } else {
            DrawPolyLines(node.position, 6, 16, 0, ColorPalette[1]);
        }
        switch (node.action)
        {
        case ConnectorAction::NONE:
            return;
        case ConnectorAction::MovementLeft:
        case ConnectorAction::MovementRight:
        case ConnectorAction::MovementUp:
        case ConnectorAction::MovementDown:
        case ConnectorAction::Jump:
        if (node.is_selected)
        {
            DrawText(innerTextAction, node.position.x - innerTextActionSize.x/2, node.position.y - innerTextActionSize.y/2, FontSize, ColorPalette[0]);
        } else
        {
            DrawText(innerTextAction, node.position.x - innerTextActionSize.x/2, node.position.y - innerTextActionSize.y/2, FontSize, ColorPalette[1]);
        }
            break;
        }
        break;
    case ConnectorType::Key:
        if (node.is_selected)
        {
            DrawCircle(node.position.x, node.position.y, 16, ColorPalette[4]);
        } else
        {
            DrawCircleLines(node.position.x, node.position.y, 16, ColorPalette[4]);
        }
        switch (node.key)
        {
        case ConnectorKey::NONE:
            return;
        case ConnectorKey::W:
        case ConnectorKey::A:
        case ConnectorKey::S:
        case ConnectorKey::D:
        case ConnectorKey::Space:
            if (node.is_selected)
            {
                DrawText(innerTextKey, node.position.x - innerTextKeySize.x/2, node.position.y - innerTextKeySize.y/2, 14, ColorPalette[0]);
            } else
            {
                DrawText(innerTextKey, node.position.x - innerTextKeySize.x/2, node.position.y - innerTextKeySize.y/2, 14, ColorPalette[4]);
            }
            break;
        }
        break;
    }
}

// Update and draw frame
void UpdateDrawFrame()
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    //----------------------------------------------------------------------------------
    UpdateGameLogic();

    // Draw
    //----------------------------------------------------------------------------------
    // Render to screen (main framebuffer)
    BeginDrawing();
        ClearBackground(ColorPalette[0]);

        // @TODO: move text out
        constexpr const char* TitleText = "Title";
        DrawText(TitleText, 8, 8, 14, RAYWHITE);

        // borders
        DrawRectangleLinesEx(ConnectorArea, 1, RAYWHITE);
        DrawRectangleLinesEx(LevelArea, 1, RAYWHITE);

        if (game_context.state == GameState::Start)
        {
            // show welcome text
            constexpr const char* WelcomeText = "TODO: Welcome";
            constexpr int WelcomeTextFontSize = 18;
            DrawText(WelcomeText, LevelArea.x + LevelArea.width/2 - MeasureText(WelcomeText, WelcomeTextFontSize)/2, 72, WelcomeTextFontSize, RAYWHITE);
        } else {

        }

        if (game_context.state == GameState::Main)
        {
            for (const auto& node : game_context.nodes)
            {
                RenderNode(node);
            }
        }

    EndDrawing();
    //----------------------------------------------------------------------------------  
}