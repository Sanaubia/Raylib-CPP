#pragma once
#include <raylib.h>
#include <vector>

class TileMap; // Forward declaration
class Building; // Forward declaration

struct GlobalResources {
    int wood = 0;
    int stone = 0;
    int food = 0; // Add this line
};

enum class ColonistState {
    Idle,
    Gathering,
    Seeking,      // Seeking warmth or shelter
    Resting,      // Resting in hut
    Dead,
    Fishing
};

class Colonist {
public:
    Vector2 position;
    Vector2 destination;
    float speed;
    bool hasDestination = false;
    bool playerControlled = false;
    
    // Needs (0-100)
    float warmth = 100.0f;
    float hunger = 100.0f;
    float energy = 100.0f;
    
    ColonistState state = ColonistState::Idle;
    float restTimer = 0.0f;
    Building* currentBuilding = nullptr; // Building colonist is inside


    float fishingTimer = 0.0f;

    Colonist(float x, float y) : position{x, y}, destination{x, y}, speed(100.0f), fishingTimer(0.0f) {}

    void SetDestination(Vector2 dest, bool isPlayerOrder = false) {
        destination = dest;
        hasDestination = true;
        playerControlled = isPlayerOrder;
    }

    void Update(float delta, TileMap& map, GlobalResources& globalRes, 
                const std::vector<Building>& buildings,
                Rectangle uiBounds = {0, 0, 0, 0}, 
                Camera2D* camera = nullptr, bool buildMode = false);

    void Draw() const {
        if (state == ColonistState::Dead) {
            DrawCircleV(position, 10, GRAY);
            DrawCircleV(position, 6, RED);
            return;
        }
        
        // Color based on state
        Color color = BLUE;
        if (state == ColonistState::Resting) color = PURPLE;
        else if (state == ColonistState::Seeking) color = ORANGE;
        
        DrawCircleV(position, 10, color);
        if (hasDestination) {
            DrawCircleV(destination, 5, playerControlled ? RED : ORANGE);
        }
        
        // Draw needs bars above colonist
        DrawNeedsBar(position.x - 15, position.y - 20, 30, 3, warmth, RED);
        DrawNeedsBar(position.x - 15, position.y - 25, 30, 3, hunger, ORANGE);
        DrawNeedsBar(position.x - 15, position.y - 30, 30, 3, energy, BLUE);
    }
    
    bool IsAlive() const { return state != ColonistState::Dead; }

private:
    bool FindNearestResource(const TileMap& map, Vector2& resourcePos);
    Building* FindNearestBuilding(const std::vector<Building>& buildings, int buildingType);
    bool IsNearBuilding(const Building& building, float radius, int tileSize) const;
    
    void DrawNeedsBar(float x, float y, float width, float height, float value, Color color) const {
        DrawRectangle(x, y, width, height, DARKGRAY);
        DrawRectangle(x, y, width * (value / 100.0f), height, color);
    }
};
