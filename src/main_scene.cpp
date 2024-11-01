#include "game.h"
#include "constants.h"
#include "types.h"
#include <chrono>
#include <raylib.h>
#include <raymath.h>


// pre define internal functions
[[nodiscard]] static bool validPreConnections(GameContext& gameContext, ConnectorNode* clicked_node, ConnectorNode* other_node);
[[nodiscard]] static bool validConnection(GameContext& gameContext, int node_selected1, int node_selected2);
[[nodiscard]] static bool validPostConnections(GameContext& gameContext, int node_selected1, int node_selected2);
static bool linkNodes(GameContext& gameContext, int node_selected1, int node_selected2);
static void unlinkNode(GameContext& gameContext, ConnectorNode& node);

void UpdateMainSceneNodes(GameContext& gameContext)
{
    // update timer
    using fsec = std::chrono::duration<float>;
    const auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(fsec{GetTime()});
    gameContext.timer = endTime - gameContext.startTime;

    // Connector Area
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const auto mouse = GetMousePosition();
        ConnectorNode* nodeClicked = nullptr;
        ConnectorNode* otherNode = nullptr;
        for (auto& node : gameContext.nodes)
        {
            if (node.data.type != ConnectorType::DISABLED)
            {
                const auto radius = [&]()
                {
                    switch (node.data.type)
                    {
                    case ConnectorType::DISABLED:
                        break;
                    case ConnectorType::Action:
                        return ActionNodeRadius;
                    case ConnectorType::Key:
                        return KeyNodeRadius;
                    }
                    return 0;
                }();
                if(CheckCollisionCircleRec(node.data.position, radius, gameContext.mouse))
                {
                    node.is_selected = true;
                    nodeClicked = &node;
                }
            }
        }
        int node_selected1 = -1;
        int node_selected2 = -1;
        for (size_t i = 0; i < gameContext.nodes.size(); ++i)
        {
            if (gameContext.nodes[i].is_selected)
            {
                node_selected1 = i;
                if (nodeClicked != &gameContext.nodes[i])
                {
                    otherNode = &gameContext.nodes[i];
                }
                break;
            }
        }
        for (size_t i = 0; i < gameContext.nodes.size(); ++i)
        {
            if (i != node_selected1 && gameContext.nodes[i].is_selected)
            {
                node_selected2 = i;
                if (nodeClicked != &gameContext.nodes[i])
                {
                    otherNode = &gameContext.nodes[i];
                }
                break;
            }
        }
        gameContext.nodeSelectionMode = node_selected1 != -1 || node_selected2 != -1;
        if (node_selected1 != -1 && node_selected2 != -1)
        {

            UpdateAllNodes(gameContext);
            if (validPreConnections(gameContext, otherNode, nodeClicked))
            {
                const auto rollback_nodes = gameContext.nodes;
                linkNodes(gameContext, node_selected1, node_selected2);
                UpdateAllNodes(gameContext);
                // check after constrains
                if (!validPostConnections(gameContext, node_selected1, node_selected2))
                {
                    // rollback to old state
                    gameContext.nodes = rollback_nodes;
                }
            }
            gameContext.nodes[node_selected1].is_selected = false;
            gameContext.nodes[node_selected2].is_selected = false;
            gameContext.nodeSelectionMode = false;
        }
        UpdateAllNodes(gameContext);
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        for (size_t i = 0; i < gameContext.nodes.size(); ++i)
        {
            auto& node = gameContext.nodes[i];
            if (node.data.type != ConnectorType::DISABLED)
            {
                if(CheckCollisionCircleRec(node.data.position, 16, gameContext.mouse))
                {
                    unlinkNode(gameContext, node);
                }
            }
        }

        // deselect all nodes
        for (auto& node : gameContext.nodes)
        {
            node.is_selected = false;
        }
        gameContext.nodeSelectionMode = false;

        UpdateAllNodes(gameContext);
    }

    if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(GoButtonRect, gameContext.mouse)) ||
        IsKeyPressed(KEY_ENTER)) {
        gameContext.state = GameState::CharacterMain;
        gameContext.leftHelperText = TextFormat(LeftHelperCharacterTextFormat);
        return;
    }
}

// pre define internal functions
static void playerReset(GameContext& gameContext);
static void playerDie(GameContext& gameContext);

void UpdateMainSceneMap(GameContext& gameContext)
{
    // Update key binds (pressed)
    if (!gameContext.playerOnVoidTile)
    {
        for (const auto& [key, actions]: gameContext.keyBinds)
        {
            if (gameContext.playerActionIndex == -1 && IsKeyPressed(static_cast<int>(key)))
            {
                gameContext.playerCurrentKey = key;
                gameContext.playerActionIndex = 0;
                gameContext.turnCooldown = std::chrono::milliseconds::zero();
                break;
            }
        }
    }
    if (gameContext.turnCooldown <= std::chrono::milliseconds::zero())
    {
        if (gameContext.playerCurrentKey != ConnectorKey::NONE && gameContext.playerActionIndex >= 0)
        {
            if (gameContext.playerActionIndex < gameContext.keyBinds[gameContext.playerCurrentKey].size())
            {
                switch(gameContext.keyBinds[gameContext.playerCurrentKey][gameContext.playerActionIndex])
                {
                case ConnectorAction::NONE:
                    break;
                case ConnectorAction::MovementLeft:
                    gameContext.playerTilesPosition.x -= 1;
                    gameContext.playerDirection = CharacterDirection::Left;
                    break;
                case ConnectorAction::MovementRight:
                    gameContext.playerTilesPosition.x += 1;
                    gameContext.playerDirection = CharacterDirection::Right;
                    break;
                case ConnectorAction::MovementUp:
                    gameContext.playerTilesPosition.y -= 1;
                    gameContext.playerDirection = CharacterDirection::Up;
                    break;
                case ConnectorAction::MovementDown:
                    gameContext.playerTilesPosition.y += 1;
                    gameContext.playerDirection = CharacterDirection::Down;
                    break;
                case ConnectorAction::Jump:
                    switch (gameContext.playerDirection)
                    {
                    case CharacterDirection::Right:
                        gameContext.playerTilesPosition.x += JumpFactor;
                        break;
                    case CharacterDirection::Left:
                        gameContext.playerTilesPosition.x -= JumpFactor;
                        break;
                    case CharacterDirection::Up:
                        gameContext.playerTilesPosition.y -= JumpFactor;
                        break;
                    case CharacterDirection::Down:
                        gameContext.playerTilesPosition.y += JumpFactor;
                        break;
                    }
                    break;
                }
            }
            gameContext.playerActionIndex++;
            gameContext.turnCooldown = TurnCooldown;
        }
    }
    else
    {
        gameContext.turnCooldown -= gameContext.delta;
    }

    // check map conditions
    if (gameContext.mapData != nullptr)
    {
        const auto* playerMapTileIndex = [&]() -> const int*
        {
            if (gameContext.mapData != nullptr && (
                gameContext.playerTilesPosition.x >= 0 && gameContext.playerTilesPosition.y >= 0 &&
                gameContext.playerTilesPosition.y < gameContext.mapData->size() && gameContext.playerTilesPosition.x < (*gameContext.mapData)[gameContext.playerTilesPosition.y].size()))
            {
                return &(*gameContext.mapData)[gameContext.playerTilesPosition.y][gameContext.playerTilesPosition.x];
            }
            return nullptr;
        }();
        const auto playerMapTile = (playerMapTileIndex != nullptr) ? static_cast<TileSet>(*playerMapTileIndex) : TileSet::Void1;
        gameContext.playerOnVoidTile = playerMapTile == TileSet::Void1 || playerMapTile == TileSet::Void2;
        gameContext.playerOnDoorTile = playerMapTile == TileSet::Door;

        // reset action (animation)
        if (gameContext.playerCurrentKey != ConnectorKey::NONE &&
            (gameContext.playerActionIndex > gameContext.keyBinds[gameContext.playerCurrentKey].size() ||
             gameContext.keyBinds[gameContext.playerCurrentKey].size() == 1)) // if only have ONE action, no need for waiting the next action
        {
            gameContext.playerCurrentKey = ConnectorKey::NONE;
            gameContext.playerActionIndex = -1;
            gameContext.turnCooldown = std::chrono::milliseconds::zero();

            // wait for death and door
            if (gameContext.playerOnVoidTile || gameContext.playerOnDoorTile)
            {
                gameContext.turnCooldown = TurnCooldown;
            }
        }

        if (gameContext.turnCooldown <= std::chrono::milliseconds::zero())
        {
            switch (playerMapTile)
            {
            case TileSet::Floor:
                break;
            case TileSet::Door:
                NextLevel(gameContext);
                break;
            case TileSet::Key:
                /// @TODO: collect key
                    break;
            case TileSet::Void1:
            case TileSet::Void2:
                playerDie(gameContext);
                break;
            }
        }

        //const auto& px = LevelMapArea.x + game_context.player_tiles_position.x*CharacterSpriteWidth;
        //const auto& py = LevelMapArea.y + game_context.player_tiles_position.y*CharacterSpriteHeight;
        //const Rectangle player_position {px, py, CharacterSpriteWidth, CharacterSpriteHeight};
    }

    // reset button
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(ResetButtonRect, gameContext.mouse)) {
        gameContext.state = GameState::NodesMain;
        playerReset(gameContext);
        UpdateAllNodes(gameContext);
        return;
    }
    if (IsKeyPressed(KEY_BACKSPACE))
    {
        playerReset(gameContext);
        gameContext.state = GameState::CharacterMain;
        gameContext.leftHelperText = TextFormat(LeftHelperCharacterTextFormat);
        return;
    }
}


static void renderNodeLines(GameContext& gameContext, const ConnectorNode& node);
static void renderNode(GameContext& gameContext, const ConnectorNode& node);
static void renderMap(GameContext& gameContext);
void RenderMainScene(GameContext& gameContext) {
    DrawRectangleLinesEx(LeftTextArea, BorderLineThick, BorderColor);
    DrawRectangleLinesEx(RightTextArea, BorderLineThick, BorderColor);

    for (const auto& node : gameContext.nodes)
    {
        renderNodeLines(gameContext, node);
    }
    // pre-view line
    if (gameContext.nodeSelectionMode && CheckCollisionRecs(ConnectorArea, gameContext.mouse))
    {
        const auto mousePos = GetMousePosition();
        for (const auto& node : gameContext.nodes)
        {
            if (node.is_selected)
            {
                DrawLineEx(node.data.position, mousePos, BorderLineThick, DisabledColor);
            }
        }
    }

    for (const auto& node : gameContext.nodes)
    {
        renderNode(gameContext, node);
    }

    // level text
    const auto startButtonTextSize = MeasureTextEx(gameContext.font, gameContext.levelHelperText.c_str(), LevelHelperTextFontSize, LevelHelperTextFontSize/10);
    DrawTextEx(gameContext.font, gameContext.levelHelperText.c_str(), {LevelArea.x + LevelArea.width/2 - startButtonTextSize.x/2, LevelArea.y + 8}, LevelHelperTextFontSize, LevelHelperTextFontSize/10, TextFontColor);

    // helper text (connections, node info)
    DrawTextEx(gameContext.font, gameContext.leftHelperText.c_str(), {LeftTextArea.x + 8, LeftTextArea.y + 8}, HelperTextFontSize, HelperTextFontSize/10, TextFontColor);

    // Main GO Button
    if (gameContext.state == GameState::NodesMain) {
        const auto goButtonTextSize = MeasureTextEx(gameContext.font, GoButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
        const auto buttonColor = (CheckCollisionRecs(GoButtonRect, gameContext.mouse)) ? ButtonHoverColor : ButtonColor;
        DrawRectangleLinesEx(GoButtonRect, ButtonLineThick, buttonColor);
        DrawTextEx(gameContext.font, GoButtonText, {GoButtonRect.x + GoButtonRect.width/2 - goButtonTextSize.x/2, GoButtonRect.y + GoButtonRect.height/2 - goButtonTextSize.y/2}, StartButtonTextFontSize, StartButtonTextFontSize/10, buttonColor);
    }
    // Character Reset Button
    if (gameContext.state == GameState::CharacterMain) {
        const auto restartButtonTextSize = MeasureTextEx(gameContext.font, RestartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
        const auto buttonColor = (CheckCollisionRecs(ResetButtonRect, gameContext.mouse)) ? ButtonHoverColor : ButtonColor;
        DrawRectangleLinesEx(ResetButtonRect, ButtonLineThick, buttonColor);
        DrawTextEx(gameContext.font, RestartButtonText, {ResetButtonRect.x + ResetButtonRect.width/2 - restartButtonTextSize.x/2, ResetButtonRect.y + GoButtonRect.height/2 - restartButtonTextSize.y/2}, StartButtonTextFontSize, StartButtonTextFontSize/10, buttonColor);
    }

    // helper text (actions, key binds)
    DrawTextEx(gameContext.font, gameContext.rightHelperText.c_str(), {RightHelperTextAreaRect.x, RightHelperTextAreaRect.y}, HelperTextFontSize, HelperTextFontSize/10, TextFontColor);

    renderMap(gameContext);

    if (gameContext.showHelp1)
    {
        DrawTexture(gameContext.instruction1Texture, InGameHelpInstruction1Area.x, InGameHelpInstruction1Area.y, NeutralTintColor);
        DrawTexture(gameContext.instruction2Texture, InGameHelpInstruction2Area.x, InGameHelpInstruction2Area.y, NeutralTintColor);

        // guidelines help icon
        const auto help3TextSize = MeasureTextEx(gameContext.font, Help3TipString, SmallHelperTextFontSize, SmallHelperTextFontSize/10);
        DrawTextEx(gameContext.font, Help3TipString, {Help3Area.x, Help3Area.y + Help3Area.height/2 - help3TextSize.y/2}, SmallHelperTextFontSize, SmallHelperTextFontSize/10, TextFontColor);

        // controls
        DrawRectangleRec(Help4Area, BackgroundColor);
        DrawRectangleLinesEx(Help4Area, BorderLineThick, BorderColor);
        DrawTextEx(gameContext.font, Help4TipString, {Help4Area.x + 4, Help4Area.y + 4}, SmallHelperTextFontSize, SmallHelperTextFontSize/10, TextFontColor);
        //// controls icons
        DrawTexturePro(gameContext.iconsControlSpriteSheetTexture, { static_cast<int>(ControlIcons::LMB)*ControlIconSpriteWidth, 0, ControlIconSpriteWidth, ControlIconSpriteHeight },
            {Help4Area.x + 5, Help4Area.y + 4 + 2*SmallHelperTextFontSize + 3, ControlIconSpriteWidth, ControlIconSpriteHeight}, {0, 0}, 0, NeutralTintColor);
        DrawTexturePro(gameContext.iconsControlSpriteSheetTexture, { static_cast<int>(ControlIcons::RMB)*ControlIconSpriteWidth, 0, ControlIconSpriteWidth, ControlIconSpriteHeight },
            {Help4Area.x + 5, Help4Area.y + 4 + 3*SmallHelperTextFontSize + 5, ControlIconSpriteWidth, ControlIconSpriteHeight}, {0, 0}, 0, NeutralTintColor);
        DrawTexturePro(gameContext.iconsControlSpriteSheetTexture, { static_cast<int>(ControlIcons::Enter)*ControlIconSpriteWidth, 0, ControlIconSpriteWidth, ControlIconSpriteHeight },
                {Help4Area.x + 5, Help4Area.y + 4 + 4*SmallHelperTextFontSize + 8, ControlIconSpriteWidth, ControlIconSpriteHeight}, {0, 0}, 0, NeutralTintColor);
        DrawTexturePro(gameContext.iconsControlSpriteSheetTexture, { static_cast<int>(ControlIcons::Backspace)*ControlIconSpriteWidth, 0, ControlIconSpriteWidth, ControlIconSpriteHeight },
                {Help4Area.x + 5, Help4Area.y + 4 + 5*SmallHelperTextFontSize + 10, ControlIconSpriteWidth, ControlIconSpriteHeight}, {0, 0}, 0, NeutralTintColor);


        // Tips
        DrawRectangleRec(Help5Area, BackgroundColor);
        DrawRectangleLinesEx(Help5Area, BorderLineThick, BorderColor);
        DrawTextEx(gameContext.font, Help5TipString, {Help5Area.x + 4, Help5Area.y + 4}, SmallHelperTextFontSize, SmallHelperTextFontSize/10, TextFontColor);
    }

    // help icon 1
    {
        const auto helpButtonTextSize = MeasureTextEx(gameContext.font, Help1IconString, HelpIconFontSize, HelpIconFontSize/10);
        const auto buttonColor = [&]()
        {
            if (gameContext.showHelp1)
            {
                return BackgroundColor;
            }

            return (CheckCollisionRecs(Help1IconArea, gameContext.mouse)) ? ButtonHoverColor : ButtonActiveColor;
        }();
        const Vector2 icon_pos_center {Help1IconArea.x + Help1IconArea.width/2, Help1IconArea.y + Help1IconArea.height/2};
        if (gameContext.showHelp1)
        {
            DrawCircle(icon_pos_center.x, icon_pos_center.y, HelpIconRadius, ButtonActiveColor);
        }
        else
        {
            DrawCircleLines(icon_pos_center.x, icon_pos_center.y, HelpIconRadius, buttonColor);
        }
        DrawTextEx(gameContext.font, Help1IconString, {Help1IconArea.x + Help1IconArea.width/2 - helpButtonTextSize.x/2, Help1IconArea.y + Help1IconArea.height/2 - helpButtonTextSize.y/2}, HelpIconFontSize, HelpIconFontSize/10, buttonColor);
    }
    // help icon 2
    {
        const auto helpButtonTextSize = MeasureTextEx(gameContext.font, Help2IconString, HelpIconFontSize, HelpIconFontSize/10);
        const auto buttonColor = [&]()
        {
            if (gameContext.showHelp2)
            {
                return BackgroundColor;
            }

            return (CheckCollisionRecs(Help2IconArea, gameContext.mouse)) ? ButtonActiveColor : ButtonHoverColor;
        }();
        const Vector2 icon_pos_center {Help2IconArea.x + Help2IconArea.width/2, Help2IconArea.y + Help2IconArea.height/2};
        if (gameContext.showHelp2)
        {
            DrawCircle(icon_pos_center.x, icon_pos_center.y, HelpIconRadius, ButtonActiveColor);
        }
        else
        {
            DrawCircleLines(icon_pos_center.x, icon_pos_center.y, HelpIconRadius, buttonColor);
        }
        DrawTextEx(gameContext.font, Help2IconString, {Help2IconArea.x + Help2IconArea.width/2 - helpButtonTextSize.x/2, Help2IconArea.y + Help2IconArea.height/2 - helpButtonTextSize.y/2}, HelpIconFontSize, HelpIconFontSize/10, buttonColor);
    }
}

//
// internal game logic
//
bool validPreConnections(GameContext& gameContext, ConnectorNode* clicked_node, ConnectorNode* other_node)
{
    if (clicked_node != nullptr && other_node != nullptr)
    {
        Vector2 startPos1 = other_node->data.position;
        Vector2 endPos1 = clicked_node->data.position;
        for (const auto& node : gameContext.nodes)
        {
            if (node.index != other_node->index && node.index != clicked_node->index)
            {
                for (const auto& connectedNodeIndex : node.direct_connections)
                {
                    if (connectedNodeIndex != -1 && connectedNodeIndex != other_node->index && connectedNodeIndex != clicked_node->index)
                    {
                        const auto& connectedNode = gameContext.nodes[connectedNodeIndex];
                        const Vector2 startPos2 = node.data.position;
                        const Vector2 endPos2 = connectedNode.data.position;

                        if (CheckCollisionLines(startPos1, endPos1, startPos2, endPos2, nullptr))
                        {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }
    return false;
}
bool validConnection(GameContext& gameContext, int node_selected1, int node_selected2)
{
    if (node_selected1 != -1 && node_selected2 != -1)
    {
        const auto& node1 = gameContext.nodes[node_selected1];
        const auto& node2 = gameContext.nodes[node_selected2];

        TraceLog(LOG_DEBUG, "Node1: type: %d, connected_counter: %d", node1.data.type, node1.connected_counter);
        TraceLog(LOG_DEBUG, "Node2: type: %d, connected_counter: %d", node2.data.type, node2.connected_counter);
        TraceLog(LOG_DEBUG, "Level: level_connections: %d/%d ", gameContext.levelConnections, gameContext.levelMaxNodeConnections);

        if (node1.data.type != node2.data.type ||
            (node1.data.type == ConnectorType::Action && node2.data.type == ConnectorType::Action))
        {
            if (node1.connected_counter >= MaxNodeConnections)
            {
                return false;
            }
            if (node2.connected_counter >= MaxNodeConnections)
            {
                return false;
            }

            // check nodes in between lines
            for (const auto& otherNode : gameContext.nodes)
            {
                if ((otherNode.index != node1.index && otherNode.index != node2.index) && !otherNode.is_selected)
                {
                    const Vector2 startPos1 = node1.data.position;
                    const Vector2 endPos1 = node2.data.position;

                    const auto otherNodeSize = [&]()
                    {
                        switch (otherNode.data.type)
                        {
                        case ConnectorType::DISABLED:
                            break;
                        case ConnectorType::Action:
                            return ActionNodeRadius;
                        case ConnectorType::Key:
                            return KeyNodeRadius;
                        }
                        return 0;
                    }();
                    const Vector2 topLeft = {otherNode.data.position.x - otherNodeSize/2, otherNode.data.position.y - otherNodeSize/2};
                    const Vector2 topRight = {otherNode.data.position.x + otherNodeSize/2, otherNode.data.position.y - otherNodeSize/2};
                    const Vector2 bottomLeft = {otherNode.data.position.x - otherNodeSize/2, otherNode.data.position.y + otherNodeSize/2};
                    const Vector2 bottomRight = {otherNode.data.position.x + otherNodeSize/2, otherNode.data.position.y + otherNodeSize/2};

                    // check lines with rectangle lines (for both action and key node)
                    if (CheckCollisionLines(startPos1, endPos1, topLeft, topRight, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, topRight, bottomRight, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, bottomRight, bottomLeft, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, bottomLeft, topLeft, nullptr))
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    }

    return false;
}
bool validPostConnections(GameContext& gameContext, int node_selected1, int node_selected2)
{
    // check max connections (per level)
    if (gameContext.levelConnections > gameContext.levelMaxNodeConnections)
    {
        return false;
    }

    // check max actions
    for(const auto& [key, actions] : gameContext.keyBinds)
    {
        if (actions.size() > gameContext.levelMaxActionsPerKey)
        {
            return false;
        }
    }

    constexpr int MaxKeysPerAction = 1;
    constexpr int MaxDirectActionPerKey = 1;
    for (const auto& node : gameContext.nodes)
    {
        // check for already connected key (in action), one key in action (line)
        if (node.data.type == ConnectorType::Action)
        {
            int countKeys = 0;
            for (const auto& connectedNodeIndex : node.connected_nodes)
            {
                const auto& connectedNode = gameContext.nodes[connectedNodeIndex];
                if (connectedNode.data.type == ConnectorType::Key)
                {
                    ++countKeys;
                }
            }
            if (countKeys > MaxKeysPerAction)
            {
                return false;
            }
        }
        // check direct connections
        if (node.data.type == ConnectorType::Key)
        {
            int countActions = 0;
            for (const auto& directConnectedNodeIndex : node.direct_connections)
            {
                if (directConnectedNodeIndex != -1)
                {
                    const auto& directConnectedNode = gameContext.nodes[directConnectedNodeIndex];
                    if (directConnectedNode.data.type == ConnectorType::Action)
                    {
                        ++countActions;
                    }
                }
            }
            if (countActions > MaxDirectActionPerKey)
            {
                return false;
            }
        }
    }

    // check connection crossing with nodes
    for (const auto& node1 : gameContext.nodes)
    {
        for (const auto& node1ConnectedNodeIndex : node1.connected_nodes)
        {
            // skip self and is not selected nodes
            if (node1.index == node1ConnectedNodeIndex || !node1.is_selected)
            {
                continue;
            }

            const auto& node1ConnectedNode = gameContext.nodes[node1ConnectedNodeIndex];
            // check only selected nodes
            if (!node1ConnectedNode.is_selected)
            {
                continue;
            }

            for (const auto& node2 : gameContext.nodes)
            {
                if ((node2.index != node1.index && node2.index != node1ConnectedNodeIndex) && node2.is_selected)
                {
                    const Vector2 startPos1 = node1.data.position;
                    const Vector2 endPos1 = node1ConnectedNode.data.position;

                    const auto node2Size = [&]()
                    {
                        switch (node2.data.type)
                        {
                        case ConnectorType::DISABLED:
                            break;
                        case ConnectorType::Action:
                            return ActionNodeRadius;
                        case ConnectorType::Key:
                            return KeyNodeRadius;
                        }
                        return 0;
                    }();
                    Vector2 topLeft = {node2.data.position.x - node2Size/2, node2.data.position.y - node2Size/2};
                    Vector2 topRight = {node2.data.position.x + node2Size/2, node2.data.position.y - node2Size/2};
                    Vector2 bottomLeft = {node2.data.position.x - node2Size/2, node2.data.position.y + node2Size/2};
                    Vector2 bottomRight = {node2.data.position.x + node2Size/2, node2.data.position.y + node2Size/2};

                    // check lines with rectangle lines (for both action and key node)
                    if (CheckCollisionLines(startPos1, endPos1, topLeft, topRight, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, topRight, bottomRight, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, bottomRight, bottomLeft, nullptr) ||
                        CheckCollisionLines(startPos1, endPos1, bottomLeft, topLeft, nullptr))
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}
bool linkNodes(GameContext& gameContext, int node_selected1, int node_selected2)
{
    /// @TODO: validate connection
    if (validConnection(gameContext, node_selected1, node_selected2))
    {
        for (auto& connectedNodeIndex : gameContext.nodes[node_selected1].direct_connections)
        {
            if (connectedNodeIndex == -1)
            {
                connectedNodeIndex = node_selected2;
                break;
            }
        }
        for (auto& connectedNodeIndex : gameContext.nodes[node_selected2].direct_connections)
        {
            if (connectedNodeIndex == -1)
            {
                connectedNodeIndex = node_selected1;
                break;
            }
        }

        UpdateAllNodes(gameContext);
        return true;
    }

    return false;
}
void unlinkNode(GameContext& gameContext, ConnectorNode& node)
{
    // unlink inner connections
    for (auto& connectedNodeIndex : node.direct_connections)
    {
        if (connectedNodeIndex != -1)
        {
            for (auto& siblingConnectedNodeIndex : gameContext.nodes[connectedNodeIndex].direct_connections)
            {
                if (node.index == siblingConnectedNodeIndex)
                {
                    siblingConnectedNodeIndex = -1;
                }
            }
            node.direct_connections.fill(-1);
        }
    }
    // check for all other nodes
    for (size_t j = 0; j < gameContext.nodes.size(); ++j)
    {
        auto& otherNode = gameContext.nodes[j];
        if (node.index != j && otherNode.data.type != ConnectorType::DISABLED)
        {
            // unlink outer connections
            for (auto& connectedNodeIndex : otherNode.direct_connections)
            {
                if (connectedNodeIndex != -1)
                {
                    for (auto& siblingConnectedNodeIndex : gameContext.nodes[connectedNodeIndex].direct_connections)
                    {
                        if (node.index == siblingConnectedNodeIndex)
                        {
                            siblingConnectedNodeIndex = -1;
                        }
                    }
                    if (connectedNodeIndex == node.index)
                    {
                        connectedNodeIndex = -1;
                    }
                }
            }
        }
    }

    // deselect all nodes
    for (auto& dnode : gameContext.nodes)
    {
        dnode.is_selected = false;
    }
    gameContext.nodeSelectionMode = false;

    UpdateAllNodes(gameContext);
}

void playerReset(GameContext& gameContext)
{
    gameContext.playerCurrentKey = ConnectorKey::NONE;
    gameContext.playerActionIndex = -1;
    gameContext.turnCooldown = std::chrono::milliseconds::zero();
    gameContext.playerTilesPosition = gameContext.playerStartTilesPosition;
    gameContext.playerDirection = gameContext.playerStartDirection;
    //game_context.state = GameState::NodesMain;
    UpdateAllNodes(gameContext);
}
void playerDie(GameContext& gameContext)
{
    playerReset(gameContext);
    gameContext.deathCount++;
    //game_context.state = GameState::NodesMain;
}


//
// internal render logic
//
void renderNodeLines(GameContext& gameContext, const ConnectorNode& node)
{
    auto lineColor = (gameContext.state == GameState::NodesMain) ? NodeLineColor : DisabledColor;
    // render Connections
    for (auto directConnectedNodeIndex : node.direct_connections)
    {
        if (directConnectedNodeIndex != -1)
        {
            const auto& siblingConnected = gameContext.nodes[directConnectedNodeIndex];
            DrawLineEx(node.data.position, siblingConnected.data.position, NodeLineThick, lineColor);
        }
    }
}

void renderNode(GameContext& gameContext, const ConnectorNode& node)
{
    // render Node
    switch (node.data.type)
    {
    case ConnectorType::DISABLED:
        return;
    case ConnectorType::Action:
    {
        auto actionColor = (gameContext.state == GameState::NodesMain) ? ActionNodeColor : DisabledColor;
        const char* innerTextAction = [&]()
        {
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
            if (IsKeyDown(KEY_F1))
            {
                // debug
                return TextFormat("%i", node.index);
            }
#endif
#endif

            switch (node.data.action)
            {
            case ConnectorAction::NONE:
                return "";
            case ConnectorAction::MovementLeft:
                return NodeActionMovementLeftString;
            case ConnectorAction::MovementRight:
                return NodeActionMovementRightString;
            case ConnectorAction::MovementUp:
                return NodeActionMovementUpString;
            case ConnectorAction::MovementDown:
                return NodeActionMovementDownString;
            case ConnectorAction::Jump:
                return NodeActionJumpString;
            }
            return "";
        }();
        const auto innerTextActionSize = MeasureTextEx(gameContext.font, innerTextAction, NodeFontSize, NodeFontSize/10);

        // draw background color on overlapping line
        DrawRing(node.data.position, 0, ActionNodeRadius, 0, 360, ActionNodeSides, BackgroundColor);
        if (node.is_selected)
        {
            DrawRing(node.data.position, 0, ActionNodeRadius, 0, 360, ActionNodeSides, actionColor);
        } else
        {
            DrawRing(node.data.position, ActionNodeRadius, ActionNodeRadius-ActionNodeRadiusThick, 0, 360, ActionNodeSides, actionColor);
        }
        switch (node.data.action)
        {
        case ConnectorAction::NONE:
            return;
        case ConnectorAction::MovementLeft:
        case ConnectorAction::MovementRight:
        case ConnectorAction::MovementUp:
        case ConnectorAction::MovementDown:
        case ConnectorAction::Jump:
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
            if (IsKeyDown(KEY_F1))
            {
                if (node.is_selected)
                {
                    DrawTextEx(gameContext.font, innerTextAction, {node.data.position.x - innerTextActionSize.x/2, node.data.position.y - innerTextActionSize.y/2}, NodeFontSize, NodeFontSize/10, BackgroundColor);
                } else
                {
                    DrawTextEx(gameContext.font,innerTextAction, {node.data.position.x - innerTextActionSize.x/2, node.data.position.y - innerTextActionSize.y/2}, NodeFontSize, NodeFontSize/10, actionColor);
                }
                break;
            }
#endif
#endif
            // Render Action Icon
            auto iconColor = node.is_selected ? BackgroundColor : actionColor;
            DrawTexturePro(gameContext.iconsSpriteSheetTexture,
                { static_cast<float>(static_cast<int>(node.data.action)*ActionIconSpriteWidth), 0, ActionIconSpriteWidth, ActionIconSpriteHeight },
                { node.data.position.x, node.data.position.y, ActionIconSpriteWidth, ActionIconSpriteHeight},
                {ActionIconSpriteWidth/2, ActionIconSpriteHeight/2}, 0, iconColor);
            break;
        }
        break;
    }
    case ConnectorType::Key:
    {
        auto keyColor = (gameContext.state == GameState::NodesMain) ? KeyNodeColor : DisabledColor;
        const char* innerTextKey = [&]()
        {
#if !defined(PLATFORM_WEB)
#ifndef NDEBUG
            if (IsKeyDown(KEY_F1))
            {
                // debug
                return TextFormat("%i", node.index);
            }
#endif
#endif

            switch (node.data.key)
            {
            case ConnectorKey::NONE:
                return "";
            case ConnectorKey::H:
                return NodeKeyHString;
            case ConnectorKey::J:
                return NodeKeyJString;
            case ConnectorKey::K:
                return NodeKeyKString;
            case ConnectorKey::L:
                return NodeKeyLString;
            case ConnectorKey::B:
                return NodeKeyBString;
            case ConnectorKey::G:
                return NodeKeyGString;
            }
            return "";
        }();
        const auto innerTextKeySize = MeasureTextEx(gameContext.font, innerTextKey, NodeFontSize, NodeFontSize/10);

        // draw background color on overlapping line
        DrawRing(node.data.position, 0, KeyNodeRadius, 0, 360, 0, BackgroundColor);
        if (node.is_selected)
        {
            DrawRing(node.data.position, 0, KeyNodeRadius, 0, 360, 0, keyColor);
        } else
        {
            DrawRing(node.data.position, KeyNodeRadius, KeyNodeRadius-KeyNodeRadiusThick, 0, 360, 0, keyColor);
        }
        switch (node.data.key)
        {
        case ConnectorKey::NONE:
            return;
        case ConnectorKey::B:
        case ConnectorKey::H:
        case ConnectorKey::J:
        case ConnectorKey::K:
        case ConnectorKey::L:
        case ConnectorKey::G:
            if (node.is_selected)
            {
                DrawTextEx(gameContext.font, innerTextKey, {node.data.position.x - innerTextKeySize.x/2, node.data.position.y - innerTextKeySize.y/2}, NodeFontSize, NodeFontSize/10, BackgroundColor);
            } else
            {
                DrawTextEx(gameContext.font, innerTextKey, {node.data.position.x - innerTextKeySize.x/2, node.data.position.y - innerTextKeySize.y/2}, NodeFontSize, NodeFontSize/10, keyColor);
            }
            break;
        }
        break;
    }
    }
}
void renderMap(GameContext& gameContext)
{
    // Render Map
    if (gameContext.mapData != nullptr)
    {
        // render level
        for(int y = 0;y < LevelMapHeight; y++)
        {
            for(int x = 0;x < LevelMapWidth; x++)
            {
                const auto& tile = (*gameContext.mapData)[y][x];

                float sx = tile*LevelTileWidth;
                float sy = 0;

                float dx = LevelMapArea.x + x*LevelTileWidth;
                float dy = LevelMapArea.y + y*LevelTileHeight;

                DrawTexturePro(gameContext.tilesetTexture,
                    { sx, sy, LevelTileWidth, LevelTileHeight },
                    { dx, dy, LevelTileWidth, LevelTileHeight},
                    {0, 0}, 0, NeutralTintColor);
            }
        }

        const float character_scale = (gameContext.playerOnVoidTile) ? PlayerOnVoidTileScale : 1.0f;
        const Rectangle character_pos { LevelMapArea.x + gameContext.playerTilesPosition.x * LevelTileWidth, LevelMapArea.y + gameContext.playerTilesPosition.y * LevelTileHeight, CharacterSpriteWidth*character_scale, CharacterSpriteHeight*character_scale};

        // render preview lines
        if (gameContext.showHelp2)
        {
            if (gameContext.playerActionIndex == -1 && gameContext.turnCooldown <= std::chrono::milliseconds::zero())
            {
                for (const auto& [key, actions] : gameContext.keyBinds)
                {
                    if (!actions.empty())
                    {
                        Vector2 tile_position = gameContext.playerTilesPosition;
                        Vector2 startPosLine {character_pos.x + character_pos.width/2, character_pos.y + character_pos.height/2};
                        Vector2 endPosLine {character_pos.x + character_pos.width/2, character_pos.y + character_pos.height/2};
                        auto preview_direction = gameContext.playerDirection;

                        bool preview_on_void_tile = false;
                        const auto isTileVoid = [&](Vector2 tp)
                        {
                            if (gameContext.mapData != nullptr &&
                                tile_position.x >= 0 && tile_position.y >= 0 &&
                                tile_position.y < gameContext.mapData->size() &&
                                tile_position.x < (*gameContext.mapData)[tile_position.y].size())
                            {
                                const auto& tile = static_cast<TileSet>((*gameContext.mapData)[tile_position.y][tile_position.x]);
                                return tile == TileSet::Void1 || tile == TileSet::Void2;
                            }
                            return true;
                        };

                        const auto movePosLineByAction = [&](Vector2& pos, Vector2* tp, auto action){
                            switch(action)
                            {
                            case ConnectorAction::NONE:
                                break;
                            case ConnectorAction::MovementRight:
                                pos.x += LevelTileWidth;
                                if (tp != nullptr) tp->x++;
                                preview_direction = CharacterDirection::Right;
                                return Vector2{1, 0};
                            case ConnectorAction::MovementLeft:
                                pos.x -= LevelTileWidth;
                                if (tp != nullptr) tp->x--;
                                preview_direction = CharacterDirection::Left;
                                return Vector2{-1, 0};
                            case ConnectorAction::MovementDown:
                                pos.y += LevelTileWidth;
                                if (tp != nullptr) tp->y++;
                                preview_direction = CharacterDirection::Down;
                                return Vector2{0, 1};
                            case ConnectorAction::MovementUp:
                                pos.y -= LevelTileWidth;
                                if (tp != nullptr) tp->y--;
                                preview_direction = CharacterDirection::Up;
                                return Vector2{0, .1};
                            case ConnectorAction::Jump:
                                switch (preview_direction)
                                {
                            case CharacterDirection::Right:
                                pos.x += JumpFactor*LevelTileWidth;
                                    if (tp != nullptr) tp->x += JumpFactor;
                                    preview_direction = CharacterDirection::Right;
                                    return Vector2{1, 0};
                            case CharacterDirection::Left:
                                pos.x -= JumpFactor*LevelTileWidth;
                                    if (tp != nullptr) tp->x -= JumpFactor;
                                    preview_direction = CharacterDirection::Left;
                                    return Vector2{-1, 0};
                            case CharacterDirection::Up:
                                pos.y -= JumpFactor*LevelTileWidth;
                                    if (tp != nullptr) tp->y -= JumpFactor;
                                    preview_direction = CharacterDirection::Up;
                                    return Vector2{0, -1};
                            case CharacterDirection::Down:
                                pos.y += JumpFactor*LevelTileWidth;
                                    if (tp != nullptr) tp->y += JumpFactor;
                                    preview_direction = CharacterDirection::Down;
                                    return Vector2{0, 1};
                                }
                                break;
                            }
                            return Vector2{1, 1};
                        };
                        Vector2 direction_vector {1, 1};
                        for (const auto& action : actions)
                        {
                            if (!preview_on_void_tile)
                            {
                                direction_vector = movePosLineByAction(endPosLine, &tile_position, action);
                                if (CheckCollisionPointRec(endPosLine, LevelMapArea))
                                {
                                    DrawLineEx(startPosLine, endPosLine, PreviewLineThick, PreviewLineColor);
                                }
                                direction_vector = movePosLineByAction(startPosLine, nullptr, action);
                            } else
                            {
                                direction_vector = movePosLineByAction(endPosLine, &tile_position, action);
                                // make dotted line
                                const auto max_step = Vector2Distance(startPosLine, endPosLine);
                                auto innerStartPosLine = startPosLine;
                                auto innerEndPosLine = Vector2MoveTowards(startPosLine, endPosLine, 2);
                                for (int step = 0;step < max_step && Vector2Distance(innerEndPosLine, endPosLine) > 0;step += 2*PreviewLineThick)
                                {
                                    if (CheckCollisionPointRec(innerEndPosLine, LevelMapArea))
                                    {
                                        DrawLineEx(innerStartPosLine, innerEndPosLine, PreviewLineThick, PreviewLineColor);
                                    }
                                    innerStartPosLine = Vector2MoveTowards(innerEndPosLine, endPosLine, PreviewLineThick);
                                    innerEndPosLine = Vector2MoveTowards(innerStartPosLine, endPosLine, 2*PreviewLineThick);
                                }
                                if (CheckCollisionPointRec(endPosLine, LevelMapArea))
                                {
                                    DrawLineEx(innerEndPosLine, endPosLine, PreviewLineThick, PreviewLineColor);
                                }
                                direction_vector = movePosLineByAction(startPosLine, nullptr, action);
                            }
                            preview_on_void_tile = preview_on_void_tile || isTileVoid(tile_position);
                        }

                        const auto keyText = [&]()
                        {
                            switch (key)
                            {
                            case ConnectorKey::NONE:
                                break;
                            case ConnectorKey::B:
                                return ConnectorKeyBString;
                            case ConnectorKey::H:
                                return ConnectorKeyHString;
                            case ConnectorKey::J:
                                return ConnectorKeyJString;
                            case ConnectorKey::K:
                                return ConnectorKeyKString;
                            case ConnectorKey::L:
                                return ConnectorKeyLString;
                            case ConnectorKey::G:
                                return ConnectorKeyGString;
                            }
                            return "";
                        }();
                        auto keyTextSize = MeasureTextEx(gameContext.font, keyText, PreviewTextFontSize, PreviewTextFontSize/10);
                        Rectangle keyTextPos {startPosLine.x + direction_vector.x*keyTextSize.x/2 + PreviewLineThick+1, startPosLine.y + direction_vector.y*keyTextSize.y/8 + PreviewLineThick, keyTextSize.x, keyTextSize.y};
                        if (CheckCollisionRecs(keyTextPos, LevelMapArea))
                        {
                            DrawTextEx(gameContext.font, keyText, {keyTextPos.x, keyTextPos.y}, PreviewTextFontSize, PreviewTextFontSize/10, PreviewLineColor);
                        }
                    }
                }
            }
        }

        // Render Character
        // only render when in map bound
        if (CheckCollisionRecs(character_pos, LevelMapArea))
        {
            DrawTexturePro(gameContext.characterSpriteSheetTexture,
                { static_cast<float>(static_cast<int>(gameContext.playerDirection)*CharacterSpriteWidth), 0, CharacterSpriteWidth, CharacterSpriteHeight },
                character_pos,
                {0, 0}, 0, NeutralTintColor);
        }
    }

    // border
    DrawRectangleLinesEx(LevelMapArea, BorderLineThick, (gameContext.state == GameState::CharacterMain) ? MapActiveBorderColor : BorderColor);
}
