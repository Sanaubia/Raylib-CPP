#include "colonist.h"
#include "tilemap.h"
#include "building.h"
#include <cmath>
#include <limits>

bool Colonist::FindNearestResource(const TileMap& map, Vector2& resourcePos) {
    float minDist = std::numeric_limits<float>::max();
    bool found = false;
    int currentTileX = static_cast<int>(position.x) / map.tileSize;
    int currentTileY = static_cast<int>(position.y) / map.tileSize;
    
    const int searchRadius = 10;
    
    for (int dy = -searchRadius; dy <= searchRadius; dy++) {
        for (int dx = -searchRadius; dx <= searchRadius; dx++) {
            int checkX = currentTileX + dx;
            int checkY = currentTileY + dy;
            
            if (checkX < 0 || checkX >= map.width || checkY < 0 || checkY >= map.height) {
                continue;
            }
            
            auto res = map.GetResourceAt(checkX, checkY);
            if (res == TileMap::ResourceType::DeadTree || res == TileMap::ResourceType::Rock) {
                Vector2 tileCenter = {
                    checkX * map.tileSize + map.tileSize / 2.0f,
                    checkY * map.tileSize + map.tileSize / 2.0f
                };
                float dx_f = tileCenter.x - position.x;
                float dy_f = tileCenter.y - position.y;
                float dist = sqrtf(dx_f * dx_f + dy_f * dy_f);
                
                if (dist < minDist) {
                    minDist = dist;
                    resourcePos = tileCenter;
                    found = true;
                }
            }
        }
    }
    
    return found;
}

Building* Colonist::FindNearestBuilding(const std::vector<Building>& buildings, int buildingType) {
    float minDist = std::numeric_limits<float>::max();
    Building* nearest = nullptr;
    
    for (auto& building : const_cast<std::vector<Building>&>(buildings)) {
        if (!building.isConstructed) continue;
        if (buildingType >= 0 && (int)building.type != buildingType) continue;
        
        Vector2 buildingCenter = {
            (building.tilePosition.x + building.width / 2.0f) * 32.0f,
            (building.tilePosition.y + building.height / 2.0f) * 32.0f
        };
        
        float dx = buildingCenter.x - position.x;
        float dy = buildingCenter.y - position.y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist < minDist) {
            minDist = dist;
            nearest = &building;
        }
    }
    
    return nearest;
}

bool Colonist::IsNearBuilding(const Building& building, float radius, int tileSize) const {
    Vector2 buildingCenter = {
        (building.tilePosition.x + building.width / 2.0f) * tileSize,
        (building.tilePosition.y + building.height / 2.0f) * tileSize
    };
    
    float dx = buildingCenter.x - position.x;
    float dy = buildingCenter.y - position.y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    return dist <= radius;
}

void Colonist::Update(float delta, TileMap& map, GlobalResources& globalRes, 
                      const std::vector<Building>& buildings,
                      Rectangle uiBounds, Camera2D* camera, bool buildMode) {
    if (state == ColonistState::Dead) return;
    
    // Decrease needs over time
    warmth -= delta * 3.0f;  // Loses warmth fast in frozen biome
    hunger -= delta * 1.5f;
    energy -= delta * 1.0f;
    
    // Check for death
    if (warmth <= 0 || hunger <= 0) {
        state = ColonistState::Dead;
        return;
    }
    
    // Increase warmth near campfires
    for (const auto& building : buildings) {
        if (building.type == BuildingType::Campfire && building.isConstructed) {
            if (IsNearBuilding(building, 80.0f, map.tileSize)) {
                warmth += delta * 8.0f;  // Campfire restores warmth quickly
                if (warmth > 100.0f) warmth = 100.0f;
            }
        }
    }
    
    // Resting in hut
    if (state == ColonistState::Resting) {
        restTimer += delta;
        energy += delta * 15.0f;
        warmth += delta * 5.0f;  // Huts provide warmth
        
        if (energy > 100.0f) energy = 100.0f;
        if (warmth > 100.0f) warmth = 100.0f;
        
        if (energy >= 100.0f || restTimer >= 5.0f) {
            state = ColonistState::Idle;
            restTimer = 0.0f;
            currentBuilding = nullptr;
        }
        return;
    }
    
    // Critical needs - seek shelter/warmth
    if (!playerControlled) {
        if (warmth < 30.0f && state != ColonistState::Seeking) {
            // Seek campfire or hut
            Building* shelter = FindNearestBuilding(buildings, -1); // Any building
            if (shelter) {
                Vector2 shelterPos = {
                    (shelter->tilePosition.x + shelter->width / 2.0f) * map.tileSize,
                    (shelter->tilePosition.y + shelter->height / 2.0f) * map.tileSize
                };
                SetDestination(shelterPos, false);
                state = ColonistState::Seeking;
            }
        } else if (energy < 20.0f && state != ColonistState::Seeking) {
            // Seek hut to rest
            Building* hut = FindNearestBuilding(buildings, (int)BuildingType::Hut);
            if (hut) {
                Vector2 hutPos = {
                    (hut->tilePosition.x + hut->width / 2.0f) * map.tileSize,
                    (hut->tilePosition.y + hut->height / 2.0f) * map.tileSize
                };
                SetDestination(hutPos, false);
                state = ColonistState::Seeking;
            }
        }
    }
    
    // Check if reached a building while seeking
    if (state == ColonistState::Seeking && !hasDestination) {
        Building* nearby = FindNearestBuilding(buildings, -1);
        if (nearby && IsNearBuilding(*nearby, 20.0f, map.tileSize)) {
            if (nearby->type == BuildingType::Hut && energy < 50.0f) {
                state = ColonistState::Resting;
                currentBuilding = nearby;
                return;
            } else {
                state = ColonistState::Idle;
            }
        } else {
            state = ColonistState::Idle;
        }
    }
    
    // Mouse click to set destination - but not if clicking on UI or in build mode
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !buildMode) {
        Vector2 mousePos = GetMousePosition();
        if (!CheckCollisionPointRec(mousePos, uiBounds)) {
            Vector2 worldPos = camera ? GetScreenToWorld2D(mousePos, *camera) : mousePos;
            SetDestination(worldPos, true);
            state = ColonistState::Idle;
        }
    }

    if (hasDestination) {
        Vector2 toDest = {destination.x - position.x, destination.y - position.y};
        float dist = sqrtf(toDest.x * toDest.x + toDest.y * toDest.y);
        
        if (dist > 5.0f) {
            Vector2 dir = {toDest.x / dist, toDest.y / dist};
            position.x += dir.x * speed * delta;
            position.y += dir.y * speed * delta;
        } else {
            position = destination;
            hasDestination = false;
            
            // Try to gather resource immediately
            int tileX = static_cast<int>(position.x) / map.tileSize;
            int tileY = static_cast<int>(position.y) / map.tileSize;
            auto res = map.GetResourceAt(tileX, tileY);
            if (res == TileMap::ResourceType::DeadTree) {
                globalRes.wood++;
                map.RemoveResourceAt(tileX, tileY);
                state = ColonistState::Gathering;
            } else if (res == TileMap::ResourceType::Rock) {
                globalRes.stone++;
                map.RemoveResourceAt(tileX, tileY);
                state = ColonistState::Gathering;
            }
            
            playerControlled = false;
        }
    } else if (state == ColonistState::Idle || state == ColonistState::Gathering) {
        // AI: Look for nearest resource if needs are okay
        if (warmth > 50.0f && energy > 30.0f) {
            Vector2 resourcePos;
            if (FindNearestResource(map, resourcePos)) {
                SetDestination(resourcePos, false);
                state = ColonistState::Gathering;
            }
        }
    }
    
    if (state == ColonistState::Idle) {
        int tileX = (int)(position.x / map.tileSize);
        int tileY = (int)(position.y / map.tileSize);
        if (map.tiles[tileY * map.width + tileX] == TileMap::TileType::FrozenLake) {
            state = ColonistState::Fishing;
            fishingTimer = 0.0f;
        }
    } else if (state == ColonistState::Fishing) {
        fishingTimer += delta;
        if (fishingTimer > 3.0f) { // 3 seconds to fish
            globalRes.food += 1; // Add food resource (add this to GlobalResources if not present)
            state = ColonistState::Idle;
        }
    }
}
