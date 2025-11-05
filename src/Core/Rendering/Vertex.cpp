#include "Core/Rendering/Vertex.hpp"

namespace Rendering
{
    Vertex::Vertex() : Vertex({}, {}, {}) {}
    Vertex::Vertex(const WorldPosition3D& localPos, const UV& uvPos, const Vec3& normal) :
        m_LocalPos(localPos), m_UVPos(uvPos), m_Normal(normal), _padding0(0), _padding1{}, _padding2(0) {}

    std::string Vertex::ToString() const
    {
        return std::format("[{}]", m_LocalPos.ToString());
    }
    Instance::Instance() : Instance(-1, {}, {}) {}
    Instance::Instance(const std::uint32_t materialIndex, const Mat4& modelMatrix, const Mat3& normalMatrix)
        : m_MaterialIndex(materialIndex), m_ModelMatrix(modelMatrix), m_NormalModelMatrix(normalMatrix), _padding{} {}

    std::string Instance::ToString() const
    {
        return std::format("[Instance MaterialIndex:{} ModelMat:{} NormalModelMat:{}]", 
            m_MaterialIndex, m_ModelMatrix.ToString(), m_NormalModelMatrix.ToString());
    }
    std::string InstanceMesh::ToString() const
    {
        return std::format("[InstanceMesh Off:{} Count:{}]", m_IndexOffset, m_NumIndices);
    }
}