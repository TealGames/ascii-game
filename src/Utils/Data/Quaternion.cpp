#include "Utils/Data/Quaternion.hpp"
#include <format>
#include <numbers>

Quat::Quat(const Vec3& radianEulerAngles) : m_X(), m_Y(), m_Z(), m_W() { FromRadianEulerAngles(*this, radianEulerAngles); }
Quat::Quat(const Mat3& matrix) : m_X(), m_Y(), m_Z(), m_W() { FromRotationMatrix(*this, matrix); }

Vec4 Quat::AsVec4() const { return Vec4(m_X, m_Y, m_Z, m_W); }
const float* Quat::GetMemPointer() const { return &m_X; }

Quat Quat::GetNormalized() const 
{
    const float length = std::sqrt(m_X * m_X + m_Y * m_Y + m_Z * m_Z + m_W * m_W);
    if (Utils::ApproximateEqualsF(length, 0)) return Quat(0, 0, 0, 1);

    return Quat(m_X / length, m_Y / length, m_Z / length, m_W / length);
}
void Quat::Normalize()
{
    const float length = std::sqrt(m_X * m_X + m_Y * m_Y + m_Z * m_Z + m_W * m_W);
    if (Utils::ApproximateEqualsF(length, 0))
    {
        m_X = 0;
        m_Y = 0;
        m_Z = 0;
        m_W = 1;
        return;
    }

    m_X /= length;
    m_Y /= length;
    m_Z /= length;
    m_W /= length;
}

Vec3 Quat::ToRadians() const
{
    //Note: ROLL(X) PITCH (y), YAW(Z)
    //Note: this method is th Tait-Bryan angle sequence (meaning we apply rotation in ZYX order)
    //and is most common method used to convert between Quat to euler. Most important is
    //to stick with it everywhere but sequence does not really matter
    const double sinr_cosp = 2 * (m_W * m_X + m_Y * m_Z);
    const double cosr_cosp = 1 - 2 * (m_X * m_X + m_Y * m_Y);
    const double roll = std::atan2(sinr_cosp, cosr_cosp);

    const double sinp = 2 * (m_W * m_Y - m_Z * m_X);
    double pitch = 0;
    if (std::abs(sinp) >= 1)
        pitch = std::copysign(std::numbers::pi / 2, sinp);
    else
        pitch = std::asin(sinp);

    const double siny_cosp = 2 * (m_W * m_Z + m_X * m_Y);
    const double cosy_cosp = 1 - 2 * (m_Y * m_Y + m_Z * m_Z);
    const double yaw = std::atan2(siny_cosp, cosy_cosp);

    return Vec3(roll, pitch, yaw);
}
Vec3 Quat::ToDegrees() const
{
    return ToRadians() * Utils::RAD_TO_DEG_CONSTANT;
}

void Quat::SetAsRadians(const Vec3& radianEulerAngle)
{
    FromRadianEulerAngles(*this, radianEulerAngle);
}
void Quat::SetAsDegrees(const Vec3& degreeEulerAngle)
{
    SetAsRadians(degreeEulerAngle * Utils::DEG_TO_RAD_CONSTANT);
}

Vec3 Quat::ApplyRotationToDir(const Vec3& v) const
{
    const Vec3 u(m_X, m_Y, m_Z);
    return u * 2.0f * DotProduct(u, v) + v * (m_W * m_W - DotProduct(u, u)) + CrossProduct(u, v) * m_W * 2.0f;
}

Quat Quat::operator*(const Quat& other) const
{
    return Quat
    (
        //X, Y, Z, W 
        m_W * other.m_X + m_X * other.m_W + m_Y * other.m_Z - m_Z * other.m_Y,
        m_W * other.m_Y - m_X * other.m_Z + m_Y * other.m_W + m_Z * other.m_X,
        m_W * other.m_Z + m_X * other.m_Y - m_Y * other.m_X + m_Z * other.m_W,
        m_W * other.m_W - m_X * other.m_X - m_Y * other.m_Y - m_Z * other.m_Z
    );
}
Quat& Quat::operator*=(const Quat& other)
{
    if (&other != this)
    {
        *this = *this * other;
    }
    return *this;
}

Quat Quat::operator*(const Vec3& radianEulerAngle) const
{
    return *this * ToQuaternion(radianEulerAngle);
}
Quat& Quat::operator*=(const Vec3& radianEulerAngle)
{
    *this = *this * ToQuaternion(radianEulerAngle);
    return *this;
}

std::string Quat::ToString(const AngleType angleType) const
{
    if (angleType == AngleType::Euler) return ToRadians().ToString();
    return std::format("({},{},{},{})", m_X, m_Y, m_Z, m_W);
}

void FromRadianEulerAngles(Quat& q, const Vec3& radianEulerAngles)
{
    const double cy = std::cos(radianEulerAngles.m_Z * 0.5);
    const double sy = std::sin(radianEulerAngles.m_Z * 0.5);
    const double cp = std::cos(radianEulerAngles.m_Y * 0.5);
    const double sp = std::sin(radianEulerAngles.m_Y * 0.5);
    const double cr = std::cos(radianEulerAngles.m_X * 0.5);
    const double sr = std::sin(radianEulerAngles.m_X * 0.5);

    q.m_W = cr * cp * cy + sr * sp * sy;
    q.m_X = sr * cp * cy - cr * sp * sy;
    q.m_Y = cr * sp * cy + sr * cp * sy;
    q.m_Z = cr * cp * sy - sr * sp * cy;
}
void FromRotationMatrix(Quat& q, const Mat3& m)
{
    ENGINE_ASSERT(m.GetColVector(0).IsUnitVector(), 
        "Failed to set quaternion from rotation matrix because col 0 is not normalized:", m.ToString());
    ENGINE_ASSERT(m.GetColVector(1).IsUnitVector(),
        "Failed to set quaternion from rotation matrix because col 1 is not normalized:", m.ToString());
    ENGINE_ASSERT(m.GetColVector(2).IsUnitVector(),
        "Failed to set quaternion from rotation matrix because col 2 is not normalized:", m.ToString());

    float trace = m.GetUnsafe(0, 0) + m.GetUnsafe(1, 1) + m.GetUnsafe(2, 2);

    if (trace > 0.0f)
    {
        float s = sqrt(trace + 1.0f) * 2.0f;
        q.m_W = 0.25f * s;
        q.m_X = (m.GetUnsafe(2, 1) - m.GetUnsafe(1, 2)) / s;
        q.m_Y = (m.GetUnsafe(0, 2) - m.GetUnsafe(2, 0)) / s;
        q.m_Z = (m.GetUnsafe(1, 0) - m.GetUnsafe(0, 1)) / s;
    }
    else if (m.GetUnsafe(0, 0) > m.GetUnsafe(1, 1) && m.GetUnsafe(0, 0) > m.GetUnsafe(2, 2))
    {
        float s = sqrt(1.0f + m.GetUnsafe(0, 0) - m.GetUnsafe(1, 1) - m.GetUnsafe(2, 2)) * 2.0f;
        q.m_W = (m.GetUnsafe(2, 1) - m.GetUnsafe(1, 2)) / s;
        q.m_X = 0.25f * s;
        q.m_Y = (m.GetUnsafe(0, 1) + m.GetUnsafe(1, 0)) / s;
        q.m_Z = (m.GetUnsafe(0, 2) + m.GetUnsafe(2, 0)) / s;
    }
    else if (m.GetUnsafe(1, 1) > m.GetUnsafe(2, 2))
    {
        float s = sqrt(1.0f + m.GetUnsafe(1, 1) - m.GetUnsafe(0, 0) - m.GetUnsafe(2, 2)) * 2.0f;
        q.m_W = (m.GetUnsafe(0, 2) - m.GetUnsafe(2, 0)) / s;
        q.m_X = (m.GetUnsafe(0, 1) + m.GetUnsafe(1, 0)) / s;
        q.m_Y = 0.25f * s;
        q.m_Z = (m.GetUnsafe(1, 2) + m.GetUnsafe(2, 1)) / s;
    }
    else
    {
        float s = sqrt(1.0f + m.GetUnsafe(2, 2) - m.GetUnsafe(0, 0) - m.GetUnsafe(1, 1)) * 2.0f;
        q.m_W = (m.GetUnsafe(1, 0) - m.GetUnsafe(0, 1)) / s;
        q.m_X = (m.GetUnsafe(0, 2) + m.GetUnsafe(2, 0)) / s;
        q.m_Y = (m.GetUnsafe(1, 2) + m.GetUnsafe(2, 1)) / s;
        q.m_Z = 0.25f * s;
    }

    q = q.GetNormalized();
}
void FromAxisAngle(Quat& q, const Vec3& axis, const float radianRotation)
{
    const Vec3 normalizedAxis = axis.GetNormalized();
    const float halfAngle = radianRotation * 0.5f;
    const float s = std::sin(halfAngle);
    const float c = std::cos(halfAngle);
    q.m_X = normalizedAxis.m_X * s;
    q.m_Y = normalizedAxis.m_Y * s; 
    q.m_Z = normalizedAxis.m_Z * s;
    q.m_W = c;
}

Quat ToQuaternion(const Vec3& radianEulerAngles)
{
    Quat q = {};
    FromRadianEulerAngles(q, radianEulerAngles);
    return q;
}
Quat ToQuaternion(const Mat3& matrix)
{
    Quat q = {};
    FromRotationMatrix(q, matrix);
    return q;
}
Quat ToQuaternion(const Vec3& axis, const float radianRotation)
{
    Quat q = {};
    FromAxisAngle(q, axis, radianRotation);
    return q;
}