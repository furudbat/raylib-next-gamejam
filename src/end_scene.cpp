#include "game.h"
#include "constants.h"
#include "types.h"
#include <raylib.h>

void UpdateEndScene(GameContext& gameContext)
{
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(StartButtonRect, gameContext.mouse) || IsKeyPressed(KEY_ENTER)) {
    // restart
    gameContext.playerCurrentKey = ConnectorKey::NONE;
    gameContext.playerActionIndex = -1;
    gameContext.playerTilesPosition = gameContext.playerStartTilesPosition;
    gameContext.playerDirection = gameContext.playerStartDirection;
    gameContext.state = GameState::Start;
    SetLevel(gameContext, StartLevel);
  }
}

void RenderEndScene(GameContext& gameContext)
{
  // title
  DrawTexture(gameContext.logoTexture, ConnectorArea.x + ConnectorArea.width/2 - gameContext.logoTexture.width/2, ConnectorArea.y + 72, NeutralTintColor);

  // show welcome text
  const auto endTextSize = MeasureTextEx(gameContext.font, EndText, EndTextFontSize, EndTextFontSize/FontSpacingFactor);
  DrawTextEx(gameContext.font, EndText, {LevelArea.x + LevelArea.width/2 - endTextSize.x/2, LevelArea.y + 64}, EndTextFontSize, EndTextFontSize/FontSpacingFactor, TextFontColor);

  const auto startButtonTextSize = MeasureTextEx(gameContext.font, EndStartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/FontSpacingFactor);
  const auto startButtonColor = (CheckCollisionRecs(StartButtonRect, gameContext.mouse)) ? ButtonHoverColor : ButtonColor;
  DrawRectangleLinesEx(StartButtonRect, ButtonLineThick, startButtonColor);
  DrawTextEx(gameContext.font, EndStartButtonText, {StartButtonRect.x + StartButtonRect.width/2 - startButtonTextSize.x/2, StartButtonRect.y + StartButtonRect.height/2 - startButtonTextSize.y/2}, StartButtonTextFontSize, StartButtonTextFontSize/FontSpacingFactor, startButtonColor);

  DrawTexture(gameContext.endTexture, LevelArea.x + LevelArea.width/2 - EndSpriteWidth/2 - 25, LevelArea.y + LevelArea.height/2 - EndSpriteHeight/2 + 15, NeutralTintColor);
}