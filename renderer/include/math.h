#pragma once
#include"types.h"
#include <cmath>
#include <algorithm>
#include <tuple>

template<typename T>
inline Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
template<typename T>
inline Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}
template<typename A, typename B>
inline Vec3<A> operator+(const Vec3<A>& a, const Vec3<B>& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}
template<typename T>
inline Vec3<T> operator-(const Vec3<T>& v, float d) { // subtract vector (d, d, d)
    return { v.x - d, v.y - d, v.z - d };
}
template<typename T>
inline Vec3<T> operator+(const Vec3<T>& v, float d) { // add vector (d, d, d)
    return { v.x + d, v.y + d, v.z + d };
}
template<typename T>
inline Vec3<T> operator*(const Vec3<T>& v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}
template<typename T>
inline T mnhtDist(const Vec3<T>& p1, const Vec3<T>& p2) {
    return abs(p1.x - p2.x) + abs(p1.y - p2.y) + abs(p1.z - p2.z);
}
template<typename T>
inline float dist(const Vec3<T>& p1, const Vec3<T>& p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}
template<typename T>
inline float dist2(const Vec3<T>& p1, const Vec3<T>& p2) {
    return pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2);
}
template<typename T>
inline T dot(const Vec3<T>& v1, const Vec3<T>& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
template<typename T>
inline Vec3<T> cross(const Vec3<T>& v1, const Vec3<T>& v2) {
    return {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x,
    };
}

template <typename T> inline int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}
template<typename T>
inline T magnitude_2(const Vec3<T>& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
inline Vec3<float> matColMul(const Mat4& matrix, Vec3<float> vec) {
    return {
        matrix[0][0] * vec.x + matrix[0][1] * vec.y + matrix[0][2] * vec.z + matrix[0][3],
        matrix[1][0] * vec.x + matrix[1][1] * vec.y + matrix[1][2] * vec.z + matrix[1][3],
        matrix[2][0] * vec.x + matrix[2][1] * vec.y + matrix[2][2] * vec.z + matrix[2][3],
    };
}
Mat4 matMul(const Mat4& mat1, const Mat4& mat2);

inline void scale(Vec3<float>& pt, float factor, const Vec3<float>& hinge) {
    float relativeX = pt.x - hinge.x;
    float relativeY = pt.y - hinge.y;
    float relativeZ = pt.z - hinge.z;

    pt.x = (relativeX * factor) + hinge.x;
    pt.y = (relativeY * factor) + hinge.y;
    pt.z = (relativeZ * factor) + hinge.z;
}
template<typename T>
std::tuple<Vec3<T>, Vec3<T>> arrangeBoundingBox(const Vec3<T>& p1, const Vec3<T>& p2) {
    return { {
        std::min(p1.x, p2.x),
        std::min(p1.y, p2.y),
        std::min(p1.z, p2.z),
    }, {
        std::max(p1.x, p2.x),
        std::max(p1.y, p2.y),
        std::max(p1.z, p2.z),
    } };
}
