#pragma once
#include <raylib.h>

class TileMap; // Forward declaration

struct GlobalResources {
    int trees = 0;
    int rocks = 0;
};

class Colonist {
public:
    Vector2 position;
    Vector2 destination;
    float speed;
    bool hasDestination = false;

    Colonist(float x, float y) : position{x, y}, destination{x, y}, speed(100.0f) {}

    void SetDestination(Vector2 dest) {
        destination = dest;
        hasDestination = true;
    }

    void Update(float delta, TileMap& map, GlobalResources& globalRes);

    void Draw() const {
        DrawCircleV(position, 10, BLUE);
        if (hasDestination) {
            DrawCircleV(destination, 5, RED);
        }
    }
};
