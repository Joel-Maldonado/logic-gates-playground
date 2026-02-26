#ifndef UI_DESIGN_TOKENS_H
#define UI_DESIGN_TOKENS_H

#include <raylib.h>
#include <string>

struct DesignTokens {
    struct ColorSet {
        Color appBackground;
        Color canvasBackground;
        Color panelBackground;
        Color panelElevated;
        Color panelBorder;
        Color textPrimary;
        Color textMuted;
        Color accentPrimary;
        Color accentWarning;
        Color accentSelection;
        Color gateFill;
        Color gateStroke;
        Color gateAccentAnd;
        Color gateAccentOr;
        Color gateAccentXor;
        Color gateAccentNot;
        Color ghostFill;
        Color ghostStroke;
        Color wireOff;
        Color wireOn;
        Color wireHover;
        Color wireSelection;
        Color pinOn;
        Color pinOff;
        Color gridMajor;
        Color gridMinor;
    } colors;

    struct Metrics {
        float leftPanelWidth;
        float rightPanelWidth;
        float topBarHeight;
        float bottomBarHeight;
        float panelPadding;
        float panelRadius;
        float gateCornerRadius;
        float strokeWidth;
        float pinRadius;
        float gridSize;
        float zoomMin;
        float zoomMax;
    } metrics;

    struct Typography {
        Font ui;
        Font mono;
        bool ownsUi;
        bool ownsMono;
        float titleSize;
        float bodySize;
        float smallSize;
    } typography;
};

DesignTokens CreateDesignTokens();
void LoadDesignFonts(DesignTokens& tokens, const std::string& assetRoot);
void UnloadDesignFonts(DesignTokens& tokens);

#endif // UI_DESIGN_TOKENS_H
