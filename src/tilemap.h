#pragma once
#include <raylib.h>
#include <vector>
#include <random>
#include <cmath>


class TileMap {
public:
    enum class TileType {
        Snow,          // Deep snow
        Ice,           // Frozen ice sheets
        FrozenLake,    // Frozen water bodies
        RockOutcrop,   // Rocky mountains
        SnowDrift      // Snow accumulation areas
    };

    enum class ResourceType {
        None,
        DeadTree,
        Rock
    };

    int width, height, tileSize;
    std::vector<TileType> tiles;
    std::vector<ResourceType> resources;

    TileMap(int w, int h, int tSize) : width(w), height(h), tileSize(tSize), tiles(w * h, TileType::Snow), resources(w * h, ResourceType::None) {}

    // Simple noise function for natural patterns
    float SimpleNoise(int x, int y, int seed) {
        int n = x + y * 57 + seed * 131;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    }

    float SmoothNoise(float x, float y, int seed) {
        float corners = (SimpleNoise(x-1, y-1, seed) + SimpleNoise(x+1, y-1, seed) + 
                        SimpleNoise(x-1, y+1, seed) + SimpleNoise(x+1, y+1, seed)) / 16.0f;
        float sides = (SimpleNoise(x-1, y, seed) + SimpleNoise(x+1, y, seed) + 
                      SimpleNoise(x, y-1, seed) + SimpleNoise(x, y+1, seed)) / 8.0f;
        float center = SimpleNoise(x, y, seed) / 4.0f;
        return corners + sides + center;
    }

    void Init() {
        int seed = GetRandomValue(0, 10000);
        
        // Generate elevation map
        std::vector<float> elevation(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float noise = 0;
                float amplitude = 1.0f;
                float frequency = 0.05f;
                
                // Multi-octave noise for more natural terrain
                for (int octave = 0; octave < 4; ++octave) {
                    noise += SmoothNoise(x * frequency, y * frequency, seed + octave) * amplitude;
                    amplitude *= 0.5f;
                    frequency *= 2.0f;
                }
                
                elevation[y * width + x] = noise;
            }
        }

        // Generate moisture map for rivers and forests
        std::vector<float> moisture(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float noise = 0;
                float amplitude = 1.0f;
                float frequency = 0.08f;
                
                for (int octave = 0; octave < 3; ++octave) {
                    noise += SmoothNoise(x * frequency, y * frequency, seed + 1000 + octave) * amplitude;
                    amplitude *= 0.5f;
                    frequency *= 2.0f;
                }
                
                moisture[y * width + x] = noise;
            }
        }

        // Assign tile types - frozen biome only
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                float elev = elevation[idx];
                float moist = moisture[idx];

                // Frozen lakes (low elevation areas)
                if (elev < -0.35f) {
                    tiles[idx] = TileType::FrozenLake;
                    resources[idx] = ResourceType::None;
                }
                // Rock outcrops (high elevation)
                else if (elev > 0.55f) {
                    tiles[idx] = TileType::RockOutcrop;
                    // Rocks in mountains
                    if (GetRandomValue(0, 3) == 0) {
                        resources[idx] = ResourceType::Rock;
                    } else {
                        resources[idx] = ResourceType::None;
                    }
                }
                // Ice sheets (low areas with high moisture)
                else if (elev < 0.0f && moist > 0.2f) {
                    tiles[idx] = TileType::Ice;
                    resources[idx] = ResourceType::None;
                }
                // Snow drifts (high moisture areas)
                else if (moist > 0.5f) {
                    tiles[idx] = TileType::SnowDrift;
                    // Buried dead trees occasionally
                    if (GetRandomValue(0, 15) == 0) {
                        resources[idx] = ResourceType::DeadTree;
                    } else {
                        resources[idx] = ResourceType::None;
                    }
                }
                // Snow (default)
                else {
                    tiles[idx] = TileType::Snow;
                    // Sparse resources scattered on snow
                    int r = GetRandomValue(0, 25);
                    if (r == 0) resources[idx] = ResourceType::DeadTree;
                    else if (r == 1) resources[idx] = ResourceType::Rock;
                    else resources[idx] = ResourceType::None;
                }
            }
        }
    }

    ResourceType GetResourceAt(int x, int y) const {
        if (x < 0 || y < 0 || x >= width || y >= height) return ResourceType::None;
        return resources[y * width + x];
    }

    void RemoveResourceAt(int x, int y) {
        if (x < 0 || y < 0 || x >= width || y >= height) return;
        resources[y * width + x] = ResourceType::None;
    }

    void Draw() const {
        const Color snowColor       = { 240, 248, 255, 255 }; // AliceBlue (bright snow)
        const Color iceColor        = { 175, 238, 238, 255 }; // PaleTurquoise (ice)
        const Color frozenLakeColor = { 200, 230, 240, 255 }; // Light frozen blue
        const Color rockOutcropColor= { 105, 105, 105, 255 }; // DimGray (dark rocks)
        const Color snowDriftColor  = { 255, 255, 255, 255 }; // Pure white (deep snow)
        const Color deadTreeColor   = { 101, 67, 33, 255 };   // SaddleBrown (dead wood)
        const Color rockColor       = { 128, 128, 128, 255 }; // Gray (rocks)

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                TileType tile = tiles[idx];
                Color color;
                switch (tile) {
                    case TileType::Snow:         color = snowColor;         break;
                    case TileType::Ice:          color = iceColor;          break;
                    case TileType::FrozenLake:   color = frozenLakeColor;   break;
                    case TileType::RockOutcrop:  color = rockOutcropColor;  break;
                    case TileType::SnowDrift:    color = snowDriftColor;    break;
                    default:                     color = WHITE;             break;
                }
                DrawRectangle(x * tileSize, y * tileSize, tileSize, tileSize, color);

                ResourceType res = resources[idx];
                if (res == ResourceType::DeadTree) {
                    // Draw as a fallen log
                    DrawRectangle(x * tileSize + tileSize/4, y * tileSize + tileSize/3, 
                                 tileSize/2, tileSize/6, deadTreeColor);
                } else if (res == ResourceType::Rock) {
                    DrawCircle(x * tileSize + tileSize/2, y * tileSize + tileSize/2, 
                              tileSize/5, rockColor);
                }
            }
        }
    }
};
