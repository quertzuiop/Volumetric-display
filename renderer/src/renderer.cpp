#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <regex>
#include <array>
#include <unordered_map>
#include <sstream>
#include <chrono>

#include "../include/types.h"
#include "../include/math.h"
#include "../include/grid.h"
#include "../include/io.h"
#include "../include/renderer.h"
using namespace std;

int getTime() {
    return chrono::duration_cast<chrono::microseconds> (chrono::system_clock::now().time_since_epoch()).count();
}

Mat4 Transformation::getMatrix() {
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
    float maxScale = max(max(transformation.scale.x, transformation.scale.y), transformation.scale.z);
    Geometry res;
    visit([&](auto&& arg)
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParticleGeometry>)
            res = ParticleGeometry{
                .pos = matColMul(tMatrix, arg.pos),
                .radius = arg.radius * maxScale
            };
        else if constexpr (std::is_same_v<T, CapsuleGeometry>)
            res = CapsuleGeometry{
                .start = matColMul(tMatrix, arg.start),
                .end = matColMul(tMatrix, arg.end),
                .radius = arg.radius * maxScale
            };
        else if constexpr (std::is_same_v<T, TriangleGeometry>)
            res = TriangleGeometry{
                .v1 = matColMul(tMatrix, arg.v1),
                .v2 = matColMul(tMatrix, arg.v2),
                .v3 = matColMul(tMatrix, arg.v3),
                .thickness = arg.thickness * maxScale
            };
        else if constexpr (std::is_same_v<T, SphereGeometry>)
            res = SphereGeometry{
                .pos = matColMul(tMatrix, arg.pos),
                .radius = arg.radius * maxScale
            };
        else if constexpr (std::is_same_v<T, CuboidGeometry>)
            res = CuboidGeometry{
                .v1 = matColMul(tMatrix, arg.v1),
                .v2 = matColMul(tMatrix, arg.v2),
                .thickness = arg.thickness * maxScale
            };
        else if constexpr (std::is_same_v<T, MeshGeometry>) {}
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
void Object::translate(Vec3 translation) {
    transformation.translation = translation;
    toRerender = true;
}
void Object::rotate(Vec3 rotation, Vec3 pivot) {
    transformation.rotation = rotation;
    transformation.pivot = pivot;
    toRerender = true;
}
void Object::scale(Vec3 factors, Vec3 pivot) {
    transformation.scale = factors;
    transformation.pivot = pivot;
    toRerender = true;
}

Scene::Scene() {
    UpdatePattern updatePattern = loadUpdatePattern("../../update_pattern_gen/output.txt");
    auto [mapping_, params_] = buildGrid(updatePattern, 20);
    mapping = mapping_;
    params = params_;
    lastId = 0;
}
ObjectId Scene::nextId() {
    return lastId++;
}
ObjectId Scene::createObject(const Geometry& initGeometry, const Color& initColor, ClippingBehavior initClippingBehavior) {
    Object newObj = Object(nextId(), initGeometry, initColor, initClippingBehavior);
    objects.push_back(newObj);
    cout<<"created object "<< objects[objects.size()].getId()<<endl;
    idToIndex[lastId] = objects.size() - 1;
    return lastId;
}
void Scene::setObjectTranslation(ObjectId id, Vec3 translation) {

}
void Scene::render() {
    printf("rendering %d objects\n", objects.size());
    Render render;
    for (Object& object : objects) {
        if (object.toRerender) {
            draw(object, render);
            object.toRerender = false;
        }
    }
    writeRenderToFile(render, "output/render.ply");
}

void Scene::draw(Object& object, Render& render) {
    auto geometry = object.getTransformedGeometry();
    auto color = object.getColor();
    auto clippingBehavior = object.getClippingBehavior();
    auto objectId = object.getId();

    printf("-drawing object with id %d\n", (int) objectId);

    visit([&](auto&& arg)
    {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, ParticleGeometry>)
        drawParticle(arg, color, clippingBehavior, objectId, render);

    else if constexpr (std::is_same_v<T, CapsuleGeometry>)
        drawCapsule(arg, color, clippingBehavior, objectId, render);

    else if constexpr (std::is_same_v<T, TriangleGeometry>)
        drawTriangle(arg, color, clippingBehavior, objectId, render);

    else if constexpr (std::is_same_v<T, SphereGeometry>)
        drawSphere(arg, color, clippingBehavior, objectId, render);

    else if constexpr (std::is_same_v<T, CuboidGeometry>)
        drawCuboid(arg, color, clippingBehavior, objectId, render);

    else if constexpr (std::is_same_v<T, MeshGeometry>)
        drawMesh(arg, color, clippingBehavior, objectId, render);
    
    else
        static_assert(false, "non-exhaustive visitor!");
    }, geometry);
}


void Scene::drawParticle( //can have parts cut off, points sampled from 1 cell
    const ParticleGeometry& geometry,
    const Color& color,
    ClippingBehavior clippingBehavior,
    ObjectId objectId,
    Render& render
){
    auto& pos = geometry.pos;    

    int index = calculateIndex(params, pos);
    auto it = mapping.find(index);
    if (it == mapping.end()) return;
    const UpdatePattern& bucket = it->second;

    float radius2 = pow(geometry.radius, 2);

    for (const UpdatePatternPoint& pt : bucket) {
        Vec3 potentialPtCoords = pt.pos;
        double d2 = dist2(pos, potentialPtCoords);
        if (d2 <= radius2) render.push_back({objectId, pt.pointDisplayParams, pos, pt.normal, color, clippingBehavior});
    }
}

void Scene::drawCapsule(
    const CapsuleGeometry& geometry,
    const Color& color,
    ClippingBehavior clippingBehavior,
    ObjectId objectId,
    Render& render
) {
    auto& start = geometry.start;
    auto& end = geometry.end;
    auto radius = geometry.radius;

    float length = dist(start, end);
    float length2 = length * length;
    float radius2 = radius * radius;

    Vec3 vec = end - start;

    auto [minV, maxV] = arrangeBoundingBox(start, end);
    
    auto bucketIndices = calculateIndicesFromBB(params, minV, maxV, radius);

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return;
        const UpdatePattern& bucket = it->second;
        for (const UpdatePatternPoint& pt : bucket) {
            const Vec3& ptCoords = pt.pos;
            Vec3 v1 = ptCoords-start;
            Vec3 v2 = ptCoords-end;
            // float d12 = dist(ptCoords, start);
            // float d22 = dist(ptCoords, end);
            
            // if (d1 + d2 > length + radius) continue; // discard point outside ellipsoid
            float dot1 = dot(v1, vec);
            float dot2 = dot(v2, vec);

            float d2;
            if (dot1 >= 0 and dot2 >=0) {
                d2 = dist2(ptCoords, end);
            } 
            else if (dot1 <= 0 and dot2 <= 0) {
                d2 = dist2(ptCoords, start);
            } 
            else {
                d2 = magnitude_2(cross(vec, v1)) / length2;
            }

            if (d2 < radius2 ) render.push_back({ objectId, pt.pointDisplayParams, ptCoords, pt.normal, color, clippingBehavior });
        }
    }
}

void Scene::drawTriangle(
    const TriangleGeometry& geometry,
    const Color& color,
    ClippingBehavior clippingBehavior,
    ObjectId objectId,
    Render& render
) {
    auto& v1 = geometry.v1;
    auto& v2 = geometry.v2;
    auto& v3 = geometry.v3;
    auto thickness = geometry.thickness;

    Vec3 minV = {
        min(min(v1.x, v2.x), v3.x),
        min(min(v1.y, v2.y), v3.y),
        min(min(v1.z, v2.z), v3.z),
    };
    Vec3 maxV = {
        max(max(v1.x, v2.x), v3.x),
        max(max(v1.y, v2.y), v3.y),
        max(max(v1.z, v2.z), v3.z),
    };

    auto bucketIndices = calculateIndicesFromBB(params, minV, maxV, thickness);

    Vec3 v21 = v2 - v1;
    Vec3 v32 = v3 - v2;
    Vec3 v13 = v1 - v3;

    float magV21 = magnitude_2(v21);
    float magV32 = magnitude_2(v32);
    float magV13 = magnitude_2(v13);

    Vec3 normal = cross(v21, v13);
    float magNormal = magnitude_2(normal);

    Vec3 c21 = cross(v21, normal);
    Vec3 c32 = cross(v32, normal);
    Vec3 c13 = cross(v13, normal);

    float thickness2 = thickness * thickness;

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return;
        const UpdatePattern& bucket = it->second;

        for (const UpdatePatternPoint& pt : bucket) {
            const Vec3& ptCoords = pt.pos;

            Vec3 p1 = ptCoords - v1;
            Vec3 p2 = ptCoords - v2;
            Vec3 p3 = ptCoords - v3;

            bool inside = (sgn(dot(c21, p1)) + sgn(dot(c21, p1)) + sgn(dot(c21, p1))) < 2.;
            float d2;
            if (inside) {
                d2 = min(min(
                    magnitude_2(v21 * clamp(dot(v21, p1) / magV21, (float)0., (float)1.) - p1),
                    magnitude_2(v32 * clamp(dot(v32, p2) / magV32, (float)0., (float)1.) - p2)),
                    magnitude_2(v13 * clamp(dot(v13, p3) / magV13, (float)0., (float)1.) - p3));
            }
            else {
                d2 = pow(dot(normal, p1), 2) /magNormal;
            }
            if (d2 < thickness2) render.push_back({ objectId, pt.pointDisplayParams, ptCoords, pt.normal, color, clippingBehavior });
        }
    }
}

void Scene::drawSphere (
    const SphereGeometry& geometry,
    const Color& color,
    ClippingBehavior clippingBehavior,
    ObjectId objectId,
    Render& render
) {
    auto& pos = geometry.pos;
    auto radius = geometry.radius;
    auto thickness = geometry.thickness;

    printf("-sphere: pos coords: %f, %f, %f\n", pos.x, pos.y, pos.z);
    printf("-radius: %f\n", radius);
    printf("-params: %f %f %\n", params.boundingBoxMax.x, params.cellSizes.x, params.gridSize);

    auto bucketIndices = calculateIndicesFromBB(params, pos, pos, radius);

    float radius2 = radius * radius;

    printf("-got %d bucket indices", (int) bucketIndices.size());

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return;
        const UpdatePattern& bucket = it->second;

        for (const UpdatePatternPoint& pt : bucket) {
            const Vec3& ptCoords = pt.pos;
            float d2 = dist2(ptCoords, pos);
            //printf("d2: %f, r2: %f\n");
            if (thickness > 0 && d2 < (2 * radius * thickness - radius2)) continue; // magic math supr
            if (d2 < radius2) render.push_back({ objectId, pt.pointDisplayParams, ptCoords, pt.normal, color, clippingBehavior });;
        }
    }
}


void Scene::drawCuboid(
    const CuboidGeometry& geometry,
    const Color& color,
    ClippingBehavior clippingBehavior,
    ObjectId objectId,
    Render& render
) {
    cout<<"drawing cuboid"<<endl;
    auto& v1 = geometry.v1;
    auto& v2 = geometry.v2;
    auto thickness = geometry.thickness;
    
    auto [minV, maxV] = arrangeBoundingBox(v1, v2);
    printf("params: %f %f %d\n", params.boundingBoxMax.x, params.cellSizes.x, params.gridSize);

    auto bucketIndices = calculateIndicesFromBB(params, minV, maxV);
    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) return;
        const UpdatePattern& bucket = it->second;

        for (const UpdatePatternPoint& pt : bucket) {
            const Vec3& ptCoords = pt.pos;
            if (thickness > 0 &&
                minV.x + thickness < ptCoords.x &&
                minV.y + thickness < ptCoords.y &&
                minV.z + thickness < ptCoords.z &&
                maxV.x - thickness > ptCoords.x &&
                maxV.y - thickness > ptCoords.y &&
                maxV.z - thickness > ptCoords.z)continue;

            if (minV.x < ptCoords.x &&
                minV.y < ptCoords.y &&
                minV.z < ptCoords.z &&
                maxV.x > ptCoords.x &&
                maxV.y > ptCoords.y &&
                maxV.z > ptCoords.z){
                    render.push_back({ objectId, pt.pointDisplayParams, ptCoords, pt.normal, color, clippingBehavior });;
                }
                    
        }
    }
    cout<<render.size()<<endl;
}

void Scene::drawMesh(
    const MeshGeometry& geometry,
    const Color& color,
    ClippingBehavior clippingBehavior,
    ObjectId objectId, 
    Render& render
) {
    const Mesh& mesh = geometry.mesh;
    const auto& vertices = mesh.vertices;
    const auto& faces = mesh.faces;

    printf("2| n vert. of mesh: %d\n", vertices.size());

    bool isWireframe = geometry.isWireframe;
    if (isWireframe) {
        for (const auto& face : faces) {
            for (int i = 0; i < 3-1; ++i) {
                for (int j = i; j < 3; ++j) {
                    drawCapsule(
                        {vertices[face[i]], vertices[face[j]], geometry.thickness},
                        color,
                        clippingBehavior,
                        objectId,
                        render
                    );
                }
            }
        }
    }
    else {
        for (const auto& face : faces) {
            drawTriangle(
                {vertices[face[0]], vertices[face[1]], vertices[face[2]], geometry.thickness},
                color,
                clippingBehavior,
                objectId,
                render
            );
        }
    }
}


// int main() {
//     string pt_cloud_path = "C:/Users/robik/volumetric display simulation/pythonScripts/Volumetric-display/update_pattern_gen/pointcloud.ply";
//     ifstream file(pt_cloud_path);
//     stringstream buffer;

//     buffer << file.rdbuf();
//     string fileStr = buffer.str();

//     ptCloud points;
//     int pointCountTarget = 0;
//     int i = 0;


//     vector<string> splitFileStr = split(fileStr, "\n");

//     for (const string& line : splitFileStr) {
//         if (i%50000 == 0) cout << i << endl;
//         i++;
//         if (line.find("element vertex") != string::npos) {
//             pointCountTarget = stoi(line.substr(15, string::npos));
//             cout << pointCountTarget << endl;
//         }
//         auto floats = extractPointCloudData(line);
//         if (floats.size() < 6) continue;
//         points.push_back({ 
//             Vec3{ floats[0], floats[1], floats[2] }, 
//             Vec3{ floats[3], floats[4], floats[5] } 
//             });
//     }

//     if (points.size() != pointCountTarget) cout << "Expected " << pointCountTarget << " Points but got " << points.size() << endl;
//     cout << "loaded points" << endl;

//     int starttime = getTime();

//     auto [mapping, params] = buildGrid(points, 25);
//     printf("built grid in: %d us\n", getTime() - starttime);

//     ptCloud test;
//     Mesh sphere = loadMeshObj("C:/Users/robik/volumetric display simulation/pythonScripts/test.obj");
//     //vertices
     
//     starttime = getTime();
    
//     for (const Vec3& ptCoords : sphere.vertices) {
//         Vec3 scaledPtCoords = transform(ptCoords, 0, 0, 20, 20);
//         ptCloud newPts = drawParticle(scaledPtCoords, mapping, params, 0.7);
//         test.insert(test.end(), newPts.begin(), newPts.end());
//     }
//     printf("drew sphere %d points in: %d us\n", sphere.vertices.size(), getTime() - starttime);
//     //edges
    
//     starttime = getTime();

//     for (const auto& lineIndices : sphere.edges) {
//         const Vec3& a = sphere.vertices[lineIndices.first], b = sphere.vertices[lineIndices.second];
//         Vec3 as = transform(a, 0, 0, 27.5, 27);
//         Vec3 bs = transform(b, 0, 0, 27.5, 27);
//         ptCloud newPts = drawLine(as, bs, mapping, params, 0.6);
//         test.insert(test.end(), newPts.begin(), newPts.end());
//     }
//     printf("drew %d sphere edges in: %d us\n", sphere.edges.size(), getTime() - starttime);

//     starttime = getTime();

//     for (const auto& lineIndices : sphere.faces) {
//         const Vec3& a = sphere.vertices[lineIndices[0]], b = sphere.vertices[lineIndices[1]], c = sphere.vertices[lineIndices[2]];
//         Vec3 as = transform(a, 0, 0, 21, 20);
//         Vec3 bs = transform(b, 0, 0, 21, 20);
//         Vec3 cs = transform(c, 0, 0, 21, 20);
//         ptCloud newPts = drawTriangle(as, bs, cs, mapping, params, 0.2);
//         test.insert(test.end(), newPts.begin(), newPts.end());
//     }
//     printf("drew %d sphere faces in: %d us\n", sphere.faces.size(), getTime() - starttime);

//     cout << "writing" << endl;
//     writePtcloudToFile(test, "C:/Users/robik/Downloads/test.ply");
// }