#include "Utils/Data/Quaternion.hpp"
#include <format>
#include <numbers>

Vec3 Quat::ToEulerAngles() const
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

Quat Quat::ToQuaternion(const Vec3& radianEulerAngles)
{
    const double cy = std::cos(radianEulerAngles.m_Z * 0.5);
    const double sy = std::sin(radianEulerAngles.m_Z * 0.5);
    const double cp = std::cos(radianEulerAngles.m_Y * 0.5);
    const double sp = std::sin(radianEulerAngles.m_Y * 0.5);
    const double cr = std::cos(radianEulerAngles.m_X * 0.5);
    const double sr = std::sin(radianEulerAngles.m_X * 0.5);

    Quat q = {};
    q.m_W = cr * cp * cy + sr * sp * sy;
    q.m_X = sr * cp * cy - cr * sp * sy;
    q.m_Y = cr * sp * cy + sr * cp * sy;
    q.m_Z = cr * cp * sy - sr * sp * cy;
    return q;
}
void Quat::SetToEulerAngle(const Vec3& radianEulerAngle)
{
    *this = ToQuaternion(radianEulerAngle);
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
        m_W*other.m_X + m_X*other.m_W + m_Y*other.m_Z - m_Z*other.m_Y,
        m_W*other.m_Y - m_X*other.m_Z + m_Y*other.m_W + m_Z*other.m_X,
        m_W*other.m_Z + m_X*other.m_Y - m_Y*other.m_X + m_Z*other.m_W,
        m_W*other.m_W - m_X*other.m_X - m_Y*other.m_Y - m_Z*other.m_Z
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

std::string Quat::ToString(const AngleType angleType) const
{
    if (angleType == AngleType::Euler) return ToEulerAngles().ToString();
    return std::format("({},{},{},{})", m_X, m_Y, m_Z, m_W);
}