#include "ui/DesignTokens.h"

#include <raylib.h>

namespace {

Font loadFontOrDefault(const std::string& path, int size, bool& owned) {
    if (FileExists(path.c_str())) {
        owned = true;
        return LoadFontEx(path.c_str(), size, nullptr, 0);
    }

    owned = false;
    return GetFontDefault();
}

} // namespace

DesignTokens CreateDesignTokens() {
    DesignTokens tokens{};

    tokens.colors.appBackground = {0x0b, 0x10, 0x14, 0xff};
    tokens.colors.canvasBackground = {0x0d, 0x17, 0x20, 0xff};
    tokens.colors.panelBackground = {0x13, 0x1e, 0x28, 0xff};
    tokens.colors.panelElevated = {0x1b, 0x28, 0x33, 0xff};
    tokens.colors.panelBorder = {0x2a, 0x3d, 0x4d, 0xff};
    tokens.colors.textPrimary = {0xeb, 0xf2, 0xf8, 0xff};
    tokens.colors.textMuted = {0xa8, 0xb8, 0xc8, 0xff};
    tokens.colors.accentPrimary = {0x36, 0xc4, 0xff, 0xff};
    tokens.colors.accentWarning = {0xff, 0x87, 0x43, 0xff};
    tokens.colors.accentSelection = {0xff, 0xb2, 0x47, 0xff};
    tokens.colors.gateFill = {0x1d, 0x2a, 0x36, 0xff};
    tokens.colors.gateStroke = {0x84, 0x9a, 0xad, 0xff};
    tokens.colors.gateAccentAnd = {0x42, 0xd2, 0xff, 0xff};
    tokens.colors.gateAccentOr = {0x5a, 0xd6, 0xb0, 0xff};
    tokens.colors.gateAccentXor = {0xff, 0xa4, 0x5d, 0xff};
    tokens.colors.gateAccentNot = {0x98, 0xbe, 0xff, 0xff};
    tokens.colors.ghostFill = {0x7c, 0x96, 0xaa, 0x58};
    tokens.colors.ghostStroke = {0x9d, 0xb9, 0xcf, 0xe0};
    tokens.colors.wireOff = {0x66, 0x7d, 0x8f, 0xff};
    tokens.colors.wireOn = tokens.colors.accentPrimary;
    tokens.colors.wireHover = {0x7f, 0xd6, 0xff, 0xff};
    tokens.colors.wireSelection = tokens.colors.accentSelection;
    tokens.colors.pinOn = tokens.colors.accentPrimary;
    tokens.colors.pinOff = {0x87, 0x98, 0xa7, 0xff};
    tokens.colors.gridMajor = {0x2b, 0x43, 0x52, 0x78};
    tokens.colors.gridMinor = {0x22, 0x34, 0x41, 0x4a};

    tokens.metrics.leftPanelWidth = 230.0f;
    tokens.metrics.rightPanelWidth = 280.0f;
    tokens.metrics.topBarHeight = 48.0f;
    tokens.metrics.bottomBarHeight = 30.0f;
    tokens.metrics.panelPadding = 10.0f;
    tokens.metrics.panelRadius = 0.18f;
    tokens.metrics.gateCornerRadius = 0.2f;
    tokens.metrics.strokeWidth = 2.0f;
    tokens.metrics.pinRadius = 5.0f;
    tokens.metrics.gridSize = 25.0f;
    tokens.metrics.zoomMin = 0.45f;
    tokens.metrics.zoomMax = 2.6f;

    tokens.typography.ui = GetFontDefault();
    tokens.typography.mono = GetFontDefault();
    tokens.typography.ownsUi = false;
    tokens.typography.ownsMono = false;
    tokens.typography.titleSize = 20.0f;
    tokens.typography.bodySize = 16.0f;
    tokens.typography.smallSize = 13.0f;

    return tokens;
}

void LoadDesignFonts(DesignTokens& tokens, const std::string& assetRoot) {
    const std::string spaceGrotesk = assetRoot + "/fonts/SpaceGrotesk-Medium.ttf";
    const std::string interTight = assetRoot + "/fonts/InterTight-Regular.ttf";
    const std::string plex = assetRoot + "/fonts/IBMPlexSans-Regular.ttf";
    const std::string mono = assetRoot + "/fonts/JetBrainsMono-Regular.ttf";

    bool ownedUi = false;
    Font ui = loadFontOrDefault(spaceGrotesk, 64, ownedUi);
    if (!ownedUi) {
        ui = loadFontOrDefault(interTight, 64, ownedUi);
    }
    if (!ownedUi) {
        ui = loadFontOrDefault(plex, 64, ownedUi);
    }

    bool ownedMono = false;
    Font monoFont = loadFontOrDefault(mono, 56, ownedMono);

    tokens.typography.ui = ui;
    tokens.typography.mono = monoFont;
    tokens.typography.ownsUi = ownedUi;
    tokens.typography.ownsMono = ownedMono;
}

void UnloadDesignFonts(DesignTokens& tokens) {
    if (tokens.typography.ownsUi) {
        UnloadFont(tokens.typography.ui);
    }
    if (tokens.typography.ownsMono) {
        UnloadFont(tokens.typography.mono);
    }

    tokens.typography.ui = GetFontDefault();
    tokens.typography.mono = GetFontDefault();
    tokens.typography.ownsUi = false;
    tokens.typography.ownsMono = false;
}
