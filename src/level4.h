#pragma once

#include "types.h"

namespace level4
{

inline constexpr Level_t MapData = {
    // clang-format: off
    LevelLine_t {3, 3, 3, 3, 3, 0, 0, 3, 0, 0, 3},
                {3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 3},
                {0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 3},
                {1, 4, 0, 0, 4, 4, 0, 4, 3, 0, 3},
                {4, 3, 0, 0, 3, 3, 3, 0, 0, 0, 3},
                {3, 3, 4, 0, 3, 3, 3, 4, 4, 4, 3},
                {3, 3, 3, 0, 3, 0, 3, 0, 0, 0, 3},
                {0, 3, 3, 4, 3, 4, 3, 4, 4, 0, 3},
                {0, 3, 3, 0, 3, 3, 3, 3, 3, 0, 3},
                {0, 3, 3, 0, 0, 0, 0, 3, 0, 0, 3}
    // clang-format: on
};

inline constexpr Vector2 CharacterStartTilesPosition = { 9,9 };
inline constexpr CharacterDirection CharacterStartDirection = CharacterDirection::Down;
inline constexpr int MaxNodeConnections = 4;
inline constexpr int MaxActionsPerKey = 2;

inline constexpr std::array<NodeData, 8> NodesData = {
    ActionNode({120, 295}, ConnectorAction::MovementLeft),
    ActionNode({265, 165}, ConnectorAction::MovementLeft),
    ActionNode({155, 165}, ConnectorAction::MovementDown),
    ActionNode({230, 260}, ConnectorAction::MovementUp),
    ActionNode({90, 230}, ConnectorAction::Jump),
    ActionNode({240, 100}, ConnectorAction::Jump),
    KeyNode({265, 295}, ConnectorKey::A),
    KeyNode({90, 100}, ConnectorKey::D),
};

}