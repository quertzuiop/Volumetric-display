#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include "../utils/utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <filesystem>
#include <fstream>


using WorldModel = vector<pair<Vec3<float>, Color>>;

int main(int argc, char* argv[]) {
    Scene scene = Scene();
    vector<string> args(argv, argv + argc);
    int padding = 2;
    if (find(args.begin(), args.end(), "-padding") != args.end()) {
        padding = getOption<float>("-padding", argc, argv);
    }
    

    ifstream file(format("assets/{}.txt", argv[1]));
    stringstream buffer;

    buffer << file.rdbuf();
    string fileStr = buffer.str();

    vector<string> splitFileStr = split(fileStr, "\n");
    printf("loaded %d blocks\n", splitFileStr.size());
    WorldModel worldModel;

    for (string s : splitFileStr) {
        cout<<s<<endl;
        
        vector<float> floatData = getFloats(s);

        Vec3<float> pos = {floatData[0], floatData[2], floatData[1]};
        Color color = {floatData[3], floatData[4], floatData[5]};

        worldModel.push_back({pos, color});
    }
    Vec3<float> min = {__FLT_MAX__, __FLT_MAX__, __FLT_MAX__};
    Vec3<float> max = {-__FLT_MAX__, -__FLT_MAX__, -__FLT_MAX__};

    Vec3<float> boundsMin = {-32 + padding, -32 + padding, 0 + padding};
    Vec3<float> boundsMax = {32 - padding, 32 - padding, 64 - padding};

    auto boundsCenter = (boundsMin + boundsMax) * 0.5;

    for (const auto& block : worldModel) {
        auto v = block.first;
        min.x = min.x < v.x ? min.x : v.x;
        min.y = min.y < v.y ? min.y : v.y;
        min.z = min.z < v.z ? min.z : v.z;
    
        max.x = max.x > v.x ? max.x : v.x;
        max.y = max.y > v.y ? max.y : v.y;
        max.z = max.z > v.z ? max.z : v.z;
    }
    max = max + 1;

    auto meshCenter = (min + max) * 0.5;
    
    auto meshSize = max - min;
    auto targetSize = boundsMax - boundsMin;

    float scaleX = (meshSize.x > 0) ? targetSize.x / meshSize.x : 1.0f;
    float scaleY = (meshSize.y > 0) ? targetSize.y / meshSize.y : 1.0f;
    float scaleZ = (meshSize.z > 0) ? targetSize.z / meshSize.z : 1.0f;

    float scaleFactor = std::min({scaleX, scaleY, scaleZ});
    std::cout<<"calculated mesh center: "<<meshCenter << ", bounds center: " <<boundsCenter << ", scale factor: " <<scaleFactor<<std::endl;
    std::cout << "min: " << min << ", max: " << max<<std::endl;
    for (auto block : worldModel) {
        auto v = block.first;
        v = v - meshCenter;
        v = v * scaleFactor;
        v = v + boundsCenter;
        CuboidGeometry g = {
            .v1 = v,
            .v2 = v + scaleFactor,
            .isWireframe = false,
        };

        scene.createObject(g, block.second);
    }

    scene.render();
}