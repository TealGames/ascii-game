#pragma once
#include "Core/Rendering/RenderCall.hpp"

class UIHierarchy;
class DebugInfo;
class CommandConsole;
class EntityEditorUI;

namespace Rendering
{
    class Renderer
    {
    private:
    public:

    private:
    public:
        Renderer();
        void InitBackend();

        void AddCircleCall(const ScreenPosition& pos, const float radius, const Utils::Color color);
        void AddRectangleCall(const ScreenPosition& pos, const Vec2& size, const Utils::Color color);
        void AddTextureCall(const ScreenPosition& pos, const Texture& tex, const float rotation, const Vec2 scale, const Utils::Color color);
        void AddTextCall(const ScreenPosition& pos, const Font& font, const char* text, const float size, const float spacing, const Utils::Color color);

        void AddLineCall(const ScreenPosition& pos, const float thickness, const Vec2& length, const Utils::Color color);
        void AddRectangleLineCall(const ScreenPosition& pos, const float thickness, const Vec2& size, const Utils::Color color);

        void PushCallsToBuffer(const std::vector<RenderCall>& calls);
        void MoveCallsToBuffer(std::vector<RenderCall>& calls);
        void RenderBuffer();
        void ClearCommandBuffers();
    };
}