#include "renderer.h"
#include "types.h"
#include "grid.h"
#include <cstdio>
#include <iostream>
#include <algorithm>

void Scene::draw(Object& object, Render& render) {
    auto geometry = object.getTransformedGeometry();
    auto color = object.getColor();
    auto clippingBehavior = object.getClippingBehavior();
    auto objectId = object.getId();

    printf("-drawing object with id %d\n", (int) objectId);
    Render pointsToAdd = {};
    visit([&](auto&& arg)
    {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, ParticleGeometry>)
        drawParticle(arg, color, clippingBehavior, objectId, pointsToAdd);

    else if constexpr (std::is_same_v<T, CapsuleGeometry>)
        drawCapsule(arg, color, clippingBehavior, objectId, pointsToAdd);

    else if constexpr (std::is_same_v<T, TriangleGeometry>)
        drawTriangle(arg, color, clippingBehavior, objectId, pointsToAdd);

    else if constexpr (std::is_same_v<T, SphereGeometry>)
        drawSphere(arg, color, clippingBehavior, objectId, pointsToAdd);

    else if constexpr (std::is_same_v<T, CuboidGeometry>)
        drawCuboid(arg, color, clippingBehavior, objectId, pointsToAdd);

    else if constexpr (std::is_same_v<T, MeshGeometry>)
        drawMesh(arg, object.getTransformation(), color, clippingBehavior, objectId, pointsToAdd);

    else if constexpr (std::is_same_v<T, TextGeometry>)
        drawText(arg, color, clippingBehavior, objectId, pointsToAdd);
    
    else
        static_assert(false, "non-exhaustive visitor!");
    }, geometry);

    //add negative points (remove ones from last render)
    for (RenderedPoint& lastRenderPoint : lastRender) {
        if (lastRenderPoint.objectId == objectId) {
            lastRenderPoint.objectId = (ObjectId)-1; // uint32_t max
            lastRenderPoint.color = BLACK;
            render.push_back(lastRenderPoint);
        }
    }
    render.insert(render.end(), pointsToAdd.begin(), pointsToAdd.end());
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
        if (d2 <= radius2) render.push_back({objectId, pt.pointDisplayParams, pos, pt.normal, dither(color, potentialPtCoords), clippingBehavior});
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

    auto vec = end - start;
    auto [minV, maxV] = arrangeBoundingBox(start, end);
    auto bucketIndices = calculateIndicesFromBB(params, minV, maxV, radius);

    for (int bucketIndex : bucketIndices) {
        auto it = mapping.find(bucketIndex);
        if (it == mapping.end()) continue;
        const UpdatePattern& bucket = it->second;
        for (const UpdatePatternPoint& pt : bucket) {
            const Vec3<float>& ptCoords = pt.pos;
            auto v1 = ptCoords-start;
            auto v2 = ptCoords-end;
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

            if (d2 < radius2 ) render.push_back({ objectId, pt.pointDisplayParams, ptCoords, pt.normal, dither(color, ptCoords), clippingBehavior });
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

    Vec3<float> minV = {
        min(min(v1.x, v2.x), v3.x),
        min(min(v1.y, v2.y), v3.y),
        min(min(v1.z, v2.z), v3.z),
    };
    Vec3<float> maxV = {
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
        if (it == mapping.end()) continue;
        const UpdatePattern& bucket = it->second;

        for (const UpdatePatternPoint& pt : bucket) {
            const auto& ptCoords = pt.pos;

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
            if (d2 < thickness2) render.push_back({ objectId, pt.pointDisplayParams, ptCoords, pt.normal, dither(color, ptCoords), clippingBehavior });
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
        if (it == mapping.end()) continue;
        const UpdatePattern& bucket = it->second;

        for (const UpdatePatternPoint& pt : bucket) {
            const auto& ptCoords = pt.pos;
            float d2 = dist2(ptCoords, pos);
            //printf("d2: %f, r2: %f\n");
            if (thickness > 0 && d2 < (2 * radius * thickness - radius2)) continue; // magic math supr
            if (d2 < radius2) render.push_back({ objectId, pt.pointDisplayParams, ptCoords, pt.normal, dither(color, ptCoords), clippingBehavior });
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
    printf("wireframe: %d\n", geometry.isWireframe);
    auto& v1 = geometry.v1;
    auto& v2 = geometry.v2;
    auto thickness = geometry.thickness;
    
    auto [minV, maxV] = arrangeBoundingBox(v1, v2);
    printf("params: %f %f %d\n", params.boundingBoxMax.x, params.cellSizes.x, params.gridSize);

    if (not geometry.isWireframe) {
        auto bucketIndices = calculateIndicesFromBB(params, minV, maxV);
        printf("got %d bucket indices\n", bucketIndices.size());
        for (int bucketIndex : bucketIndices) {
            auto it = mapping.find(bucketIndex);
            if (it == mapping.end()) continue;
            const UpdatePattern& bucket = it->second;
            for (const UpdatePatternPoint& pt : bucket) {
                const auto& ptCoords = pt.pos;
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
                    render.push_back({ objectId, pt.pointDisplayParams, ptCoords, pt.normal, dither(color, ptCoords), clippingBehavior });;
                }
            }
        }
    } else {
        //draw only edges, not diagonals
        for (uint combinedCoord1 = 0; combinedCoord1 < 2*2*2; combinedCoord1++) {            
            Vec3 p1 = {
                (combinedCoord1 & 1) ? minV.x : maxV.x,
                (combinedCoord1 & 2) ? minV.y : maxV.y,
                (combinedCoord1 & 4) ? minV.z : maxV.z
            };
            for (uint shift : array<uint, 3> {1, 2, 4}) {
                if ((combinedCoord1 & shift) != 0) { continue; } // new combined would be outside cube / break due to carry
                uint combinedCoord2 = combinedCoord1 + shift;

                Vec3 p2 = {
                    (combinedCoord2 & 1) ? minV.x : maxV.x, 
                    (combinedCoord2 & 2) ? minV.y : maxV.y, 
                    (combinedCoord2 & 4) ? minV.z : maxV.z
                }; 
                drawCapsule({p1, p2, thickness}, color, clippingBehavior, objectId, render);
            }
        }
    }
    cout<<render.size()<<endl;
}


void Scene::drawMesh(
    const MeshGeometry& geometry,
    const Transformation& transformation,
    const Color& color,
    ClippingBehavior clippingBehavior,
    ObjectId objectId, 
    Render& render
) {
    const Mesh& mesh = geometry.mesh;
    const auto& vertices = mesh.vertices;
    const auto& faces = mesh.faces;

    const auto& tMatrix = transformation.getMatrix();
    float maxScale = max(max(transformation.scale.x, transformation.scale.y), transformation.scale.z);

    printf("2| n vert. of mesh: %d\n", vertices.size());

    bool isWireframe = geometry.isWireframe;
    if (isWireframe) {
        for (const auto& face : faces) {
            for (int i = 0; i < 3-1; ++i) {
                for (int j = i; j < 3; ++j) {
                    drawCapsule(
                        {
                            matColMul(tMatrix, vertices[face[i]]), 
                            matColMul(tMatrix, vertices[face[j]]),
                            geometry.thickness
                        },
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
                {
                    matColMul(tMatrix, vertices[face[0]]),
                    matColMul(tMatrix, vertices[face[1]]), 
                    matColMul(tMatrix, vertices[face[2]]), 
                    geometry.thickness
                },
                color,
                clippingBehavior,
                objectId,
                render
            );
        }
    }
}

void Scene::drawText(
    const TextGeometry& geometry,
    const Color& color,
    ClippingBehavior clippingBehavior,
    ObjectId objectId,
    Render& render
) {
    float s = geometry.size;
    float t = geometry.thickness;
    float cursorOffset = 0.0f;

    auto mapCoords = [&](float cx, float cy) -> Vec3<float> {
        float actualX = (cx + cursorOffset) * s;
        float actualY = cy * s;
        Vec3<float> p = geometry.pos;
        
        switch (geometry.orientation) {
            case TextOrientation::POS_Z: return {p.x - actualX, p.y - actualY, p.z};
            case TextOrientation::NEG_Z: return {p.x + actualX, p.y - actualY, p.z};
            case TextOrientation::POS_X: return {p.x, p.y - actualX, p.z + actualY};
            case TextOrientation::NEG_X: return {p.x, p.y + actualX, p.z + actualY};
            case TextOrientation::POS_Y: return {p.x - actualX, p.y, p.z + actualY};
            case TextOrientation::NEG_Y: return {p.x + actualX, p.y, p.z + actualY};
            default: return {p.x + actualX, p.y + actualY, p.z};
        }
    };

    auto addLine = [&](float x1, float y1, float x2, float y2) {
        Vec3<float> start = mapCoords(x1, y1);
        Vec3<float> end = mapCoords(x2, y2);
        CapsuleGeometry cap = {start, end, t};
        drawCapsule(cap, color, clippingBehavior, objectId, render);
    };

    for (char c : geometry.text) {
        c = std::toupper(c);
        
        if (c == 'A') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0); addLine(0,0.5, 1,0.5); }
        else if (c == 'B') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0); addLine(1,0, 0,0); addLine(0,0.5, 1,0.5); }
        else if (c == 'C') { addLine(1,1, 0,1); addLine(0,1, 0,0); addLine(0,0, 1,0); }
        else if (c == 'D') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0); addLine(1,0, 0,0); }
        else if (c == 'E') { addLine(1,1, 0,1); addLine(0,1, 0,0); addLine(0,0, 1,0); addLine(0,0.5, 1,0.5); }
        else if (c == 'F') { addLine(1,1, 0,1); addLine(0,1, 0,0); addLine(0,0.5, 1,0.5); }
        else if (c == 'G') { addLine(1,1, 0,1); addLine(0,1, 0,0); addLine(0,0, 1,0); addLine(1,0, 1,0.5); addLine(0.5,0.5, 1,0.5); }
        else if (c == 'H') { addLine(0,1, 0,0); addLine(1,1, 1,0); addLine(0,0.5, 1,0.5); }
        else if (c == 'I') { addLine(0.5,1, 0.5,0); addLine(0,1, 1,1); addLine(0,0, 1,0); }
        else if (c == 'J') { addLine(1,1, 1,0); addLine(1,0, 0,0); addLine(0,0, 0,0.5); }
        else if (c == 'K') { addLine(0,1, 0,0); addLine(1,1, 0,0.5); addLine(0,0.5, 1,0); }
        else if (c == 'L') { addLine(0,1, 0,0); addLine(0,0, 1,0); }
        else if (c == 'M') { addLine(0,0, 0,1); addLine(0,1, 0.5,0.5); addLine(0.5,0.5, 1,1); addLine(1,1, 1,0); }
        else if (c == 'N') { addLine(0,0, 0,1); addLine(0,1, 1,0); addLine(1,0, 1,1); }
        else if (c == 'O') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0); addLine(1,0, 0,0); }
        else if (c == 'P') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0.5); addLine(1,0.5, 0,0.5); }
        else if (c == 'Q') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0); addLine(1,0, 0,0); addLine(0.5,0.5, 1,0); }
        else if (c == 'R') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0.5); addLine(1,0.5, 0,0.5); addLine(0,0.5, 1,0); }
        else if (c == 'S') { addLine(1,1, 0,1); addLine(0,1, 0,0.5); addLine(0,0.5, 1,0.5); addLine(1,0.5, 1,0); addLine(1,0, 0,0); }
        else if (c == 'T') { addLine(0,1, 1,1); addLine(0.5,1, 0.5,0); }
        else if (c == 'U') { addLine(0,1, 0,0); addLine(0,0, 1,0); addLine(1,0, 1,1); }
        else if (c == 'V') { addLine(0,1, 0.5,0); addLine(0.5,0, 1,1); }
        else if (c == 'W') { addLine(0,1, 0,0); addLine(0,0, 0.5,0.5); addLine(0.5,0.5, 1,0); addLine(1,0, 1,1); }
        else if (c == 'X') { addLine(0,1, 1,0); addLine(1,1, 0,0); }
        else if (c == 'Y') { addLine(0,1, 0.5,0.5); addLine(1,1, 0.5,0.5); addLine(0.5,0.5, 0.5,0); }
        else if (c == 'Z') { addLine(0,1, 1,1); addLine(1,1, 0,0); addLine(0,0, 1,0); }
        else if (c == '0') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0); addLine(1,0, 0,0); addLine(0,0, 1,1); }
        else if (c == '1') { addLine(0.5,0, 0.5,1); addLine(0.25,0.75, 0.5,1); }
        else if (c == '2') { addLine(0,1, 1,1); addLine(1,1, 1,0.5); addLine(1,0.5, 0,0.5); addLine(0,0.5, 0,0); addLine(0,0, 1,0); }
        else if (c == '3') { addLine(0,1, 1,1); addLine(1,1, 1,0); addLine(1,0, 0,0); addLine(0,0.5, 1,0.5); }
        else if (c == '4') { addLine(0,1, 0,0.5); addLine(0,0.5, 1,0.5); addLine(1,1, 1,0); }
        else if (c == '5') { addLine(1,1, 0,1); addLine(0,1, 0,0.5); addLine(0,0.5, 1,0.5); addLine(1,0.5, 1,0); addLine(1,0, 0,0); }
        else if (c == '6') { addLine(1,1, 0,1); addLine(0,1, 0,0); addLine(0,0, 1,0); addLine(1,0, 1,0.5); addLine(1,0.5, 0,0.5); }
        else if (c == '7') { addLine(0,1, 1,1); addLine(1,1, 1,0); }
        else if (c == '8') { addLine(0,0, 0,1); addLine(0,1, 1,1); addLine(1,1, 1,0); addLine(1,0, 0,0); addLine(0,0.5, 1,0.5); }
        else if (c == '9') { addLine(1,0, 1,1); addLine(1,1, 0,1); addLine(0,1, 0,0.5); addLine(0,0.5, 1,0.5); }
        else if (c == '-') { addLine(0,0.5, 1,0.5); }
        else if (c == '_') { addLine(0,0, 1,0); }
        
        cursorOffset += 1.5f; 
    }
}