#pragma once

#include "types.h"

namespace level3
{

inline constexpr Level_t MapData = {
    // clang-format: off
    LevelLine_t {3, 3, 3, 3, 3, 3, 0, 0, 0, 1, 3},
                {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
                {3, 3, 3, 3, 3, 3, 0, 3, 3, 3, 3},
                {3, 3, 0, 0, 3, 0, 0, 3, 3, 3, 3},
                {3, 3, 4, 4, 3, 4, 4, 3, 3, 3, 3},
                {3, 3, 0, 3, 3, 3, 3, 3, 3, 3, 3},
                {3, 3, 0, 3, 3, 0, 0, 0, 0, 0, 0},
                {0, 0, 0, 3, 3, 0, 4, 4, 4, 0, 0},
                {4, 4, 4, 3, 3, 0, 3, 0, 3, 4, 4},
                {3, 3, 3, 3, 3, 4, 3, 4, 3, 3, 3}
    // clang-format: on
};

inline constexpr Vector2 CharacterStartTilesPosition = { 0,7 };
inline constexpr CharacterDirection CharacterStartDirection = CharacterDirection::Right;
inline constexpr int MaxNodeConnections = 3;
inline constexpr int MaxActionsPerKey = 2;

inline constexpr std::array<NodeData, 6> NodesData = {
    ActionNode({185, 135}, ConnectorAction::MovementUp),
    ActionNode({280, 200}, ConnectorAction::MovementUp),
    ActionNode({155, 200}, ConnectorAction::MovementRight),
    ActionNode({220, 230}, ConnectorAction::Jump),
    KeyNode({90, 100}, ConnectorKey::A),
    KeyNode({280,135}, ConnectorKey::S),
};

}