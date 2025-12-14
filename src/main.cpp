#include <raylib.h>
#include <list>
struct Building {
    Vector2 position;
    Building() : position{0, 0} {}
    Building(Vector2 pos) : position(pos) {}
};
    std::list<Building> buildings;
#include <raylib.h>
#include "colonist.h"
#include "tilemap.h"

int main() 
{
    const Color darkGreen = {20, 160, 133, 255};
    
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 600;
    
    Colonist colonist(400, 300);
    TileMap map(25, 19, 32); // 25x19 tiles, 32px each (fits 800x608)
    GlobalResources globalRes;
    
    InitWindow(screenWidth, screenHeight, "My first RAYLIB program!");
    map.Init();
    SetTargetFPS(60);
    
    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        colonist.Update(delta, map, globalRes);

        // UI panel dimensions
        const int panelX = 0, panelY = 0, panelW = 220, panelH = 90;
        const int buttonX = 10, buttonY = 55, buttonW = 200, buttonH = 28;
        bool canBuild = (globalRes.trees >= 5 && globalRes.rocks >= 2);
        bool buildClicked = false;

        // Build Hut button logic
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (mouse.x >= buttonX && mouse.x <= buttonX + buttonW && mouse.y >= buttonY && mouse.y <= buttonY + buttonH && canBuild) {
                // Place hut at colonist's position
                buildings.emplace_back(colonist.position);
                globalRes.trees -= 5;
                globalRes.rocks -= 2;
                buildClicked = true;
            }
        }

        BeginDrawing();
            ClearBackground(darkGreen);
            map.Draw();
            // Draw all buildings
            for (const auto& b : buildings) {
                DrawRectangle(b.position.x - 12, b.position.y - 12, 24, 24, BROWN);
                DrawRectangle(b.position.x - 8, b.position.y - 8, 16, 16, YELLOW);
            }
            colonist.Draw();

            // UI panel
            DrawRectangle(panelX, panelY, panelW, panelH, Fade(GRAY, 0.85f));
            DrawRectangleLines(panelX, panelY, panelW, panelH, DARKGRAY);
            DrawText("Colony Resources", panelX + 10, panelY + 8, 18, WHITE);
            DrawText(TextFormat("Trees: %d", globalRes.trees), panelX + 10, panelY + 30, 18, GREEN);
            DrawText(TextFormat("Rocks: %d", globalRes.rocks), panelX + 110, panelY + 30, 18, LIGHTGRAY);

            // Build Hut button
            Color btnColor = canBuild ? (buildClicked ? DARKGREEN : BLUE) : DARKGRAY;
            DrawRectangle(buttonX, buttonY, buttonW, buttonH, btnColor);
            DrawRectangleLines(buttonX, buttonY, buttonW, buttonH, BLACK);
            DrawText("Build Hut (5 trees, 2 rocks)", buttonX + 10, buttonY + 6, 16, WHITE);
            if (!canBuild) {
                DrawText("Not enough resources!", buttonX + 10, buttonY + buttonH + 2, 16, RED);
            }
        EndDrawing();
    }
    
    CloseWindow();
}
