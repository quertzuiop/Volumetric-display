# Volumetric display
The code for the software part of my graduation project at SSPŠ is stored here. The 3d models can be found in [this](https://cad.onshape.com/documents/511c9212d0a2d905b0dfb74b/w/9b3b828cfae5ab9d0141f07e/e/48d7492fafb73baf2ace87d0?renderMode=0&uiState=6998445690e5d9c5d21373ef)
Onshape document.

A custom rendering library is featured, which can render using points from any pointcloud file. It also supports dithering using an 8x8x8 Bayer matrix.

This code will only run correctly on a Raspberry Pi 4 due to processor specific GPIO settings. Other models will likely be too slow, maybe except for RPi 3.

## Running
I used DietPi as my operating system. First, connect to your Pi by SSH (desktop will work too, but make sure you are connected to the internet). Make sure there is not too much other stuff running, as this may cause artefacting of the displays. 

First clone the repo:

```
git clone https://github.com/quertzuiop/Volumetric-display.git
```
Next build the software:

``` 
(cd driver/ && mkdir build && make)
(cd speedRegulator/ && mkdir build && make)
(cd apps/showObj && mkdir build && make)
(cd apps/snake && mkdir build && make)
```
You will also need FastAPI:

```
pip install fastapi
```

Next, launch the control panel server. This will create the driver and process for reading rotation speed. It will also create a web page that can be used for keyboard input to apps running on the display.

```
cd controlPanel/backend/ && fastapi dev main.py
``` 

Now, you can run an app. I have made 2 example apps.
## .obj displayer
This app reads a .obj file and scales it to fit in the display. Textures are not supported yet.

Upload your .obj files to ```apps/showObj/assets```.
Run the app with
```
cd apps/showObj/
build/main [filename]
```
The filename should be passed without the .obj extension.
There are also a few options:
- ```-color``` possible values are ```white, black, red, green, blue, magenta, cyan, yellow```.
- ```-padding``` (float) the offset from the walls of the display. The object
- ```-thickness``` (float) the line width of the rendered wireframe.
## Snake
The second app is the classis snake game. The playing field is 5x5x8. To control the snake, press ```W, A, S, D, I, J``` while focused onto the control panel web page.

## Docs
### Creating an  app
If you wish to write you own app, you will need some boilerplate code.

Create a new folder in ```apps/```.
make folders and files like this:
```
apps/yourApp/
  |- build/
  |- main.cpp
  |- Makefile
```
You can copy and paste the makefile from one of the default apps.

In main.cpp paste this code:
``` cpp
#include "renderer.h"
#include "types.h"

int main() {
    Scene scene = Scene();
}
```
## Renderer
All objects are managed through ```Scene```. The scene has objects. Each object consists of a geometry, color and internally a transformation. There are a few types of geometry:

```ParticleGeometry```: A sphere defined by its position and radius. Is limited to one cell of the spatial index so it should not be used for large or accurate spheres. 

```CapsuleGeometry```: A line or cylinder with rounded ends, defined by a start point, end point, and radius.

```TriangleGeometry```: A triangle in 3D space defined by three vertices and a thickness.

```SphereGeometry```: A sphere defined by a center position, radius, and optional wall thickness. By default is filled in.

```CuboidGeometry```: A box defined by two opposite corner vertices (v1 and v2). It can optionally be rendered as a wireframe.

```MeshGeometry```: A custom 3D mesh with its own transformations. It can also be rendered as a wireframe. Can be loaded from an .obj file using ```loadMeshObj()``` accessible by  ```#include io.h```

```TextGeometry```: 3D text defined by a string, position, size, and an orientation in 3D space (e.g., facing POS_X, NEG_Y, etc.).

### Creating and Modifying Objects
To add an object to the scene, define its geometry and call ```scene.createObject()```. This method takes a ```Geometry``` struct, a ```Color``` (also clipping behavior but other than AND not implemented), and returns an ```ObjectId```. This ID is used to reference the object in the scene.

Colors are defined using RGB floats between 0 and 1 (e.g., ```{1.0f, 0.5f, 0.0f}```). There are also predefined constants available: ```RED, GREEN, BLUE, WHITE, BLACK, CYAN, MAGENTA, YELLOW```.

Once an object is created, you can modify it using its ```ObjectId```.

**Transformations:** 

Move, rotate, or scale the object using ```scene.setObjectTranslation(id, {x, y, z})```, ```scene.setObjectRotation(id, {x, y, z})```, or ```scene.setObjectScale(id, {x, y, z})```. You can also set an intrinsic pivot point using ```scene.setObjectIntrinsicPivot(id, {x, y, z})```.

**Appearance:**

Update an object's look on the fly using ```scene.setObjectGeometry(id, newGeometry)``` or ```scene.setObjectColor(id, newColor)```.

**Removal:** 

Delete a specific object using ```scene.removeObject(id)```, or clear the entire display by calling ```scene.wipe()```.

### Rendering and Input
**Rendering:**

To push your changes and draw the current scene to the volumetric display, call ```scene.render()```.

**Input:**

You can read user input from the control panel web interface using ```scene.getPressedKeys()```, which returns an array of the last 8 pressed characters.

### Example Usage
Here is a simple example that creates a 10x10x10 grid of colored cuboids and renders them to the display:

```cpp
#include "renderer.h"
#include "types.h"

int main() {
    Scene scene = Scene();

    float size = 4.5f;
    CuboidGeometry cube = {.v1 = {-size/2, -size/2, -size/2}, .v2 = {size/2, size, size/2} };

    // Generate a 10x10x10 grid of cubes
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 10; ++k) {
                // Create the object with an RGB color based on its position in the grid
                ObjectId newObj = scene.createObject(cube, {(float) i / 9, (float) k / 9, (float) j / 9});
                
                // Translate the object to its correct grid position
                scene.setObjectTranslation(newObj, {22.5f - size*(i+1), 22.5f - size*(j+1), size*(k+1)});
            }
        }
    }

    // Push the objects to the display
    scene.render();
}
```