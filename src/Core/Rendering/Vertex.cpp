#include "Core/Rendering/Vertex.hpp"

namespace Rendering
{
    std::string Vertex::ToString() const
    {
        return std::format("[{}]", m_LocalPos.ToString());
    }
    std::string InstanceData::ToString() const
    {
        return std::format("[Color:{} ModelMatrix:{}]", m_Color.ToString(), m_ModelMatrix.ToString());
    }
}