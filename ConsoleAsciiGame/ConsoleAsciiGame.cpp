#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include "raylib.h"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "Point2DInt.hpp"
#include "Point2D.hpp"
#include "Player.hpp"
#include "EntityRenderer.hpp"
#include "LightSource.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "Globals.hpp"
#include "RenderLayer.hpp"
#include "GameRenderer.hpp"
#include "SceneManager.hpp"

#ifdef ENABLE_MEMORY_LEAK_DETECTION
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif

std::chrono::time_point<std::chrono::high_resolution_clock> LastTime;
std::chrono::time_point<std::chrono::high_resolution_clock> CurrentTime;
float DeltaTime = 0;

//NOTE: all positions are (x/Col, y/row) but buffer position is (row, col)

//struct RenderLayer
//{
//    TextBuffer m_TextBuffer;
//    const TextBuffer m_DefaultTextBuffer;
//    //Where the text buffer starts relative to the position of the max text buffer
//    Utils::Point2DInt m_StartPos;
//
//    RenderLayer(const TextBuffer& buffer, const Utils::Point2DInt startPos) :
//        m_TextBuffer(buffer), m_DefaultTextBuffer(m_TextBuffer), m_StartPos(startPos)
//    {
//
//    }
//
//    /// <summary>
//    /// Resets the buffer back to the original
//    /// </summary>
//    void ResetToDefault()
//    {
//        Utils::Point2DInt currentPos = {};
//        for (int r = 0; r < m_TextBuffer.m_HEIGHT; r++)
//        {
//            for (int c = 0; c < m_TextBuffer.m_WIDTH; c++)
//            {
//                currentPos = {r, c};
//                m_TextBuffer.SetAt(currentPos, *m_DefaultTextBuffer.GetAt(currentPos));
//            }
//        }
//    }
//
//    /// <summary>
//    /// Will return the amount of spacing between characters in (WIDTH, HEIGHT)
//    /// </summary>
//    /// <returns></returns>
//    Utils::Point2DInt CalculateCharSpacing() const
//    {
//        Utils::Point2DInt totalCharSpace = {-1, -1};
//        char drawStr[2] = { '1', '\0' };
//        for (int r = 0; r < m_TextBuffer.m_HEIGHT; r++)
//        {
//            totalCharSpace.m_Y += TEXT_BUFFER_FONT;
//
//            if (totalCharSpace.m_X != -1) continue;
//            for (int c = 0; c < m_TextBuffer.m_WIDTH; c++)
//            {
//                drawStr[0] = m_TextBuffer.GetAt({ r, c })->m_Char;
//                totalCharSpace.m_X+= MeasureText(drawStr, TEXT_BUFFER_FONT);
//            }
//        }
//
//        int widthSpacing = static_cast<double>(SCREEN_WIDTH - totalCharSpace.m_X) / (m_TextBuffer.m_WIDTH - 1);
//        int heightSpacing = static_cast<double>(SCREEN_HEIGHT - totalCharSpace.m_Y) / (m_TextBuffer.m_HEIGHT - 1);
//        return Utils::Point2DInt(widthSpacing, heightSpacing);
//    }
//
//    std::string ToString() const
//    {
//        return m_TextBuffer.ToString();
//    }
//};

//All of the render layers in order from one drawn first to last
static std::vector<RenderLayer*> RenderLayers = {};

//TextBuffer CollapseLayersToSingle(const std::vector<RenderLayer*>& layers)
//{
//    TextBuffer newBuffer = TextBuffer(TEXT_BUFFER_MAX_WIDTH, TEXT_BUFFER_MAX_HEIGHT, TextChar{});
//    Utils::Point2DInt globalPos = {};
//    Utils::Point2DInt localPos = {};
//    Utils::Point2DInt startPos = {};
//
//    Utils::Point2DInt half = { layers[1]->m_SquaredTextBuffer.m_HEIGHT / 2,  layers[1]->m_SquaredTextBuffer.m_WIDTH / 2};
//   
//    for (int r = 0; r < newBuffer.m_HEIGHT; r++)
//    {
//        for (int c = 0; c < newBuffer.m_WIDTH; c++)
//        {
//            globalPos = { r, c };
//            for (int layerIndex = layers.size() - 1; layerIndex >= 0; layerIndex--)
//            {
//                /*std::cout << std::format("ayer index: {}/{} of r: {} c:{}", 
//                    std::to_string(layerIndex), std::to_string(layers.size()), std::to_string(r), std::to_string(c)) << std::endl;*/
//
//                const RenderLayer& layer = *layers[layerIndex];
//                startPos = layer.m_StartPos;
//
//                //If the global pos we are looking at is not contianed within the layer's buffer, continue
//                if (globalPos.m_X < startPos.m_X || globalPos.m_X >= startPos.m_X + layer.m_SquaredTextBuffer.m_HEIGHT ||
//                    globalPos.m_Y < startPos.m_Y || globalPos.m_Y >= startPos.m_Y + layer.m_SquaredTextBuffer.m_WIDTH)
//                    continue;
//
//                localPos = globalPos - startPos;
//
//               /* Utils::Log(std::format("Attempting char at: {} of HEGIHT: {} WIDTH: {}", 
//                    localPos.ToString(), std::to_string(layer.m_TextBuffer.m_HEIGHT), std::to_string(layer.m_TextBuffer.m_WIDTH)));*/
//
//                const TextChar* textChar = layer.m_SquaredTextBuffer.GetAt(localPos);
//                
//                //If the converted pos relative to the layer is placeholder continue
//                if (textChar->m_Char == EMPTY_CHAR_PLACEHOLDER)
//                    continue;
//
//                newBuffer.SetAt(globalPos, *textChar);
//                break;
//            }   
//        }
//    }
//    return newBuffer;
//}



//void RenderPlayerLight(TextBuffer& buffer, const Utils::Point2DInt& centerPos, const int& radius)
//{
//    const Utils::Point2DInt maxXY = { centerPos.m_X + radius, centerPos.m_Y + radius};
//    const Utils::Point2DInt minXY = { centerPos.m_X - radius, centerPos.m_Y - radius };
//
//    const int positiveSidePoints = std::min(TEXT_BUFFER_MAX_HEIGHT, TEXT_BUFFER_MAX_WIDTH) / radius;
//    const float pointXValIncrement = 0.5;
//    
//    float y = 0;
//    float x = minXY.m_X;
//    std::vector<Utils::Point2D> bottomCirclePoints = {};
//    while (x <= maxXY.m_X)
//    {
//        y = std::sqrt(std::pow(radius, 2)- std::pow(x- centerPos.m_X, 2))+ centerPos.m_Y; 
//        bottomCirclePoints.push_back({ x, y });
//
//        x += pointXValIncrement;
//    }
//    Utils::Log(std::format("Circle points: {}", Utils::ToStringIterable<std::vector<Utils::Point2D>, Utils::Point2D>(bottomCirclePoints)));
//
//    //TODO: ideally, this color should not be hardcoded but should apply
//    //a yellow (or other) color filter using math of the current color
//    const Color lightGreen = { 163, 213, 137, 255};
//    const float RENDERED_THRESHOLD = 0.5f;
//    float decimalVal = 0;
//    bool isZero = false;
//    Utils::Point2DInt bufferPos = {};
//    for (const auto& point : bottomCirclePoints)
//    {
//        if (point.m_X <= centerPos.m_X)  bufferPos.m_X = std::floor(point.m_X);
//        else bufferPos.m_X = std::ceil(point.m_X);
//        if (!buffer.IsValidCol(bufferPos.m_X)) continue;
//
//        bufferPos.m_Y = std::ceil(point.m_Y);
//        if(!buffer.IsValidRow(bufferPos.m_Y)) continue;
//        
//        Utils::Log(std::format("Circle Y: {} -> {}", std::to_string(0), std::to_string(bufferPos.m_Y - centerPos.m_Y)));
//        for (int circleY = 0; circleY <= bufferPos.m_Y- centerPos.m_Y; circleY++)
//        {
//            buffer.SetAt(centerPos.m_Y+ circleY, bufferPos.m_X, lightGreen);
//            Utils::Log(std::format("Setting pos: {} to lgiht green", Utils::Point2DInt({ centerPos.m_Y + circleY, bufferPos.m_X }).ToString()));
//            if (circleY != 0) buffer.SetAt(centerPos.m_Y- circleY, bufferPos.m_X, lightGreen);
//        }
//    }
//}

int main() {

#ifdef ENABLE_MEMORY_LEAK_DETECTION
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    const Color darkGreen = Color{ 57, 104, 48, 255};

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME.c_str());
    SetTargetFPS(60);

    SceneManagement::SceneManager sceneManager = SceneManagement::SceneManager("scenes");
    Utils::Log("Finished constructor of scene manager");
    
    sceneManager.TrySetActiveScene(0);
    Utils::Log(std::format("Finished setting active scene. valid scene {}", std::to_string(sceneManager.GetActiveScene()!=nullptr)));
    Utils::Log(sceneManager.GetActiveScene()->ToStringLayers());

    //TODO: the UI layer should be a separate layer that will be combined by the camera with rendered layers
    //RenderLayers.reserve(3);
    /*RenderLayer uiLayer(TextBuffer(TEXT_BUFFER_MAX_WIDTH, TEXT_BUFFER_MAX_HEIGHT, TextChar{}), {0, 0});
    uiLayer.m_SquaredTextBuffer.SetAt({ 0, 0 }, TextChar(RED, 'O'));
    uiLayer.m_SquaredTextBuffer.SetAt({ 0, 1 }, TextChar(RED, 'O'));*/

    /*Utils::Point2DInt playAreaOffset = {(TEXT_BUFFER_MAX_HEIGHT- PLAY_AREA_TEXT_BUFFER_HEIGHT)/2, (TEXT_BUFFER_MAX_WIDTH- PLAY_AREA_TEXT_BUFFER_WIDTH)/2};
    RenderLayer backgroundLayer(TextBuffer(PLAY_AREA_TEXT_BUFFER_WIDTH, PLAY_AREA_TEXT_BUFFER_HEIGHT, TextChar{darkGreen, '^'}), TEXT_BUFFER_FONT, CHAR_SPACING);
    RenderLayer playerLayer(TextBuffer(PLAY_AREA_TEXT_BUFFER_WIDTH, PLAY_AREA_TEXT_BUFFER_HEIGHT, TextChar{}));*/

   /* RenderLayers.push_back(&backgroundLayer);
    RenderLayers.push_back(&playerLayer);
    RenderLayers.push_back(&uiLayer);*/

    //RenderLayer playerLayer= RenderLayer(TextBuffer{PLAY_AREA_TEXT_BUFFER_WIDTH, PLAY_AREA_TEXT_BUFFER_HEIGHT, TextChar()}, TEXT_BUFFER_FONT, CHAR_SPACING);
    RenderLayer& backgroundLayer = *sceneManager.GetActiveSceneMutable()->GetLayersMutable()[0];
    RenderLayer& playerLayer= *sceneManager.GetActiveSceneMutable()->GetLayersMutable()[1];
    //ECS::Player player = ECS::Player(Utils::Point2DInt{ TEXT_BUFFER_MAX_WIDTH / 2, TEXT_BUFFER_MAX_HEIGHT / 2 }, 'o');

    //TODO: ideally, the custom script like Player should handle the creation of other components
    ECS::Transform transform = ECS::Transform(Utils::Point2DInt{ 3, 0 });
    //ECS::EntityRenderer renderer = ECS::EntityRenderer(transform, playerLayer.m_SquaredTextBuffer, { {TextChar(GRAY, '0')},  {TextChar(GRAY, '4')} });
    ECS::EntityRenderer renderer = ECS::EntityRenderer(transform, playerLayer.m_SquaredTextBuffer, 
        { {TextChar(GRAY, 'H')}});
    ECS::Player playerComponent = ECS::Player(transform, 'o');
    ECS::LightSource lightSource = ECS::LightSource(transform, renderer, 
        std::vector<TextBuffer*>{ &backgroundLayer.m_SquaredTextBuffer, &playerLayer.m_SquaredTextBuffer },
        ColorGradient(Color(243, 208, 67, 255),  Color(228, 8, 10, 255)), 8, 255, 1.2f);
    ECS::Entity player = ECS::Entity("Player", transform);
    player.TryAddComponent(&renderer);
    player.TryAddComponent(&playerComponent);
    player.TryAddComponent(&lightSource);

    ECS::Transform mainCameraTransform = ECS::Transform({0, 0});
    ECS::Entity mainCamera = ECS::Entity("MainCamera", mainCameraTransform, ECS::MIN_PRIORITY);
    ECS::Camera camera = ECS::Camera(mainCameraTransform, sceneManager, 
        ECS::CameraSettings(player, Utils::Point2DInt(20, 20)));
    mainCamera.TryAddComponent<ECS::Camera>(&camera);
    sceneManager.GetActiveSceneMutable()->AddEntity(mainCamera);
    sceneManager.GetActiveSceneMutable()->AddEntity(player);

    LastTime = std::chrono::high_resolution_clock().now();

    sceneManager.GetActiveSceneMutable()->Start();

    constexpr int FRAME_LIMIT = 10;
    constexpr bool SHOW_FPS = true;
    int currentFrameCounter = 0;

    //TODO: it seems like doing this over and over might be slow
    //so we need some optimizations
    while (!WindowShouldClose()) 
    {
        if (FRAME_LIMIT != -1 || SHOW_FPS)
        {
            Utils::Log(std::format("FRAME: {}/{} FPS:{}\n--------------------------------------------\n", 
                std::to_string(currentFrameCounter + 1), std::to_string(FRAME_LIMIT), std::to_string(GetFPS())));
        }

        CurrentTime = std::chrono::high_resolution_clock().now();
        DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - LastTime).count();

        sceneManager.GetActiveSceneMutable()->UpdateStart(DeltaTime);
        sceneManager.GetActiveSceneMutable()->UpdateEnd(DeltaTime);

        //std::cout << "Player Pos: " << player.TryGetComponent<ECS::Transform>()->m_Pos.ToString() << std::endl;s

        //We need to reset to default since previous changes were baked into the buffer
        //so we need to clear it for a fresh update
        //for (const auto& layer : RenderLayers) layer->ResetToDefault();
        
        //RenderPlayerLight(textBuffer, player.m_PlayerPos, 5);
        //player.UpdateStart(DeltaTime);
        //Utils::Point2DInt half = { playerLayer.m_TextBuffer.m_HEIGHT / 2,  playerLayer.m_TextBuffer.m_WIDTH / 2 };
       /* Utils::Log(std::format("CHecking from main: {}", Utils::ToString(playerLayer.m_TextBuffer.GetAt(half)->m_Char)));
        std::cout << "PLAYER LAYER: " << playerLayer.ToString() << std::endl;*/

        //RenderBuffer(CollapseLayersToSingle(RenderLayers));

        //player.UpdateEnd(DeltaTime);
        LastTime = CurrentTime;

       /* std::string layerStr = "";
        for (const auto& layer : RenderLayers)
        {
            layerStr += "\nLAYER:\n";
            layerStr += layer->ToString();
        }
        std::cout << "ALL LAYERS: " << layerStr << std::endl;
        return 0;*/
       
       /* if (IsKeyPressed(KEY_DOWN))
        {
            player.m_PlayerPos.m_Y++;
        }
        else if (IsKeyPressed(KEY_UP))
        {
            player.m_PlayerPos.m_Y--;
        }
        else if (IsKeyPressed(KEY_RIGHT))
        {
            player.m_PlayerPos.m_X++;
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            player.m_PlayerPos.m_X--;
        }*/
        if (FRAME_LIMIT == -1) continue;

        currentFrameCounter++;
        if (currentFrameCounter>= FRAME_LIMIT)
            return 0;
    }

    CloseWindow();
    return 0;
}
