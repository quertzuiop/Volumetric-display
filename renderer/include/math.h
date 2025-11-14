#pragma once
#include"types.h"

inline Vec3 operator-(const Vec3& a, const Vec3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
inline Vec3 operator+(const Vec3& a, const Vec3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}
inline Vec3 operator-(const Vec3& v, float d) { // subtract vector (d, d, d)
    return { v.x - d, v.y - d, v.z - d };
}
inline Vec3 operator+(const Vec3& v, float d) { // add vector (d, d, d)
    return { v.x + d, v.y + d, v.z + d };
}
inline Vec3 operator*(const Vec3& v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}

inline float mnhtDist(const Vec3& p1, const Vec3& p2) {
    return abs(p1.x - p2.x) + abs(p1.y - p2.y) + abs(p1.z - p2.z);
}

inline float dist(const Vec3& p1, const Vec3& p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}

inline float dist2(const Vec3& p1, const Vec3& p2) {
    return pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2);
}

inline float dot(const Vec3& v1, const Vec3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vec3 cross(const Vec3& v1, const Vec3& v2) {
    return {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x,
    };
}

template <typename T> inline int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

inline float magnitude_2(const Vec3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline Vec3 transform(const Vec3& pt, float dx = 0, float dy = 0, float dz = 0, float scale = 1.) {
    return { pt.x * scale + dx, pt.y * scale + dy, pt.z * scale + dz };
}

inline Point transform(const Point& pt, float dx = 0, float dy = 0, float dz = 0, float scale = 1.) {
    return { transform(pt.first, dx, dy, dz, scale), pt.second };
}

std::tuple<Vec3, Vec3> arrangeBoundingBox(const Vec3& p1, const Vec3& p2);
