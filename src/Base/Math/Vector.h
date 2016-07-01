#pragma once
#include "Base/Algorithm.h"
#include <stdint.h>

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4201)

/*
===============================================================================
Vec2 - 2D Vector Template
===============================================================================
*/
template <typename T>
struct Vec2
{
    union
    {
        struct
        {
            T x, y;
        };
        struct
        {
            T u, v;
        };

        T valuePtr[2];
    };

    Vec2();
    explicit Vec2(const T x, const T y);

    Vec2 operator-() const;
    Vec2 operator*(const float a) const;
    Vec2 operator+(const Vec2 &a) const;
    Vec2 operator-(const Vec2 &a) const;
    Vec2 &operator+=(const Vec2 &a);
    Vec2 &operator-=(const Vec2 &a);
    Vec2 &operator*=(const float a);

    float LengthSqr() const;
    float Length() const;
    void Clamp(const Vec2 &min, const Vec2 &max);
    void Saturate();
};

typedef Vec2<float> Vec2f;
typedef Vec2<int32_t> Vec2i;
typedef Vec2<uint32_t> Vec2ui;

template <typename T>
inline Vec2<T> operator*(float a, const Vec2<T> &b)
{
    Vec2<T> result = b*a;
    return result;
}

template <typename T>
Vec2<T>::Vec2()
{
}

template <typename T>
Vec2<T>::Vec2(const T x, const T y)
{
    this->x = x;
    this->y = y;
}

template <typename T>
inline Vec2<T> Vec2<T>::operator-() const
{
    return Vec2(-x, -y);
}

template <typename T>
inline Vec2<T> Vec2<T>::operator*(const float a) const
{
    return Vec2(x * a, y * a);
}

template <typename T>
inline Vec2<T> Vec2<T>::operator+(const Vec2 &a) const
{
    return Vec2(x + a.x, y + a.y);
}

template <typename T>
inline Vec2<T> Vec2<T>::operator-(const Vec2 &a) const
{
    return Vec2(x - a.x, y - a.y);
}

template <typename T>
inline Vec2<T> &Vec2<T>::operator+=(const Vec2 &a)
{
    x += a.x;
    y += a.y;
    return *this;
}

template <typename T>
inline Vec2<T> &Vec2<T>::operator-=(const Vec2 &a)
{
    x -= a.x;
    y -= a.y;
    return *this;
}

template <typename T>
inline Vec2<T> &Vec2<T>::operator*=(const float a)
{
    x *= a;
    y *= a;
    return *this;
}

template <typename T>
inline float Vec2<T>::LengthSqr() const
{
    return x*x + y*y;
}

template <typename T>
inline float Vec2<T>::Length() const
{
    float sqrLength = LengthSqr();
    return sqrt(sqrLength);
}

template <typename T>
inline void Vec2<T>::Clamp(const Vec2 &min, const Vec2 &max)
{
    x = Clamp(x, min.x, max.x);
    y = Clamp(y, min.y, max.y);
}

template <typename T>
inline void Vec2<T>::Saturate()
{
    x = Saturate(x);
    y = Saturate(y);
}

template <typename T>
inline Vec2<T> Normalize(const Vec2<T> &a)
{
    Vec2<T> result = a * (1.0f / a.Length());
    return result;
}

/*
===============================================================================
Vec3 - 3D Vector Template
===============================================================================
*/
template <typename T>
struct Vec3
{
    union
    {
        struct
        {
            T x, y, z;
        };
        struct
        {
            T r, g, b;
        };
        struct
        {
            T u, v, w;
        };
        struct
        {
            Vec2<T> xy;
            T _ignored0;
        };
        struct
        {
            Vec2<T> uv;
            T _ignored1;
        };

        T valuePtr[3];
    };

    Vec3();
    explicit Vec3(const T x, const T y, const T z);

    Vec3 operator-() const;
    Vec3 operator*(const float a) const;
    Vec3 operator+(const Vec3 &a) const;
    Vec3 operator-(const Vec3 &a) const;
    Vec3 &operator+=(const Vec3 &a);
    Vec3 &operator-=(const Vec3 &a);
    Vec3 &operator*=(const float a);

    float LengthSqr() const;
    float Length() const;
    void Clamp(const Vec3 &min, const Vec3 &max);
    void Saturate();
};

typedef Vec3<float> Vec3f;
typedef Vec3<int32_t> Vec3i;
typedef Vec3<uint32_t> Vec3ui;

template <typename T>
inline Vec3<T> operator*(float a, const Vec3<T> &b)
{
    Vec3<T> result = b*a;
    return result;
}

template <typename T> 
Vec3<T>::Vec3()
{
}

template <typename T>
Vec3<T>::Vec3(const T x, const T y, const T z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

template <typename T>
inline Vec3<T> Vec3<T>::operator-() const
{
    return Vec3(-x, -y, -z);
}

template <typename T>
inline Vec3<T> Vec3<T>::operator*(const float a) const
{
    return Vec3(x * a, y * a, z * a);
}

template <typename T>
inline Vec3<T> Vec3<T>::operator+(const Vec3 &a) const
{
    return Vec3(x + a.x, y + a.y, z * a.z);
}

template <typename T>
inline Vec3<T> Vec3<T>::operator-(const Vec3 &a) const
{
    return Vec3(x - a.x, y - a.y, z - a.z);
}

template <typename T>
inline Vec3<T> &Vec3<T>::operator+=(const Vec3 &a)
{
    x += a.x;
    y += a.y;
    z += a.z;
    return *this;
}

template <typename T>
inline Vec3<T> &Vec3<T>::operator-=(const Vec3 &a)
{
    x -= a.x;
    y -= a.y;
    z -= a.z;
    return *this;
}

template <typename T>
inline Vec3<T> &Vec3<T>::operator*=(const float a)
{
    x *= a;
    y *= a;
    z *= a;
    return *this;
}

template <typename T>
inline float Vec3<T>::LengthSqr() const
{
    return x*x + y*y + z*z;
}

template <typename T>
inline float Vec3<T>::Length() const
{
    float sqrLength = LengthSqr();
    return sqrt(sqrLength);
}

template <typename T>
inline void Vec3<T>::Clamp(const Vec3 &min, const Vec3 &max)
{
    x = Clamp(x, min.x, max.x);
    y = Clamp(y, min.y, max.y);
    z = Clamp(z, min.z, max.z);
}

template <typename T>
inline void Vec3<T>::Saturate()
{
    x = Saturate(x);
    y = Saturate(y);
    z = Saturate(z);
}

template <typename T>
inline Vec3<T> Normalize(const Vec3<T> &a)
{
    Vec3<T> result = a * (1.0f / a.Length());
    return result;
}

template <typename T>
inline Vec3<T> CrossProduct(const Vec3<T> &a, const Vec3<T> &b)
{
    return Vec3<T>(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

/*
===============================================================================
Vec4 - 4D Vector Template
===============================================================================
*/
template <typename T>
struct Vec4
{
    union
    {
        struct
        {
            T x, y, z, w;
        };
        struct
        {
            T r, g, b, a;
        };
        struct
        {
            Vec3<T> xyz;
            T _ignored0;
        };
        struct
        {
            Vec3<T> rgb;
            T _ignored1;
        };

        T valuePtr[4];
    };

    Vec4();
    explicit Vec4(const T x, const T y, const T z, const T w);

    Vec4 operator-() const;
    Vec4 operator*(const float a) const;
    Vec4 operator+(const Vec4 &a) const;
    Vec4 operator-(const Vec4 &a) const;
    Vec4 &operator+=(const Vec4 &a);
    Vec4 &operator-=(const Vec4 &a);
    Vec4 &operator*=(const float a);

    float LengthSqr() const;
    float Length() const;
    void Clamp(const Vec4 &min, const Vec4 &max);
    void Saturate();
};

typedef Vec4<float> Vec4f;
typedef Vec4<int32_t> Vec4i;
typedef Vec4<uint32_t> Vec4ui;

template <typename T>
inline Vec4<T> operator*(float a, const Vec4<T> &b)
{
    Vec4<T> result = b*a;
    return result;
}

template <typename T>
Vec4<T>::Vec4()
{
}

template <typename T>
Vec4<T>::Vec4(const T x, const T y, const T z, const T w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

template <typename T>
inline Vec4<T> Vec4<T>::operator-() const
{
    return Vec4(-x, -y, -z, -w);
}

template <typename T>
inline Vec4<T> Vec4<T>::operator*(const float scale) const
{
    return Vec4(x * scale, y * scale, z * scale, w * scale);
}

template <typename T>
inline Vec4<T> Vec4<T>::operator+(const Vec4 &rhs) const
{
    return Vec4(x + rhs.x, y + rhs.y, z * rhs.z, w * rhs.w);
}

template <typename T>
inline Vec4<T> Vec4<T>::operator-(const Vec4 &rhs) const
{
    return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

template <typename T>
inline Vec4<T> &Vec4<T>::operator+=(const Vec4 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

template <typename T>
inline Vec4<T> &Vec4<T>::operator-=(const Vec4 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

template <typename T>
inline Vec4<T> &Vec4<T>::operator*=(const float rhs)
{
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
    return *this;
}

template <typename T>
inline float Vec4<T>::LengthSqr() const
{
    return x*x + y*y + z*z + w*w;
}

template <typename T>
inline float Vec4<T>::Length() const
{
    float sqrLength = LengthSqr();
    return sqrt(sqrLength);
}

template <typename T>
inline void Vec4<T>::Clamp(const Vec4 &min, const Vec4 &max)
{
    x = Clamp(x, min.x, max.x);
    y = Clamp(y, min.y, max.y);
    z = Clamp(z, min.z, max.z);
    w = Clamp(w, min.w, max.w);
}

template <typename T>
inline void Vec4<T>::Saturate()
{
    x = Saturate(x);
    y = Saturate(y);
    z = Saturate(z);
    w = Saturate(w);
}

template <typename T>
inline Vec4<T> Normalize(const Vec4<T> &a)
{
    Vec4<T> result = a * (1.0f / a.Length());
    return result;
}