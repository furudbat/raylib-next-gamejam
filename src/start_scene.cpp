#include "game.h"
#include <raylib.h>

void UpdateStartScene(GameContext& gameContext)
{
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionRecs(StartButtonRect, gameContext.mouse) || IsKeyPressed(KEY_ENTER)) {
    SetLevel(gameContext, StartLevel);
  }
}

void RenderStartScene(GameContext& gameContext)
{
  // title
  DrawTexture(gameContext.logoTexture, ConnectorArea.x + ConnectorArea.width/2 - gameContext.logoTexture.width/2, ConnectorArea.y + 72, NeutralTintColor);

  const auto startButtonTextSize = MeasureTextEx(gameContext.font, WelcomeStartButtonText, StartButtonTextFontSize, StartButtonTextFontSize/10);
  const auto startButtonColor = (CheckCollisionRecs(StartButtonRect, gameContext.mouse)) ? ButtonHoverColor : ButtonColor;
  DrawRectangleLinesEx(StartButtonRect, ButtonLineThick, startButtonColor);
  DrawTextEx(gameContext.font, WelcomeStartButtonText, {StartButtonRect.x + StartButtonRect.width/2 - startButtonTextSize.x/2, StartButtonRect.y + StartButtonRect.height/2 - startButtonTextSize.y/2}, StartButtonTextFontSize, StartButtonTextFontSize/10, startButtonColor);

  // footer
  const auto footerTextSize = MeasureTextEx(gameContext.font, WelcomeFooterText, WelcomeFooterTextFontSize, WelcomeFooterTextFontSize/10);
  DrawTextEx(gameContext.font, WelcomeFooterText, {WelcomeFooterTextArea.x + WelcomeFooterTextArea.width/2 - footerTextSize.x/2, WelcomeFooterTextArea.y}, WelcomeFooterTextFontSize, WelcomeFooterTextFontSize/10, TextFontColor);

  // show welcome text
  const auto welcomeTextSize = MeasureTextEx(gameContext.font, WelcomeText, WelcomeTextFontSize, WelcomeTextFontSize/10);
  DrawTextEx(gameContext.font, WelcomeText, {WelcomeTextArea.x + LevelArea.width/2 - welcomeTextSize.x/2, WelcomeTextArea.y}, WelcomeTextFontSize, WelcomeTextFontSize/10, TextFontColor);

  DrawTexture(gameContext.instruction1Texture, HelpInstruction1Area.x, HelpInstruction1Area.y, NeutralTintColor);
  DrawTexture(gameContext.instruction2Texture, HelpInstruction2Area.x, HelpInstruction2Area.y, NeutralTintColor);
}