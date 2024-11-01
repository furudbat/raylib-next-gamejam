#pragma once

#include "types.h"

namespace level1
{

inline constexpr Level_t MapData = {
    // clang-format off
    LevelLine_t{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
              {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
              {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3},
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3},
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3},
              {4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 3},
              {3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 3},
              {3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 3},
              {3, 3, 3, 3, 3, 3, 3, 0, 1, 0, 3}
    // clang-format on
};

inline constexpr Vector2 CharacterStartTilesPosition = {0, 4};
inline constexpr CharacterDirection CharacterStartDirection = CharacterDirection::Right;
inline constexpr int MaxNodeConnections = 2;
inline constexpr int MaxActionsPerKey = 2;

inline constexpr std::array<NodeData, 4> NodesData = {
    ActionNode({120, 100}, ConnectorAction::MovementRight),
    ActionNode({220, 200}, ConnectorAction::MovementDown),
    KeyNode({280, 100}, ConnectorKey::J),
    KeyNode({120, 260}, ConnectorKey::L),
};

} // namespace level1