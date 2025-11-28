#pragma once
#include<vector>
#include<tuple>
#include<unordered_map>
#include <variant>
#include "types.h"
#include "math.h"

using namespace std;

struct Transformation {
    Vec3 translation = {0, 0, 0};
    Vec3 rotation = {0, 0, 0};
    Vec3 scale = {1, 1, 1};
    Vec3 pivot = { 0, 0, 0 };
    Mat4 getMatrix() const;
};

struct ParticleGeometry {
    Vec3 pos;
    float radius;
};
struct CapsuleGeometry {
    Vec3 start;
    Vec3 end;
    float radius;
};

struct TriangleGeometry {
    Vec3 v1;
    Vec3 v2;
    Vec3 v3;
    float thickness;
};

struct SphereGeometry {
    Vec3 pos;
    float radius;
    float thickness = 0.;
};

struct CuboidGeometry {
    Vec3 v1;
    Vec3 v2;
    float thickness = 0.;
};

struct MeshGeometry {
    Mesh mesh;
    bool isWireframe;
    Transformation transformation;
    float thickness = 0.;
};

using Geometry = variant<ParticleGeometry, CapsuleGeometry, TriangleGeometry, SphereGeometry, CuboidGeometry, MeshGeometry>;




class Object {
    public:
        bool toRerender = true;
        ObjectId getId() const { return id; }
        const Geometry& getGeometry() const { return geometry; }
        const Transformation& getTransformation() const { return transformation; }
        const Color& getColor() const { return color; }
        ClippingBehavior getClippingBehavior() const { return clippingBehavior; }
        
        Geometry getTransformedGeometry();
        void setGeometry(Geometry newGeometry);
        void setColor(Color newColor);
        void translate(Vec3 translation);
        void rotate(Vec3 newRotation);
        void scale(Vec3 newScale);
        void setPivot(Vec3 newPivot);

        Object(ObjectId initId, Geometry initGeometry, Color initColor, ClippingBehavior initClippingBehavior);
    private:
        const Geometry& transformGeometry();
        ObjectId id;
        Geometry geometry;
        Transformation transformation;
        ClippingBehavior clippingBehavior;
        Color color;
};

class Scene {
    public: 
        GridParams params;
        std::unordered_map<int, UpdatePattern> mapping;
        ObjectId createObject(const Geometry& initGeometry, const Color& initColor, ClippingBehavior initClippingBehavior=ADD);
        Object& getObject(ObjectId);
        void render();
        void setObjectGeometry(ObjectId id, Geometry newGeometry);
        void setObjectColor(ObjectId id, Color newColor);
        void setObjectTranslation(ObjectId id, Vec3 newTranslation);
        void setObjectRotation(ObjectId id, Vec3 newRotation);
        void setObjectScale(ObjectId id, Vec3 newScale);
        void setObjectIntrinsicPivot(ObjectId id, Vec3 newPivot);

    Scene();
    private:
        ObjectId lastId = 0;
        vector<Object> objects = {};
        unordered_map<ObjectId, uint32_t> idToIndex;
        ObjectId nextId();

        void draw(Object& object, Render& render);
        void drawParticle(
            const ParticleGeometry& geometry,
            const Color& color,
            ClippingBehavior clippingBehavior,
            ObjectId objectId,
            Render& render
        );
        void drawCapsule(
            const CapsuleGeometry& geometry,
            const Color& color, 
            ClippingBehavior clippingBehavior,
            ObjectId objectId, 
            Render& render
        );
        void drawTriangle(
            const TriangleGeometry& geometry, 
            const Color& color,
            ClippingBehavior clippingBehavior,
            ObjectId objectId, 
            Render& render
        );
        void drawSphere(
            const SphereGeometry& geometry, 
            const Color& color,
            ClippingBehavior clippingBehavior,
            ObjectId objectId,
            Render& render
        );
        void drawCuboid(
            const CuboidGeometry& geometry,
            const Color& color,
            ClippingBehavior clippingBehavior,
            ObjectId objectId, 
            Render& render
        );
        void drawMesh(
            const MeshGeometry& geometry,
            const Transformation& transformation,
            const Color& color,
            ClippingBehavior clippingBehavior,
            ObjectId objectId, 
            Render& render
        );
};
// scene = Scene()
// snakeContainer = scene.addContainer()
// 
// snakeContainer.push_back(drawCylinder())
// snakeContainer.pop(0)

// scene = Scene()
// scene.add("snakeSegment5", drawCylinder())
// scene.remove("snakeSegment0")
// scene.move("player", 1, 0, 0)
// snakeContainer.pop(0)