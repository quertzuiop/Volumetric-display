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
using namespace std;

typedef struct {
    float x;
    float y;
    float z;
} vertex;

vertex operator-(const vertex& a, const vertex& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
vertex operator*(const vertex& v, float s) {
    return { v.x * s, v.y * s, v.z * s};
}
struct gridData {
    vertex boundingBoxMin;
    vertex boundingBoxMax;
    int gridSize;
    vertex cellSizes;
};

struct mesh {
    vector<vertex> vertices;
    vector<pair<int, int>> edges;
    vector<array<int, 3>> faces;
};

using point = pair<vertex, vertex>;
using ptCloud = vector<point>;

inline int packIndex(int ix, int iy, int iz) {
    return (ix << 22) | (iy << 11) | iz;
}

inline array<int, 3> unpackIndex(int packed) {
    return { packed >> 22, (packed >> 11) & ((1 << 11) - 1), packed & ((1 << 11) - 1)};
}

inline float mnhtDist(const vertex& p1, const vertex& p2) {
    return abs(p1.x - p2.x) + abs(p1.y - p2.y) + abs(p1.z - p2.z);
}

inline float dist(const vertex& p1, const vertex& p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}

inline float dist2(const vertex& p1, const vertex& p2) {
    return pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2);
}

inline float dot(const vertex& v1, const vertex& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline vertex cross(const vertex& v1, const vertex& v2) {
    return {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x,
    };
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

inline float magnitude_2(const vertex& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline vertex transform(const vertex& pt, float dx=0, float dy=0, float dz=0, float scale=1.) {
    return { pt.x * scale + dx, pt.y * scale + dy, pt.z * scale + dz };
}

point transform(const point& pt, float dx = 0, float dy = 0, float dz = 0, float scale = 1.) {
    return { transform(pt.first, dx, dy, dz, scale), pt.second };
}

int getTime() {
    return chrono::duration_cast<chrono::milliseconds> (chrono::system_clock::now().time_since_epoch()).count();
}

int calculateIndex(const gridData& gridData, const vertex& ptCoords) {
    const vertex& min = gridData.boundingBoxMin;
    const vertex& cellSizes = gridData.cellSizes;

    int bitMax = pow(2, 10);

    int ix = floor((ptCoords.x - min.x) / cellSizes.y);
    int iy = floor((ptCoords.y - min.y) / cellSizes.x);
    int iz = floor((ptCoords.z - min.z) / cellSizes.z);

    //ix - (10 bits), iy - (11 bits), iz - (11bits)
    return packIndex(ix, iy, iz);
}

vector<int> calculateIndicesFromBB(const gridData& gridData, const vertex& min, const vertex& max, float padding = 0.) {
    const vertex& boxMin = gridData.boundingBoxMin;
    const vertex& boxMax = gridData.boundingBoxMax;

    const vertex& cellSizes = gridData.cellSizes;

    int minIx = floor((min.x - boxMin.x - padding) / cellSizes.y);
    int minIy = floor((min.y - boxMin.y - padding) / cellSizes.x);
    int minIz = floor((min.z - boxMin.z - padding) / cellSizes.z);

    int maxIx = floor((max.x - boxMin.x + padding) / cellSizes.y);
    int maxIy = floor((max.y - boxMin.y + padding) / cellSizes.x);
    int maxIz = floor((max.z - boxMin.z + padding) / cellSizes.z);
    //printf("min values: %d %d %d\nmax values: %d %d %d\n", minIx, minIy, minIz, maxIx, maxIy, maxIz);
    vector<int> res;
    res.reserve((maxIx - minIx + 1) * (maxIy - minIy + 1) * (maxIz - minIz + 1));
    for (int ix = minIx; ix <= maxIx; ix++) {
        for (int iy = minIy; iy <= maxIy; iy++) {
            for (int iz = minIz; iz <= maxIz; iz++) {
                res.push_back(packIndex(ix, iy, iz));
            }
        }
    }
    return res;
}

tuple<unordered_map<int, ptCloud>, gridData> buildGrid(const ptCloud& points, int ptsPerCell) {
    int numPoints = points.size();
    int nCells = numPoints / ptsPerCell;
    int gridSize = ceil(pow(nCells, 1. / 3.));

    //find bounding box
    vertex Min = { 0, 0, 0 };
    vertex Max = { 0, 0, 0 };

    for (const point& pt : points) {
        vertex ptCoords = pt.first;
        Min.x = min(Min.x, ptCoords.x);
        Min.y = min(Min.y, ptCoords.y);
        Min.z = min(Min.z, ptCoords.z);
                
        Max.x = max(Max.x, ptCoords.x);
        Max.y = max(Max.y, ptCoords.y);
        Max.z = max(Max.z, ptCoords.z);
    }

    cout << "grid size: " << gridSize << endl;

    float cellSizeZ = (Max.z - Min.z) / gridSize;
    float cellSizeX = (Max.x - Min.x) / gridSize;
    float cellSizeY = (Max.y - Min.y) / gridSize;

    gridData gridData = {
        Min,
        Max,
        gridSize,
        vertex {cellSizeX, cellSizeY, cellSizeZ}
    };
    cout << "cells sizes: " << cellSizeX << ", " << cellSizeY << ", " << cellSizeZ << endl;
    unordered_map<int, ptCloud> map;

    for (int i = 0; i < points.size(); i++) { 
        int packed = calculateIndex(gridData, points[i].first);
        auto& mapVal = map[packed];
        mapVal.push_back(std::move(points[i]));
        //array<int, 3> unpacked = unpackIndex(packed);
        //if (unpacked != array<int, 3>{ix, iy, iz}) cout << "failed. reconstructed:" << endl;
        //cout << bitset<32>(packed).to_string() << endl;
    }
    return { map, gridData };
}

vector<float> getFloats(string str) {
    vector<float> res;
    regex nums_regex("[+-]?([0-9]*[.])?[0-9]+");
    auto words_begin = sregex_iterator(str.begin(), str.end(), nums_regex);
    auto words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; ++i)
    {
        smatch match = *i;
        string match_str = match.str();
        res.push_back((float) stod(match_str));
        //cout << match_str;

    }
    //cout << endl;
    return res;

}

vector<string> split(const string& s, const string& delimiter=" ") {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

vector<float> extractPointCloudData(string str, int nDataPoints = 6) { 
    size_t pos_start = 0, pos_end; 
    string token; 
    vector<float> res; 
    if (str.find(".") == string::npos || str == "format ascii 1.0") return res; 
    for (int i = 0; i < nDataPoints - 1; i++) { 
        if ((pos_end = str.find(" ", pos_start)) == string::npos) {
            cout << "warning: Not enough values in string: " << str << endl;
            return res; 
        }
        token = str.substr(pos_start, pos_end - pos_start); 
        pos_start = pos_end + 1;
        res.push_back(stof(token));
    } res.push_back(stof(str.substr(pos_start)));
    return res; 
}
ptCloud drawPoint(
    const vertex& ptCoords,
    const unordered_map<int, ptCloud>& mapping, 
    const gridData& gridData, 
    float radius, bool useMnht=false
){
    int index = calculateIndex(gridData, ptCoords);
    auto it = mapping.find(index);
    if (it == mapping.end()) return {};
    const ptCloud& bucket = it->second;

    ptCloud res;

    for (const point& potentialPt : bucket) {
        vertex potentialPtCoords = potentialPt.first;
        double d = dist(ptCoords, potentialPtCoords);
        if (d <= radius) res.push_back(potentialPt);
    }
    return res;
}

ptCloud drawLine(
    const vertex& start,
    const vertex& end, 
    const unordered_map<int, ptCloud>& mapping,
    const gridData& gridData,
    float radius
) {
    float length = dist(start, end);
    float length2 = length * length;
    float radius2 = radius * radius;

    vertex vec = end - start;
    vertex minV = {
        min(start.x, end.x),
        min(start.y, end.y),
        min(start.z, end.z),
    };
    vertex maxV = {
        max(start.x, end.x),
        max(start.y, end.y),
        max(start.z, end.z),
    };
    auto bucketIndices = calculateIndicesFromBB(gridData, minV, maxV, radius);

    ptCloud res;

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return {};
        const ptCloud& bucket = it->second;
        for (const point& pt : bucket) {
            const vertex& ptCoords = pt.first;
            vertex v1 = start - ptCoords;

            float d1 = dist(ptCoords, start);
            float d2 = dist(ptCoords, end);

            if (d1 + d2 > length + radius) continue; // discard point outside ellipsoid
            float dSquared = magnitude_2(cross(vec, v1)) / length2;

            if (dSquared <= radius2) {
                res.push_back(pt);
            }
        }
    }
    return res;
}

ptCloud drawTriangle(
    const vertex& v1,
    const vertex& v2,
    const vertex& v3,
    const unordered_map<int, ptCloud>& mapping,
    const gridData& gridData,
    float radius
) {
    vertex minV = {
        min(min(v1.x, v2.x), v3.x),
        min(min(v1.y, v2.y), v3.y),
        min(min(v1.z, v2.z), v3.z),
    };
    vertex maxV = {
        max(max(v1.x, v2.x), v3.x),
        max(max(v1.y, v2.y), v3.y),
        max(max(v1.z, v2.z), v3.z),
    };

    auto bucketIndices = calculateIndicesFromBB(gridData, minV, maxV, radius);

    vertex v21 = v2 - v1;
    vertex v32 = v3 - v2;
    vertex v13 = v1 - v3;

    float magV21 = magnitude_2(v21);
    float magV32 = magnitude_2(v32);
    float magV13 = magnitude_2(v13);

    vertex normal = cross(v21, v13);
    float magNormal = magnitude_2(normal);

    vertex c21 = cross(v21, normal);
    vertex c32 = cross(v32, normal);
    vertex c13 = cross(v13, normal);

    ptCloud res;

    float radius2 = radius * radius;

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return {};
        const ptCloud& bucket = it->second;

        for (const point& pt : bucket) {
            const vertex& ptCoords = pt.first;

            vertex p1 = ptCoords - v1;
            vertex p2 = ptCoords - v2;
            vertex p3 = ptCoords - v3;

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

void writePtcloudToFile(const ptCloud& points, const string& path) {
    ofstream PtCloudFile(path);
    PtCloudFile << "ply\n"
        "format ascii 1.0\n"
        "element vertex " << points.size() << "\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "property float nx\n"
        "property float ny\n"
        "property float nz\n"
        "end_header\n";

    for (const point& pt: points) {
        PtCloudFile << pt.first.x << " " << pt.first.y << " " << pt.first.z << " " << pt.second.x << " " << pt.second.y << " " << pt.second.z << endl;
    }
}

vector<vertex> loadPointsObj(string path) {
    ifstream file(path);
    string str;
    string file_contents;
    vector<vertex> vertices;

    while (getline(file, str)) {
        if (str.find("v ") != string::npos) {
            vector<float> coords = getFloats(str);
            vertices.push_back({ coords[0], coords[1], coords[2] });
        }
        else if (str.find("f ") != string::npos) {
    
        }
    }
    return vertices;
}
mesh loadMeshObj(string path) {
    ifstream file(path);
    string str;
    string file_contents;
    mesh res;
    vector<vertex>& vertices = res.vertices;
    vector<pair<int, int>>& edges = res.edges;
    vector<array<int, 3>>& faces = res.faces;

    while (getline(file, str)) {
        if (str.find("v ") != string::npos) {
            vector<float> coords = getFloats(str);
            vertices.push_back({ coords[0], coords[1], coords[2] });
        }
        else if (str.find("f ") != string::npos) {
            array<int, 3> face;

            vector<string> faceInfo = split(str.substr(2));
            if (faceInfo.size() > 3) cerr << "Warning: obj file has face with more than 3 vertices";
            for (int i = 0; i < 3; i++) {
                string vertexIndexStr = split(faceInfo[i], "/")[0];
                face[i] = (stoi(vertexIndexStr)) - 1; //ply indexing starts from 1
            }
            faces.push_back(face);

            //iterate over all unordered pairs
            for (int i = 0; i < 2; ++i) {
                for (int j = i + 1; i < 3; ++i) {
                    edges.push_back({ face[i], face[j] });
                }
            }
        }
    }
    return res;
}

int main() {
    //0.4s
    string pt_cloud_path = "C:/Users/robik/volumetric display simulation/pythonScripts/Volumetric-display/update_pattern_gen/pointcloud.ply";
    ifstream file(pt_cloud_path);
    stringstream buffer;

    int starttime = getTime();
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
            vertex{ floats[0], floats[1], floats[2] }, 
            vertex{ floats[3], floats[4], floats[5] } 
            });
    }

    printf("Loaded points in: %d ms\n", getTime() - starttime);
    if (points.size() != pointCountTarget) cout << "Expected " << pointCountTarget << " Points but got " << points.size() << endl;
    cout << "loaded points" << endl;

    auto [mapping, gridData] = buildGrid(points, 20);

    ptCloud test;
    //vector<vertex> sphere = loadPointsObj("C:/Users/robik/volumetric display simulation/pythonScripts/test.obj");
    //for (vertex ptCoords : sphere) {
    //    vertex scaledPtCoords = transform(ptCoords, 0, 0, 20, 20);
    //    //cout << scaledPtCoords.x << " " << scaledPtCoords.y << " " << scaledPtCoords.z << endl;
    //    ptCloud newPts = drawPoint(scaledPtCoords, mapping, gridData, 1);
    //    //cout << newPts.size() << " new points" << endl;
    //    test.insert(test.end(), newPts.begin(), newPts.end());
    //    //cout << test.size() << "new len" << endl;
    //}
    mesh sphere = loadMeshObj("C:/Users/robik/volumetric display simulation/pythonScripts/test.obj");
    //vertices
    // 
    //for (const vertex& ptCoords : sphere.vertices) {
    //    vertex scaledPtCoords = transform(ptCoords, 0, 0, 20, 20);
    //    //cout << scaledPtCoords.x << " " << scaledPtCoords.y << " " << scaledPtCoords.z << endl;
    //    ptCloud newPts = drawPoint(scaledPtCoords, mapping, gridData, 0.7);
    //    //cout << newPts.size() << " new points" << endl;
    //    test.insert(test.end(), newPts.begin(), newPts.end());
    //    //cout << test.size() << "new len" << endl;
    //}
    //edges
    //
    for (const auto& lineIndices : sphere.edges) {
        const vertex& a = sphere.vertices[lineIndices.first], b = sphere.vertices[lineIndices.second];
        vertex as = transform(a, 0, 0, 27.5, 27);
        vertex bs = transform(b, 0, 0, 27.5, 27);
        ptCloud newPts = drawLine(as, bs, mapping, gridData, 0.6);
        test.insert(test.end(), newPts.begin(), newPts.end());
    }
    cout << "writing" << endl;
    writePtcloudToFile(test, "C:/Users/robik/Downloads/test.ply");

    //for (const auto& lineIndices : sphere.faces) {
    //    const vertex& a = sphere.vertices[lineIndices[0]], b = sphere.vertices[lineIndices[1]], c = sphere.vertices[lineIndices[2]];
    //    vertex as = transform(a, 0, 0, 21, 20);
    //    vertex bs = transform(b, 0, 0, 21, 20);
    //    vertex cs = transform(c, 0, 0, 21, 20);
    //    ptCloud newPts = drawTriangle(as, bs, cs, mapping, gridData, 0.2);
    //    test.insert(test.end(), newPts.begin(), newPts.end());
    //}
    cout << "writing" << endl;
    writePtcloudToFile(test, "C:/Users/robik/Downloads/test.ply");
}