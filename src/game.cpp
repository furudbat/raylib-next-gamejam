#include "game.h"
#include "constants.h"
#include "types.h"
#include <raylib.h>
#include <array>
#include <chrono>
#include <unordered_map>
#include <vector>
// Levels
#include "level1.h"
#include "level2.h"
#include "level3.h"
#include "level4.h"
#include "level5.h"

//
// global game functions
//
void SetLevel(GameContext& gameContext, int level)
{
    using fsec = std::chrono::duration<float>;
    gameContext.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(fsec{GetTime()});
    gameContext.state = GameState::NodesMain;
    gameContext.level = level;
    gameContext.levelConnections = 0;
    gameContext.playerCurrentKey = ConnectorKey::NONE;
    gameContext.playerActionIndex = -1;
    gameContext.deathCount = 0;
    switch (gameContext.level)
    {
        case 1:
            gameContext.nodes = CreateLevelNodes(level1::NodesData);
            gameContext.mapData = &level1::MapData;
            gameContext.playerTilesPosition = level1::CharacterStartTilesPosition;
            gameContext.playerStartTilesPosition = level1::CharacterStartTilesPosition;
            gameContext.playerStartDirection = level1::CharacterStartDirection;
            gameContext.playerDirection = level1::CharacterStartDirection;
            gameContext.levelMaxNodeConnections = level1::MaxNodeConnections;
            gameContext.levelMaxActionsPerKey = level1::MaxActionsPerKey;
            gameContext.showHelp2 = true;
            break;
        case 2:
            gameContext.nodes = CreateLevelNodes(level2::NodesData);
            gameContext.mapData = &level2::MapData;
            gameContext.playerTilesPosition = level2::CharacterStartTilesPosition;
            gameContext.playerStartTilesPosition = level2::CharacterStartTilesPosition;
            gameContext.playerStartDirection = level2::CharacterStartDirection;
            gameContext.playerDirection = level2::CharacterStartDirection;
            gameContext.levelMaxNodeConnections = level2::MaxNodeConnections;
            gameContext.levelMaxActionsPerKey = level2::MaxActionsPerKey;
            if (!gameContext.manuelHelp)
            {
                gameContext.showHelp2 = true;
            }
            break;
        case 3:
            gameContext.nodes = CreateLevelNodes(level3::NodesData);
            gameContext.mapData = &level3::MapData;
            gameContext.playerTilesPosition = level3::CharacterStartTilesPosition;
            gameContext.playerStartTilesPosition = level3::CharacterStartTilesPosition;
            gameContext.playerStartDirection = level3::CharacterStartDirection;
            gameContext.playerDirection = level3::CharacterStartDirection;
            gameContext.levelMaxNodeConnections = level3::MaxNodeConnections;
            gameContext.levelMaxActionsPerKey = level3::MaxActionsPerKey;
            if (!gameContext.manuelHelp)
            {
                gameContext.showHelp2 = true;
            }
            break;
        case 4:
            gameContext.nodes = CreateLevelNodes(level4::NodesData);
            gameContext.mapData = &level4::MapData;
            gameContext.playerTilesPosition = level4::CharacterStartTilesPosition;
            gameContext.playerStartTilesPosition = level4::CharacterStartTilesPosition;
            gameContext.playerStartDirection = level4::CharacterStartDirection;
            gameContext.playerDirection = level4::CharacterStartDirection;
            gameContext.levelMaxNodeConnections = level4::MaxNodeConnections;
            gameContext.levelMaxActionsPerKey = level4::MaxActionsPerKey;
            break;
        case 5:
            gameContext.nodes = CreateLevelNodes(level5::NodesData);
            gameContext.mapData = &level5::MapData;
            gameContext.playerTilesPosition = level5::CharacterStartTilesPosition;
            gameContext.playerStartTilesPosition = level5::CharacterStartTilesPosition;
            gameContext.playerStartDirection = level5::CharacterStartDirection;
            gameContext.playerDirection = level5::CharacterStartDirection;
            gameContext.levelMaxNodeConnections = level5::MaxNodeConnections;
            gameContext.levelMaxActionsPerKey = level5::MaxActionsPerKey;
            break;
            /// @TODO: add (new) levels, don't forget to update MaxLevels
        default: TraceLog(LOG_ERROR, "Error Not Found: %i", gameContext.level);
    }
    UpdateAllNodes(gameContext);

    gameContext.levelHelperText = TextFormat(LevelsHelperFormat, gameContext.level);
}
void NextLevel(GameContext& gameContext)
{
    if (gameContext.level > 0 && gameContext.level < MaxLevels)
    {
        SetLevel(gameContext, gameContext.level + 1);
    }
    else if (gameContext.level == MaxLevels)
    {
        gameContext.state = GameState::End;
        return;
    }
}

static void updateCountConnectedNode(GameContext& gameContext, ConnectorNode& node);
static void updateNodeConnections(GameContext& gameContext, ConnectorNode& node);
static void updateKeyBinds(GameContext& gameContext);
static void updateLevelConnectionCount(GameContext& gameContext);
void UpdateAllNodes(GameContext& gameContext)
{
    for (auto& node : gameContext.nodes)
    {
        updateNodeConnections(gameContext, node);
        updateCountConnectedNode(gameContext, node);
    }
    /// @TODO: minimize updateing node properties

    updateLevelConnectionCount(gameContext);
    updateKeyBinds(gameContext);

    if (gameContext.levelConnections > 0)
    {
        gameContext.leftHelperText = TextFormat(
            LeftHelperTextConnectionsFormat,
            gameContext.levelConnections,
            gameContext.levelMaxNodeConnections,
            gameContext.levelMaxActionsPerKey);
    }
    else
    {
        gameContext.leftHelperText = TextFormat(LeftHelperNoConnectionsTextFormat);
    }
}

void updateLevelConnectionCount(GameContext& gameContext)
{
    gameContext.levelConnections = 0;
    for (const auto& node : gameContext.nodes)
    {
        if (node.data.type != ConnectorType::DISABLED)
        {
            for (const auto& connectedNodeIndex : node.direct_connections)
            {
                if (connectedNodeIndex != -1)
                {
                    gameContext.levelConnections++;
                }
            }
        }
    }

    /// @NOTE(workaround): for bidirectional connections
    gameContext.levelConnections /= 2;
}
void updateCountConnectedNode(GameContext& gameContext, ConnectorNode& node)
{
    node.connected_counter = 0;
    for (size_t i = 0; i < gameContext.nodes.size(); ++i)
    {
        if (i != node.index)
        {
            for (const auto& connectedNodeIndex : gameContext.nodes[i].direct_connections)
            {
                if (connectedNodeIndex != -1 && connectedNodeIndex == node.index)
                {
                    node.connected_counter++;
                }
            }
        }
    }
    updateNodeConnections(gameContext, node);
}
void updateKeyBinds(GameContext& gameContext)
{
    gameContext.keyBinds.clear();
    for (const auto& node : gameContext.nodes)
    {
        if (node.data.type == ConnectorType::Key && node.index != -1 && !node.connected_actions.empty())
        {
            switch (node.data.key)
            {
                case ConnectorKey::NONE: break;
                case ConnectorKey::H:
                case ConnectorKey::J:
                case ConnectorKey::K:
                case ConnectorKey::L:
                case ConnectorKey::B:
                case ConnectorKey::G: gameContext.keyBinds[node.data.key] = {}; break;
            }

            for (size_t i = 0; i < node.connected_actions.size(); ++i)
            {
                const auto& connectedAction = node.connected_actions[i];
                switch (connectedAction)
                {
                    case ConnectorAction::NONE: break;
                    case ConnectorAction::MovementLeft:
                    case ConnectorAction::MovementRight:
                    case ConnectorAction::MovementUp:
                    case ConnectorAction::MovementDown:
                    case ConnectorAction::Jump: gameContext.keyBinds[node.data.key].push_back(connectedAction); break;
                }
            }
        }
    }

    // update helper text
    gameContext.rightHelperText.clear();
    for (const auto& [key, actions] : gameContext.keyBinds)
    {
        switch (key)
        {
            case ConnectorKey::NONE: break;
            case ConnectorKey::H: gameContext.rightHelperText += TextFormat(" %s: ", ConnectorKeyHString); break;
            case ConnectorKey::J: gameContext.rightHelperText += TextFormat(" %s: ", ConnectorKeyJString); break;
            case ConnectorKey::K: gameContext.rightHelperText += TextFormat(" %s: ", ConnectorKeyKString); break;
            case ConnectorKey::L: gameContext.rightHelperText += TextFormat(" %s: ", ConnectorKeyLString); break;
            case ConnectorKey::B: gameContext.rightHelperText += TextFormat(" %s: ", ConnectorKeyBString); break;
            case ConnectorKey::G: gameContext.rightHelperText += TextFormat(" %s: ", ConnectorKeyGString); break;
        }
        for (size_t i = 0; i < actions.size(); ++i)
        {
            const auto& connectedAction = actions[i];
            switch (connectedAction)
            {
                case ConnectorAction::NONE: break;
                case ConnectorAction::MovementLeft:
                    gameContext.rightHelperText += TextFormat("%s", ConnectorActionMovementLeftString);
                    break;
                case ConnectorAction::MovementRight:
                    gameContext.rightHelperText += TextFormat("%s", ConnectorActionMovementRightString);
                    break;
                case ConnectorAction::MovementUp:
                    gameContext.rightHelperText += TextFormat("%s", ConnectorActionMovementUpString);
                    break;
                case ConnectorAction::MovementDown:
                    gameContext.rightHelperText += TextFormat("%s", ConnectorActionMovementDownString);
                    break;
                case ConnectorAction::Jump:
                    gameContext.rightHelperText += TextFormat("%s", ConnectorActionJumpString);
                    break;
            }
            if (i < actions.size() - 1)
            {
                gameContext.rightHelperText += " -> ";
            }
        }
        gameContext.rightHelperText += "\n";
    }
    gameContext.rightHelperText = utils::rtrim(gameContext.rightHelperText);
    if (gameContext.rightHelperText.empty())
    {
        gameContext.rightHelperText = TextFormat(RightHelperTextNoKeyBindsFormat);
    }
}
void updateNodeConnections(GameContext& gameContext, ConnectorNode& node)
{
    const auto addConnection = [&](int root_node_index, const ConnectorNode& connect_node)
    {
        if (connect_node.data.type != ConnectorType::DISABLED)
        {
            for (const auto& directConnectedNodeIndex : connect_node.direct_connections)
            {
                if (directConnectedNodeIndex != -1 && directConnectedNodeIndex != root_node_index)
                {
                    if (node.connected_nodes.size() < MaxIndirectConnections)
                    {
                        node.connected_nodes.emplace(directConnectedNodeIndex);
                    }
                }
            }
        }
    };

    node.connected_nodes.clear();
    // add direct nodes
    for (const auto& directConnectedNodeIndex : node.direct_connections)
    {
        if (directConnectedNodeIndex != -1)
        {
            node.connected_nodes.emplace(directConnectedNodeIndex);
        }
    }
    // add indirect node (connection)
    if (node.data.type != ConnectorType::DISABLED)
    {
        // check direct connect with the other node
        addConnection(node.index, node);
        /// @TODO: use recursion, for going deeper in the graph
        for (const auto& connectedNodeIndex : node.direct_connections)
        {
            if (node.index != connectedNodeIndex && connectedNodeIndex != -1)
            {
                addConnection(node.index, gameContext.nodes[connectedNodeIndex]);
                for (const auto& innerConnectedNodeIndex1 : gameContext.nodes[connectedNodeIndex].direct_connections)
                {
                    if (node.index != innerConnectedNodeIndex1 && innerConnectedNodeIndex1 != -1)
                    {
                        addConnection(node.index, gameContext.nodes[innerConnectedNodeIndex1]);
                        for (const auto& inner_connected_node_index_2 :
                             gameContext.nodes[innerConnectedNodeIndex1].direct_connections)
                        {
                            if (node.index != inner_connected_node_index_2 && inner_connected_node_index_2 != -1)
                            {
                                addConnection(node.index, gameContext.nodes[inner_connected_node_index_2]);
                                for (const auto& innerConnectedNodeIndex3 :
                                     gameContext.nodes[inner_connected_node_index_2].direct_connections)
                                {
                                    if (node.index != innerConnectedNodeIndex3 && innerConnectedNodeIndex3 != -1)
                                    {
                                        addConnection(node.index, gameContext.nodes[innerConnectedNodeIndex3]);
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
            const auto& connected_node = gameContext.nodes[connected_node_index];
            if (connected_node.data.type == ConnectorType::Action)
            {
                node.connected_actions.push_back(connected_node.data.action);
            }
        }
    }
    updateKeyBinds(gameContext);

    // debug
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
    if (IsKeyDown(KEY_F2))
    {
        TraceLog(LOG_DEBUG, "node %d -> ", node.index);
        for (const auto& connectedIndex : node.connected_nodes)
        {
            TraceLog(LOG_DEBUG, " %d", connectedIndex);
        }
    }
#endif
#endif
}