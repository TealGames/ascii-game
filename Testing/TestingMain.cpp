#include "raylib.h"
#include <string>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;
constexpr int TARGET_FPS = 60;

int main()
{
    // Initialize the window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TESTING SANDBOX");
    SetTargetFPS(TARGET_FPS);

    Color fadedYellow;
    fadedYellow.r = 255;
    fadedYellow.g = 236;
    fadedYellow.b = 161;
    fadedYellow.a = 100;
    Image lightImage = GenImageGradientRadial(256, 256, 0.0f, fadedYellow, BLANK);
    Texture2D lightTexture = LoadTextureFromImage(lightImage);
    UnloadImage(lightImage); // Unload image after creating texture

    // Position for the light
    Vector2 lightPosition = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

    // Load a font (you can use any font you like)
    Font font = GetFontDefault();

    // Define the text character you want to use as a texture
    const std::string text = "`";


    // Set text size and color
    int textSize = 100;
    Color textColor = GREEN;

    // Create an empty image to store the text
    Image textImage = GenImageColor(200, 200, RAYWHITE);  // Create an image with white background

    // Create a render target texture (a framebuffer) to draw the text
    RenderTexture2D renderTexture = LoadRenderTexture(200, 200);

    // Begin drawing to the render target
    BeginTextureMode(renderTexture);

    ClearBackground(BLANK);  // Clear the render target with white

    // Draw the text on the render target (with custom color and size)
    DrawTextEx(font, text.c_str(), Vector2{50, 50}, textSize, 2, textColor);

    // End drawing to the render target
    EndTextureMode();

    // Convert the render texture to an image
    textImage = LoadImageFromTexture(renderTexture.texture);
    ImageFlipVertical(&textImage);

    // Convert the image to a texture
    Texture2D textTexture = LoadTextureFromImage(textImage);

    // Unload the image since we no longer need it
    UnloadImage(textImage);

    char testChar[2] = { 'C', '\0'};
    const float fontSize = 12*20;
    Vector2 size = MeasureTextEx(font, testChar, fontSize, 0);

    // Main game loop
    while (!WindowShouldClose()) 
    {
        lightPosition = GetMousePosition();
        lightPosition.x -= (lightTexture.width / 2);
        lightPosition.y -= (lightTexture.height / 2);

        // Begin drawing
        BeginDrawing();

        ClearBackground(BLACK);
        DrawFPS(0,0);

        DrawTextEx(font, testChar, { 0, 0 }, fontSize, 0, GREEN);
        DrawTextEx(font, testChar, size, fontSize, 0, RED);

        /*
        int x = 0;
        int y = 0;
        bool found = false;
        for (int i = 0; i < 10; i++)
        {
            y = 0;
            x = i * 50;
            // Draw the texture (text as texture) at position (400, 300)
            DrawTexture(textTexture, x, y, WHITE);
        }
        
        //good: blend additive
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexture(lightTexture, lightPosition.x, lightPosition.y, WHITE);
        EndBlendMode();
        */

        // End drawing
        EndDrawing();
    }

    // Clean up resources
    UnloadTexture(textTexture);
    UnloadFont(font);

    // Close the window
    CloseWindow();

    return 0;
}