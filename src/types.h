#pragma once

#include "constants.h"
#include <raylib.h>
#include <array>
#include <vector>
#include <unordered_set>
#include <cstdint>

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
enum class ConnectorKey : int
{
    NONE = KEY_NULL,
    W = KEY_W,
    A = KEY_A,
    S = KEY_S,
    D = KEY_D,
    Space = KEY_SPACE,
};

// also sprite indexes
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
    Void1,
    Void2
};
enum class ConnectorAction : int
{
    NONE = -1,
    MovementRight = 0,
    MovementLeft = 1,
    MovementDown = 2,
    MovementUp = 3,
    Jump = 4,
};

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