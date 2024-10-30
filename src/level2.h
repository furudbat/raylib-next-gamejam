#pragma once

#include "types.h"

namespace level2
{

inline constexpr Level_t MapData = {
    // clang-format: off
    LevelLine_t {3, 3, 3, 3, 0, 0, 0, 3, 3, 3, 3},
              {3, 3, 3, 3, 0, 0, 0, 3, 3, 3, 3},
              {3, 3, 3, 3, 0, 0, 0, 3, 3, 3, 3},
              {3, 3, 3, 3, 0, 0, 0, 3, 3, 3, 3},
              {3, 3, 3, 3, 4, 4, 4, 3, 3, 3, 3},
              {3, 3, 3, 3, 0, 0, 0, 3, 0, 0, 0},
              {3, 3, 3, 3, 0, 0, 0, 3, 0, 0, 1},
              {3, 3, 3, 3, 0, 0, 0, 3, 0, 0, 0},
              {3, 3, 3, 3, 4, 4, 4, 3, 4, 4, 4},
              {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}
    // clang-format: on
};

inline constexpr Vector2 CharacterStartTilesPosition = { 5,0 };
inline constexpr CharacterDirection CharacterStartDirection = CharacterDirection::Down;
inline constexpr int MaxNodeConnections = 3;
inline constexpr int MaxActionsPerKey = 2;

inline constexpr std::array<NodeData, 6> NodesData = {
    ActionNode({125, 165}, ConnectorAction::MovementRight),
    ActionNode({90, 100}, ConnectorAction::MovementDown),
    ActionNode({280, 230}, ConnectorAction::Jump),
    KeyNode({230, 100}, ConnectorKey::B),
    KeyNode({280, 100}, ConnectorKey::J),
    KeyNode({240, 145}, ConnectorKey::H),
};

}