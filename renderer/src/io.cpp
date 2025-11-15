#include <vector>
#include <array>
#include <string>
#include <regex>
#include <fstream>
#include <iostream>

#include "types.h"
using namespace std;

vector<float> getFloats(string str) {
    vector<float> res;
    regex nums_regex("[+-]?([0-9]*[.])?[0-9]+");
    auto words_begin = sregex_iterator(str.begin(), str.end(), nums_regex);
    auto words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; ++i)
    {
        smatch match = *i;
        string match_str = match.str();
        res.push_back((float)stod(match_str));
        //cout << match_str;

    }
    //cout << endl;
    return res;

}

vector<string> split(const string& s, const string& delimiter = " ") {
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

    for (const Point& pt : points) {
        PtCloudFile << pt.first.x << " " << pt.first.y << " " << pt.first.z << " " << pt.second.x << " " << pt.second.y << " " << pt.second.z << endl;
    }
}

vector<Vec3> loadPointsObj(string path) {
    ifstream file(path);
    string str;
    string file_contents;
    vector<Vec3> vertices;

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
Mesh loadMeshObj(string path) {
    ifstream file(path);
    string str;
    string file_contents;
    Mesh res;
    vector<Vec3>& vertices = res.vertices;
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

UpdatePattern loadUpdatePattern(string path) {

}