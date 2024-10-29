#pragma once

#include <raylib.h>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

/// Color Palette
inline constexpr std::array<Color, 8> ColorPalette {
    Color{39, 41, 70, 255},     // Background
    Color{237, 160, 49, 255},
    Color{231, 255, 238, 255},
    Color{96, 34, 34, 255},
    Color{103, 160, 160, 255},
    Color{129, 15, 166, 255},
    Color{237, 35, 61, 255},
    RAYWHITE,
};


/// Rectangle/Areas
inline constexpr Rectangle ConnectorArea {
    0, 0, 400, 450,
};
inline constexpr Rectangle LevelArea {
    401, 0, 399, 450,
};

inline constexpr Rectangle LevelMapArea {
    435, 35, 325, 320
};
inline constexpr Rectangle LeftTextArea {
    25, 375, 352, 64,
};
inline constexpr Rectangle RightTextArea {
    425, 375, 352, 64,
};

inline constexpr Rectangle StartButtonRect {
    400 / 2 - 8*12/2,
    2*450/3 - 32/2,
    8*12,
    32,
};
inline constexpr Rectangle StartMainCharacterButtonRect {
    LeftTextArea.x + LeftTextArea.width - 8*12 - 8,
    LeftTextArea.y + LeftTextArea.height/2 - 32/2,
    8*12,
    32,
};
inline constexpr Rectangle ResetMainCharacterButtonRect {
    RightTextArea.x + 8,
    RightTextArea.y + RightTextArea.height/2 - 32/2,
    8*12,
    32,
};
inline constexpr Rectangle HelperTextAreaRect {
    ResetMainCharacterButtonRect.x + ResetMainCharacterButtonRect.width + 6,
    ResetMainCharacterButtonRect.y,
    RightTextArea.width/2,
    RightTextArea.height,
};

// Constants
inline constexpr int MaxNodeConnections = 2;
inline constexpr int MaxIndirectConnections = 4;
inline constexpr int MaxNodesInLevel = 10;

/// enums
enum class GameState
{
    Start,
    NodesMain,
    CharacterMain,
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
enum class CharacterDirection : uint8_t
{
    Right,
    Left,
    Up,
    Down
};
enum class TileSet : uint8_t
{
    Floor,
    Door,
    Key,
    Void,
};

/// Font size and style (colors)
inline constexpr int TextFontSize = 14;
inline constexpr int HelperTextFontSize = 12;
inline constexpr auto TextFontColor = ColorPalette[2];
inline constexpr auto BackgroundColor = ColorPalette[0];
inline constexpr auto BorderColor = ColorPalette[2];
inline constexpr auto ButtonColor = ColorPalette[2];
inline constexpr int TitleFontSize = 14;
//// Start
///
inline constexpr int TitleTextFontSize = 32;
inline constexpr int WelcomeTextFontSize = 16;
inline constexpr int StartButtonTextFontSize = 18;
inline constexpr int NodeFontSize = 16;
//// Nodes
inline constexpr int NodeLineThick = 2;
inline constexpr auto NodeLineColor = ColorPalette[2];
//// ActionNode
inline constexpr int ActionNodeSides = 6;
inline constexpr int ActionNodeRadius = 18;
inline constexpr int ActionNodeRotation = 0;
inline constexpr auto ActionNodeColor = ColorPalette[1];
//// KeyNode
inline constexpr int KeyNodeRadius = 18;
inline constexpr auto KeyNodeColor = ColorPalette[4];
//// Map
inline constexpr auto TileMapColor = ColorPalette[2];

/// Types
struct NodeData {
    Vector2 position{0, 0};
    ConnectorAction action{ConnectorAction::NONE};
    ConnectorKey key{ConnectorKey::NONE};
    ConnectorType type{ConnectorType::DISABLED};
};
struct ConnectorNode {
    int index{-1};
    NodeData data;
    int connected_counter{0};
    bool is_selected{false};
    std::array<int, MaxNodeConnections> direct_connections{};

    // computed
    std::unordered_set<int> connected_nodes{};
    std::vector<ConnectorAction> connected_actions{};

    ConnectorNode()
    {
        direct_connections.fill(-1);
        connected_nodes.reserve(MaxIndirectConnections);
        connected_actions.reserve(MaxIndirectConnections);
    }
};

inline constexpr void setActionNode(ConnectorNode& node, Vector2 pos, ConnectorAction action)
{
    node.data.position = pos;
    node.data.action = action;
    node.data.key = ConnectorKey::NONE;
    node.data.type = ConnectorType::Action;
    node.is_selected = false;
}
inline constexpr void setKeyNode(ConnectorNode& node, Vector2 pos, ConnectorKey key)
{
    node.data.position = pos;
    node.data.action = ConnectorAction::NONE;
    node.data.key = key;
    node.data.type = ConnectorType::Key;
    node.is_selected = false;
}
inline constexpr void clearKeyNode(ConnectorNode& node)
{
    //node.index = -1;
    node.data.position = {0, 0};
    node.data.action = ConnectorAction::NONE;
    node.data.key = ConnectorKey::NONE;
    node.data.type = ConnectorType::DISABLED;
    node.is_selected = false;
}

inline constexpr NodeData ActionNode(Vector2 pos, ConnectorAction action) {
    return {
        .position = pos,
        .action = action,
        .key = ConnectorKey::NONE,
        .type = ConnectorType::Action,
    };
}
inline constexpr NodeData KeyNode(Vector2 pos, ConnectorKey key) {
    return {
        .position = pos,
        .action = ConnectorAction::NONE,
        .key = key,
        .type = ConnectorType::Key,
    };
}

/// Level Settings
inline constexpr int LevelTilesWidth = 11;
inline constexpr int LevelTilesHeight = 10;
inline constexpr int LevelTilesetWidth = 32;
inline constexpr int LevelTilesetHeight = 32;
inline constexpr int MaxLevels = 1;
//// Character
inline constexpr int CharacterSpriteWidth = 32;
inline constexpr int CharacterSpriteHeight = 32;

using LevelLine_t = std::array<int, LevelTilesWidth>;
using Level_t = std::array<LevelLine_t, LevelTilesHeight>;

using GameLevelNodes = std::array<ConnectorNode, MaxNodesInLevel>;

template<size_t N>
requires (N <= MaxNodesInLevel)
auto GetLevelNodes(const std::array<NodeData, N>& data)
{
    GameLevelNodes ret;
    for (size_t i = 0; i < data.size(); ++i)
    {
        ret[i] = {};
        ret[i].data = data[i];
        //ret[i].index = static_cast<int>(i);
    }
    for (size_t i = 0; i < ret.size(); ++i)
    {
        ret[i].index = static_cast<int>(i);
    }
    return ret;
}