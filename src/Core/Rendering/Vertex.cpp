#include "Core/Rendering/Vertex.hpp"
#include "Utils/MathAdvanced.hpp"

namespace Rendering
{
    Vertex::Vertex() : Vertex({}, {}, {}) {}
    Vertex::Vertex(const WorldPosition3D& localPos, const UV& uvPos, const Vec3& normal) :
        m_LocalPos(localPos), m_UVPos(uvPos), m_Normal(normal), _padding0(0), _padding1{}, _padding2(0) {}

    std::string Vertex::ToString() const
    {
        return std::format("[Pos:{} Normal:{}]", m_LocalPos.ToString(), m_Normal.ToString());
    }

    AABB3D CalculateTriangleAABB(const Triangle& triangle, const Vertex* vertexArray)
    {
        const WorldPosition3D world0 = vertexArray[triangle.m_VertexIndex0].m_LocalPos;
        const WorldPosition3D world1 = vertexArray[triangle.m_VertexIndex1].m_LocalPos;
        const WorldPosition3D world2 = vertexArray[triangle.m_VertexIndex2].m_LocalPos;
        const AABB3D aabb = AABB3D(Min(world0, world1, world2), Max(world0, world1, world2));

        ENGINE_ASSERT(Utils::IsWithinBounds(aabb, world0) && Utils::IsWithinBounds(aabb, world1) && Utils::IsWithinBounds(aabb, world2), 
            "Attempted to calculate triangle AABB3d givne vertices: {}, {}, {} but some did not fit within bounds formed: {}", 
            world0.ToString(), world1.ToString(), world2.ToString(), aabb.ToString());

        return aabb;
    }
    WorldPosition3D CalculateTriangleCenter(const Triangle& triangle, const Vertex* vertexArray)
    {
        const WorldPosition3D world0 = vertexArray[triangle.m_VertexIndex0].m_LocalPos;
        const WorldPosition3D world1 = vertexArray[triangle.m_VertexIndex1].m_LocalPos;
        const WorldPosition3D world2 = vertexArray[triangle.m_VertexIndex2].m_LocalPos;
        return (world0 + world1 + world2) / 3;
    }

    Instance::Instance() : Instance(-1, -1, {}, {}) {}
    Instance::Instance(const std::uint32_t materialIndex, const std::uint32_t meshIndex, const Mat4& modelMatrix, const Mat3& normalMatrix)
        : m_MaterialIndex(materialIndex), m_MeshIndex(meshIndex), m_ModelMatrix(modelMatrix), 
          //NOTE: we use the utility function rather than Inverse() directly on Matrix since it is optimized for model matrices
          m_InverseModelMatrix(Utils::CalculateInverseModelMatrix(m_ModelMatrix)),
          m_NormalModelMatrix(normalMatrix), _padding{} {}

    std::string Instance::ToString() const
    {
        return std::format("[Instance MaterialIndex:{} ModelMat:{} NormalModelMat:{}]", 
            m_MaterialIndex, m_ModelMatrix.ToString(), m_NormalModelMatrix.ToString());
    }

    std::string InstanceMesh::ToString() const
    {
        return std::format("[InstanceMesh Off:{} Count:{}]", m_IndexOffset, m_NumIndices);
    }

    VertexUI::VertexUI() : VertexUI({}, {}) {}
    VertexUI::VertexUI(const Vec2& pos, const UV& uv) :
        m_LocalRectPos(pos), m_UVPos(uv) {}

    InstanceUI::InstanceUI() : InstanceUI(COLOR_WHITE, -1, 0, {}) {}
    InstanceUI::InstanceUI(const HDRColor& color, int textureIndex, const float depth, const Mat3& modelMatrix)
        : m_Color(color), m_TextureIndex(textureIndex), m_Depth(depth), m_ModelMatrix(modelMatrix) {}
    std::string InstanceUI::ToString() const
    {
        return std::format("[InstanceUI color:{} texIdx:{} depth:{} modelMat:{}]", 
            m_Color.ToString(), m_TextureIndex, m_Depth, m_ModelMatrix.ToString());
    }
}