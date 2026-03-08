#include "../../renderer/include/renderer.h"
#include "../../renderer/include/types.h"
#include <math.h>
#include <chrono>
#include <thread>

using namespace std;

int main() {
    Scene scene = Scene();
    while (true) {
        auto start = chrono::steady_clock::now().time_since_epoch().count();
        scene.removeAllObjects();
        for (float x = -32; x < 32; x++) {
            for (float y = -32; y < 32; y++) {
                float z = sin(x/10. + chrono::steady_clock::now().time_since_epoch().count()/1000000000.) * 10  + sin(y/10.) * 10 + 20;
                
                ParticleGeometry g = {
                    .pos = {x, y, z},
                    .radius = 0.8
                };
                
                scene.createObject(g, BLUE);
            }
        }
        scene.wipeScreen();
        scene.render();
        auto end = chrono::steady_clock::now().time_since_epoch().count();
        printf("fps: %f", 1000000000./(end-start));
        // this_thread::sleep_for(chrono::milliseconds(50));
    }
}
