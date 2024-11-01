#pragma once

#include "types.h"

namespace level5
{

inline constexpr Level_t MapData = {
    // clang-format off
    LevelLine_t{3, 3, 3, 3, 3, 0, 0, 3, 0, 0, 3},
              {3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 3},
              {0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 3},
              {1, 4, 0, 0, 4, 4, 0, 4, 3, 0, 3},
              {4, 3, 0, 0, 3, 3, 4, 0, 0, 4, 3},
              {3, 3, 4, 0, 3, 3, 3, 4, 4, 0, 3},
              {3, 3, 3, 0, 3, 0, 3, 0, 0, 0, 3},
              {0, 3, 3, 4, 3, 4, 3, 4, 4, 0, 3},
              {0, 3, 3, 0, 3, 3, 3, 3, 3, 0, 3},
              {0, 3, 3, 0, 0, 0, 0, 3, 0, 0, 3}
    // clang-format on
};

inline constexpr Vector2 CharacterStartTilesPosition = {9, 9};
inline constexpr CharacterDirection CharacterStartDirection = CharacterDirection::Up;
inline constexpr int MaxNodeConnections = 4;
inline constexpr int MaxActionsPerKey = 2;

inline constexpr std::array<NodeData, 8> NodesData = {
    ActionNode({120, 295}, ConnectorAction::MovementLeft),
    ActionNode({280, 165}, ConnectorAction::MovementLeft),
    ActionNode({155, 165}, ConnectorAction::MovementDown),
    ActionNode({220, 260}, ConnectorAction::MovementUp),
    ActionNode({90, 230}, ConnectorAction::Jump),
    ActionNode({220, 100}, ConnectorAction::Jump),
    KeyNode({120, 100}, ConnectorKey::B),
    KeyNode({275, 310}, ConnectorKey::G),
};

} // namespace level5