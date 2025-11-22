#pragma once
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/Data/Matrix.hpp"
#include "Utils/Data/Color.hpp"
#include "Utils/Data/AABB.hpp"

namespace Rendering
{
    struct Vertex
    {
        /// <summary>
        /// The local position of the vertex relative to object's center
        /// NOTE: it is importat this is local pos since the transformation
        /// into global pos is done in shader
        /// </summary>
        WorldPosition3D m_LocalPos = {};
        float _padding0;
        /// <summary>
        /// The texture position that corresponds to this vertex
        /// </summary>
        UV m_UVPos = {};
        float _padding1[2];
        /// <summary>
        /// The unit vector that points in direction 
        /// that is perpendicular to object surface
        /// </summary>
        Vec3 m_Normal = {};
        float _padding2;

        Vertex();
        Vertex(const WorldPosition3D& localPos, const UV& uvPos, const Vec3& normal);

        std::string ToString() const;
    };
    
    struct Triangle
    {
        Vertex m_Vertex0;
        Vertex m_Vertex1;
        Vertex m_Vertex2;

        bool IsIntersectedByRay(const WorldPosition3D& rayOrigin, const WorldPosition3D& rayDir);
        WorldPosition3D GetCenter() const;

        AABB3D GetBounds() const;
    };

    struct Instance
    {
        //Color m_Color;
        std::uint32_t m_MaterialIndex;
        float _padding[3];
        Mat4 m_ModelMatrix;
        //NOTE: we use a mat3x4 (rows x cols) even though we only need mat3 because
        //due to std430 alignment, vec3 needs 4 bytes extra padding
        //so to avoid akward splitting we use mat3x4
        Mat3x4 m_NormalModelMatrix;

        Instance();
        Instance(const std::uint32_t materialIndex, const Mat4& modelMatrix, const Mat3& normalMatrix);
        std::string ToString() const;
    };

    struct InstanceMesh
    {
        uint32_t m_IndexOffset = 0;
        //The total number of indices to read for vertices
        uint32_t m_NumIndices = 0;

        std::string ToString() const;
    };

    using IndexType = std::uint32_t;
}