#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <regex>
#include <iterator>
#include <array>
#include <unordered_map>
#include <bitset>

using namespace std;

typedef struct {
    float x;
    float y;
    float z;
} Vertex3D;

struct gridData {
    Vertex3D boundingBoxMin;
    Vertex3D boundingBoxMax;
};
using ptCloud = vector<pair<Vertex3D, Vertex3D>>;

int toSigned(int n) {
    return (n < 0) ? -2 * n - 1 : 2 * n;
}

inline int packIndex(int ix, int iy, int iz) {
    return (ix << 22) | (iy << 11) | iz;
}

inline array<int, 3> unpackIndex(int packed) {
    return { packed >> 22, (packed >> 11) & ((1 << 11) - 1), packed & ((1 << 11) - 1)};
}

float mnhtDist(Vertex3D p1, Vertex3D p2) {
    return abs(p1.x - p2.x) + abs(p1.y - p2.y) + abs(p1.z - p2.z);
}

unordered_map<int, ptCloud> buildGrid(ptCloud points, int ptsPerCell) {
    int numPoints = points.size();
    int nCells = numPoints / ptsPerCell;
    int gridSize = ceil(pow(nCells, 1. / 3.));

    //find bounding box
    Vertex3D Min = { 0, 0, 0 };
    Vertex3D Max = { 0, 0, 0 };

    for (pair<Vertex3D, Vertex3D> point : points) {
        Vertex3D pointCoords = point.first;
        Min.x = min(Min.x, pointCoords.x);
        Min.y = min(Min.y, pointCoords.y);
        Min.z = min(Min.z, pointCoords.z);
                
        Max.x = max(Max.x, pointCoords.x);
        Max.y = max(Max.y, pointCoords.y);
        Max.z = max(Max.z, pointCoords.z);
    }

    cout << "grid size: " << gridSize << endl;
    float cellSizeZ = (Max.z - Min.z) / gridSize;
    float cellSizeX = (Max.x - Min.x) / gridSize;
    float cellSizeY = (Max.y - Min.y) / gridSize;

    unordered_map<int, ptCloud> map;
    bool warnBitPacking = false;
    int bitMax = pow(2, 10);

    for (int i = 0; i < points.size(); i++) {
        Vertex3D pointCoords = points[i].first;
        int ix = floor((pointCoords.x - Min.x) / cellSizeX);
        int iy = floor((pointCoords.y - Min.y) / cellSizeY);
        int iz = floor((pointCoords.z - Min.z) / cellSizeZ);

        //cout << ix << " " << iy << " " << iz << endl;
        if ( !(
            (-bitMax < ix && ix < bitMax) || 
            (-bitMax < iy && iy < bitMax) || 
            (-bitMax < iz && iz < bitMax) )
        ) {
            warnBitPacking = true;
        }
        //ix - (10 bits), iy - (11 bits), iz - (11bits)
        int packed = packIndex(ix, iy, iz);
        auto mapVal = map[packed];
        mapVal.push_back(points[i]);
        map[packed] = mapVal;
        //array<int, 3> unpacked = unpackIndex(packed);
        //if (unpacked != array<int, 3>{ix, iy, iz}) cout << "failed. reconstructed:" << endl;
        //cout << bitset<32>(packed).to_string() << endl;
    }
    return map;
}
vector<double> getFloats(string str) {
    vector<double> res;
    regex nums_regex("[+-]?([0-9]*[.])?[0-9]+");
    auto words_begin = sregex_iterator(str.begin(), str.end(), nums_regex);
    auto words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; ++i)
    {
        smatch match = *i;
        string match_str = match.str();
        res.push_back(stod(match_str));
        //cout << match_str;

    }
    //cout << endl;
    return res;

}
vector<string> split(string s, string delimiter=" ") {
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

vector<float> extractPointData(string str, int nDataPoints=6) {
    size_t pos_start = 0, pos_end;
    string token;
    vector<float> res;

    if (str.find(".") == string::npos || str == "format ascii 1.0") return res;

    for (int i = 0; i < nDataPoints-1; i++) {
        if ((pos_end = str.find(" ", pos_start)) == string::npos) {
            cout << "warning: Not enough values in string: " << str << endl;
            return res;
        }
        token = str.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + 1;
        res.push_back(stof(token));
    }

    res.push_back(stof(str.substr(pos_start)));
    return res;
}

ptCloud kClosestNeigbors(ptCloud points, float radius) {

}
void writePtcloudToFile(ptCloud points, string path) {
    ofstream PtCloudFile(path);
    PtCloudFile << endl;
}
int main() {
    string path = "C:/Users/robik/volumetric display simulation/pythonScripts/test.obj";
    string pt_cloud_path = "C:/Users/robik/volumetric display simulation/pythonScripts/Volumetric-display/update_pattern_gen/pointcloud.ply";
    ifstream file(pt_cloud_path);
    string str;
    string file_contents;
    //vector<vector<double>> vertices;
    
    //while (getline(file, str)) {
    //    cout << str <<endl;
    //    if (str.find("v ") != string::npos) {
    //        vector<double> coords = getFloats(str);
    //        vertices.push_back(coords);
    //    }
    //    else if (str.find("f ") != string::npos) {
    //
    //    }
    //}

    ptCloud points;
    int pointCountTarget = 0;
    int i = 0;
    while (getline(file, str)) {
        if (i%50000 == 0) cout << i << endl;
        i++;
        if (str.find("element vertex") != string::npos) {
            pointCountTarget = stoi(str.substr(15, string::npos));
            cout << pointCountTarget << endl;
        }
        auto floats = extractPointData(str);
        if (floats.size() < 6) continue;
        points.push_back({ 
            Vertex3D{ (float)floats[0], (float)floats[1], (float)floats[2] }, 
            Vertex3D{ (float)floats[3], (float)floats[4], (float)floats[5] } 
            });
    }

    if (points.size() != pointCountTarget) cout << "Expected " << pointCountTarget << " Points but got " << points.size() << endl;
    cout << "loaded points" << endl;
    auto map = buildGrid(points, 20);
}