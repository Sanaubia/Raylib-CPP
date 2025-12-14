#pragma once
#include <raylib.h>
#include <vector>
#include <random>


class TileMap {
public:
    enum class TileType {
        Grass,
        Water,
        Forest,
        Mountain,
        Sand
    };

    enum class ResourceType {
        None,
        Tree,
        Rock
    };

    int width, height, tileSize;
    std::vector<TileType> tiles;
    std::vector<ResourceType> resources;

    TileMap(int w, int h, int tSize) : width(w), height(h), tileSize(tSize), tiles(w * h, TileType::Grass), resources(w * h, ResourceType::None) {}

    void Init() {
        for (int i = 0; i < width * height; ++i) {
            tiles[i] = static_cast<TileType>(GetRandomValue(0, 4));
            // Place resources only on certain tile types
            if (tiles[i] == TileType::Grass || tiles[i] == TileType::Forest) {
                int r = GetRandomValue(0, 9); // 20% tree, 10% rock
                if (r == 0 || r == 1) resources[i] = ResourceType::Tree;
                else if (r == 2) resources[i] = ResourceType::Rock;
                else resources[i] = ResourceType::None;
            } else {
                resources[i] = ResourceType::None;
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
        const Color grassColor    = { 34, 139, 34, 255 };   // ForestGreen
        const Color waterColor    = {  0, 191, 255, 255 };  // DeepSkyBlue
        const Color forestColor   = {  0, 100,  0, 255 };   // DarkGreen
        const Color mountainColor = { 169, 169, 169, 255 }; // DarkGray
        const Color sandColor     = { 237, 201, 175, 255 }; // Sandy
        const Color treeColor     = { 34, 80, 34, 255 };    // Darker green
        const Color rockColor     = { 120, 120, 120, 255 }; // Gray

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                TileType tile = tiles[idx];
                Color color;
                switch (tile) {
                    case TileType::Grass:    color = grassColor;    break;
                    case TileType::Water:    color = waterColor;    break;
                    case TileType::Forest:   color = forestColor;   break;
                    case TileType::Mountain: color = mountainColor; break;
                    case TileType::Sand:     color = sandColor;     break;
                    default:                 color = WHITE;         break;
                }
                DrawRectangle(x * tileSize, y * tileSize, tileSize, tileSize, color);

                // Draw resource if present
                ResourceType res = resources[idx];
                if (res == ResourceType::Tree) {
                    DrawCircle(x * tileSize + tileSize/2, y * tileSize + tileSize/2, tileSize/4, treeColor);
                } else if (res == ResourceType::Rock) {
                    DrawCircle(x * tileSize + tileSize/2, y * tileSize + tileSize/2, tileSize/5, rockColor);
                }
            }
        }
    }
};
