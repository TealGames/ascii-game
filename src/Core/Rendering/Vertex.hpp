#pragma once
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/Data/Matrix.hpp"

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
        /// <summary>
        /// The texture position that corresponds to this vertex
        /// </summary>
        UV m_UVPos = {};
        /// <summary>
        /// The unit vector that points in direction 
        /// that is perpendicular to object surface
        /// </summary>
        Vec3 m_Normal = {};

        std::string ToString() const;
    };

    struct InstanceData
    {
        Vec4 m_Color;
        Mat4 m_ModelMatrix;
        Mat3 m_NormalModelMatrix;

        std::string ToString() const;
    };

    using IndexType = std::uint32_t;
}