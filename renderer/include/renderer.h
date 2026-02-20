#pragma once
#include<vector>
#include<unordered_map>
#include <variant>

#include "types.h"
#include "linalg.h"
#include "dither.h"
#include "shm.h"

using namespace std;

struct Transformation {
    Vec3<float> translation = {0, 0, 0};
    Vec3<float> rotation = {0, 0, 0};
    Vec3<float> scale = {1, 1, 1};
    Vec3<float> pivot = { 0, 0, 0 };
    Mat4 getMatrix() const;
};

struct ParticleGeometry {
    Vec3<float> pos;
    float radius;
};
struct CapsuleGeometry {
    Vec3<float> start;
    Vec3<float> end;
    float radius;
};

struct TriangleGeometry {
    Vec3<float> v1;
    Vec3<float> v2;
    Vec3<float> v3;
    float thickness;
};

struct SphereGeometry {
    Vec3<float> pos;
    float radius;
    float thickness = 0.;
};

struct CuboidGeometry {
    Vec3<float> v1;
    Vec3<float> v2;
    float thickness = 0.;
    bool isWireframe = false;
};

struct MeshGeometry {
    Mesh mesh;
    bool isWireframe;
    Transformation transformation;
    float thickness = 0.;
};

enum class TextOrientation {
    POS_X, NEG_X,
    POS_Y, NEG_Y,
    POS_Z, NEG_Z
};

// Update your TextGeometry struct
struct TextGeometry {
    std::string text;
    Vec3<float> pos;
    float size;
    float thickness = 0.;
    TextOrientation orientation = TextOrientation::POS_Y;
};

using Geometry = variant<ParticleGeometry, CapsuleGeometry, TriangleGeometry, SphereGeometry, CuboidGeometry, MeshGeometry, TextGeometry>;




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
        void translate(Vec3<float> translation);
        void rotate(Vec3<float> newRotation);
        void scale(Vec3<float> newScale);
        void setPivot(Vec3<float> newPivot);

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
        void render(bool writeToFile = false);

        KeyboardState getPressedKeys();

        void setObjectGeometry(ObjectId id, Geometry newGeometry);
        void setObjectColor(ObjectId id, Color newColor);
        void setObjectTranslation(ObjectId id, Vec3<float> newTranslation);
        void setObjectRotation(ObjectId id, Vec3<float> newRotation);
        void setObjectScale(ObjectId id, Vec3<float> newScale);
        void setObjectIntrinsicPivot(ObjectId id, Vec3<float> newPivot);

        void wipe();
        void removeObject(ObjectId objectId);
        
    Scene();
    private:
        ObjectId lastId = 0;
        vector<Object> objects = {};
        vector<ObjectId> toRemove = {};
        unordered_map<ObjectId, uint32_t> idToIndex;
        ObjectId nextId();

        ShmLayout* shmPointer;

        Render lastRender = {};
        
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
        void drawText(
            const TextGeometry& geometry,
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