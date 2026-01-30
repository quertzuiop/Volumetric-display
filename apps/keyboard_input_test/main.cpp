#include "types.h"
#include "renderer.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main() {
    Scene scene = Scene();

    while (true) {
        cout << " Pressed keys: ";
        for (char key : scene.getPressedKeys()) {
            printf("%s, ", key);
        }
        cout<<endl;
        sleep(2);
    }
}