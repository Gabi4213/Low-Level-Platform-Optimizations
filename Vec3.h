#pragma once

#include <cmath>

class Vec3 
{
public:
    float x, y, z;

    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // overload the minus operator
    Vec3 operator-(const Vec3& other) const
    {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    // overload the multiply operator
    Vec3 operator*(float scalar) const
    {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    // overload the addition operator
    Vec3 operator+(const Vec3& other) const
    {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    // overload the divide operator
    Vec3 operator/(float scalar) const
    {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    // Normalize the vector
    void normalise()
    {
        float length = std::sqrt(x * x + y * y + z * z);
        if (length != 0) 
        {
            x /= length;
            y /= length;
            z /= length;
        }
    }

    // get the length of a vector
    float length() const 
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    float Distance(const Vec3& other) const 
    {
        return std::sqrt(std::pow(x - other.x, 2) +
            std::pow(y - other.y, 2) +
            std::pow(z - other.z, 2));
    }
};

