#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

#include "types.h"
#include "linalg.h"
#include "grid.h"
#include "io.h"
#include "renderer.h"
#include "dither.h"
#include "shm.h"

using namespace std;

int getTime() {
    return chrono::duration_cast<chrono::microseconds> (chrono::system_clock::now().time_since_epoch()).count();
}

Mat4 Transformation::getMatrix() const {
    float x = rotation.x;
    float y = rotation.y;
    float z = rotation.z;

    Mat4 rMatrixX = {{
        {1, 0,      0,       0},
        {0, cos(x), -sin(x), 0},
        {0, sin(x),  cos(x), 0},
        {0, 0,      0,       1}
    }};
    Mat4 rMatrixY = {{
        {cos(y),  0, sin(y), 0},
        {0,       1, 0,      0},
        {-sin(y), 0, cos(y), 0},
        {0, 0, 0, 1}
    }};
    Mat4 rMatrixZ = {{
        {cos(z), -sin(z), 0, 0},
        {sin(z),  cos(z), 0, 0},
        {0,      0,       1, 0},
        {0,      0,       0, 1}
    }};
    Mat4 sMatrix = {{
        {scale.x, 0,       0,       0},
        {0,       scale.y, 0,       0},
        {0,       0,       scale.z, 0},
        {0,       0,       0,       1}
    }};
    Mat4 rMatrix = matMul(matMul(rMatrixX, rMatrixY), rMatrixZ);
    Mat4 matrix = matMul(rMatrix, sMatrix);

    matrix[0][3] = translation.x;
    matrix[1][3] = translation.y;
    matrix[2][3] = translation.z;
    // printf("new translation x in getMatrix(): %f\n", translation.x);
    return matrix;
}

Object::Object(ObjectId initId, Geometry initGeometry, Color initColor, ClippingBehavior initClippingBehavior = ADD) {
    id = initId;
    geometry = initGeometry;
    color = initColor;
    clippingBehavior = initClippingBehavior;
}

Geometry Object::getTransformedGeometry() {
    Mat4 tMatrix = transformation.getMatrix();
    const Vec3<float>& pivot = transformation.pivot;
    float maxScale = max({transformation.scale.x, transformation.scale.y, transformation.scale.z});
    Geometry res;
    visit([&](auto&& arg)
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParticleGeometry>)
        res = ParticleGeometry{
            .pos = pivot + matColMul(tMatrix, arg.pos - pivot),
            .radius = arg.radius * maxScale
        };
        else if constexpr (std::is_same_v<T, CapsuleGeometry>)
        res = CapsuleGeometry{
            .start = pivot + matColMul(tMatrix, arg.start - pivot),
            .end = pivot + matColMul(tMatrix, arg.end - pivot),
            .radius = arg.radius * maxScale
        };
        else if constexpr (std::is_same_v<T, TriangleGeometry>)
        res = TriangleGeometry{
            .v1 = pivot + matColMul(tMatrix, arg.v1 - pivot),
            .v2 = pivot + matColMul(tMatrix, arg.v2 - pivot),
            .v3 = pivot + matColMul(tMatrix, arg.v3 - pivot),
            .thickness = arg.thickness * maxScale
        };
        else if constexpr (std::is_same_v<T, SphereGeometry>)
            res = SphereGeometry{
                .pos = pivot + matColMul(tMatrix, arg.pos - pivot),
                .radius = arg.radius * maxScale
            };
        else if constexpr (std::is_same_v<T, CuboidGeometry>)
        res = CuboidGeometry{
            .v1 = pivot + matColMul(tMatrix, arg.v1 - pivot),
            .v2 = pivot + matColMul(tMatrix, arg.v2 - pivot),
            .thickness = arg.thickness * maxScale,
            .isWireframe = arg.isWireframe
        };
        else if constexpr (std::is_same_v<T, MeshGeometry>) 
        res = MeshGeometry{
            .mesh = arg.mesh,
            .isWireframe = arg.isWireframe,
            .transformation = transformation,
            .thickness = arg.thickness
        };
        else if constexpr (std::is_same_v<T, TextGeometry>) {
            res = arg;
            if (tMatrix != (Mat4) {{
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1}
            }}) {
                cerr<<"Warning: Transformation logic for text is not implemented. change geometry instead."<<endl;
            }
        }
        else
        static_assert(false, "non-exhaustive visitor!");
    }, geometry);
        
    return res;
}
void Object::setGeometry(Geometry newGeometry) {
    geometry = newGeometry;
    toRerender = true;
}
void Object::setColor(Color newColor) {
    color = newColor;
    toRerender = true;
}
void Object::translate(Vec3<float> translation) {
    transformation.translation = translation;
    toRerender = true;
}
void Object::rotate(Vec3<float> rotation) {
    transformation.rotation = rotation;
    toRerender = true;
}
void Object::scale(Vec3<float> factors) {
    transformation.scale = factors;
    toRerender = true;
}
void Object::setPivot(Vec3<float> pivot) {
    transformation.pivot = pivot;
    toRerender = true;
}

Scene::Scene() {
    cout<<"loading update pattern..."<<endl;
    UpdatePattern updatePattern = loadUpdatePattern("../../update_pattern_gen/output.txt");
    cout<<"building grid..."<<endl;
    auto [mapping_, params_] = buildGrid(updatePattern, 20);
    mapping = mapping_;
    params = params_;
    lastId = 0;

    cout<<"opening shm..."<<endl;

    shmPointer = openShm("vdshm");
    if (shmPointer == nullptr) { 
        printf("SHM failed to open");
        return; 
    }
    cout<<"wiping voxel data..."<<endl;
    for (auto& slice : shmPointer->data) {
        for (auto& voxel : slice.data) {
            voxel = 0;
        }
    }
}
ObjectId Scene::nextId() {
    printf("next id: %d", lastId+1);
    return ++lastId;
}
ObjectId Scene::createObject(const Geometry& initGeometry, const Color& initColor, ClippingBehavior initClippingBehavior) {
    ObjectId newId = nextId();
    Object newObj = Object(newId, initGeometry, initColor, initClippingBehavior);
    objects.push_back(newObj);
    // cout<<"created object "<< newId<<endl;
    idToIndex[lastId] = objects.size() - 1;
    return lastId;
}
Object& Scene::getObject(ObjectId id) {
    for (auto& object : objects) {
        if (object.getId() == id) {
            return object;
        }
    }
    throw invalid_argument("No object found with this id.");
    cerr<<"No object found"<<endl;
}
void Scene::setObjectGeometry(ObjectId id, Geometry newGeometry) {
    auto& object = getObject(id);
    object.setGeometry(newGeometry);
}

void Scene::setObjectTranslation(ObjectId id, Vec3<float> translation) {
    auto& object = getObject(id);
    object.translate(translation);
}

void Scene::setObjectScale(ObjectId id, Vec3<float> factors) {
    auto& object = getObject(id);
    object.scale(factors);
}

void Scene::setObjectRotation(ObjectId id, Vec3<float> rotation) {
    auto& object = getObject(id);
    object.rotate(rotation);
}

void Scene::setObjectIntrinsicPivot(ObjectId id, Vec3<float> newPivot) {
    auto& object = getObject(id);
    object.setPivot(newPivot);
}

void Scene::render(bool writeToFile) {
    printf("rendering %d objects\n", objects.size());
    Render render;
    for (auto objToRemove : toRemove) {
        for (RenderedPoint& lastRenderPoint : lastRender) {
            if (lastRenderPoint.objectId == objToRemove) {
                lastRenderPoint.objectId = (ObjectId)-1; // uint32_t max
                lastRenderPoint.color = BLACK;
                render.push_back(lastRenderPoint);
            }
        }
    }
    for (Object& object : objects) {
        if (object.toRerender) {
            draw(object, render);
            object.toRerender = false;
        }
    }
    printf("writing render with %d points\n", render.size());
    lastRender = render;
    if (writeToFile) {
        writeRenderToFile(render, "output/render.ply");
    } else {
        ShmVoxelFrame& frame = shmPointer->data;
        for (const RenderedPoint& renderedPoint : render) {
            const PointDisplayParams& params = renderedPoint.pointDisplayParams;
            ShmVoxelSlice& targetSlice = frame[params.sliceIndex];
            uint8_t& colIndex = params.isDisplay1 ? targetSlice.index1 : targetSlice.index2;
            colIndex = params.colIndex;

            int baseIndexNumber = (static_cast<int>(!params.isDisplay1) * 128) + static_cast<int>(!params.isSide1)*64;
            targetSlice.data[baseIndexNumber+params.rowIndex] = static_cast<uint8_t>(renderedPoint.color);
        }
    }
}

void Scene::wipe() {
    objects = {};
    ShmVoxelFrame& frame = shmPointer->data;
    for (auto& slice : frame) {
        for (auto& voxel : slice.data) {
            voxel = 0;
        }
    }
}

void Scene::removeObject(ObjectId objectId) {
    toRemove.push_back(objectId);
    for (int i = 0; i < objects.size(); i++) {
        if (objects[i].getId() == objectId) {
            objects.erase(objects.begin() + i);
        }
    }
}

KeyboardState Scene::getPressedKeys() {
    auto upper =  shmPointer->keyboardState;
    KeyboardState lower;
    for (int i = 0; i < upper.size(); i++) {
        lower[i] = std::tolower(upper[i]);
    }
    return lower;
}
