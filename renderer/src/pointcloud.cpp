#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <regex>
#include <iterator>
#include <array>
#include <unordered_map>
#include <bitset>
#include <sstream>
#include <chrono>

#include "types.h"
#include "math.h"
#include "grid.h"
#include "io.h"
using namespace std;

int getTime() {
    return chrono::duration_cast<chrono::microseconds> (chrono::system_clock::now().time_since_epoch()).count();
}

ptCloud drawParticle( //can have parts cut off, points sampled from 1 cell
    const Vec3& pos,
    const unordered_map<int, ptCloud>& mapping, 
    const GridParams& params, 
    float radius, bool useMnht=false
){
    int index = calculateIndex(params, pos);
    auto it = mapping.find(index);
    if (it == mapping.end()) return {};
    const ptCloud& bucket = it->second;

    float radius2 = radius * radius;
    ptCloud res;

    for (const Point& potentialPt : bucket) {
        Vec3 potentialPtCoords = potentialPt.first;
        double d2 = dist2(pos, potentialPtCoords);
        if (d2 <= radius2) res.push_back(potentialPt);
    }
    return res;
}

ptCloud drawLine(
    const Vec3& start,
    const Vec3& end, 
    const unordered_map<int, ptCloud>& mapping,
    const GridParams& params,
    float radius
) {
    float length = dist(start, end);
    float length2 = length * length;
    float radius2 = radius * radius;

    Vec3 vec = end - start;

    auto [minV, maxV] = arrangeBoundingBox(start, end);
    
    auto bucketIndices = calculateIndicesFromBB(params, minV, maxV, radius);

    ptCloud res;

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return {};
        const ptCloud& bucket = it->second;
        for (const Point& pt : bucket) {
            const Vec3& ptCoords = pt.first;
            Vec3 v1 = start - ptCoords;

            // float d12 = dist(ptCoords, start);
            // float d22 = dist(ptCoords, end);

            // if (d1 + d2 > length + radius) continue; // discard point outside ellipsoid
            float dSquared = magnitude_2(cross(vec, v1)) / length2;

            if (dSquared <= radius2) {
                res.push_back(pt);
            }
        }
    }
    return res;
}

ptCloud drawTriangle(
    const Vec3& v1,
    const Vec3& v2,
    const Vec3& v3,
    const unordered_map<int, ptCloud>& mapping,
    const GridParams& params,
    float radius
) {
    Vec3 minV = {
        min(min(v1.x, v2.x), v3.x),
        min(min(v1.y, v2.y), v3.y),
        min(min(v1.z, v2.z), v3.z),
    };
    Vec3 maxV = {
        max(max(v1.x, v2.x), v3.x),
        max(max(v1.y, v2.y), v3.y),
        max(max(v1.z, v2.z), v3.z),
    };

    auto bucketIndices = calculateIndicesFromBB(params, minV, maxV, radius);

    Vec3 v21 = v2 - v1;
    Vec3 v32 = v3 - v2;
    Vec3 v13 = v1 - v3;

    float magV21 = magnitude_2(v21);
    float magV32 = magnitude_2(v32);
    float magV13 = magnitude_2(v13);

    Vec3 normal = cross(v21, v13);
    float magNormal = magnitude_2(normal);

    Vec3 c21 = cross(v21, normal);
    Vec3 c32 = cross(v32, normal);
    Vec3 c13 = cross(v13, normal);

    ptCloud res;

    float radius2 = radius * radius;

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return {};
        const ptCloud& bucket = it->second;

        for (const Point& pt : bucket) {
            const Vec3& ptCoords = pt.first;

            Vec3 p1 = ptCoords - v1;
            Vec3 p2 = ptCoords - v2;
            Vec3 p3 = ptCoords - v3;

            bool inside = (sgn(dot(c21, p1)) + sgn(dot(c21, p1)) + sgn(dot(c21, p1))) < 2.;
            float d2;
            if (inside) {
                d2 = min(min(
                    magnitude_2(v21 * clamp(dot(v21, p1) / magV21, (float)0., (float)1.) - p1),
                    magnitude_2(v32 * clamp(dot(v32, p2) / magV32, (float)0., (float)1.) - p2)),
                    magnitude_2(v13 * clamp(dot(v13, p3) / magV13, (float)0., (float)1.) - p3));
            }
            else {
                d2 = pow(dot(normal, p1), 2) /magNormal;
            }
            if (d2 < radius2) res.push_back(pt);
        }
    }
    return res;
}

ptCloud drawSphere(
    const Vec3& pos,
    const unordered_map<int, ptCloud>& mapping,
    const GridParams& params,
    float radius, float thickness = 0.
) {
    Vec3 minV = pos - radius;
    Vec3 maxV = pos + radius;

    auto bucketIndices = calculateIndicesFromBB(params, minV, maxV, radius);
    ptCloud res;

    float radius2 = radius * radius;

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return {};
        const ptCloud& bucket = it->second;

        for (const Point& pt : bucket) {
            const Vec3& ptCoords = pt.first;
            float d2 = dist2(ptCoords, pos);
            if (thickness > 0 && d2 < (2 * radius * thickness - radius2)) continue; // magic math supr
            if (d2 < radius2) res.push_back(pt);
        }
    }
}


ptCloud drawCuboid( //untested
    const Vec3& v1,
    const Vec3& v2,
    const unordered_map<int, ptCloud>& mapping,
    const GridParams& params,
    float thickness = 0.
) {
    auto [minV, maxV] = arrangeBoundingBox(v1, v2);

    auto bucketIndices = calculateIndicesFromBB(params, minV, maxV);
    ptCloud res;

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return {};
        const ptCloud& bucket = it->second;

        for (const Point& pt : bucket) {
            const Vec3& ptCoords = pt.first;

            if (minV.x + thickness < ptCoords.x &&
                minV.y + thickness < ptCoords.y &&
                minV.z + thickness < ptCoords.z &&
                maxV.x - thickness > ptCoords.x &&
                maxV.y - thickness > ptCoords.y &&
                maxV.z - thickness > ptCoords.z) continue;

            if (minV.x < ptCoords.x &&
                minV.y < ptCoords.y &&
                minV.z < ptCoords.z &&
                maxV.x > ptCoords.x &&
                maxV.y > ptCoords.y &&
                maxV.z > ptCoords.z) res.push_back(pt);
        }
    }
    return res;
}


int main() {
    //0.4s
    string pt_cloud_path = "C:/Users/robik/volumetric display simulation/pythonScripts/Volumetric-display/update_pattern_gen/pointcloud.ply";
    ifstream file(pt_cloud_path);
    stringstream buffer;

    //1.25 s
    buffer << file.rdbuf();
    string fileStr = buffer.str();

    ptCloud points;
    int pointCountTarget = 0;
    int i = 0;


    vector<string> splitFileStr = split(fileStr, "\n");

    for (const string& line : splitFileStr) {
        if (i%50000 == 0) cout << i << endl;
        i++;
        if (line.find("element vertex") != string::npos) {
            pointCountTarget = stoi(line.substr(15, string::npos));
            cout << pointCountTarget << endl;
        }
        auto floats = extractPointCloudData(line);
        if (floats.size() < 6) continue;
        points.push_back({ 
            Vec3{ floats[0], floats[1], floats[2] }, 
            Vec3{ floats[3], floats[4], floats[5] } 
            });
    }

    if (points.size() != pointCountTarget) cout << "Expected " << pointCountTarget << " Points but got " << points.size() << endl;
    cout << "loaded points" << endl;

    int starttime = getTime();

    auto [mapping, params] = buildGrid(points, 25);
    printf("built grid in: %d us\n", getTime() - starttime);

    ptCloud test;
    Mesh sphere = loadMeshObj("C:/Users/robik/volumetric display simulation/pythonScripts/test.obj");
    //vertices
     
    starttime = getTime();
    
    for (const Vec3& ptCoords : sphere.vertices) {
        Vec3 scaledPtCoords = transform(ptCoords, 0, 0, 20, 20);
        ptCloud newPts = drawParticle(scaledPtCoords, mapping, params, 0.7);
        test.insert(test.end(), newPts.begin(), newPts.end());
    }
    printf("drew sphere %d points in: %d us\n", sphere.vertices.size(), getTime() - starttime);
    //edges
    
    starttime = getTime();

    for (const auto& lineIndices : sphere.edges) {
        const Vec3& a = sphere.vertices[lineIndices.first], b = sphere.vertices[lineIndices.second];
        Vec3 as = transform(a, 0, 0, 27.5, 27);
        Vec3 bs = transform(b, 0, 0, 27.5, 27);
        ptCloud newPts = drawLine(as, bs, mapping, params, 0.6);
        test.insert(test.end(), newPts.begin(), newPts.end());
    }
    printf("drew %d sphere edges in: %d us\n", sphere.edges.size(), getTime() - starttime);

    starttime = getTime();

    for (const auto& lineIndices : sphere.faces) {
        const Vec3& a = sphere.vertices[lineIndices[0]], b = sphere.vertices[lineIndices[1]], c = sphere.vertices[lineIndices[2]];
        Vec3 as = transform(a, 0, 0, 21, 20);
        Vec3 bs = transform(b, 0, 0, 21, 20);
        Vec3 cs = transform(c, 0, 0, 21, 20);
        ptCloud newPts = drawTriangle(as, bs, cs, mapping, params, 0.2);
        test.insert(test.end(), newPts.begin(), newPts.end());
    }
    printf("drew %d sphere faces in: %d us\n", sphere.faces.size(), getTime() - starttime);

    cout << "writing" << endl;
    writePtcloudToFile(test, "C:/Users/robik/Downloads/test.ply");
}