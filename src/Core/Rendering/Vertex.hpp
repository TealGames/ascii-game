#pragma once
#include "Utils/Math/WorldPosition.hpp"
#include "Utils/Data/Color.hpp"
#include "Utils/Data/MemoryInterval.hpp"
#include "Utils/Math/MathAdvanced.hpp"

namespace Rendering
{
    enum class PrimitiveType : std::uint8_t
    {
        Triangle    = 0,
        Square      = 1
    };

    enum class BasicMeshType : std::uint8_t
    {
        Cube                = 0,
        Sphere              = 1,
        Plane               = 2,
    };

    //---------------------------------------------------------------------------------------------
    //                          3D GEOMETRY VERTICES/INSTANCES
    //---------------------------------------------------------------------------------------------
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

    using IndexType = std::uint32_t;
    
    template<typename T>
    struct TriangleBase
    {
        T m_0;
        T m_1;
        T m_2;

        TriangleBase() : TriangleBase(0, 0, 0) {}
        TriangleBase(const T first, const T second, const T third)
            : m_0(first), m_1(second), m_2(third) {}
        TriangleBase(const T offset, const T* arr)
            : m_0(offset + *arr), m_1(offset + *(arr + 1)), m_2(offset + *(arr + 2)) {}
        TriangleBase(const T offset, const std::array<T, 3>& triangle)
            : m_0(offset + triangle[0]), m_1(offset + triangle[1]), m_2(offset + triangle[2]) {}
        TriangleBase(const T offset, const TriangleBase<T>& other)
            : m_0(offset + other.m_0), m_1(offset + other.m_1), m_2(offset + other.m_2) {}

        std::string ToString() const
        {
            return std::format("[Triangle 0:{} 1:{} 2:{}]", m_0, m_1, m_2);
        }
    };
    using IndexTriangle = TriangleBase<IndexType>;
    using Vec3Triangle = TriangleBase<Vec3>;

    template<size_t ALIGN = 0>
    AABB3DBase<ALIGN> CalculateTriangleAABB(const IndexTriangle& triangle, const Vertex* vertexArray)
    {
        const WorldPosition3D world0 = vertexArray[triangle.m_0].m_LocalPos;
        const WorldPosition3D world1 = vertexArray[triangle.m_1].m_LocalPos;
        const WorldPosition3D world2 = vertexArray[triangle.m_2].m_LocalPos;
        const AABB3DBase<ALIGN> aabb = AABB3D(Min(world0, world1, world2), Max(world0, world1, world2));

        ENGINE_ASSERT(Utils::IsWithinBounds(aabb, world0) && Utils::IsWithinBounds(aabb, world1) && Utils::IsWithinBounds(aabb, world2),
            "Attempted to calculate triangle AABB3d givne vertices: {}, {}, {} but some did not fit within bounds formed: {}",
            world0.ToString(), world1.ToString(), world2.ToString(), aabb.ToString());

        return aabb;
    }
    WorldPosition3D CalculateTriangleCenter(const IndexTriangle& triangle, const Vertex* vertexArray);

    using Mat3Std430 = MatrixType<float, 3, 3, MatrixMajorOrder::Column, sizeof(float) * 4>;
    struct Instance
    {
        //Color m_Color;
        std::uint32_t m_MaterialIndex;
        std::uint32_t m_MeshIndex;
        float _padding[2];
        Mat4 m_ModelMatrix;
        Mat4 m_InverseModelMatrix;
        //NOTE: we use a mat4x3 (4 rows, 3 cols) because we use COLUMN MAJOR STORAGE
        //(to not need transpose on OpenGL matrix upload) and since the NormalModelMatrix is 3x3
        //and OpenGL expects std::430 rules (flaot vec3 needs 4 byte padding), we add padding
        Mat3Std430 m_NormalModelMatrix;

        Instance();
        Instance(const std::uint32_t materialIndex, const std::uint32_t meshIndex, 
            const Mat4& modelMatrix, const Mat3& normalMatrix);
        std::string ToString() const;
    };

    struct InstanceMesh
    {
        uint32_t m_IndexOffset = 0;
        //The total number of indices to read for vertices
        uint32_t m_NumIndices = 0;

        ArrayInterval m_BLASTreesInterval = {};

        std::string ToString() const;
    };


    //---------------------------------------------------------------------------------------------
    //                          UI VERTICES/INSTANCES
    //---------------------------------------------------------------------------------------------
    struct VertexUI
    {
        Vec2 m_LocalRectPos;
        UV m_UVPos = {};

        VertexUI();
        VertexUI(const Vec2& pos, const UV& uv);
    };
    struct InstanceUI
    {
        HDRColor m_Color;
        int m_TextureIndex;
        float m_Depth;
        Mat3 m_ModelMatrix;

        InstanceUI();
        InstanceUI(const HDRColor& color, int textureIndex, const float depth, const Mat3& modelMatrix);
        std::string ToString() const;
    };
}