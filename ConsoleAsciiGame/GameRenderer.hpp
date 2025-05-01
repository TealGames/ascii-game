#pragma once
#include <optional>
#include <cstdint>
#include "Point2DInt.hpp"
#include "TextBuffer.hpp"
#include "LineBuffer.hpp"
#include "ColliderOutlineBuffer.hpp"
#include "IBasicRenderable.hpp"

class DebugInfo;
class CommandConsole;
class EntityEditorGUI;

namespace Rendering
{
    struct RenderInfo
    {
        std::optional<Utils::Point2DInt> m_CharSpacing;
        Utils::Point2DInt m_Padding;
        Utils::Point2DInt m_ScreenSize;
        std::uint16_t m_FontSize;
        bool m_CenterBuffer;
    };
    void RenderBuffer(const FragmentedTextBuffer* layerBuffers, const ColliderOutlineBuffer* outlineBuffer= nullptr,
        const LineBuffer* lineBuffer = nullptr, const std::vector<IBasicRenderable*>& renderables = {});
}