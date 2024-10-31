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
    Color{200, 200, 200, 255},  // light grey
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
inline constexpr Rectangle GoButtonRect {
    RightTextArea.x + 8,
    RightTextArea.y + RightTextArea.height/2 - 50/2,
    8*12,
    50,
};
inline constexpr Rectangle ResetButtonRect = GoButtonRect;

inline constexpr Rectangle RightHelperTextAreaRect {
    ResetButtonRect.x + ResetButtonRect.width + 6,
    RightTextArea.y + 10,
    RightTextArea.width/2,
    RightTextArea.height,
};


inline constexpr Rectangle WelcomeTextArea {
    LevelArea.x,
    LevelArea.y + 38, LevelArea.width, LevelArea.height/5,
};
inline constexpr Rectangle WelcomeFooterTextArea {
    ConnectorArea.x,
    ConnectorArea.y + ConnectorArea.height - 32, ConnectorArea.width, ConnectorArea.height/10,
};

inline constexpr Rectangle HelpInstruction1Area {
    LevelArea.x + 43,
    138, 86, 198,
};
inline constexpr Rectangle HelpInstruction2Area {
    LevelArea.x + LevelArea.width - 86 - 43,
    138, 86, 198,
};
inline constexpr Rectangle InGameHelpInstruction1Area {
    LevelArea.x + 43,
    56, 86, 198,
};
inline constexpr Rectangle InGameHelpInstruction2Area {
    LevelArea.x + LevelArea.width - 86 - 43,
    56, 86, 295,
};

// help
inline constexpr Rectangle Help1IconArea {
    LevelArea.x + LevelArea.width - 9*2 - 24, 10, 9*2, 9*2,
};
// guidelines
inline constexpr Rectangle Help2IconArea {
    Help1IconArea.x - 8 - 9*2, 10, 9*2, 9*2,
};
// guidelines label
inline constexpr Rectangle Help3Area {
    Help2IconArea.x - 8 - 9*2 - 24, 10, 24, 18,
};
// Tips
inline constexpr Rectangle Help4Area {
    InGameHelpInstruction1Area.x + 3,
    InGameHelpInstruction1Area.y + 198 + 4, 86 + 2 + 134, 198/3 + 22,
};
// Tips
inline constexpr Rectangle Help5Area {
    InGameHelpInstruction1Area.x + 4 + InGameHelpInstruction1Area.width,
    InGameHelpInstruction1Area.y + 3, 134, 192,
};

/// Font size and style (colors)
inline constexpr int TextFontSize = 14;
inline constexpr int HelperTextFontSize = 14;
inline constexpr int SmallHelperTextFontSize = 10;
inline constexpr int LevelHelperTextFontSize = 16;
inline constexpr auto TextFontColor = ColorPalette[2];
inline constexpr auto BackgroundColor = ColorPalette[0];
inline constexpr auto BorderColor = ColorPalette[2];
inline constexpr int WindowBorderLineThick = 1;
inline constexpr int BorderLineThick = 2;
inline constexpr auto ButtonColor = ColorPalette[2];
inline constexpr auto ButtonHoverColor = ColorPalette[1];
inline constexpr auto ButtonActiveColor = ColorPalette[5];
inline constexpr int ButtonLineThick = 4;
//// Start
inline constexpr int TitleTextFontSize = 32;
inline constexpr int SubTitleTextFontSize = 20;
inline constexpr int WelcomeTextFontSize = 14;
inline constexpr int StartButtonTextFontSize = 18;
inline constexpr int EndTextFontSize = 16;
inline constexpr int WelcomeFooterTextFontSize = 14;
//// Nodes
inline constexpr int NodeFontSize = 16;
inline constexpr int NodeLineThick = 2;
inline constexpr auto NodeLineColor = ColorPalette[2];
inline constexpr auto DisabledColor = ColorPalette[6];
//// ActionNode
inline constexpr int ActionNodeSides = 6;
inline constexpr int ActionNodeRadius = 18;
inline constexpr int ActionNodeRadiusThick = 2;
inline constexpr auto ActionNodeColor = ColorPalette[5];
//// KeyNode
inline constexpr int KeyNodeRadius = 18;
inline constexpr int KeyNodeRadiusThick = 2;
inline constexpr auto KeyNodeColor = ColorPalette[1];
//// Map
inline constexpr int HelpIconFontSize = 12;
inline constexpr int HelpIconRadius = 9;
inline constexpr int PreviewLineThick = 2;
inline constexpr auto PreviewLineColor = ColorPalette[7];
inline constexpr int PreviewTextFontSize = 12;
inline constexpr auto MapActiveBorderColor = ColorPalette[5];

inline constexpr float PlayerOnVoidTileScale = 0.8f;
inline constexpr std::chrono::milliseconds TurnCooldown{15*16};

/// Level Settings
inline constexpr int LevelMapWidth = 11;
inline constexpr int LevelMapHeight = 10;
inline constexpr int LevelTileWidth = 32;
inline constexpr int LevelTileHeight = 32;
inline constexpr int MaxLevels = 5;
inline constexpr int StartLevel = 1; // for testing
inline constexpr int JumpFactor = 2; // factor * tile size
//// Character
inline constexpr int CharacterSpriteWidth = 32;
inline constexpr int CharacterSpriteHeight = 32;
inline constexpr int EndSpriteWidth = 128;
inline constexpr int EndSpriteHeight = 128;
//// Icon
inline constexpr int ActionIconSpriteWidth = 32;
inline constexpr int ActionIconSpriteHeight = 32;
inline constexpr int ControlIconSpriteWidth = 10;
inline constexpr int ControlIconSpriteHeight = 10;

/// Node Settings
inline constexpr int MaxNodeConnections = 2;
inline constexpr int MaxIndirectConnections = 4;
inline constexpr int MaxNodesInLevel = 10;

/// strings
constexpr const char* TitleText = "";
constexpr const char* SubTitleText = "";
constexpr const char* WelcomeFooterText = "Copyright (c) 2024 furudbat";
constexpr const char* WelcomeText = R"(Connect Action- and Key-Neurons on
the left side to bind your keys.

Left-Click: select and link Neurons
Right-Click: delete connections
Rules:
)";
constexpr const char* EndText = R"(Thanks for Playing.

this game was made for
raylib NEXT gamejam 2024











Credits:
 furudbat - Programming
 blacktiger5 - Art, Level Design

Copyright (c) 2024 furudbat
)";
//// Buttons
inline constexpr const char* WelcomeStartButtonText = "START";
inline constexpr const char* GoButtonText = "GO";
inline constexpr const char* RestartButtonText = "RESET";
inline constexpr const char* EndStartButtonText = "RESTART";
//// Helper Text
inline constexpr const char* LeftHelperTextConnectionsFormat = R"(Max. Connections: %02d/%02d
Max. Chain Length: %d
Click on GO when you are ready.
)";
inline constexpr const char* LeftHelperNoConnectionsTextFormat = "Connect Actions and Key-Neurons,\nthen press GO.";
inline constexpr const char* LeftHelperCharacterTextFormat = "Move your Character and \nreach the door.";
inline constexpr const char* RightHelperTextNoKeyBindsFormat = "No Key-Binds";
inline constexpr const char* LevelsHelperFormat = "Level: %d";
///// Key (enum strings)
inline constexpr const char* ConnectorKeyHString = "H";
inline constexpr const char* ConnectorKeyJString = "J";
inline constexpr const char* ConnectorKeyKString = "K";
inline constexpr const char* ConnectorKeyLString = "L";
inline constexpr const char* ConnectorKeyBString = "B";
inline constexpr const char* ConnectorKeyGString = "G";
///// Key (button strings)
inline constexpr const char* NodeKeyHString = "H";
inline constexpr const char* NodeKeyJString = "J";
inline constexpr const char* NodeKeyKString = "K";
inline constexpr const char* NodeKeyLString = "L";
inline constexpr const char* NodeKeyBString = "B";
inline constexpr const char* NodeKeyGString = "G";
///// Action (enum string)
inline constexpr const char* ConnectorActionMovementLeftString = "Left";
inline constexpr const char* ConnectorActionMovementRightString = "Right";
inline constexpr const char* ConnectorActionMovementUpString = "Up";
inline constexpr const char* ConnectorActionMovementDownString = "Down";
inline constexpr const char* ConnectorActionJumpString = "Jump";
///// Action (button string)
inline constexpr const char* NodeActionMovementLeftString = "L";
inline constexpr const char* NodeActionMovementRightString = "R";
inline constexpr const char* NodeActionMovementUpString = "UP";
inline constexpr const char* NodeActionMovementDownString = "DW";
inline constexpr const char* NodeActionJumpString = "JP";
///// misc
inline constexpr const char* Help1IconString = "?";
inline constexpr const char* Help2IconString = "L";
inline constexpr const char* Help3TipString = "Guide->";
inline constexpr const char* Help5TipString = R"(Hints:

You jump in the
direction you are
facing.

You can turn ON
Guidelines when
connections exists

Watch your
"Max. Connections"
and "Chain Length"
)";
inline constexpr const char* Help4TipString = R"(Controls:

  on Neurons to connect
  on Neurons to disconnect
  quick start the level
  quick reset the Level
)";