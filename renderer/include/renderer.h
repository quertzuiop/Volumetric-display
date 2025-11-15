#pragma once
#include<vector>
#include<tuple>
#include<unordered_map>
#include"types.h"

using namespace std;

struct ParticleGeometry {
    Vec3 hinge;
    Vec3 pos;
    float radius;
};

struct CapsuleGeometry {
    Vec3 hinge;
    Vec3 start;
    Vec3 end;
    float radius;
};

struct TriangleGeometry {
    Vec3 hinge;
    Vec3 v1;
    Vec3 v2;
    Vec3 v3;
    float thickness;
};

struct SphereGeometry {
    Vec3 hinge;
    Vec3 pos;
    float radius;
    float thickness = 0.;
};

struct CuboidGeometry {
    Vec3 hinge;
    Vec3 v1;
    Vec3 v2;
    float thickness = 0.;
};

struct MeshGeometry {
    Vec3 hinge;
    Mesh mesh;
};

using Geometry = std::variant<ParticleGeometry, CapsuleGeometry, TriangleGeometry, SphereGeometry, CuboidGeometry, MeshGeometry>;

class Object {
    public:
        ObjectId getId();
        void setGeometry(Geometry newGeometry);
        void setColor(Color newColor);
        void setTranslation(Vec3 newTranslation);
        void setRotation(Vec3 newRotation);
        void setScale(Vec3 newScale);

        Object(ObjectId initId, Geometry initGeometry, Color initColor, ClippingBehavior initClippingBehavior = ADD);
    private:
        ObjectId id;
        Geometry geometry;
        Color color;
        ClippingBehavior clippinBehavior;

        Vec3 translation = { 0, 0, 0 };
        Vec3 rotation = { 0, 0, 0 };
        Vec3 scale = { 1, 1, 1 };
};

class Scene {
    public: 
        GridParams params;
        std::unordered_map<int, ptCloud> mapping;
        ObjectId createObject(Geometry initGeometry, Color initColor);
        void render();
        void setObjectGeometry(ObjectId id, Geometry newGeometry);
        void setObjectColor(ObjectId id, Color newColor);
        void setObjectTranslation(ObjectId id, Vec3 newTranslation);
        void setObjectRotation(ObjectId id, Vec3 newRotation);
        void setObjectScale(ObjectId id, Vec3 newScale);

        ptCloud drawParticle( //can have parts cut off, points sampled from 1 cell
            const Vec3& pos,
            const unordered_map<int, ptCloud>& mapping,
            const GridParams& params,
            float radius, bool useMnht = false
        );
        ptCloud drawLine(
            const Vec3& start,
            const Vec3& end,
            const unordered_map<int, ptCloud>& mapping,
            const GridParams& params,
            float radius
        );
        ptCloud drawTriangle(
            const Vec3& v1,
            const Vec3& v2,
            const Vec3& v3,
            const unordered_map<int, ptCloud>& mapping,
            const GridParams& params,
            float radius
        );
        ptCloud drawSphere(
            const Vec3& pos,
            const unordered_map<int, ptCloud>& mapping,
            const GridParams& params,
            float radius, float thickness = 0.
        );
        ptCloud drawCuboid( //untested
            const Vec3& v1,
            const Vec3& v2,
            const unordered_map<int, ptCloud>& mapping,
            const GridParams& params,
            float thickness = 0.
        );
        void drawMesh();
    Scene();
    private:
        ObjectId lastId = 0;
        vector<Object> objects = {};
        unordered_map<ObjectId, uint32_t> idToIndex;
        ObjectId nextId();
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