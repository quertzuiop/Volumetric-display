#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include "../../renderer/include/io.h"
#include "../utils/utils.h"
#include <unordered_map>
#include <iostream>

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
    float padding = 0.0f;

    
    if (argc < 2) {
        cerr<<"No object file specified. Terminating"<<endl;
        return;
    }
    vector<string> args(argv, argv + argc);

    Mesh mesh = loadMeshObj(format("assets/{}.obj", args[1]));

    mesh.center();
    
    if (find(args.begin(), args.end(), "-color") != args.end()) {
        color = colorMap[getOption<string>("-color", argc, argv)];
    }
    MeshGeometry meshGeom = {
        .mesh = mesh,
        .isWireframe = isWireframe,
        .thickness = 0.5
    };
    auto meshObj = scene.createObject(meshGeom, color);
    
    scene.render();
}   