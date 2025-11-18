#pragma once
#include<vector>
#include<tuple>
#include<unordered_map>
#include <variant>
#include"types.h"

using namespace std;

struct ParticleGeometry {
    Vec3 pos;
    float radius;
    Vec3 hinge = {0, 0, 0};
};

struct CapsuleGeometry {
    Vec3 start;
    Vec3 end;
    float radius;
    Vec3 hinge = {0, 0, 0};
};

struct TriangleGeometry {
    Vec3 v1;
    Vec3 v2;
    Vec3 v3;
    float thickness;
    Vec3 hinge = {0, 0, 0};
};

struct SphereGeometry {
    Vec3 pos;
    float radius;
    float thickness = 0.;
    Vec3 hinge = {0, 0, 0};
};

struct CuboidGeometry {
    Vec3 v1;
    Vec3 v2;
    float thickness = 0.;
    Vec3 hinge = {0, 0, 0};
};

//struct MeshGeometry {
//    Vec3 hinge;
//    Mesh mesh;
//};

using Geometry = variant<ParticleGeometry, CapsuleGeometry, TriangleGeometry, SphereGeometry, CuboidGeometry>;

class Object {
    public:
        ObjectId getId() const { return id; }
        bool needsRerendering() const { return toRerender; }
        const Geometry& getGeometry() const { return geometry; }
        const Color& getColor() const { return color; }
        const Transformation& getTransformation() const { return transformation; }
        ClippingBehavior getClippingBehavior() const { return clippingBehavior; }

        void setGeometry(Geometry newGeometry);
        void setColor(Color newColor);
        void setTranslation(Vec3 newTranslation);
        void setRotation(Vec3 newRotation);
        void setScale(Vec3 newScale);

        Object(ObjectId initId, Geometry initGeometry, Color initColor, ClippingBehavior initClippingBehavior);
    private:
        ObjectId id;
        Geometry geometry;
        Color color;
        ClippingBehavior clippingBehavior;
        bool toRerender = true;
        Transformation transformation;
};

class Scene {
    public: 
        GridParams params;
        std::unordered_map<int, UpdatePattern> mapping;
        ObjectId createObject(const Geometry& initGeometry, const Color& initColor, ClippingBehavior initClippingBehavior=ADD);
        void render();
        void setObjectGeometry(ObjectId id, Geometry newGeometry);
        void setObjectColor(ObjectId id, Color newColor);
        void setObjectTranslation(ObjectId id, Vec3 newTranslation);
        void setObjectRotation(ObjectId id, Vec3 newRotation);
        void setObjectScale(ObjectId id, Vec3 newScale);

    Scene();
    private:
        ObjectId lastId = 0;
        vector<Object> objects = {};
        unordered_map<ObjectId, uint32_t> idToIndex;
        ObjectId nextId();

        void draw(const Object& object, Render& render);
        void drawParticle(
            const ParticleGeometry& geometry,
            const Color& color,
            const Transformation& transformation,
            ClippingBehavior clippingBehavior,
            ObjectId objectId,
            Render& render
        );
        void drawCapsule(
            const CapsuleGeometry& geometry,
            const Color& color, 
            const Transformation& transformation,
            ClippingBehavior clippingBehavior,
            ObjectId objectId, 
            Render& render
        );
        void drawTriangle(
            const TriangleGeometry& geometry, 
            const Color& color,
            const Transformation& transformation, 
            ClippingBehavior clippingBehavior,
            ObjectId objectId, 
            Render& render
        );
        void drawSphere(
            const SphereGeometry& geometry, 
            const Color& color,
            const Transformation& transformation,
            ClippingBehavior clippingBehavior,
            ObjectId objectId,
            Render& render
        );
        void drawCuboid(
            const CuboidGeometry& geometry,
            const Color& color,
            const Transformation& transformation, 
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