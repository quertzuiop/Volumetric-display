#include "vector"
#include "string"

#include "types.h"

using namespace std;

vector<float> getFloats(string str);

vector<string> split(const string& s, const string& delimiter = " ");

vector<float> extractPointCloudData(string str, int nDataPoints = 6);

void writePtcloudToFile(const ptCloud& points, const string& path);

vector<Vec3> loadPointsObj(string path);

Mesh loadMeshObj(string path);

UpdatePattern loadUpdatePattern(string path);