#ifndef VISUAL_EFFECTS_H
#define VISUAL_EFFECTS_H

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "app/Config.h"

/**
 * Utility class for visual effects and animations
 */
class VisualEffects {
public:
    // Animation timing functions
    static float easeInOut(float t);
    static float easeOut(float t);
    static float easeIn(float t);
    static float bounce(float t);
    
    // Color utilities
    static Color lerpColor(Color start, Color end, float t);
    static Color addGlow(Color base, float intensity);
    static Color createGradient(Color top, Color bottom, float y, float height);
    
    // Shape rendering with effects
    static void drawRoundedRectangleGradient(Rectangle rect, float roundness, 
                                           Color topColor, Color bottomColor);
    static void drawRoundedRectangleWithShadow(Rectangle rect, float roundness, 
                                             Color fillColor, Color shadowColor, 
                                             Vector2 shadowOffset);
    static void drawCircleWithGlow(Vector2 center, float radius, Color color, 
                                 Color glowColor, float glowRadius);
    static void drawLineWithGlow(Vector2 start, Vector2 end, float thickness, 
                               Color color, Color glowColor, float glowThickness);
    
    // Animation helpers
    static float getPulseValue(float speed = Config::PULSE_SPEED);
    static float getHoverTransition(bool isHovered, float& currentValue, float deltaTime);
    static Vector2 getShakeOffset(float intensity, float frequency);
    
    // Particle effects
    struct Particle {
        Vector2 position;
        Vector2 velocity;
        Color color;
        float life;
        float maxLife;
        float size;
    };
    
    static void updateParticle(Particle& particle, float deltaTime);
    static void drawParticle(const Particle& particle);
    
    // Wire signal animation
    static void drawAnimatedSignal(const std::vector<Vector2>& path, float progress, 
                                 Color signalColor, float signalSize);
    
    // Text effects
    static void drawTextWithShadow(const char* text, Vector2 position, float fontSize, 
                                 Color textColor, Color shadowColor, Vector2 shadowOffset);
    static void drawTextWithGlow(const char* text, Vector2 position, float fontSize, 
                               Color textColor, Color glowColor, float glowRadius);
    
private:
    static float getTime();
};

#endif // VISUAL_EFFECTS_H
