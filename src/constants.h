#pragma once

#include <raylib.h>
#include <array>
#include <unordered_map>

/// Color Palette
inline constexpr std::array<Color, 8> ColorPalette {
    Color{39, 41, 70, 255},     // Background
    Color{103, 160, 180, 255},  // Key
    Color{231, 255, 238, 255},  // Border
    Color{107, 55, 42, 255},    //
    Color{125, 78, 60, 255},    //
    Color{237, 160, 49, 255},   // Action
    Color{130, 130, 139, 255},  // Grey/disabled
    RAYWHITE,                            // PLACEHOLDER (free)
};
inline constexpr Color NeutralTintColor = WHITE; ///< for rendering texture in full color (art is also restricted to the 8-color rule)


/// Rectangle/Areas
inline constexpr Rectangle ConnectorArea {
    0, 0, 400, 450,
};
inline constexpr Rectangle LevelArea {
    401, 0, 399, 450,
};

inline constexpr Rectangle LevelMapArea {
    425, 35, 11*32, 10*32
};
inline constexpr Rectangle LeftTextArea {
    25, 375, 352, 64,
};
inline constexpr Rectangle RightTextArea {
    425, 375, 11*32, 64,
};

inline constexpr Rectangle StartButtonRect {
    400 / 2 - 8*16/2,
    2*450/3 - 32/2,
    8*16,
    32,
};
inline constexpr Rectangle StartMainCharacterButtonRect {
    RightTextArea.x + 8,
    RightTextArea.y + RightTextArea.height/2 - 32/2,
    8*12,
    32,
};
inline constexpr Rectangle ResetMainCharacterButtonRect {
    RightTextArea.x + 8,
    RightTextArea.y + RightTextArea.height/2 - 32/2,
    8*12,
    32,
};
inline constexpr Rectangle RightHelperTextAreaRect {
    ResetMainCharacterButtonRect.x + ResetMainCharacterButtonRect.width + 6,
    RightTextArea.y + 8,
    RightTextArea.width/2,
    RightTextArea.height,
};


inline constexpr Rectangle HelpInstruction1Area {
    LevelArea.x + 32, 112, 86, 198,
};
inline constexpr Rectangle HelpInstruction2Area {
    LevelArea.x + LevelArea.width - 86 - 32, 112, 86, 198,
};

/// Font size and style (colors)
inline constexpr int TextFontSize = 14;
inline constexpr int HelperTextFontSize = 14;
inline constexpr int LevelHelperTextFontSize = 16;
inline constexpr auto TextFontColor = ColorPalette[2];
inline constexpr auto BackgroundColor = ColorPalette[0];
inline constexpr auto BorderColor = ColorPalette[2];
inline constexpr int BorderLineThick = 1;
inline constexpr auto ButtonColor = ColorPalette[2];
inline constexpr auto ButtonHoverColor = ColorPalette[1];
inline constexpr int ButtonLineThick = 1;
//// Start
inline constexpr int TitleTextFontSize = 32;
inline constexpr int SubTitleTextFontSize = 20;
inline constexpr int WelcomeTextFontSize = 16;
inline constexpr int StartButtonTextFontSize = 18;
inline constexpr int EndTextFontSize = 18;
//// Nodes
inline constexpr int NodeFontSize = 16;
inline constexpr int NodeLineThick = 2;
inline constexpr auto NodeLineColor = ColorPalette[2];
inline constexpr auto DisabledColor = ColorPalette[6];
//// ActionNode
inline constexpr int ActionNodeSides = 6;
inline constexpr int ActionNodeRadius = 18;
inline constexpr int ActionNodeRotation = 0;
inline constexpr auto ActionNodeColor = ColorPalette[5];
//// KeyNode
inline constexpr int KeyNodeRadius = 18;
inline constexpr auto KeyNodeColor = ColorPalette[1];
//// Map
//inline constexpr auto TileMapColor = ColorPalette[2];

/// Level Settings
inline constexpr int LevelTilesWidth = 11;
inline constexpr int LevelTilesHeight = 10;
inline constexpr int LevelTilesetWidth = 32;
inline constexpr int LevelTilesetHeight = 32;
inline constexpr int MaxLevels = 4;
inline constexpr int StartLevel = 1; // for testing
//// Character
inline constexpr int CharacterSpriteWidth = 32;
inline constexpr int CharacterSpriteHeight = 32;
//// Icon
inline constexpr int ActionIconSpriteWidth = 32;
inline constexpr int ActionIconSpriteHeight = 32;

/// Node Settings
inline constexpr int MaxNodeConnections = 2;
inline constexpr int MaxIndirectConnections = 4;
inline constexpr int MaxNodesInLevel = 10;

/// strings
constexpr const char* TitleText = "";
constexpr const char* SubTitleText = "";
constexpr const char* WelcomeText = R"(Connect Action- and Key-Nodes on the left side
to bind your keys.
)";
constexpr const char* EndText = R"(Thanks for Playing.

this game was made for
raylib NEXT gamejam 2024









Credits:
  furudbat - Developer
  blacktiger5 - Art, Level Design

Copyright (c) 2024 furudbat
)";
//// Buttons
constexpr const char* WelcomeStartButtonText = "START";
constexpr const char* GoButtonText = "GO";
constexpr const char* RestartButtonText = "RESET";
constexpr const char* EndStartButtonText = "RESTART";
//// Helper Text
constexpr const char* LeftHelperTextConnectionsFormat = "Max. Connections: %02d/%02d\nMax. Actions: %d";
constexpr const char* LeftHelperNoConnectionsTextFormat = "Connect Actions and Key-Nodes, before GO.";
constexpr const char* LeftHelperCharacterTextFormat = "Move your Character and reach the door.";
constexpr const char* RightHelperTextNoKeyBindsFormat = "No Key-Binds";
constexpr const char* LevelsHelperFormat = "Level: %d";
///// Key (enum strings)
constexpr const char* ConnectorKeyWString = "W";
constexpr const char* ConnectorKeyAString = "A";
constexpr const char* ConnectorKeySString = "S";
constexpr const char* ConnectorKeyDString = "D";
constexpr const char* ConnectorKeySpaceString = "SPACE";
///// Key (button strings)
constexpr const char* NodeKeyWString = "W";
constexpr const char* NodeKeyAString = "A";
constexpr const char* NodeKeySString = "S";
constexpr const char* NodeKeyDString = "D";
constexpr const char* NodeKeySpaceString = "_";
///// Action (enum string)
constexpr const char* ConnectorActionMovementLeftString = "Left";
constexpr const char* ConnectorActionMovementRightString = "Right";
constexpr const char* ConnectorActionMovementUpString = "Up";
constexpr const char* ConnectorActionMovementDownString = "Down";
constexpr const char* ConnectorActionJumpString = "Jump";
///// Action (button string)
constexpr const char* NodeActionMovementLeftString = "L";
constexpr const char* NodeActionMovementRightString = "R";
constexpr const char* NodeActionMovementUpString = "UP";
constexpr const char* NodeActionMovementDownString = "DW";
constexpr const char* NodeActionJumpString = "JP";
