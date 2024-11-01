#pragma once

#include "constants.h"
#include "types.h"
#include <raylib.h>
#include <chrono>
#include <string>
#include <unordered_map>

struct GameContext
{
    // textures
    Font font{};
    Texture2D logoTexture{};
    Texture2D instruction1Texture{};
    Texture2D instruction2Texture{};
    Texture2D tilesetTexture{};
    Texture2D endTexture{};
    Texture2D characterSpriteSheetTexture{};
    Texture2D iconsSpriteSheetTexture{};
    Texture2D iconsControlSpriteSheetTexture{};
    std::chrono::milliseconds delta{std::chrono::milliseconds::zero()};

    // scene data
    Rectangle mouse{0, 0, 0, 0};
    GameState state{GameState::Start};

    // level/player data
    std::chrono::milliseconds timer{std::chrono::milliseconds::zero()};
    std::chrono::milliseconds startTime{std::chrono::milliseconds::zero()};
    //// level data
    GameLevelNodes nodes{};
    const Level_t* mapData{nullptr};
    int level{0};
    int levelMaxNodeConnections{0};
    int levelMaxActionsPerKey{0};
    int levelConnections{0};
    //// player data
    std::chrono::milliseconds turnCooldown{std::chrono::milliseconds::zero()};
    Vector2 playerStartTilesPosition{0, 0};
    Vector2 playerTilesPosition{0, 0};
    CharacterDirection playerStartDirection{CharacterDirection::Right};
    CharacterDirection playerDirection{CharacterDirection::Right};
    ConnectorKey playerCurrentKey{ConnectorKey::NONE};
    int playerActionIndex{-1};
    int deathCount{0};
    bool playerOnVoidTile{false};
    bool playerOnDoorTile{false};
    bool showHelp1{false};
    bool showHelp2{false};
    bool manuelHelp{false};


    // computed
    std::unordered_map<ConnectorKey, std::vector<ConnectorAction>> keyBinds;
    std::string leftHelperText;
    std::string levelHelperText;
    std::string rightHelperText;
    bool nodeSelectionMode{false};

    GameContext()
    {
        static_assert(MaxNodeConnections > 0);
        keyBinds.reserve(5 * MaxNodeConnections);
        leftHelperText.reserve(24 * 4);
        levelHelperText.reserve(24);
        rightHelperText.reserve(24 * 4);
    }
};

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
inline static constexpr int ScreenWidth = 800;
inline static constexpr int ScreenHeight = 450;

extern void UpdateAllNodes(GameContext& gameContext);
extern void SetLevel(GameContext& gameContext, int level);
extern void NextLevel(GameContext& gameContext);

// start_scene.cpp
extern void UpdateStartScene(GameContext& gameContext);
extern void RenderStartScene(GameContext& gameContext);

// main_scene.cpp
extern void UpdateMainSceneNodes(GameContext& gameContext);
extern void UpdateMainSceneMap(GameContext& gameContext);
extern void RenderMainScene(GameContext& gameContext);

// end_scene.cpp
extern void UpdateEndScene(GameContext& gameContext);
extern void RenderEndScene(GameContext& gameContext);


/// utils
namespace utils
{
inline static constexpr const char* WHITESPACE = " \n\r\t\f\v";
inline std::string ltrim(std::string_view s)
{
    const auto start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : std::string{s.substr(start)};
}
inline std::string rtrim(std::string_view s)
{
    const auto end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : std::string{s.substr(0, end + 1)};
}
inline std::string trim(std::string_view s)
{
    return rtrim(ltrim(s));
}
} // namespace utils