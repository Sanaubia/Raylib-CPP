#include "colonist.h"
#include "tilemap.h"
#include <cmath>

void Colonist::Update(float delta, TileMap& map, GlobalResources& globalRes) {
    // Mouse click to set destination
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        SetDestination(GetMousePosition());
    }

    if (hasDestination) {
        Vector2 toDest = {destination.x - position.x, destination.y - position.y};
        float dist = sqrtf(toDest.x * toDest.x + toDest.y * toDest.y);
        if (dist > 2.0f) {
            Vector2 dir = {toDest.x / dist, toDest.y / dist};
            position.x += dir.x * speed * delta;
            position.y += dir.y * speed * delta;
        } else {
            position = destination;
            hasDestination = false;
        }
    } else {
        // Simple AI random walk fallback
        position.x += (GetRandomValue(-1, 1)) * speed * delta * 0.1f;
        position.y += (GetRandomValue(-1, 1)) * speed * delta * 0.1f;
    }

    // Gather resource if standing on one
    int tileX = static_cast<int>(position.x) / map.tileSize;
    int tileY = static_cast<int>(position.y) / map.tileSize;
    auto res = map.GetResourceAt(tileX, tileY);
    if (res == TileMap::ResourceType::Tree) {
        globalRes.trees++;
        map.RemoveResourceAt(tileX, tileY);
    } else if (res == TileMap::ResourceType::Rock) {
        globalRes.rocks++;
        map.RemoveResourceAt(tileX, tileY);
    }
}
