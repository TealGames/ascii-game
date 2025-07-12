#pragma once
#include <optional>
#include <cstdint>
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

        void AddCircleCall(const ScreenPosition& pos, const float radius, const Color color);
        void AddRectangleCall(const ScreenPosition& pos, const Vec2& size, const Color color);
        void AddTextureCall(const ScreenPosition& pos, const Texture& tex, const float rotation, const Vec2 scale, const Color color);
        void AddTextCall(const ScreenPosition& pos, const Font& font, const char* text, const float size, const float spacing, const Color color);

        void AddLineCall(const ScreenPosition& pos, const float thickness, const Vec2& length, const Color color);
        void AddRectangleLineCall(const ScreenPosition& pos, const float thickness, const Vec2& size, const Color color);

        void PushCallsToBuffer(const std::vector<RenderCall>& calls);
        void MoveCallsToBuffer(std::vector<RenderCall>& calls);
        void RenderBuffer();
        void ClearBuffer();
    };
}