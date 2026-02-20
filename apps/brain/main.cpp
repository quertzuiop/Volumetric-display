#include "renderer.h"
#include "types.h"
#include "io.h"
#include "../utils/utils.h"

#include <fstream>

int main(int argc, char* argv[]) {
    Scene scene = Scene();

    ifstream file("assets/1.txt");
    string str;
    string file_contents;
    vector<Vec3<float>> vertices;

    while (getline(file, str)) {
        vector<float> coords = getFloats(str);
        vertices.push_back({ coords[0], coords[1], coords[2]*stof(argv[1]) });
    }
    
    for (auto& vertex : vertices) {
        ParticleGeometry g =  {
            .pos = vertex,
            .radius = 0.3
        };
        scene.createObject(g, BLUE);
    }
    scene.render();
}