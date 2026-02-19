#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include "../utils/utils.h"
#include <unordered_map>
#include <iostream>
#include <stdlib.h>

using namespace std;
unordered_map<string, Color> colorMap = {
    {"white", WHITE},
    {"black", BLACK},
    {"red", RED},
    {"blue", BLUE},
    {"green", GREEN},
    {"yellow", YELLOW},
    {"cyan", CYAN},
    {"magenta", MAGENTA}
};


int main(int argc, char* argv[]) {
    Scene scene = Scene();
    Color color = WHITE;
    bool isWireframe = true;
    float padding = 2.0f;
    float thickness = 0.5f;

    
    if (argc < 2) {
        cerr<<"No object file specified."<<endl;
        return 0;
    }
    vector<string> args(argv, argv + argc);

    Mesh mesh = loadMeshObj(format("assets/{}.obj", args[1]));

    if (find(args.begin(), args.end(), "-padding") != args.end()) {
        padding = getOption<float>("-padding", argc, argv);
    }
    
    if (find(args.begin(), args.end(), "-thickness") != args.end()) {
        thickness = getOption<float>("-thickness", argc, argv);
    }
    
    if (find(args.begin(), args.end(), "-color") != args.end()) {
        color = colorMap[getOption<string>("-color", argc, argv)];
    }

    mesh.center(padding);

    MeshGeometry meshGeom = {
        .mesh = mesh,
        .isWireframe = isWireframe,
        .thickness = thickness
    };

    auto meshObj = scene.createObject(meshGeom, color);
    scene.render();
}   