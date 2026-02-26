#include "core/GateSymbolGeometry.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float INPUT_PIN_TOP_RATIO = 1.0f / 3.0f;
constexpr float INPUT_PIN_BOTTOM_RATIO = 2.0f / 3.0f;
constexpr float PIN_CENTER_RATIO = 0.5f;

constexpr float AND_WIDTH_RATIO = 0.85f;

constexpr float OR_XOR_WIDTH_RATIO = 0.8f;
constexpr float OR_XOR_MAX_WIDTH_BY_HEIGHT = 0.876f;
constexpr float OR_XOR_CURVE_DEPTH_RATIO = 0.12f;
constexpr float XOR_REAR_CURVE_OFFSET_RATIO = 0.55f;

constexpr float NOT_WIDTH_RATIO = 0.7f;
constexpr float NOT_TRIANGLE_ASPECT_RATIO = 0.866f;
constexpr float NOT_BUBBLE_RADIUS_RATIO = 1.0f / 12.0f;

} // namespace

namespace GateSymbolGeometry {

BodyProfile andProfile(Rectangle bounds) {
    const float actualWidth = bounds.width * AND_WIDTH_RATIO;
    const float leftX = bounds.x + (bounds.width - actualWidth) * 0.5f;
    return {
        leftX,
        leftX + actualWidth,
        bounds.y + bounds.height * 0.5f,
        actualWidth,
        0.0f
    };
}

BodyProfile orXorProfile(Rectangle bounds) {
    const float actualWidth = std::min(bounds.width * OR_XOR_WIDTH_RATIO,
                                       bounds.height * OR_XOR_MAX_WIDTH_BY_HEIGHT);
    const float leftX = bounds.x + (bounds.width - actualWidth) * 0.5f;
    return {
        leftX,
        leftX + actualWidth,
        bounds.y + bounds.height * 0.5f,
        actualWidth,
        bounds.height * OR_XOR_CURVE_DEPTH_RATIO
    };
}

BodyProfile notProfile(Rectangle bounds) {
    const float idealWidth = bounds.height * NOT_TRIANGLE_ASPECT_RATIO;
    const float actualWidth = std::min(bounds.width * NOT_WIDTH_RATIO, idealWidth);
    const float leftX = bounds.x + (bounds.width - actualWidth) * 0.5f;
    return {
        leftX,
        leftX + actualWidth,
        bounds.y + bounds.height * 0.5f,
        actualWidth,
        0.0f
    };
}

float curvedBackX(const BodyProfile& profile, float t) {
    const float normalized = 2.0f * t - 1.0f;
    const float amount = 1.0f - normalized * normalized;
    return profile.leftX + profile.curveDepth * amount;
}

float xorRearCurveX(const BodyProfile& profile, float t) {
    const float normalized = 2.0f * t - 1.0f;
    const float amount = 1.0f - normalized * normalized;
    const float rearBase = profile.leftX - profile.curveDepth * XOR_REAR_CURVE_OFFSET_RATIO;
    return rearBase + profile.curveDepth * amount;
}

Vector2 notBubbleCenter(const BodyProfile& profile) {
    return {profile.rightX, profile.centerY};
}

float notBubbleRadius(float height) {
    return std::max(3.0f, height * NOT_BUBBLE_RADIUS_RATIO);
}

std::vector<Vector2> pinOffsets(GateKind kind, Vector2 size) {
    const Rectangle localBounds = {0.0f, 0.0f, size.x, size.y};

    switch (kind) {
        case GateKind::INPUT_SOURCE:
            return {{size.x, size.y * PIN_CENTER_RATIO}};

        case GateKind::OUTPUT_SINK:
            return {{0.0f, size.y * PIN_CENTER_RATIO}};

        case GateKind::AND_GATE: {
            const BodyProfile profile = andProfile(localBounds);
            return {
                {profile.leftX, size.y * INPUT_PIN_TOP_RATIO},
                {profile.leftX, size.y * INPUT_PIN_BOTTOM_RATIO},
                {profile.rightX, size.y * PIN_CENTER_RATIO}
            };
        }

        case GateKind::OR_GATE:
        case GateKind::XOR_GATE: {
            const BodyProfile profile = orXorProfile(localBounds);
            return {
                {curvedBackX(profile, INPUT_PIN_TOP_RATIO), size.y * INPUT_PIN_TOP_RATIO},
                {curvedBackX(profile, INPUT_PIN_BOTTOM_RATIO), size.y * INPUT_PIN_BOTTOM_RATIO},
                {profile.rightX, size.y * PIN_CENTER_RATIO}
            };
        }

        case GateKind::NOT_GATE: {
            const BodyProfile profile = notProfile(localBounds);
            const float bubbleRadius = notBubbleRadius(size.y);
            const Vector2 bubble = notBubbleCenter(profile);
            return {
                {profile.leftX, size.y * PIN_CENTER_RATIO},
                {bubble.x + bubbleRadius, size.y * PIN_CENTER_RATIO}
            };
        }
    }

    return {};
}

std::vector<Vector2> pinAnchors(GateKind kind, Rectangle bounds) {
    std::vector<Vector2> offsets = pinOffsets(kind, {bounds.width, bounds.height});
    for (Vector2& offset : offsets) {
        offset.x += bounds.x;
        offset.y += bounds.y;
    }
    return offsets;
}

} // namespace GateSymbolGeometry
