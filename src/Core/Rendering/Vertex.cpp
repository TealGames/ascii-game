#include "Core/Rendering/Vertex.hpp"
#include "Utils/MathAdvanced.hpp"

namespace Rendering
{
    Vertex::Vertex() : Vertex({}, {}, {}) {}
    Vertex::Vertex(const WorldPosition3D& localPos, const UV& uvPos, const Vec3& normal) :
        m_LocalPos(localPos), m_UVPos(uvPos), m_Normal(normal), _padding0(0), _padding1{}, _padding2(0) {}

    std::string Vertex::ToString() const
    {
        return std::format("[{}]", m_LocalPos.ToString());
    }

    AABB3D CalculateTriangleAABB(const Triangle& triangle, const Vertex* vertexArray)
    {
        const WorldPosition3D world0 = vertexArray[triangle.m_VertexIndex0].m_LocalPos;
        const WorldPosition3D world1 = vertexArray[triangle.m_VertexIndex1].m_LocalPos;
        const WorldPosition3D world2 = vertexArray[triangle.m_VertexIndex2].m_LocalPos;
        const AABB3D aabb = AABB3D(Min(world0, world1, world2), Max(world0, world1, world2));

        if (!Utils::IsWithinBounds(aabb, world0) || !Utils::IsWithinBounds(aabb, world1) || !Utils::IsWithinBounds(aabb, world2))
        {
            LogError(std::format("Given v0:{} v1:{} v2:{} -> aabb:{}",
                world0.ToString(), world1.ToString(), world2.ToString(), aabb.ToString()));
        }

        return aabb;
    }
    WorldPosition3D CalculateTriangleCenter(const Triangle& triangle, const Vertex* vertexArray)
    {
        const WorldPosition3D world0 = vertexArray[triangle.m_VertexIndex0].m_LocalPos;
        const WorldPosition3D world1 = vertexArray[triangle.m_VertexIndex1].m_LocalPos;
        const WorldPosition3D world2 = vertexArray[triangle.m_VertexIndex2].m_LocalPos;
        return (world0 + world1 + world2) / 3;
    }

   /* bool Triangle::IsIntersectedByRay(const WorldPosition3D& rayOrigin, const WorldPosition3D& rayDir)
    {
        return Utils::RayIntersectsTriangle(m_Vertex0.m_LocalPos, m_Vertex1.m_LocalPos, m_Vertex2.m_LocalPos, rayOrigin, rayDir);
    }
    WorldPosition3D Triangle::GetCenter() const
    {
        return (m_Vertex0.m_LocalPos + m_Vertex1.m_LocalPos + m_Vertex2.m_LocalPos) / 3;
    }
    AABB3D Triangle::GetBounds() const
    {
        return AABB3D(Min(m_Vertex0.m_LocalPos, m_Vertex1.m_LocalPos, m_Vertex2.m_LocalPos), 
                      Max(m_Vertex0.m_LocalPos, m_Vertex1.m_LocalPos, m_Vertex2.m_LocalPos));
    }*/

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
}