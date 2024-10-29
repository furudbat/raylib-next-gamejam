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
#include <vector>
#include <set>

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
#include "types.h"
#include "level1.h"

struct GameContext
{
    Texture2D tileset_texture{0};
    Texture2D character_sprite_sheet_texture{0};
    std::chrono::milliseconds delta{std::chrono::milliseconds::zero()};

    GameState state{GameState::Start};
    std::chrono::milliseconds timer{std::chrono::milliseconds::zero()};
    std::chrono::milliseconds start_time{std::chrono::milliseconds::zero()};
    GameLevelNodes nodes;
    const Level_t* map_data{nullptr};

    int level{0};
    Vector2 player_tiles_position{0, 0};
    Vector2 player_start_tiles_position{0, 0};
    CharacterDirection player_direction{CharacterDirection::Right};
    int level_max_node_connections{0};
    int level_connections{0};
    ConnectorKey player_current_key{ConnectorKey::NONE};
    int player_action_index{-1};


    // computed
    std::unordered_map<ConnectorKey, std::vector<ConnectorAction>> key_binds;
    std::string helper_lines{};

    GameContext()
    {
        helper_lines.reserve(24*4);
    }

    void clearNodes ()
    {
        assert(nodes.size() <= std::numeric_limits<int>::max());
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            nodes[i] = {};
            nodes[i].index = static_cast<int>(i);
        }
    }
};

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
inline constexpr int ScreenWidth = 800;
inline constexpr int ScreenHeight = 450;

/// @NOTE: game needs to be static for emscripten, see UpdateDrawFrame (no parameter passing)
static GameContext game_context;

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
    InitWindow(ScreenWidth, ScreenHeight, "Neuron Controls - raylib NEXT gamejam 2024");
    
    game_context.tileset_texture = LoadTexture("resources/tileset.png");
    game_context.character_sprite_sheet_texture = LoadTexture("resources/character.png");

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
    
    UnloadTexture(game_context.tileset_texture);
    UnloadTexture(game_context.character_sprite_sheet_texture);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
static void SetLevel(int level)
{
    using fsec = std::chrono::duration<float>;
    game_context.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(fsec{GetTime()});
    game_context.state = GameState::NodesMain;
    game_context.level = level;
    game_context.level_connections = 0;
    game_context.player_current_key = ConnectorKey::NONE;
    game_context.player_action_index = -1;
    switch(game_context.level)
    {
        case 1:
            game_context.nodes = GetLevelNodes(level1::NodesData);
            game_context.map_data = &level1::MapData;
            game_context.player_tiles_position = level1::CharacterStartTilesPosition;
            game_context.player_start_tiles_position = level1::CharacterStartTilesPosition;
            game_context.player_direction = level1::CharacterStartDirection;
            game_context.level_max_node_connections = level1::MaxNodeConnections;
            break;
        default: TraceLog(LOG_ERROR, "Error Not Found: %i", game_context.level);
    }
}
static void UpdateStart()
{
    const auto mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(StartButtonRect, {mouse.x, mouse.y , 8, 8})) {
        SetLevel(1);
    }
}

static bool linkNodes(int node_selected1, int node_selected2);
[[nodiscard]] static bool validConnection(int node_selected1, int node_selected2);
static void unlinkNode(ConnectorNode& node);
static void updateCountConnectedNode(ConnectorNode& node);
static void updateNodeConnections(ConnectorNode& node);
static void updateAllNodes();
static void UpdateNodesMain()
{
    // update timer
    using fsec = std::chrono::duration<float>;
    const auto end_time = std::chrono::duration_cast<std::chrono::milliseconds>(fsec{GetTime()});
    game_context.timer = end_time - game_context.start_time;

    // Connector Area
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const auto mouse = GetMousePosition();
        for (auto& node : game_context.nodes)
        {
            if (node.data.type != ConnectorType::DISABLED)
            {
                if(CheckCollisionCircleRec(node.data.position, 16, {mouse.x, mouse.y, 8, 8}))
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
        updateAllNodes();
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        const auto mouse = GetMousePosition();
        for (size_t i = 0; i < game_context.nodes.size(); ++i)
        {
            auto& node = game_context.nodes[i];
            if (node.data.type != ConnectorType::DISABLED)
            {
                if(CheckCollisionCircleRec(node.data.position, 16, {mouse.x, mouse.y, 8, 8}))
                {
                    unlinkNode(node);
                }
            }
        }
        updateAllNodes();
    }

    const auto mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(StartMainCharacterButtonRect, {mouse.x, mouse.y , 8, 8})) {
        game_context.state = GameState::CharacterMain;
        return;
    }
}


static void updateLevelConnectionCount()
{
    game_context.level_connections = 0;
    for (const auto& node : game_context.nodes)
    {
        if (node.data.type != ConnectorType::DISABLED)
        {
            for (const auto& connected_node_index : node.direct_connections)
            {
                if (connected_node_index != -1)
                {
                    game_context.level_connections++;
                }
            }
        }
    }
    /// @NOTE(workaround): for bidirectional connections
    game_context.level_connections /= 2;
}
bool linkNodes(int node_selected1, int node_selected2)
{
    updateLevelConnectionCount();
    // check max connections (per level)
    if (game_context.level_connections >= game_context.level_max_node_connections)
    {
        return false;
    }

    /// @TODO: validate connection
    if (validConnection(node_selected1, node_selected2))
    {
        for (auto& connected_node_index : game_context.nodes[node_selected1].direct_connections)
        {
            if (connected_node_index == -1)
            {
                connected_node_index = node_selected2;
                break;
            }
        }
        for (auto& connected_node_index : game_context.nodes[node_selected2].direct_connections)
        {
            if (connected_node_index == -1)
            {
                connected_node_index = node_selected1;
                break;
            }
        }

        /// @TODO: optimize count with `game_context.nodes[node_selected1].connected_counter++` ???
        updateCountConnectedNode(game_context.nodes[node_selected1]);
        updateCountConnectedNode(game_context.nodes[node_selected2]);
        updateLevelConnectionCount();
        return true;
    }

    return false;
}
bool validConnection(int node_selected1, int node_selected2)
{
    if (node_selected1 != -1 && node_selected2 != -1)
    {
        if (game_context.nodes[node_selected1].data.type != game_context.nodes[node_selected2].data.type ||
            (game_context.nodes[node_selected1].data.type == ConnectorType::Action && game_context.nodes[node_selected2].data.type == ConnectorType::Action))
        {
            if (game_context.nodes[node_selected1].connected_counter >= MaxNodeConnections)
            {
                return false;
            }
            if (game_context.nodes[node_selected2].connected_counter >= MaxNodeConnections)
            {
                return false;
            }

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
        if (node.index != j && other_node.data.type != ConnectorType::DISABLED)
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
    updateLevelConnectionCount();
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
void UpdateKeyBinds()
{
    game_context.key_binds.clear();
    for (const auto& node : game_context.nodes)
    {
        if (node.data.type == ConnectorType::Key && node.index != -1 && !node.connected_actions.empty())
        {
            switch (node.data.key)
            {
            case ConnectorKey::NONE:
                break;
            case ConnectorKey::W:
            case ConnectorKey::S:
            case ConnectorKey::D:
            case ConnectorKey::Space:
                game_context.key_binds[node.data.key] = {};
                break;
            }

            for (size_t i = 0;i < node.connected_actions.size();++i)
            {
                const auto& connected_action = node.connected_actions[i];
                switch (connected_action)
                {
                case ConnectorAction::NONE:
                    break;
                case ConnectorAction::MovementLeft:
                case ConnectorAction::MovementRight:
                case ConnectorAction::MovementUp:
                case ConnectorAction::MovementDown:
                case ConnectorAction::Jump:
                    game_context.key_binds[node.data.key].push_back(connected_action);
                    break;
                }
            }
        }
    }
}
void updateNodeConnections(ConnectorNode& node)
{
    const auto addConnection = [&](int root_node_index, const ConnectorNode& connect_node)
    {
        if (connect_node.data.type != ConnectorType::DISABLED)
        {
            for (const auto& direct_connected_node_index : connect_node.direct_connections)
            {
                if (direct_connected_node_index != -1 && direct_connected_node_index != root_node_index)
                {
                    if(node.connected_nodes.size() < MaxIndirectConnections)
                    {
                        node.connected_nodes.emplace(direct_connected_node_index);
                    }
                }
            }
        }
    };

    node.connected_nodes.clear();
    // add direct nodes
    for (const auto& direct_connected_node_index : node.direct_connections)
    {
        if (direct_connected_node_index != -1)
        {
            node.connected_nodes.emplace(direct_connected_node_index);
        }
    }
    // add indirect node (connection)
    if (node.data.type != ConnectorType::DISABLED)
    {
        // check direct connect with the other node
        addConnection(node.index, node);
        /// @TODO: use recursion, for going deeper in the graph
        for (const auto& connected_node_index : node.direct_connections)
        {
            if (node.index != connected_node_index && connected_node_index != -1)
            {
                addConnection(node.index, game_context.nodes[connected_node_index]);
                for (const auto& inner_connected_node_index_1 : game_context.nodes[connected_node_index].direct_connections)
                {
                    if (node.index != inner_connected_node_index_1 && inner_connected_node_index_1 != -1)
                    {
                        addConnection(node.index, game_context.nodes[inner_connected_node_index_1]);
                        for (const auto& inner_connected_node_index_2 : game_context.nodes[inner_connected_node_index_1].direct_connections)
                        {
                            if (node.index != inner_connected_node_index_2 && inner_connected_node_index_2 != -1)
                            {
                                addConnection(node.index, game_context.nodes[inner_connected_node_index_2]);
                                for (const auto& inner_connected_node_index_3 : game_context.nodes[inner_connected_node_index_2].direct_connections)
                                {
                                    if (node.index != inner_connected_node_index_3 && inner_connected_node_index_3 != -1)
                                    {
                                        addConnection(node.index, game_context.nodes[inner_connected_node_index_3]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // update actions
    node.connected_actions.clear();
    /// @NOTE(workaround): reverse set to the order of the actions is right
    std::vector connected_nodes_arr(node.connected_nodes.begin(), node.connected_nodes.end());
    for (auto it = connected_nodes_arr.rbegin(); it != connected_nodes_arr.rend(); ++it)
    {
        const auto& connected_node_index = *it;
        if (connected_node_index != -1 && connected_node_index != node.index)
        {
            const auto& connected_node = game_context.nodes[connected_node_index];
            if (connected_node.data.type == ConnectorType::Action)
            {
                node.connected_actions.push_back(connected_node.data.action);
            }
        }
    }

    UpdateKeyBinds();

    // debug
    if (1) {
        TraceLog(LOG_DEBUG, "node %d -> ", node.index);
        for(const auto& connected_index : node.connected_nodes)
        {
            TraceLog(LOG_DEBUG, " %d", connected_index);
        }
    }
}
void updateAllNodes()
{
    for(auto& node : game_context.nodes)
    {
        updateCountConnectedNode(node);
    }
}

static void NextLevel()
{
    if (game_context.level > 0 && game_context.level < MaxLevels)
    {
        SetLevel(game_context.level+1);
    } else if (game_context.level == MaxLevels)
    {
        game_context.state = GameState::End;
        return;
    }
}
static void PlayerDie()
{
    game_context.state = GameState::NodesMain;
    game_context.player_current_key = ConnectorKey::NONE;
    game_context.player_action_index = -1;
    game_context.player_tiles_position = game_context.player_start_tiles_position;
}
static void UpdateCharacterMain()
{
    const auto mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(ResetMainCharacterButtonRect, {mouse.x, mouse.y , 8, 8})) {
        game_context.state = GameState::NodesMain;
        switch (game_context.level)
        {
        case 1:
            game_context.player_direction = level1::CharacterStartDirection;
            game_context.player_tiles_position = level1::CharacterStartTilesPosition;
            game_context.player_current_key = ConnectorKey::NONE;
            game_context.player_action_index = -1;
            break;
        }
        return;
    }

    // Update key binds (pressed)
    for (const auto& [key, actions]: game_context.key_binds)
    {
        if (IsKeyPressed(static_cast<int>(key)))
        {
            game_context.player_current_key = key;
            game_context.player_action_index = 0;
            break;
        }
    }
    if (game_context.player_current_key != ConnectorKey::NONE && game_context.player_action_index != -1)
    {
        switch(game_context.key_binds[game_context.player_current_key][game_context.player_action_index])
        {
        case ConnectorAction::NONE:
            break;
        case ConnectorAction::MovementLeft:
            game_context.player_tiles_position.x -= 1;
            game_context.player_direction = CharacterDirection::Left;
            break;
        case ConnectorAction::MovementRight:
            game_context.player_tiles_position.x += 1;
            game_context.player_direction = CharacterDirection::Right;
            break;
        case ConnectorAction::MovementUp:
            game_context.player_tiles_position.y -= 1;
            game_context.player_direction = CharacterDirection::Up;
            break;
        case ConnectorAction::MovementDown:
            game_context.player_tiles_position.y += 1;
            game_context.player_direction = CharacterDirection::Down;
            break;
        case ConnectorAction::Jump:
            switch (game_context.player_direction)
            {
            case CharacterDirection::Right:
                game_context.player_tiles_position.x += 2;
                break;
            case CharacterDirection::Left:
                game_context.player_tiles_position.x -= 2;
                break;
            case CharacterDirection::Up:
                game_context.player_tiles_position.y -= 2;
                break;
            case CharacterDirection::Down:
                game_context.player_tiles_position.y += 2;
                break;
            }
            break;
        }
        if (game_context.player_action_index < game_context.key_binds[game_context.player_current_key].size())
        {
            game_context.player_action_index++;
        } else
        {
            game_context.player_current_key = ConnectorKey::NONE;
            game_context.player_action_index = -1;
        }
    }

    // check map conditions
    {
        if (game_context.map_data != nullptr)
        {
            const auto* player_map_tile_index = [&]() -> const int*
            {
                if (game_context.map_data != nullptr && (
                    game_context.player_tiles_position.x >= 0 && game_context.player_tiles_position.y >= 0 &&
                    game_context.player_tiles_position.y < game_context.map_data->size() && game_context.player_tiles_position.x < (*game_context.map_data)[game_context.player_tiles_position.y].size()))
                {
                    return &(*game_context.map_data)[game_context.player_tiles_position.y][game_context.player_tiles_position.x];
                }
                return nullptr;
            }();
            const auto player_map_tile = (player_map_tile_index != nullptr) ? static_cast<TileSet>(*player_map_tile_index) : TileSet::Void;
            switch (player_map_tile)
            {
            case TileSet::Floor:
                break;
            case TileSet::Door:
                NextLevel();
                break;
            case TileSet::Key:
                /// @TODO: collect key
                break;
            case TileSet::Void:
                PlayerDie();
                break;
            }

            //const auto& px = LevelMapArea.x + game_context.player_tiles_position.x*CharacterSpriteWidth;
            //const auto& py = LevelMapArea.y + game_context.player_tiles_position.y*CharacterSpriteHeight;
            //const Rectangle player_position {px, py, CharacterSpriteWidth, CharacterSpriteHeight};
        }
    }
}
static void UpdateEnd()
{
    // restart
    if (IsKeyPressed(KEY_ENTER))
    {
        game_context = {};
        game_context.state = GameState::Start;
        SetLevel(1);
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
        case GameState::NodesMain:
            UpdateNodesMain();
            break;
        case GameState::CharacterMain:
            UpdateCharacterMain();
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
            DrawLineEx(node.data.position, sibling_connected.data.position, NodeLineThick, NodeLineColor);
        }
    }

    // render Node
    switch (node.data.type)
    {
    case ConnectorType::DISABLED:
        return;
    case ConnectorType::Action:
        {
            const char* innerTextAction = [&]()
            {
                // debug
                //return TextFormat("%i", node.index);

                switch (node.data.action)
                {
                case ConnectorAction::NONE:
                    return "";
                case ConnectorAction::MovementLeft:
                    return "<-";
                case ConnectorAction::MovementRight:
                    return "->";
                case ConnectorAction::MovementUp:
                    return "^";
                case ConnectorAction::MovementDown:
                    return "v";
                case ConnectorAction::Jump:
                    return "JP";
                }
                return "";
            }();
            const auto innerTextActionSize = MeasureTextEx(GetFontDefault(), innerTextAction, NodeFontSize, NodeFontSize/10);

            DrawPoly(node.data.position, ActionNodeSides, ActionNodeRadius, ActionNodeRotation, BackgroundColor);
            if (node.is_selected)
            {
                DrawPoly(node.data.position, ActionNodeSides, ActionNodeRadius, ActionNodeRotation, ActionNodeColor);
            } else {
                DrawPolyLines(node.data.position, ActionNodeSides, ActionNodeRadius, ActionNodeRotation, ActionNodeColor);
            }
            switch (node.data.action)
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
                DrawText(innerTextAction, node.data.position.x - innerTextActionSize.x/2, node.data.position.y - innerTextActionSize.y/2, NodeFontSize, BackgroundColor);
            } else
            {
                DrawText(innerTextAction, node.data.position.x - innerTextActionSize.x/2, node.data.position.y - innerTextActionSize.y/2, NodeFontSize, ActionNodeColor);
            }
                break;
            }
            break;
        }
    case ConnectorType::Key:
        {
            const char* innerTextKey = [&]()
            {
                // debug
                //return TextFormat("%i", node.index);

                switch (node.data.key)
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
            const auto innerTextKeySize = MeasureTextEx(GetFontDefault(), innerTextKey, NodeFontSize, NodeFontSize/10);

            DrawCircle(node.data.position.x, node.data.position.y, KeyNodeRadius, BackgroundColor);
            if (node.is_selected)
            {
                DrawCircle(node.data.position.x, node.data.position.y, KeyNodeRadius, KeyNodeColor);
            } else
            {
                DrawCircleLines(node.data.position.x, node.data.position.y, KeyNodeRadius, KeyNodeColor);
            }
            switch (node.data.key)
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
                    DrawText(innerTextKey, node.data.position.x - innerTextKeySize.x/2, node.data.position.y - innerTextKeySize.y/2, NodeFontSize, BackgroundColor);
                } else
                {
                    DrawText(innerTextKey, node.data.position.x - innerTextKeySize.x/2, node.data.position.y - innerTextKeySize.y/2, NodeFontSize, KeyNodeColor);
                }
                break;
            }
            break;
        }
    }
}
void RenderMap()
{
    // border
    DrawRectangleLinesEx(LevelMapArea, 1, BorderColor);

    // Render Map
    if (game_context.map_data != nullptr)
    {
        for(int y = 0;y < LevelTilesHeight; y++)
        {
            for(int x = 0;x < LevelTilesWidth; x++)
            {
                const auto& tile = (*game_context.map_data)[y][x];

                float sx = tile*LevelTilesetWidth;
                float sy = 0;

                float dx = LevelMapArea.x + x*LevelTilesetWidth;
                float dy = LevelMapArea.y + y*LevelTilesetHeight;

                DrawTexturePro(game_context.tileset_texture,
                    { sx, sy, LevelTilesetWidth, LevelTilesetHeight },
                    { dx, dy, LevelTilesetWidth, LevelTilesetHeight},
                    {0, 0}, 0, WHITE);
            }
        }

        // Render Character
        DrawTexturePro(game_context.character_sprite_sheet_texture,
            { static_cast<float>(static_cast<int>(game_context.player_direction)*CharacterSpriteWidth), 0, CharacterSpriteWidth, CharacterSpriteHeight },
            { LevelMapArea.x + game_context.player_tiles_position.x * LevelTilesetWidth, LevelMapArea.y + game_context.player_tiles_position.y * LevelTilesetHeight, CharacterSpriteWidth, CharacterSpriteHeight},
            {0, 0}, 0, WHITE);
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

        // @TODO: move text out ???
        //constexpr const char* TitleText = "Neuron Control - Connect Action-Nodes with Key-Binding-Nodes";
        //DrawText(TitleText, 8, 8, TitleFontSize, TextFontColor);

        // borders
        DrawRectangleLinesEx(ConnectorArea, 1, BorderColor);
        DrawRectangleLinesEx(LevelArea, 1, BorderColor);

        if (game_context.state == GameState::Start)
        {
            // show welcome text
            constexpr const char* TitleText = "Neuron Controls";
            const auto titleTextSize = MeasureTextEx(GetFontDefault(), TitleText, TitleTextFontSize, TitleTextFontSize/10);
            DrawText(TitleText, ConnectorArea.x + ConnectorArea.width/2 - titleTextSize.x/2, ConnectorArea.y + 72, TitleTextFontSize, TextFontColor);

            constexpr const char* WelcomeText = "Connect Actions and Key-Binds on the left side.";
            const auto welcomeTextSize = MeasureTextEx(GetFontDefault(), WelcomeText, WelcomeTextFontSize, WelcomeTextFontSize/10);
            DrawText(WelcomeText, LevelArea.x + LevelArea.width/2 - welcomeTextSize.x/2, LevelArea.y + 72, WelcomeTextFontSize, TextFontColor);

            constexpr const char* StartButtonText = "START";
            const auto startButtonTextSize = MeasureTextEx(GetFontDefault(), StartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
            DrawRectangleLinesEx(StartButtonRect, 1, ButtonColor);
            DrawText(StartButtonText, StartButtonRect.x + StartButtonRect.width/2 - startButtonTextSize.x/2, StartButtonRect.y + StartButtonRect.height/2 - startButtonTextSize.y/2, StartButtonTextFontSize, TextFontColor);
        }
        else if (game_context.state == GameState::NodesMain || game_context.state == GameState::CharacterMain)
        {
            DrawRectangleLinesEx(LeftTextArea, 1, BorderColor);
            DrawRectangleLinesEx(RightTextArea, 1, BorderColor);

            for (const auto& node : game_context.nodes)
            {
                RenderNode(node);
            }

            DrawText(TextFormat("Connections: %02d/%02d", game_context.level_connections, game_context.level_max_node_connections), LeftTextArea.x + 8, LeftTextArea.y + 8, HelperTextFontSize, TextFontColor);

            // Main Start Button
            if (game_context.state == GameState::NodesMain) {
                constexpr const char* StartButtonText = "GO";
                const auto startButtonTextSize = MeasureTextEx(GetFontDefault(), StartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
                DrawRectangleLinesEx(StartMainCharacterButtonRect, 1, ButtonColor);
                DrawText(StartButtonText, StartMainCharacterButtonRect.x + StartMainCharacterButtonRect.width/2 - startButtonTextSize.x/2, StartMainCharacterButtonRect.y + StartMainCharacterButtonRect.height/2 - startButtonTextSize.y/2, StartButtonTextFontSize, TextFontColor);
            }
            // Reset Button
            if (game_context.state == GameState::CharacterMain) {
                constexpr const char* RestartButtonText = "RESET";
                const auto restartButtonTextSize = MeasureTextEx(GetFontDefault(), RestartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
                DrawRectangleLinesEx(ResetMainCharacterButtonRect, 1, ButtonColor);
                DrawText(RestartButtonText, ResetMainCharacterButtonRect.x + ResetMainCharacterButtonRect.width/2 - restartButtonTextSize.x/2, ResetMainCharacterButtonRect.y + StartMainCharacterButtonRect.height/2 - restartButtonTextSize.y/2, StartButtonTextFontSize, TextFontColor);
            }

            /// @TODO: extract into own function
            // helper text
            {
                game_context.helper_lines.clear();
                for (const auto& [key, actions] : game_context.key_binds)
                {
                    switch (key)
                    {
                    case ConnectorKey::NONE:
                        break;
                    case ConnectorKey::W:
                        game_context.helper_lines += TextFormat("    W: ");
                        break;
                    case ConnectorKey::A:
                        game_context.helper_lines += TextFormat("    A: ");
                        break;
                    case ConnectorKey::S:
                        game_context.helper_lines += TextFormat("    S: ");
                        break;
                    case ConnectorKey::D:
                        game_context.helper_lines += TextFormat("    D: ");
                        break;
                    case ConnectorKey::Space:
                        game_context.helper_lines += TextFormat("SPACE: ");
                        break;
                    }
                    for (size_t i = 0;i < actions.size();++i)
                    {
                        const auto& connected_action = actions[i];
                        switch (connected_action)
                        {
                        case ConnectorAction::NONE:
                            break;
                        case ConnectorAction::MovementLeft:
                            game_context.helper_lines += TextFormat("Left");
                            break;
                        case ConnectorAction::MovementRight:
                            game_context.helper_lines += TextFormat("Right");
                            break;
                        case ConnectorAction::MovementUp:
                            game_context.helper_lines += TextFormat("Up");
                            break;
                        case ConnectorAction::MovementDown:
                            game_context.helper_lines += TextFormat("Down");
                            break;
                        case ConnectorAction::Jump:
                            game_context.helper_lines += TextFormat("Jump");
                            break;
                        }
                        if (i < actions.size()-1)
                        {
                            game_context.helper_lines += " -> ";
                        }
                    }
                    game_context.helper_lines += "\n";
                }
                DrawText(game_context.helper_lines.c_str(), HelperTextAreaRect.x, HelperTextAreaRect.y, HelperTextFontSize, TextFontColor);
            }

            RenderMap();
        }

    EndDrawing();
    //----------------------------------------------------------------------------------  
}