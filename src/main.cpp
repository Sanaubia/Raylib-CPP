#include <raylib.h>
#include <vector>
#include "colonist.h"
#include "tilemap.h"
#include "building.h"


int main() 
{
    const Color darkGreen = {20, 160, 133, 255};
    
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 600;
    
    Colonist colonist(400, 300);
    TileMap map(50, 38, 32); // Larger map: 50x38 tiles, 32px each (1600x1216)
    GlobalResources globalRes;
    
    InitWindow(screenWidth, screenHeight, "Frozen Colony Sim");
    map.Init();
    SetTargetFPS(60);
    
    // Setup camera
    Camera2D camera = { 0 };
    camera.target = { colonist.position.x, colonist.position.y };
    camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    // Building system
    std::vector<Building> buildings;
    int selectedBuildingType = 0;
    bool buildMode = false;
    
    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        
        // Camera controls
        const float cameraMoveSpeed = 300.0f;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    camera.target.y -= cameraMoveSpeed * delta;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  camera.target.y += cameraMoveSpeed * delta;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  camera.target.x -= cameraMoveSpeed * delta;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) camera.target.x += cameraMoveSpeed * delta;
        
        // Zoom controls
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            camera.zoom += wheel * 0.1f;
            if (camera.zoom < 0.5f) camera.zoom = 0.5f;
            if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        }
        
        // Update buildings
        for (auto& building : buildings) {
            building.Update(delta);
        }
        
        // UI panel dimensions
        const int panelX = 0, panelY = 0, panelW = 250, panelH = 160;
        Rectangle uiBounds = {(float)panelX, (float)panelY, (float)panelW, (float)panelH};
        
        colonist.Update(delta, map, globalRes, buildings, uiBounds, &camera, buildMode);

        // Restart on death
        if (colonist.state == ColonistState::Dead && IsKeyPressed(KEY_R)) {
            colonist = Colonist(400, 300);
            buildings.clear();
            globalRes.wood = 0;
            globalRes.stone = 0;
            map.Init();
        }
        
        // Building selection with number keys
        if (IsKeyPressed(KEY_ONE))   selectedBuildingType = 0;
        if (IsKeyPressed(KEY_TWO))   selectedBuildingType = 1;
        if (IsKeyPressed(KEY_THREE)) selectedBuildingType = 2;
        if (IsKeyPressed(KEY_B))     buildMode = !buildMode;
        
        // Building placement
        if (buildMode && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            if (!CheckCollisionPointRec(mousePos, uiBounds)) {
                Vector2 worldPos = GetScreenToWorld2D(mousePos, camera);
                int tileX = (int)(worldPos.x / map.tileSize);
                int tileY = (int)(worldPos.y / map.tileSize);
                
                const BuildingBlueprint& bp = BUILDING_BLUEPRINTS[selectedBuildingType];
                
                // Check if player has enough resources
                if (globalRes.wood >= bp.woodCost && globalRes.stone >= bp.stoneCost) {
                    // Check if space is free
                    bool canPlace = true;
                    for (const auto& b : buildings) {
                        if (b.Overlaps(tileX, tileY, bp.width, bp.height)) {
                            canPlace = false;
                            break;
                        }
                    }
                    
                    // Check map bounds
                    if (tileX < 0 || tileY < 0 || tileX + bp.width > map.width || tileY + bp.height > map.height) {
                        canPlace = false;
                    }
                    
                    if (canPlace) {
                        buildings.emplace_back(bp.type, tileX, tileY, bp.width, bp.height);
                        globalRes.wood -= bp.woodCost;
                        globalRes.stone -= bp.stoneCost;
                        buildMode = false;
                    }
                }
            }
        }

        BeginDrawing();
            ClearBackground(darkGreen);
            
            // Draw world with camera
            BeginMode2D(camera);
                map.Draw();
                
                // Draw all buildings
                for (const auto& b : buildings) {
                    b.Draw(map.tileSize);
                }
                
                // Draw building preview in build mode
                if (buildMode) {
                    Vector2 mousePos = GetMousePosition();
                    Vector2 worldPos = GetScreenToWorld2D(mousePos, camera);
                    int tileX = (int)(worldPos.x / map.tileSize);
                    int tileY = (int)(worldPos.y / map.tileSize);
                    const BuildingBlueprint& bp = BUILDING_BLUEPRINTS[selectedBuildingType];
                    DrawRectangle(tileX * map.tileSize, tileY * map.tileSize, 
                                bp.width * map.tileSize, bp.height * map.tileSize, 
                                Fade(bp.color, 0.5f));
                }
                
                colonist.Draw();
            EndMode2D();

            // UI panel (screen space - not affected by camera)
            DrawRectangle(panelX, panelY, panelW, panelH, Fade(GRAY, 0.9f));
            DrawRectangleLines(panelX, panelY, panelW, panelH, DARKGRAY);
            DrawText("Colony Resources", panelX + 10, panelY + 8, 18, WHITE);
            DrawText(TextFormat("Wood: %d", globalRes.wood), panelX + 10, panelY + 30, 18, BROWN);
            DrawText(TextFormat("Stone: %d", globalRes.stone), panelX + 130, panelY + 30, 18, LIGHTGRAY);
            
            // Building menu
            DrawText("Buildings: (B to toggle build mode)", panelX + 10, panelY + 52, 14, WHITE);
            for (int i = 0; i < NUM_BUILDING_TYPES; i++) {
                const BuildingBlueprint& bp = BUILDING_BLUEPRINTS[i];
                int btnY = panelY + 70 + i * 25;
                bool canAfford = globalRes.wood >= bp.woodCost && globalRes.stone >= bp.stoneCost;
                bool isSelected = (i == selectedBuildingType && buildMode);
                
                Color btnColor = isSelected ? GREEN : (canAfford ? bp.color : DARKGRAY);
                DrawRectangle(panelX + 10, btnY, 230, 20, btnColor);
                DrawRectangleLines(panelX + 10, btnY, 230, 20, BLACK);
                DrawText(TextFormat("%d: %s (%dW %dS)", i + 1, bp.name, bp.woodCost, bp.stoneCost),
                        panelX + 15, btnY + 3, 14, WHITE);
            }
            
            // Colonist status panel (bottom right)
            int statusX = screenWidth - 260;
            int statusY = screenHeight - 110;
            DrawRectangle(statusX, statusY, 250, 100, Fade(GRAY, 0.9f));
            DrawRectangleLines(statusX, statusY, 250, 100, DARKGRAY);
            
            const char* stateText = "IDLE";
            Color stateColor = WHITE;
            switch (colonist.state) {
                case ColonistState::Gathering: stateText = "GATHERING"; stateColor = GREEN; break;
                case ColonistState::Seeking: stateText = "SEEKING WARMTH"; stateColor = ORANGE; break;
                case ColonistState::Resting: stateText = "RESTING"; stateColor = PURPLE; break;
                case ColonistState::Dead: stateText = "DEAD"; stateColor = RED; break;
                default: stateText = "IDLE"; stateColor = WHITE; break;
            }
            
            DrawText(TextFormat("Colonist: %s", stateText), statusX + 10, statusY + 8, 16, stateColor);
            DrawText("Warmth:", statusX + 10, statusY + 30, 14, WHITE);
            DrawRectangle(statusX + 80, statusY + 30, 150, 14, DARKGRAY);
            DrawRectangle(statusX + 80, statusY + 30, 150 * (colonist.warmth / 100.0f), 14, RED);
            DrawText(TextFormat("%.0f%%", colonist.warmth), statusX + 235, statusY + 30, 12, WHITE);
            
            DrawText("Hunger:", statusX + 10, statusY + 48, 14, WHITE);
            DrawRectangle(statusX + 80, statusY + 48, 150, 14, DARKGRAY);
            DrawRectangle(statusX + 80, statusY + 48, 150 * (colonist.hunger / 100.0f), 14, ORANGE);
            DrawText(TextFormat("%.0f%%", colonist.hunger), statusX + 235, statusY + 48, 12, WHITE);
            
            DrawText("Energy:", statusX + 10, statusY + 66, 14, WHITE);
            DrawRectangle(statusX + 80, statusY + 66, 150, 14, DARKGRAY);
            DrawRectangle(statusX + 80, statusY + 66, 150 * (colonist.energy / 100.0f), 14, SKYBLUE);
            DrawText(TextFormat("%.0f%%", colonist.energy), statusX + 235, statusY + 66, 12, WHITE);
            
            // Death message
            if (colonist.state == ColonistState::Dead) {
                DrawRectangle(screenWidth / 2 - 150, screenHeight / 2 - 50, 300, 100, Fade(BLACK, 0.8f));
                DrawText("COLONIST DIED!", screenWidth / 2 - 80, screenHeight / 2 - 30, 24, RED);
                DrawText("Press R to restart", screenWidth / 2 - 70, screenHeight / 2 + 10, 16, WHITE);
            }
            
            // Controls help
            DrawText(TextFormat("Zoom: %.1fx | WASD: Move | B: Build | 1-3: Select", camera.zoom), 
                    10, screenHeight - 25, 14, WHITE);
            DrawText(TextFormat("Buildings: %d | Mode: %s", buildings.size(), buildMode ? "BUILD" : "NORMAL"),
                    10, screenHeight - 45, 14, buildMode ? YELLOW : WHITE);
        EndDrawing();
    }
    
    CloseWindow();
}
