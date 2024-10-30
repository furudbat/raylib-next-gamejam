#pragma once

#include "types.h"

namespace level3
{

inline constexpr Level_t MapData = {
    // clang-format: off
    LevelLine_t {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
              {3, 3, 0, 3, 3, 3, 3, 3, 3, 3, 3},
              {3, 3, 0, 3, 0, 0, 0, 0, 3, 3, 3},
              {3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3},
              {3, 3, 0, 0, 4, 0, 4, 0, 3, 3, 3},
              {3, 3, 0, 4, 3, 4, 3, 0, 3, 3, 3},
              {3, 3, 4, 3, 3, 3, 3, 1, 3, 3, 3},
              {3, 3, 3, 0, 3, 0, 3, 4, 3, 3, 3},
              {3, 3, 0, 0, 3, 4, 3, 3, 3, 3, 3},
              {3, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3}
    // clang-format: on
};

inline constexpr Vector2 CharacterStartTilesPosition = { 2,2 };
inline constexpr CharacterDirection CharacterStartDirection = CharacterDirection::Down;
inline constexpr int MaxNodeConnections = 4;
inline constexpr int MaxActionsPerKey = 2;

inline constexpr std::array<NodeData, 6> NodesData = {
    ActionNode({185, 230}, ConnectorAction::MovementRight),
    ActionNode({90, 135}, ConnectorAction::MovementDown),
    ActionNode({220, 165}, ConnectorAction::MovementDown),
    ActionNode({280, 165}, ConnectorAction::MovementUp),
    KeyNode({90, 200}, ConnectorKey::W),
    KeyNode({185, 100}, ConnectorKey::S),
};

}