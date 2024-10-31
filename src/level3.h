#pragma once

#include "types.h"

namespace level3
{

inline constexpr Level_t MapData = {
    // clang-format: off
    LevelLine_t {0, 0, 3, 3, 3, 3, 3, 3, 3, 0, 3},
                {4, 4, 3, 3, 3, 3, 3, 3, 0, 0, 3},
                {3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 3},
                {3, 3, 0, 3, 3, 3, 3, 3, 3, 3, 3},
                {3, 3, 0, 0, 3, 3, 3, 0, 3, 3, 3},
                {3, 3, 4, 0, 0, 3, 0, 0, 1, 3, 3},
                {3, 3, 3, 4, 0, 0, 0, 4, 4, 3, 3},
                {0, 0, 0, 3, 4, 4, 4, 3, 3, 3, 3},
                {4, 4, 0, 3, 3, 3, 3, 0, 0, 3, 3},
                {3, 3, 4, 3, 3, 3, 3, 4, 4, 3, 3}
    // clang-format: on
};

inline constexpr Vector2 CharacterStartTilesPosition = { 2,3 };
inline constexpr CharacterDirection CharacterStartDirection = CharacterDirection::Down;
inline constexpr int MaxNodeConnections = 4;
inline constexpr int MaxActionsPerKey = 2;

inline constexpr std::array<NodeData, 6> NodesData = {
    ActionNode({225, 165}, ConnectorAction::MovementRight),
    ActionNode({90, 135}, ConnectorAction::MovementRight),
    ActionNode({280, 100}, ConnectorAction::MovementDown),
    ActionNode({185, 230}, ConnectorAction::MovementUp),
    KeyNode({90, 230}, ConnectorKey::L),
    KeyNode({220, 100}, ConnectorKey::H),
};

}