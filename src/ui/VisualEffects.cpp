#include "ui/VisualEffects.h"
#include <cmath>
#include <algorithm>
#include <vector>

float VisualEffects::easeInOut(float t) {
    // Cubic ease-in-out: slow start, fast middle, slow end
    return t < 0.5f ? 2.0f * t * t : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
}

float VisualEffects::easeOut(float t) {
    // Quadratic ease-out: fast start, slow end
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float VisualEffects::easeIn(float t) {
    // Quadratic ease-in: slow start, fast end
    return t * t;
}

float VisualEffects::bounce(float t) {
    // Bounce easing with 4 decreasing bounces using magic numbers from CSS spec
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

Color VisualEffects::lerpColor(Color start, Color end, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return {
        (unsigned char)(start.r + t * (end.r - start.r)),
        (unsigned char)(start.g + t * (end.g - start.g)),
        (unsigned char)(start.b + t * (end.b - start.b)),
        (unsigned char)(start.a + t * (end.a - start.a))
    };
}

Color VisualEffects::addGlow(Color base, float intensity) {
    intensity = std::clamp(intensity, 0.0f, 1.0f);
    // Brighten each RGB component towards white (255) by intensity amount
    return {
        (unsigned char)std::min(255, (int)(base.r + intensity * (255 - base.r))),
        (unsigned char)std::min(255, (int)(base.g + intensity * (255 - base.g))),
        (unsigned char)std::min(255, (int)(base.b + intensity * (255 - base.b))),
        base.a
    };
}

Color VisualEffects::createGradient(Color top, Color bottom, float y, float height) {
    float t = std::clamp(y / height, 0.0f, 1.0f);
    return lerpColor(top, bottom, t);
}

void VisualEffects::drawRoundedRectangleGradient(Rectangle rect, float roundness,
                                               Color topColor, Color bottomColor) {
    // Draw gradient using multiple horizontal lines, all with proper rounding
    int steps = (int)rect.height;
    for (int i = 0; i < steps; i++) {
        float t = (float)i / (float)(steps - 1);  // Normalize position (0.0 to 1.0)
        Color lineColor = lerpColor(topColor, bottomColor, t);
        Rectangle lineRect = {rect.x, rect.y + i, rect.width, 1.0f};

        // All lines with rounded corners to maintain proper shape
        DrawRectangleRounded(lineRect, roundness, 8, lineColor);
    }
}

void VisualEffects::drawRoundedRectangleWithShadow(Rectangle rect, float roundness,
                                                 Color fillColor, Color shadowColor,
                                                 Vector2 shadowOffset) {
    // Draw shadow
    Rectangle shadowRect = {
        rect.x + shadowOffset.x,
        rect.y + shadowOffset.y,
        rect.width,
        rect.height
    };
    DrawRectangleRounded(shadowRect, roundness, 8, shadowColor);

    // Draw main rectangle
    DrawRectangleRounded(rect, roundness, 8, fillColor);
}

void VisualEffects::drawCircleWithGlow(Vector2 center, float radius, Color color,
                                     Color glowColor, float glowRadius) {
    // Draw glow layers from largest to smallest for proper alpha blending
    for (int i = 3; i >= 1; i--) {
        float currentRadius = radius + (glowRadius * i / 3.0f);
        Color currentGlow = glowColor;
        currentGlow.a = (unsigned char)(glowColor.a / (i + 1));  // Fade alpha with distance
        DrawCircleV(center, currentRadius, currentGlow);
    }

    // Draw main circle
    DrawCircleV(center, radius, color);
}

void VisualEffects::drawLineWithGlow(Vector2 start, Vector2 end, float thickness,
                                   Color color, Color glowColor, float glowThickness) {
    // Draw glow layers
    for (int i = 3; i >= 1; i--) {
        float currentThickness = thickness + (glowThickness * i / 3.0f);
        Color currentGlow = glowColor;
        currentGlow.a = (unsigned char)(glowColor.a / (i + 1));
        DrawLineEx(start, end, currentThickness, currentGlow);
    }

    // Draw main line
    DrawLineEx(start, end, thickness, color);
}

float VisualEffects::getPulseValue(float speed) {
    // Convert sine wave (-1 to 1) to pulse value (0 to 1)
    return (sinf(getTime() * speed) + 1.0f) * 0.5f;
}

float VisualEffects::getHoverTransition(bool isHovered, float& currentValue, float deltaTime) {
    float target = isHovered ? 1.0f : 0.0f;
    float speed = Config::HOVER_TRANSITION_SPEED;

    // Smoothly transition current value towards target
    if (currentValue < target) {
        currentValue = std::min(target, currentValue + speed * deltaTime);
    } else if (currentValue > target) {
        currentValue = std::max(target, currentValue - speed * deltaTime);
    }

    return currentValue;
}

Vector2 VisualEffects::getShakeOffset(float intensity, float frequency) {
    float time = getTime();
    // Use different frequencies for X and Y to create natural shake motion
    return {
        sinf(time * frequency) * intensity,
        cosf(time * frequency * 1.3f) * intensity
    };
}

void VisualEffects::updateParticle(Particle& particle, float deltaTime) {
    particle.position.x += particle.velocity.x * deltaTime;
    particle.position.y += particle.velocity.y * deltaTime;
    particle.life -= deltaTime;

    // Fade out over time based on remaining life percentage
    float lifeRatio = particle.life / particle.maxLife;
    particle.color.a = (unsigned char)(255 * lifeRatio);
}

void VisualEffects::drawParticle(const Particle& particle) {
    DrawCircleV(particle.position, particle.size, particle.color);
}

void VisualEffects::drawAnimatedSignal(const std::vector<Vector2>& path, float progress,
                                     Color signalColor, float signalSize) {
    if (path.size() < 2) return;

    // Calculate total path length
    float totalLength = 0.0f;
    std::vector<float> segmentLengths;
    for (size_t i = 0; i < path.size() - 1; i++) {
        float length = Vector2Distance(path[i], path[i + 1]);
        segmentLengths.push_back(length);
        totalLength += length;
    }

    // Find position along path
    float targetDistance = progress * totalLength;
    float currentDistance = 0.0f;

    for (size_t i = 0; i < segmentLengths.size(); i++) {
        if (currentDistance + segmentLengths[i] >= targetDistance) {
            float t = (targetDistance - currentDistance) / segmentLengths[i];
            Vector2 position = Vector2Lerp(path[i], path[i + 1], t);

            // Draw signal with glow
            drawCircleWithGlow(position, signalSize, signalColor,
                             Fade(signalColor, 0.3f), signalSize * 2.0f);
            break;
        }
        currentDistance += segmentLengths[i];
    }
}

void VisualEffects::drawTextWithShadow(const char* text, Vector2 position, float fontSize,
                                     Color textColor, Color shadowColor, Vector2 shadowOffset) {
    Vector2 shadowPos = {position.x + shadowOffset.x, position.y + shadowOffset.y};
    DrawTextEx(GetFontDefault(), text, shadowPos, fontSize, 1.0f, shadowColor);
    DrawTextEx(GetFontDefault(), text, position, fontSize, 1.0f, textColor);
}

void VisualEffects::drawTextWithGlow(const char* text, Vector2 position, float fontSize,
                                   Color textColor, Color glowColor, float glowRadius) {
    // Use glowRadius to determine the maximum glow extent
    int maxGlowRadius = (int)glowRadius;

    // Draw glow layers
    for (int i = maxGlowRadius; i >= 1; i--) {
        Color currentGlow = glowColor;
        currentGlow.a = (unsigned char)(glowColor.a / (i + 1));

        for (int x = -i; x <= i; x++) {
            for (int y = -i; y <= i; y++) {
                if (x * x + y * y <= i * i) {
                    Vector2 glowPos = {position.x + x, position.y + y};
                    DrawTextEx(GetFontDefault(), text, glowPos, fontSize, 1.0f, currentGlow);
                }
            }
        }
    }

    // Draw main text
    DrawTextEx(GetFontDefault(), text, position, fontSize, 1.0f, textColor);
}

float VisualEffects::getTime() {
    return GetTime();
}
