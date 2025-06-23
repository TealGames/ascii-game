#pragma once
#include <optional>
#include <cstdint>
#include "RenderCall.hpp"

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
        void AddRectangleCall(const ScreenPosition& pos, const Vec2Int& size, const Color color);
        void AddTextureCall(const ScreenPosition& pos, const Texture& tex, const float rotation, const float scale, const Color color);
        void AddTextCall(const ScreenPosition& pos, const Font& font, const char* text, const float size, const float spacing, const Color color);

        void RenderQueue();
    };
}