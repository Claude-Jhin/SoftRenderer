#pragma once
#include <ostream>
#include <vector>

template <class t>
struct Vec2
{
    union
    {
        struct
        {
            t u, v;
        };

        struct
        {
            t x, y;
        };

        t raw[2];
    };

    Vec2<t>() : u(t()), v(t())
    {
    }

    Vec2<t>(t _u, t _v): u(_u), v(_v)
    {
    }

    Vec2<t>(const Vec2<t>& other) : u(t()), v(t()) { *this = other; }

    Vec2<t>& operator=(const Vec2<t>& other)
    {
        if (this != &other)
        {
            u = other.u;
            v = other.v;
        }
        return *this;
    }

    inline Vec2<t> operator+(const Vec2<t>& other) const { return Vec2<t>(u + other.u, v + other.v); }
    inline Vec2<t> operator-(const Vec2<t>& other) const { return Vec2<t>(u - other.u, v - other.v); }
    inline Vec2<t> operator*(float f) const { return Vec2<t>(u * f, v * f); }
    inline t operator^(const Vec2<t>& other) const { return u * other.v - v * other.u; }
    inline t norm() const { return std::sqrt(x * x + y * y); }
    inline t& operator[](const int i) { return i <= 0 ? x : y; }

    template <class>
    friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t>
struct Vec3
{
    union
    {
        struct
        {
            t x, y, z;
        };

        struct
        {
            t ivert, iuv, inorm;
        };

        t raw[3];
    };

    Vec3(): x(t()), y(t()), z(t())
    {
    }

    Vec3(t _x, t _y, t _z): x(_x), y(_y), z(_z)
    {
    }

    template <class otherType>
    Vec3(const Vec3<otherType>& other);

    Vec3<t>(const Vec3<t>& other): x(t()), y(t()), z(t()) { *this = other; }

    Vec3<t>& operator=(const Vec3<t>& other)
    {
        if (this != &other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
        }

        return *this;
    }

    inline Vec3<t> operator^(const Vec3<t>& v) const
    {
        return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    inline Vec3<t> operator+(const Vec3<t>& v) const
    {
        return Vec3<t>(x + v.x, y + v.y, z + v.z);
    }

    inline Vec3<t> operator-(const Vec3<t>& v) const
    {
        return Vec3<t>(x - v.x, y - v.y, z - v.z);
    }

    inline Vec3<t> operator*(float f) const
    {
        return Vec3<t>(x * f, y * f, z * f);
    }

    inline t operator*(const Vec3<t>& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    float norm() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3<t>& normalize(t l = 1)
    {
        *this = (*this) * (l / norm());
        return *this;
    }

    t& operator[](const int i)
    {
        if (i <= 0)
        {
            return x;
        }
        else if (i == 1)
        {
            return y;
        }
        else
        {
            return z;
        }
    }

    template <class>
    friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};


typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

template <>
template <>
Vec3<int>::Vec3(const Vec3<float>& other);

template <>
template <>
Vec3<float>::Vec3(const Vec3<int>& other);

template <class t>
std::ostream& operator<<(std::ostream& s, Vec2<t>& v)
{
    s << "(" << v.x << ", " << v.y << ")\n";
    return s;
}

template <class t>
std::ostream& operator<<(std::ostream& s, Vec3<t>& v)
{
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}

const int DEFAULT_ALLOC = 4;

class Matrix
{
public:
    Matrix(int r = DEFAULT_ALLOC, int c = DEFAULT_ALLOC);
    inline int nrows();
    inline int ncols();

    static Matrix identity(int dimensions);
    std::vector<float>& operator[](const int i);
    Matrix operator*(const Matrix& a);
    Matrix transpose();
    Matrix inverse();

    friend std::ostream& operator<<(std::ostream& s, Matrix& m);
private:
    std::vector<std::vector<float>> m;
    int rows, cols;
};
