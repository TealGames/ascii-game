#include "Core/Rendering/RenderUnit.hpp"


namespace Rendering
{
    std::string RenderBatch::ToString() const
    {
        return std::format("[Batch Shader:{} Texture:{} VertexStart:{} VertexCount:{} "
            "IndexStart:{} IndexCount:{} InstancesStart:{} InstancesCount:{}]",
            m_Shader != nullptr, m_Texture != nullptr, m_VertexStartIndex, m_VertexCount,
            m_IndicesStartIndex, m_IndicesCount, m_InstanceStartIndex, m_InstanceCount);
    }
}