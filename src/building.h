#pragma once
#include <raylib.h>
#include <vector>

enum class BuildingType {
    Hut,           // Provides shelter and warmth
    Storage,       // Stores resources
    Campfire       // Provides warmth and light
};

struct BuildingBlueprint {
    BuildingType type;
    const char* name;
    int woodCost;
    int stoneCost;
    int width;      // In tiles
    int height;     // In tiles
    Color color;
};

class Building {
public:
    BuildingType type;
    Vector2 tilePosition;  // Grid position (tile coordinates)
    int width, height;     // Size in tiles
    bool isConstructed;    // Building can be under construction
    float constructionProgress;
    
    Building(BuildingType t, int tx, int ty, int w, int h) 
        : type(t), tilePosition{(float)tx, (float)ty}, width(w), height(h), 
          isConstructed(false), constructionProgress(0.0f) {}
    
    void Update(float delta) {
        if (!isConstructed) {
            constructionProgress += delta * 0.3f; // Construction speed
            if (constructionProgress >= 1.0f) {
                constructionProgress = 1.0f;
                isConstructed = true;
            }
        }
    }
    
    void Draw(int tileSize) const {
        int px = tilePosition.x * tileSize;
        int py = tilePosition.y * tileSize;
        int pw = width * tileSize;
        int ph = height * tileSize;
        
        Color baseColor, roofColor;
        const char* label = "";
        
        switch (type) {
            case BuildingType::Hut:
                baseColor = BROWN;
                roofColor = DARKBROWN;
                label = "HUT";
                break;
            case BuildingType::Storage:
                baseColor = GRAY;
                roofColor = DARKGRAY;
                label = "STORAGE";
                break;
            case BuildingType::Campfire:
                baseColor = ORANGE;
                roofColor = RED;
                label = "FIRE";
                break;
        }
        
        if (!isConstructed) {
            // Show construction progress
            DrawRectangle(px, py, pw, ph, Fade(baseColor, 0.3f));
            DrawRectangleLines(px, py, pw, ph, YELLOW);
            int progressBarHeight = 4;
            int progressWidth = pw * constructionProgress;
            DrawRectangle(px, py + ph - progressBarHeight, progressWidth, progressBarHeight, GREEN);
        } else {
            // Draw completed building
            DrawRectangle(px, py + ph / 3, pw, ph * 2 / 3, baseColor);
            
            // Draw roof
            Vector2 roof[3] = {
                {(float)px, (float)(py + ph / 3)},
                {(float)(px + pw / 2), (float)py},
                {(float)(px + pw), (float)(py + ph / 3)}
            };
            DrawTriangle(roof[0], roof[1], roof[2], roofColor);
            
            // Draw door/window
            if (type == BuildingType::Hut) {
                DrawRectangle(px + pw / 2 - 4, py + ph - 12, 8, 12, DARKBROWN);
            }
            
            // Label
            int fontSize = 10;
            int textWidth = MeasureText(label, fontSize);
            DrawText(label, px + (pw - textWidth) / 2, py + ph / 2, fontSize, WHITE);
        }
    }
    
    // Check if a point is inside this building
    bool Contains(int tileX, int tileY) const {
        return tileX >= tilePosition.x && tileX < tilePosition.x + width &&
               tileY >= tilePosition.y && tileY < tilePosition.y + height;
    }
    
    // Check if building overlaps with another position
    bool Overlaps(int tileX, int tileY, int w, int h) const {
        return !(tileX + w <= tilePosition.x || 
                 tileX >= tilePosition.x + width ||
                 tileY + h <= tilePosition.y || 
                 tileY >= tilePosition.y + height);
    }
};

// Available building blueprints
static const BuildingBlueprint BUILDING_BLUEPRINTS[] = {
    {BuildingType::Hut,      "Hut",      5, 2, 2, 2, BROWN},
    {BuildingType::Storage,  "Storage",  8, 5, 3, 2, GRAY},
    {BuildingType::Campfire, "Campfire", 3, 1, 1, 1, ORANGE}
};

static const int NUM_BUILDING_TYPES = 3;
