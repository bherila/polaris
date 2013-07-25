#pragma once

#include "spatialdatastructure.h"
#include "sceneobject.h"
#include "pmath.h"
#include "camera.h"

#include <vector>
#include <string>

using namespace std;


struct SurfacePoint {
    SurfacePoint();
    SceneObject *object;
    Ray incoming;
    Vector4 intersection;
    Vector4 normal; //w should = 0 - this is really a direction and should tech be a v3
    real_t t;
};


class Scene
{
public:
    Scene();
    ~Scene();
    /**
      Finds the nearest object of intersection with the ray. Data about the intersection
      is stored in the SurfacePoint.  If there is no intersection returns false.
      **/
    bool getNearestObject(const Ray &r, SurfacePoint &intersect, RenderThread *thread);
    /**
      Adds an object to the scene.
      **/
    void addObject(SceneObject* object);
    int  getNObjects() const { return m_objects.size(); };

    void parseFile(const std::string &filename);

    /**
      Prepare the scene for rendering.  Should be called after all scene objects
      have been added (camera, SceneObjects, lights).  If any of these items are changed
      you MUST re-init the scene.
      **/
    void init();

    Ray getWorldRay(real_t filmx, real_t filmy);

    Camera *getCamera() {
        return m_camera;
    }

    std::vector<SceneObject *> getEmitters() {
        return m_emitters;
    }

    void setCamera(Camera *camera) {
        m_camera = camera;
    }

//protected:
    SceneObjectList m_objects;  //List of all objects (including emitters in the scene)
    SceneObjectList m_emitters; //List of emitters in the scene
    SpatialAccel *m_spatialaccel;
    Camera *m_camera;
};

