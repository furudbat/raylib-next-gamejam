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
#include <raymath.h>

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <array>
#include <chrono>
#include <vector>

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
#include "constants.h"
#include "types.h"
// Levels
#include "level1.h"
#include "level2.h"
#include "level3.h"
#include "level4.h"
#include "level5.h"

struct GameContext
{
    // textures
    Texture2D logo_texture{0};
    Texture2D instruction1_texture{0};
    Texture2D instruction2_texture{0};
    Texture2D tileset_texture{0};
    Texture2D character_sprite_sheet_texture{0};
    Texture2D icons_sprite_sheet_texture{0};
    std::chrono::milliseconds delta{std::chrono::milliseconds::zero()};

    // scene data
    Rectangle mouse{0,0,0,0};
    GameState state{GameState::Start};

    // level/player data
    std::chrono::milliseconds timer{std::chrono::milliseconds::zero()};
    std::chrono::milliseconds start_time{std::chrono::milliseconds::zero()};
    //// level data
    GameLevelNodes nodes{};
    const Level_t* map_data{nullptr};
    int level{0};
    int level_max_node_connections{0};
    int level_max_actions_per_key{0};
    int level_connections{0};
    //// player data
    std::chrono::milliseconds turn_cooldown{std::chrono::milliseconds::zero()};
    Vector2 player_start_tiles_position{0, 0};
    Vector2 player_tiles_position{0, 0};
    CharacterDirection player_start_direction{CharacterDirection::Right};
    CharacterDirection player_direction{CharacterDirection::Right};
    ConnectorKey player_current_key{ConnectorKey::NONE};
    bool player_on_void_tile{false};
    bool player_on_door_tile{false};
    bool show_help_1{false};
    bool show_help_2{false};
    int player_action_index{-1};


    // computed
    std::unordered_map<ConnectorKey, std::vector<ConnectorAction>> key_binds;
    std::string left_helper_text{};
    std::string level_helper_text{};
    std::string right_helper_text{};
    bool node_selection_mode{false};

    GameContext()
    {
        key_binds.reserve(5*MaxNodeConnections);
        left_helper_text.reserve(24*4);
        level_helper_text.reserve(24);
        right_helper_text.reserve(24*4);
    }
};

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
inline constexpr int ScreenWidth = 800;
inline constexpr int ScreenHeight = 450;

/// @NOTE: game needs to be global for emscripten, see UpdateDrawFrame (no parameter passing)
static GameContext game_context;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateGameLogic();
static void UpdateDrawFrame();      // Update and Draw one frame

/// utils
constexpr const char* WHITESPACE = " \n\r\t\f\v";
std::string ltrim(std::string_view s)
{
    const auto start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : std::string{s.substr(start)};
}
std::string rtrim(std::string_view s)
{
    const auto end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : std::string{s.substr(0, end + 1)};
}
std::string trim(std::string_view s) {
    return rtrim(ltrim(s));
}

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
    InitWindow(ScreenWidth, ScreenHeight, "NeuroCircuit - raylib NEXT gamejam 2024");

    game_context.logo_texture = LoadTexture("resources/logo.png");
    game_context.instruction1_texture = LoadTexture("resources/instruction1.png");
    game_context.instruction2_texture = LoadTexture("resources/instruction2.png");
    game_context.tileset_texture = LoadTexture("resources/tileset.png");
    game_context.character_sprite_sheet_texture = LoadTexture("resources/character.png");
    game_context.icons_sprite_sheet_texture = LoadTexture("resources/icons.png");

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

    UnloadTexture(game_context.logo_texture);
    UnloadTexture(game_context.instruction1_texture);
    UnloadTexture(game_context.instruction2_texture);
    UnloadTexture(game_context.tileset_texture);
    UnloadTexture(game_context.character_sprite_sheet_texture);
    UnloadTexture(game_context.icons_sprite_sheet_texture);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
static void UpdateAllNodes();
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
            game_context.nodes = CreateLevelNodes(level1::NodesData);
            game_context.map_data = &level1::MapData;
            game_context.player_tiles_position = level1::CharacterStartTilesPosition;
            game_context.player_start_tiles_position = level1::CharacterStartTilesPosition;
            game_context.player_start_direction = level1::CharacterStartDirection;
            game_context.player_direction = level1::CharacterStartDirection;
            game_context.level_max_node_connections = level1::MaxNodeConnections;
            game_context.level_max_actions_per_key = level1::MaxActionsPerKey;
            break;
        case 2:
            game_context.nodes = CreateLevelNodes(level2::NodesData);
            game_context.map_data = &level2::MapData;
            game_context.player_tiles_position = level2::CharacterStartTilesPosition;
            game_context.player_start_tiles_position = level2::CharacterStartTilesPosition;
            game_context.player_start_direction = level2::CharacterStartDirection;
            game_context.player_direction = level2::CharacterStartDirection;
            game_context.level_max_node_connections = level2::MaxNodeConnections;
            game_context.level_max_actions_per_key = level2::MaxActionsPerKey;
            break;
        case 3:
            game_context.nodes = CreateLevelNodes(level3::NodesData);
            game_context.map_data = &level3::MapData;
            game_context.player_tiles_position = level3::CharacterStartTilesPosition;
            game_context.player_start_tiles_position = level3::CharacterStartTilesPosition;
            game_context.player_start_direction = level3::CharacterStartDirection;
            game_context.player_direction = level3::CharacterStartDirection;
            game_context.level_max_node_connections = level3::MaxNodeConnections;
            game_context.level_max_actions_per_key = level3::MaxActionsPerKey;
            break;
        case 4:
            game_context.nodes = CreateLevelNodes(level4::NodesData);
            game_context.map_data = &level4::MapData;
            game_context.player_tiles_position = level4::CharacterStartTilesPosition;
            game_context.player_start_tiles_position = level4::CharacterStartTilesPosition;
            game_context.player_start_direction = level4::CharacterStartDirection;
            game_context.player_direction = level4::CharacterStartDirection;
            game_context.level_max_node_connections = level4::MaxNodeConnections;
            game_context.level_max_actions_per_key = level4::MaxActionsPerKey;
        break;
        case 5:
            game_context.nodes = CreateLevelNodes(level5::NodesData);
            game_context.map_data = &level5::MapData;
            game_context.player_tiles_position = level5::CharacterStartTilesPosition;
            game_context.player_start_tiles_position = level5::CharacterStartTilesPosition;
            game_context.player_start_direction = level5::CharacterStartDirection;
            game_context.player_direction = level5::CharacterStartDirection;
            game_context.level_max_node_connections = level5::MaxNodeConnections;
            game_context.level_max_actions_per_key = level5::MaxActionsPerKey;
            break;
        /// @TODO: add (new) levels, don't forget to update MaxLevels
        default: TraceLog(LOG_ERROR, "Error Not Found: %i", game_context.level);
    }
    UpdateAllNodes();

    game_context.level_helper_text = TextFormat(LevelsHelperFormat, game_context.level);
}


static bool linkNodes(int node_selected1, int node_selected2);
[[nodiscard]] static bool validConnection(int node_selected1, int node_selected2);
[[nodiscard]] static bool validPostConnections(int node_selected1, int node_selected2);
[[nodiscard]] static bool validPreConnections(ConnectorNode* clicked_node, ConnectorNode* other_node);
static void updateKeyBinds();
static void unlinkNode(ConnectorNode& node);
static void updateCountConnectedNode(ConnectorNode& node);
static void updateNodeConnections(ConnectorNode& node);
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
        ConnectorNode* node_clicked = nullptr;
        ConnectorNode* other_node = nullptr;
        for (auto& node : game_context.nodes)
        {
            if (node.data.type != ConnectorType::DISABLED)
            {
                const auto radius = [&]()
                {
                    switch (node.data.type)
                    {
                    case ConnectorType::DISABLED:
                        break;
                    case ConnectorType::Action:
                        return ActionNodeRadius;
                    case ConnectorType::Key:
                        return KeyNodeRadius;
                    }
                    return 0;
                }();
                if(CheckCollisionCircleRec(node.data.position, radius, game_context.mouse))
                {
                    node.is_selected = true;
                    node_clicked = &node;
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
                if (node_clicked != &game_context.nodes[i])
                {
                    other_node = &game_context.nodes[i];
                }
                break;
            }
        }
        for (size_t i = 0; i < game_context.nodes.size(); ++i)
        {
            if (i != node_selected1 && game_context.nodes[i].is_selected)
            {
                node_selected2 = i;
                if (node_clicked != &game_context.nodes[i])
                {
                    other_node = &game_context.nodes[i];
                }
                break;
            }
        }
        game_context.node_selection_mode = node_selected1 != -1 || node_selected2 != -1;
        if (node_selected1 != -1 && node_selected2 != -1)
        {

            UpdateAllNodes();
            if (validPreConnections(other_node, node_clicked))
            {
                const auto rollback_nodes = game_context.nodes;
                linkNodes(node_selected1, node_selected2);
                UpdateAllNodes();
                // check after constrains
                if (!validPostConnections(node_selected1, node_selected2))
                {
                    // rollback to old state
                    game_context.nodes = rollback_nodes;
                }
            }
            game_context.nodes[node_selected1].is_selected = false;
            game_context.nodes[node_selected2].is_selected = false;
            game_context.node_selection_mode = false;
        }
        UpdateAllNodes();
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        for (size_t i = 0; i < game_context.nodes.size(); ++i)
        {
            auto& node = game_context.nodes[i];
            if (node.data.type != ConnectorType::DISABLED)
            {
                if(CheckCollisionCircleRec(node.data.position, 16, game_context.mouse))
                {
                    unlinkNode(node);
                }
            }
        }

        // deselect all nodes
        for (auto& node : game_context.nodes)
        {
            node.is_selected = false;
        }
        game_context.node_selection_mode = false;

        UpdateAllNodes();
    }

    if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(StartMainCharacterButtonRect, game_context.mouse)) ||
        IsKeyPressed(KEY_ENTER)) {
        game_context.state = GameState::CharacterMain;
        game_context.left_helper_text = TextFormat(LeftHelperCharacterTextFormat);
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

        UpdateAllNodes();
        return true;
    }

    return false;
}
bool validConnection(int node_selected1, int node_selected2)
{
    if (node_selected1 != -1 && node_selected2 != -1)
    {
        const auto& node1 = game_context.nodes[node_selected1];
        const auto& node2 = game_context.nodes[node_selected2];

        TraceLog(LOG_DEBUG, "Node1: type: %d, connected_counter: %d", node1.data.type, node1.connected_counter);
        TraceLog(LOG_DEBUG, "Node2: type: %d, connected_counter: %d", node2.data.type, node2.connected_counter);
        TraceLog(LOG_DEBUG, "Level: level_connections: %d/%d ", game_context.level_connections, game_context.level_max_node_connections);

        if (node1.data.type != node2.data.type ||
            (node1.data.type == ConnectorType::Action && node2.data.type == ConnectorType::Action))
        {
            if (node1.connected_counter >= MaxNodeConnections)
            {
                return false;
            }
            if (node2.connected_counter >= MaxNodeConnections)
            {
                return false;
            }

            return true;
        }
    }

    return false;
}
bool validPreConnections(ConnectorNode* clicked_node, ConnectorNode* other_node)
{
    if (clicked_node != nullptr && other_node != nullptr)
    {
        Vector2 startPos1 = other_node->data.position;
        Vector2 endPos1 = clicked_node->data.position;
        for (const auto& node : game_context.nodes)
        {
            if (node.index != other_node->index && node.index != clicked_node->index)
            {
                for (const auto& connected_node_index : node.direct_connections)
                {
                    if (connected_node_index != -1 && connected_node_index != other_node->index && connected_node_index != clicked_node->index)
                    {
                        const auto& connected_node = game_context.nodes[connected_node_index];
                        Vector2 startPos2 = node.data.position;
                        Vector2 endPos2 = connected_node.data.position;

                        if (CheckCollisionLines(startPos1, endPos1, startPos2, endPos2, nullptr))
                        {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }
    return false;
}
bool validPostConnections(int node_selected1, int node_selected2)
{
    // check max connections (per level)
    if (game_context.level_connections > game_context.level_max_node_connections)
    {
        return false;
    }

    // check max actions
    for(const auto& [key, actions] : game_context.key_binds)
    {
        if (actions.size() > game_context.level_max_actions_per_key)
        {
            return false;
        }
    }

    constexpr int MaxKeysPerAction = 1;
    constexpr int MaxDirectActionPerKey = 1;
    for (const auto& node : game_context.nodes)
    {
         // check for already connected key (in action), one key in action (line)
        if (node.data.type == ConnectorType::Action)
        {
            int count_keys = 0;
            for (const auto& connected_node_index : node.connected_nodes)
            {
                const auto& connected_node = game_context.nodes[connected_node_index];
                if (connected_node.data.type == ConnectorType::Key)
                {
                    ++count_keys;
                }
            }
            if (count_keys > MaxKeysPerAction)
            {
                return false;
            }
        }
        // check direct connections
        if (node.data.type == ConnectorType::Key)
        {
            int count_actions = 0;
            for (const auto& direct_connected_node_index : node.direct_connections)
            {
                if (direct_connected_node_index != -1)
                {
                    const auto& direct_connected_node = game_context.nodes[direct_connected_node_index];
                    if (direct_connected_node.data.type == ConnectorType::Action)
                    {
                        ++count_actions;
                    }
                }
            }
            if (count_actions > MaxDirectActionPerKey)
            {
                return false;
            }
        }
    }

    // check connection crossing with nodes
    for (const auto& node1 : game_context.nodes)
    {
        for (const auto& node1_connected_node_index : node1.connected_nodes)
        {
            // skip self and is not selected nodes
            if (node1.index == node1_connected_node_index || !node1.is_selected)
            {
                continue;
            }

            const auto& node1_connected_node = game_context.nodes[node1_connected_node_index];
            // check only selected nodes
            if (!node1_connected_node.is_selected)
            {
                continue;
            }

            for (const auto& node2 : game_context.nodes)
            {
                if ((node2.index != node1.index && node2.index != node1_connected_node_index) && node2.is_selected)
                {
                    Vector2 startPos1 = node1.data.position;
                    Vector2 endPos1 = node1_connected_node.data.position;

                    const auto node2_size = [&]()
                    {
                        switch (node2.data.type)
                        {
                        case ConnectorType::DISABLED:
                            break;
                        case ConnectorType::Action:
                            return ActionNodeRadius;
                        case ConnectorType::Key:
                            return KeyNodeRadius;
                        }
                        return 0;
                    }();
                    Vector2 topLeft = {node2.data.position.x - node2_size/2, node2.data.position.y - node2_size/2};
                    Vector2 topRight = {node2.data.position.x + node2_size/2, node2.data.position.y - node2_size/2};
                    Vector2 bottomLeft = {node2.data.position.x - node2_size/2, node2.data.position.y + node2_size/2};
                    Vector2 bottomRight = {node2.data.position.x + node2_size/2, node2.data.position.y + node2_size/2};

                    // check lines with rectangle lines (for both action and key node)
                    if (CheckCollisionLines(startPos1, endPos1, topLeft, topRight, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, topRight, bottomRight, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, bottomRight, bottomLeft, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, bottomLeft, topLeft, nullptr))
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
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

    // deselect all nodes
    for (auto& node : game_context.nodes)
    {
        node.is_selected = false;
    }
    game_context.node_selection_mode = false;

    UpdateAllNodes();
}
void updateCountConnectedNode(ConnectorNode& node)
{
    node.connected_counter = 0;
    for (size_t i = 0; i < game_context.nodes.size(); ++i)
    {
        if (i != node.index)
        {
            for (const auto& connected_node_index : game_context.nodes[i].direct_connections)
            {
                if (connected_node_index != -1 && connected_node_index == node.index)
                {
                    node.connected_counter++;
                }
            }
        }
    }
    updateNodeConnections(node);
}
void updateKeyBinds()
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
            case ConnectorKey::A:
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

    // update helper text
    game_context.right_helper_text.clear();
    for (const auto& [key, actions] : game_context.key_binds)
    {
        switch (key)
        {
        case ConnectorKey::NONE:
            break;
        case ConnectorKey::W:
            game_context.right_helper_text += TextFormat("%5s: ", ConnectorKeyWString);
            break;
        case ConnectorKey::A:
            game_context.right_helper_text += TextFormat("%5s: ", ConnectorKeyAString);
            break;
        case ConnectorKey::S:
            game_context.right_helper_text += TextFormat("%5s: ", ConnectorKeySString);
            break;
        case ConnectorKey::D:
            game_context.right_helper_text += TextFormat("%5s: ", ConnectorKeyDString);
            break;
        case ConnectorKey::Space:
            game_context.right_helper_text += TextFormat("%5s: ", ConnectorKeySpaceString);
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
                game_context.right_helper_text += TextFormat("%s", ConnectorActionMovementLeftString);
                break;
            case ConnectorAction::MovementRight:
                game_context.right_helper_text += TextFormat("%s", ConnectorActionMovementRightString);
                break;
            case ConnectorAction::MovementUp:
                game_context.right_helper_text += TextFormat("%s", ConnectorActionMovementUpString);
                break;
            case ConnectorAction::MovementDown:
                game_context.right_helper_text += TextFormat("%s", ConnectorActionMovementDownString);
                break;
            case ConnectorAction::Jump:
                game_context.right_helper_text += TextFormat("%s", ConnectorActionJumpString);
                break;
            }
            if (i < actions.size()-1)
            {
                game_context.right_helper_text += " -> ";
            }
        }
        game_context.right_helper_text += "\n";
    }
    game_context.right_helper_text = rtrim(game_context.right_helper_text);
    if (game_context.right_helper_text.empty())
    {
        game_context.right_helper_text = TextFormat(RightHelperTextNoKeyBindsFormat);
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
    updateKeyBinds();

    // debug
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
    if (IsKeyDown(KEY_F2))
    {
        TraceLog(LOG_DEBUG, "node %d -> ", node.index);
        for(const auto& connected_index : node.connected_nodes)
        {
            TraceLog(LOG_DEBUG, " %d", connected_index);
        }
    }
#endif
#endif
}
void UpdateAllNodes()
{
    for(auto& node : game_context.nodes)
    {
        updateNodeConnections(node);
        updateCountConnectedNode(node);
    }
    /// @TODO: minimize updateing node properties

    updateLevelConnectionCount();
    updateKeyBinds();

    if (game_context.level_connections > 0)
    {
        game_context.left_helper_text = TextFormat(LeftHelperTextConnectionsFormat, game_context.level_connections, game_context.level_max_node_connections, game_context.level_max_actions_per_key);
    }
    else
    {
        game_context.left_helper_text = TextFormat(LeftHelperNoConnectionsTextFormat);
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
    game_context.player_current_key = ConnectorKey::NONE;
    game_context.player_action_index = -1;
    game_context.turn_cooldown = std::chrono::milliseconds::zero();
    game_context.player_tiles_position = game_context.player_start_tiles_position;
    game_context.player_direction = game_context.player_start_direction;
    //game_context.state = GameState::NodesMain;
}
static void UpdateCharacterMain()
{
    // reset button
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(ResetMainCharacterButtonRect, game_context.mouse)) {
        game_context.state = GameState::NodesMain;
        game_context.player_current_key = ConnectorKey::NONE;
        game_context.player_action_index = -1;
        game_context.turn_cooldown = std::chrono::milliseconds::zero();
        game_context.player_tiles_position = game_context.player_start_tiles_position;
        game_context.player_direction = game_context.player_start_direction;
        UpdateAllNodes();
        return;
    }

    // Update key binds (pressed)
    if (!game_context.player_on_void_tile)
    {
        for (const auto& [key, actions]: game_context.key_binds)
        {
            if (game_context.player_action_index == -1 && IsKeyPressed(static_cast<int>(key)))
            {
                game_context.player_current_key = key;
                game_context.player_action_index = 0;
                game_context.turn_cooldown = std::chrono::milliseconds::zero();
                break;
            }
        }
    }
    if (game_context.turn_cooldown <= std::chrono::milliseconds::zero())
    {
        if (game_context.player_current_key != ConnectorKey::NONE && game_context.player_action_index >= 0)
        {
            if (game_context.player_action_index < game_context.key_binds[game_context.player_current_key].size())
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
                    game_context.player_tiles_position.x += JumpFactor;
                        break;
                case CharacterDirection::Left:
                    game_context.player_tiles_position.x -= JumpFactor;
                        break;
                case CharacterDirection::Up:
                    game_context.player_tiles_position.y -= JumpFactor;
                        break;
                case CharacterDirection::Down:
                    game_context.player_tiles_position.y += JumpFactor;
                        break;
                    }
                    break;
                }
            }
            game_context.player_action_index++;
            game_context.turn_cooldown = TurnCooldown;
        }
    }
    else
    {
        game_context.turn_cooldown -= game_context.delta;
    }

    // check map conditions
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
        const auto player_map_tile = (player_map_tile_index != nullptr) ? static_cast<TileSet>(*player_map_tile_index) : TileSet::Void1;
        game_context.player_on_void_tile = player_map_tile == TileSet::Void1 || player_map_tile == TileSet::Void2;
        game_context.player_on_door_tile = player_map_tile == TileSet::Door;

        // reset action (animation)
        if (game_context.player_current_key != ConnectorKey::NONE &&
            (game_context.player_action_index > game_context.key_binds[game_context.player_current_key].size() ||
             game_context.key_binds[game_context.player_current_key].size() == 1)) // if only have ONE action, no need for waiting the next action
        {
            game_context.player_current_key = ConnectorKey::NONE;
            game_context.player_action_index = -1;
            game_context.turn_cooldown = std::chrono::milliseconds::zero();

            // wait for death and door
            if (game_context.player_on_void_tile || game_context.player_on_door_tile)
            {
                game_context.turn_cooldown = TurnCooldown;
            }
        }

        if (game_context.turn_cooldown <= std::chrono::milliseconds::zero())
        {
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
            case TileSet::Void1:
            case TileSet::Void2:
                PlayerDie();
                break;
            }
        }

        //const auto& px = LevelMapArea.x + game_context.player_tiles_position.x*CharacterSpriteWidth;
        //const auto& py = LevelMapArea.y + game_context.player_tiles_position.y*CharacterSpriteHeight;
        //const Rectangle player_position {px, py, CharacterSpriteWidth, CharacterSpriteHeight};
    }

}


static void UpdateStart()
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(StartButtonRect, game_context.mouse) || IsKeyPressed(KEY_ENTER)) {
        SetLevel(StartLevel);
    }
}
static void UpdateEnd()
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(StartButtonRect, game_context.mouse) || IsKeyPressed(KEY_ENTER)) {
        // restart
        game_context.player_current_key = ConnectorKey::NONE;
        game_context.player_action_index = -1;
        game_context.player_tiles_position = game_context.player_start_tiles_position;
        game_context.player_direction = game_context.player_start_direction;
        game_context.state = GameState::Start;
        SetLevel(StartLevel);
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
    if (game_context.state == GameState::NodesMain || game_context.state == GameState::CharacterMain)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(Help1IconArea, game_context.mouse))
        {
            game_context.show_help_1 = !game_context.show_help_1;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(Help2IconArea, game_context.mouse))
        {
            game_context.show_help_2 = !game_context.show_help_2;
        }
    }

    // dev tools (debug)
#if !defined(PLATFORM_WEB)
    #ifndef NDEBUG
    if (IsKeyPressed(KEY_F3))
    {
        NextLevel();
    }
    #endif
#endif
}

void RenderNodeLines(const ConnectorNode& node)
{
    auto lineColor = (game_context.state == GameState::NodesMain) ? NodeLineColor : DisabledColor;
    // render Connections
    for (auto direct_connected_node_index : node.direct_connections)
    {
        if (direct_connected_node_index != -1)
        {
            const auto& sibling_connected = game_context.nodes[direct_connected_node_index];
            DrawLineEx(node.data.position, sibling_connected.data.position, NodeLineThick, lineColor);
        }
    }
}
void RenderNode(const ConnectorNode& node)
{
    // render Node
    switch (node.data.type)
    {
    case ConnectorType::DISABLED:
        return;
    case ConnectorType::Action:
        {
            auto actionColor = (game_context.state == GameState::NodesMain) ? ActionNodeColor : DisabledColor;
            const char* innerTextAction = [&]()
            {
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
                if (IsKeyDown(KEY_F1))
                {
                    // debug
                    return TextFormat("%i", node.index);
                }
#endif
#endif

                switch (node.data.action)
                {
                case ConnectorAction::NONE:
                    return "";
                case ConnectorAction::MovementLeft:
                    return NodeActionMovementLeftString;
                case ConnectorAction::MovementRight:
                    return NodeActionMovementRightString;
                case ConnectorAction::MovementUp:
                    return NodeActionMovementUpString;
                case ConnectorAction::MovementDown:
                    return NodeActionMovementDownString;
                case ConnectorAction::Jump:
                    return NodeActionJumpString;
                }
                return "";
            }();
            const auto innerTextActionSize = MeasureTextEx(GetFontDefault(), innerTextAction, NodeFontSize, NodeFontSize/10);

            // draw background color on overlapping line
            DrawPoly(node.data.position, ActionNodeSides, ActionNodeRadius, ActionNodeRotation, BackgroundColor);
            if (node.is_selected)
            {
                DrawPoly(node.data.position, ActionNodeSides, ActionNodeRadius, ActionNodeRotation, actionColor);
            } else {
                DrawPolyLines(node.data.position, ActionNodeSides, ActionNodeRadius, ActionNodeRotation, actionColor);
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
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
                if (IsKeyDown(KEY_F1))
                {
                    if (node.is_selected)
                    {
                        DrawText(innerTextAction, node.data.position.x - innerTextActionSize.x/2, node.data.position.y - innerTextActionSize.y/2, NodeFontSize, BackgroundColor);
                    } else
                    {
                        DrawText(innerTextAction, node.data.position.x - innerTextActionSize.x/2, node.data.position.y - innerTextActionSize.y/2, NodeFontSize, actionColor);
                    }
                    break;
                }
#endif
#endif
                // Render Action Icon
                auto iconColor = node.is_selected ? BackgroundColor : actionColor;
                DrawTexturePro(game_context.icons_sprite_sheet_texture,
                    { static_cast<float>(static_cast<int>(node.data.action)*ActionIconSpriteWidth), 0, ActionIconSpriteWidth, ActionIconSpriteHeight },
                    { node.data.position.x, node.data.position.y, ActionIconSpriteWidth, ActionIconSpriteHeight},
                    {ActionIconSpriteWidth/2, ActionIconSpriteHeight/2}, 0, iconColor);
                break;
            }
            break;
        }
    case ConnectorType::Key:
        {
            auto keyColor = (game_context.state == GameState::NodesMain) ? KeyNodeColor : DisabledColor;
            const char* innerTextKey = [&]()
            {
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
                if (IsKeyDown(KEY_F1))
                {
                    // debug
                    return TextFormat("%i", node.index);
                }
#endif
#endif

                switch (node.data.key)
                {
                case ConnectorKey::NONE:
                    return "";
                case ConnectorKey::W:
                    return NodeKeyWString;
                case ConnectorKey::A:
                    return NodeKeyAString;
                case ConnectorKey::S:
                    return NodeKeySString;
                case ConnectorKey::D:
                    return NodeKeyDString;
                case ConnectorKey::Space:
                    return NodeKeySpaceString;
                }
                return "";
            }();
            const auto innerTextKeySize = MeasureTextEx(GetFontDefault(), innerTextKey, NodeFontSize, NodeFontSize/10);

            // draw background color on overlapping line
            DrawCircle(node.data.position.x, node.data.position.y, KeyNodeRadius, BackgroundColor);
            if (node.is_selected)
            {
                DrawCircle(node.data.position.x, node.data.position.y, KeyNodeRadius, keyColor);
            } else
            {
                DrawCircleLines(node.data.position.x, node.data.position.y, KeyNodeRadius, keyColor);
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
                    DrawText(innerTextKey, node.data.position.x - innerTextKeySize.x/2, node.data.position.y - innerTextKeySize.y/2, NodeFontSize, keyColor);
                }
                break;
            }
            break;
        }
    }
}
void RenderMap()
{
    // Render Map
    if (game_context.map_data != nullptr)
    {
        // render level
        for(int y = 0;y < LevelMapHeight; y++)
        {
            for(int x = 0;x < LevelMapWidth; x++)
            {
                const auto& tile = (*game_context.map_data)[y][x];

                float sx = tile*LevelTileWidth;
                float sy = 0;

                float dx = LevelMapArea.x + x*LevelTileWidth;
                float dy = LevelMapArea.y + y*LevelTileHeight;

                DrawTexturePro(game_context.tileset_texture,
                    { sx, sy, LevelTileWidth, LevelTileHeight },
                    { dx, dy, LevelTileWidth, LevelTileHeight},
                    {0, 0}, 0, NeutralTintColor);
            }
        }

        const float character_scale = (game_context.player_on_void_tile) ? PlayerOnVoidTileScale : 1.0f;
        const Rectangle character_pos { LevelMapArea.x + game_context.player_tiles_position.x * LevelTileWidth, LevelMapArea.y + game_context.player_tiles_position.y * LevelTileHeight, CharacterSpriteWidth*character_scale, CharacterSpriteHeight*character_scale};

        // render preview lines
        if (game_context.show_help_2)
        {
            if (game_context.player_action_index == -1 && game_context.turn_cooldown <= std::chrono::milliseconds::zero())
            {
                for (const auto& [key, actions] : game_context.key_binds)
                {
                    if (!actions.empty())
                    {
                        Vector2 tile_position = game_context.player_tiles_position;
                        Vector2 startPosLine {character_pos.x + character_pos.width/2, character_pos.y + character_pos.height/2};
                        Vector2 endPosLine {character_pos.x + character_pos.width/2, character_pos.y + character_pos.height/2};
                        auto preview_direction = game_context.player_direction;

                        bool preview_on_void_tile = false;
                        const auto isTileVoid = [&](Vector2 tp)
                        {
                            if (game_context.map_data != nullptr &&
                                tile_position.x >= 0 && tile_position.y >= 0 &&
                                tile_position.y < game_context.map_data->size() &&
                                tile_position.x < (*game_context.map_data)[tile_position.y].size())
                            {
                                const auto& tile = static_cast<TileSet>((*game_context.map_data)[tile_position.y][tile_position.x]);
                                return tile == TileSet::Void1 || tile == TileSet::Void2;
                            }
                            return true;
                        };

                        const auto movePosLineByAction = [&](Vector2& pos, Vector2* tp, auto action){
                            switch(action)
                            {
                            case ConnectorAction::NONE:
                                break;
                            case ConnectorAction::MovementRight:
                                pos.x += LevelTileWidth;
                                if (tp != nullptr) tp->x++;
                                preview_direction = CharacterDirection::Right;
                                return Vector2{1, 0};
                            case ConnectorAction::MovementLeft:
                                pos.x -= LevelTileWidth;
                                if (tp != nullptr) tp->x--;
                                preview_direction = CharacterDirection::Left;
                                return Vector2{-1, 0};
                            case ConnectorAction::MovementDown:
                                pos.y += LevelTileWidth;
                                if (tp != nullptr) tp->y++;
                                preview_direction = CharacterDirection::Down;
                                return Vector2{0, 1};
                            case ConnectorAction::MovementUp:
                                pos.y -= LevelTileWidth;
                                if (tp != nullptr) tp->y--;
                                preview_direction = CharacterDirection::Up;
                                return Vector2{0, .1};
                            case ConnectorAction::Jump:
                                switch (preview_direction)
                                {
                                case CharacterDirection::Right:
                                    pos.x += JumpFactor*LevelTileWidth;
                                    if (tp != nullptr) tp->x += JumpFactor;
                                    preview_direction = CharacterDirection::Right;
                                    return Vector2{1, 0};
                                case CharacterDirection::Left:
                                    pos.x -= JumpFactor*LevelTileWidth;
                                    if (tp != nullptr) tp->x -= JumpFactor;
                                    preview_direction = CharacterDirection::Left;
                                    return Vector2{-1, 0};
                                case CharacterDirection::Up:
                                    pos.y -= JumpFactor*LevelTileWidth;
                                    if (tp != nullptr) tp->y -= JumpFactor;
                                    preview_direction = CharacterDirection::Up;
                                    return Vector2{0, -1};
                                case CharacterDirection::Down:
                                    pos.y += JumpFactor*LevelTileWidth;
                                    if (tp != nullptr) tp->y += JumpFactor;
                                    preview_direction = CharacterDirection::Down;
                                    return Vector2{0, 1};
                                }
                                break;
                            }
                            return Vector2{1, 1};
                        };
                        Vector2 direction_vector {1, 1};
                        for (const auto& action : actions)
                        {
                            if (!preview_on_void_tile)
                            {
                                direction_vector = movePosLineByAction(endPosLine, &tile_position, action);
                                if (CheckCollisionPointRec(endPosLine, LevelMapArea))
                                {
                                    DrawLineEx(startPosLine, endPosLine, PreviewLineThick, PreviewLineColor);
                                }
                                direction_vector = movePosLineByAction(startPosLine, nullptr, action);
                            } else
                            {
                                direction_vector = movePosLineByAction(endPosLine, &tile_position, action);
                                // make dotted line
                                const auto max_step = Vector2Distance(startPosLine, endPosLine);
                                auto innerStartPosLine = startPosLine;
                                auto innerEndPosLine = Vector2MoveTowards(startPosLine, endPosLine, 2);
                                for (int step = 0;step < max_step && Vector2Distance(innerEndPosLine, endPosLine) > 0;step += 2*PreviewLineThick)
                                {
                                    if (CheckCollisionPointRec(innerEndPosLine, LevelMapArea))
                                    {
                                        DrawLineEx(innerStartPosLine, innerEndPosLine, PreviewLineThick, PreviewLineColor);
                                    }
                                    innerStartPosLine = Vector2MoveTowards(innerEndPosLine, endPosLine, PreviewLineThick);
                                    innerEndPosLine = Vector2MoveTowards(innerStartPosLine, endPosLine, 2*PreviewLineThick);
                                }
                                if (CheckCollisionPointRec(endPosLine, LevelMapArea))
                                {
                                    DrawLineEx(innerEndPosLine, endPosLine, PreviewLineThick, PreviewLineColor);
                                }
                                direction_vector = movePosLineByAction(startPosLine, nullptr, action);
                            }
                            preview_on_void_tile = preview_on_void_tile || isTileVoid(tile_position);
                        }

                        const auto keyText = [&]()
                        {
                            switch (key)
                            {
                            case ConnectorKey::NONE:
                                break;
                            case ConnectorKey::W:
                                return ConnectorKeyWString;
                            case ConnectorKey::A:
                                return ConnectorKeyAString;
                            case ConnectorKey::S:
                                return ConnectorKeySString;
                            case ConnectorKey::D:
                                return ConnectorKeyDString;
                            case ConnectorKey::Space:
                                return ConnectorKeySpaceString;
                            }
                            return "";
                        }();
                        auto keyTextSize = MeasureTextEx(GetFontDefault(), keyText, PreviewTextFontSize, PreviewTextFontSize/10);
                        Rectangle keyTextPos {startPosLine.x + direction_vector.x*keyTextSize.x/2 + PreviewLineThick+1, startPosLine.y + direction_vector.y*keyTextSize.y/8 + PreviewLineThick, keyTextSize.x, keyTextSize.y};
                        if (CheckCollisionRecs(keyTextPos, LevelMapArea))
                        {
                            DrawText(keyText, keyTextPos.x, keyTextPos.y, PreviewTextFontSize, PreviewLineColor);
                        }
                    }
                }
            }
        }

        // Render Character
        // only render when in map bound
        if (CheckCollisionRecs(character_pos, LevelMapArea))
        {
            DrawTexturePro(game_context.character_sprite_sheet_texture,
                { static_cast<float>(static_cast<int>(game_context.player_direction)*CharacterSpriteWidth), 0, CharacterSpriteWidth, CharacterSpriteHeight },
                character_pos,
                {0, 0}, 0, NeutralTintColor);
        }
    }

    // border
    DrawRectangleLinesEx(LevelMapArea, BorderLineThick, (game_context.state == GameState::CharacterMain) ? MapActiveBorderColor : BorderColor);
}


// Update and draw frame
void UpdateDrawFrame()
{
    auto mouse_pos = GetMousePosition();
    game_context.mouse = {mouse_pos.x, mouse_pos.y ,8 ,8};

    // Update
    UpdateGameLogic();

    // Draw
    //----------------------------------------------------------------------------------
    // Render to screen (main framebuffer)
    BeginDrawing();
        ClearBackground(BackgroundColor);

        // borders
        DrawRectangleLinesEx(ConnectorArea, BorderLineThick, BorderColor);
        DrawRectangleLinesEx(LevelArea, BorderLineThick, BorderColor);

        if (game_context.state == GameState::Start)
        {
            // title
            //const auto titleTextSize = MeasureTextEx(GetFontDefault(), TitleText, TitleTextFontSize, TitleTextFontSize/10);
            //DrawText(TitleText, ConnectorArea.x + ConnectorArea.width/2 - titleTextSize.x/2, ConnectorArea.y + 72, TitleTextFontSize, TextFontColor);
            //const auto subTitleTextSize = MeasureTextEx(GetFontDefault(), SubTitleText, SubTitleTextFontSize, SubTitleTextFontSize/10);
            //DrawText(SubTitleText, ConnectorArea.x + ConnectorArea.width/2 - titleTextSize.x/2, ConnectorArea.y + 72 + titleTextSize.y + 4, SubTitleTextFontSize, DisabledColor);
            DrawTexture(game_context.logo_texture, ConnectorArea.x + ConnectorArea.width/2 - game_context.logo_texture.width/2, ConnectorArea.y + 72, NeutralTintColor);

            const auto startButtonTextSize = MeasureTextEx(GetFontDefault(), WelcomeStartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
            const auto startButtonColor = (CheckCollisionRecs(StartButtonRect, game_context.mouse)) ? ButtonHoverColor : ButtonColor;
            DrawRectangleLinesEx(StartButtonRect, ButtonLineThick, startButtonColor);
            DrawText(WelcomeStartButtonText, StartButtonRect.x + StartButtonRect.width/2 - startButtonTextSize.x/2, StartButtonRect.y + StartButtonRect.height/2 - startButtonTextSize.y/2, StartButtonTextFontSize, startButtonColor);

            // show welcome text
            const auto welcomeTextSize = MeasureTextEx(GetFontDefault(), WelcomeText, WelcomeTextFontSize, WelcomeTextFontSize/10);
            DrawText(WelcomeText, WelcomeTextArea.x + LevelArea.width/2 - welcomeTextSize.x/2, WelcomeTextArea.y, WelcomeTextFontSize, TextFontColor);

            DrawTexture(game_context.instruction1_texture, HelpInstruction1Area.x, HelpInstruction1Area.y, NeutralTintColor);
            DrawTexture(game_context.instruction2_texture, HelpInstruction2Area.x, HelpInstruction2Area.y, NeutralTintColor);
        }
        else if (game_context.state == GameState::NodesMain || game_context.state == GameState::CharacterMain)
        {
            DrawRectangleLinesEx(LeftTextArea, BorderLineThick, BorderColor);
            DrawRectangleLinesEx(RightTextArea, BorderLineThick, BorderColor);

            for (const auto& node : game_context.nodes)
            {
                RenderNodeLines(node);
            }
            // pre-view line
            if (game_context.node_selection_mode && CheckCollisionRecs(ConnectorArea, game_context.mouse))
            {
                const auto mouse_pos = GetMousePosition();
                for (const auto& node : game_context.nodes)
                {
                    if (node.is_selected)
                    {
                        DrawLineEx(node.data.position, mouse_pos, BorderLineThick, DisabledColor);
                    }
                }
            }

            for (const auto& node : game_context.nodes)
            {
                RenderNode(node);
            }

            // level text
            const auto startButtonTextSize = MeasureTextEx(GetFontDefault(), game_context.level_helper_text.c_str(), LevelHelperTextFontSize, LevelHelperTextFontSize/10);
            DrawText(game_context.level_helper_text.c_str(), LevelArea.x + LevelArea.width/2 - startButtonTextSize.x/2, LevelArea.y + 8, LevelHelperTextFontSize, TextFontColor);

            // helper text (connections, node info)
            DrawText(game_context.left_helper_text.c_str(), LeftTextArea.x + 8, LeftTextArea.y + 8, HelperTextFontSize, TextFontColor);

            // Main GO Button
            if (game_context.state == GameState::NodesMain) {
                const auto startButtonTextSize = MeasureTextEx(GetFontDefault(), GoButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
                const auto buttonColor = (CheckCollisionRecs(StartMainCharacterButtonRect, game_context.mouse)) ? ButtonHoverColor : ButtonColor;
                DrawRectangleLinesEx(StartMainCharacterButtonRect, ButtonLineThick, buttonColor);
                DrawText(GoButtonText, StartMainCharacterButtonRect.x + StartMainCharacterButtonRect.width/2 - startButtonTextSize.x/2, StartMainCharacterButtonRect.y + StartMainCharacterButtonRect.height/2 - startButtonTextSize.y/2, StartButtonTextFontSize, buttonColor);
            }
            // Character Reset Button
            if (game_context.state == GameState::CharacterMain) {
                const auto restartButtonTextSize = MeasureTextEx(GetFontDefault(), RestartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
                const auto buttonColor = (CheckCollisionRecs(ResetMainCharacterButtonRect, game_context.mouse)) ? ButtonHoverColor : ButtonColor;
                DrawRectangleLinesEx(ResetMainCharacterButtonRect, ButtonLineThick, buttonColor);
                DrawText(RestartButtonText, ResetMainCharacterButtonRect.x + ResetMainCharacterButtonRect.width/2 - restartButtonTextSize.x/2, ResetMainCharacterButtonRect.y + StartMainCharacterButtonRect.height/2 - restartButtonTextSize.y/2, StartButtonTextFontSize, buttonColor);
            }

            // helper text (actions, key binds)
            DrawText(game_context.right_helper_text.c_str(), RightHelperTextAreaRect.x, RightHelperTextAreaRect.y, HelperTextFontSize, TextFontColor);

            RenderMap();

            if (game_context.show_help_1)
            {
                DrawTexture(game_context.instruction1_texture, InGameHelpInstruction1Area.x, InGameHelpInstruction1Area.y, NeutralTintColor);
                DrawTexture(game_context.instruction2_texture, InGameHelpInstruction2Area.x, InGameHelpInstruction2Area.y, NeutralTintColor);
            }

            // help icon 1
            {
                const auto helpButtonTextSize = MeasureTextEx(GetFontDefault(), Help1IconString, HelpIconFontSize, HelpIconFontSize/10);
                const auto buttonColor = [&]()
                {
                    if (game_context.show_help_1)
                    {
                        return ButtonHoverColor;
                    }

                    return (CheckCollisionRecs(Help1IconArea, game_context.mouse)) ? ButtonHoverColor : DisabledColor; ///< use disabled color for more subtle coloring
                }();
                DrawCircleLines(Help1IconArea.x + Help1IconArea.width/2, Help1IconArea.y + Help1IconArea.height/2, HelpIconRadius, buttonColor);
                DrawText(Help1IconString, Help1IconArea.x + Help1IconArea.width/2 - helpButtonTextSize.x/2, Help1IconArea.y + Help1IconArea.height/2 - helpButtonTextSize.y/2, HelpIconFontSize, buttonColor);
            }
            // help icon 2
            {
                const auto helpButtonTextSize = MeasureTextEx(GetFontDefault(), Help2IconString, HelpIconFontSize, HelpIconFontSize/10);
                const auto buttonColor = [&]()
                {
                    if (game_context.show_help_2)
                    {
                        return ButtonHoverColor;
                    }

                    return (CheckCollisionRecs(Help2IconArea, game_context.mouse)) ? ButtonHoverColor : DisabledColor; ///< use disabled color for more subtle coloring
                }();
                DrawCircleLines(Help2IconArea.x + Help2IconArea.width/2, Help2IconArea.y + Help2IconArea.height/2, HelpIconRadius, buttonColor);
                DrawText(Help2IconString, Help2IconArea.x + Help2IconArea.width/2 - helpButtonTextSize.x/2, Help2IconArea.y + Help2IconArea.height/2 - helpButtonTextSize.y/2, HelpIconFontSize, buttonColor);
            }
        }
        else if (game_context.state == GameState::End)
        {
            // title
            //const auto titleTextSize = MeasureTextEx(GetFontDefault(), TitleText, TitleTextFontSize, TitleTextFontSize/10);
            //DrawText(TitleText, ConnectorArea.x + ConnectorArea.width/2 - titleTextSize.x/2, ConnectorArea.y + 72, TitleTextFontSize, TextFontColor);
            //const auto subTitleTextSize = MeasureTextEx(GetFontDefault(), SubTitleText, SubTitleTextFontSize, SubTitleTextFontSize/10);
            //DrawText(SubTitleText, ConnectorArea.x + ConnectorArea.width/2 - titleTextSize.x/2, ConnectorArea.y + 72 + titleTextSize.y + 4, SubTitleTextFontSize, DisabledColor);
            DrawTexture(game_context.logo_texture, ConnectorArea.x + ConnectorArea.width/2 - game_context.logo_texture.width/2, ConnectorArea.y + 72, NeutralTintColor);

            // show welcome text
            const auto endTextSize = MeasureTextEx(GetFontDefault(), EndText, EndTextFontSize, EndTextFontSize/10);
            DrawText(EndText, LevelArea.x + LevelArea.width/2 - endTextSize.x/2, LevelArea.y + 64, EndTextFontSize, TextFontColor);

            const auto startButtonTextSize = MeasureTextEx(GetFontDefault(), EndStartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
            const auto startButtonColor = (CheckCollisionRecs(StartButtonRect, game_context.mouse)) ? ButtonHoverColor : ButtonColor;
            DrawRectangleLinesEx(StartButtonRect, ButtonLineThick, startButtonColor);
            DrawText(EndStartButtonText, StartButtonRect.x + StartButtonRect.width/2 - startButtonTextSize.x/2, StartButtonRect.y + StartButtonRect.height/2 - startButtonTextSize.y/2, StartButtonTextFontSize, startButtonColor);
        }

    EndDrawing();
    //----------------------------------------------------------------------------------  
}