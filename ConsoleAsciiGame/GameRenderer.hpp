#pragma once
#include <optional>
#include "TextBuffer.hpp"
#include "Point2DInt.hpp"
#include "ColliderOutlineBuffer.hpp"
#include "LineBuffer.hpp"
#include "DebugInfo.hpp"
#include "CommandConsole.hpp"
#include "EntityEditorGUI.hpp"

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

    void RenderBuffer(const TextBufferMixed& buffer, const RenderInfo& renderInfo);
    void RenderBuffer(const TextBufferMixed& layerBuffers, const ColliderOutlineBuffer* outlineBuffer= nullptr, 
        const LineBuffer* lineBuffer=nullptr, const DebugInfo* debugInfo=nullptr, CommandConsole* console=nullptr, 
        EntityEditorGUI* editor=nullptr);
}