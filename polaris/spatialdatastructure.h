#pragma once

#include "sceneobject.h"
#include <vector>

typedef std::vector<SceneObject*>       SceneObjectList;
typedef SceneObjectList::const_iterator SceneObjectListIter;

class SurfacePoint;
class RenderThread;
class SpatialAccel
{
public:

    /*!
      @brief Returns a reference to the intersected object.
     */
    virtual SceneObject *getIntersection(const Ray *ray, SurfacePoint *sp, RenderThread *thread) = 0;

protected:
    std::vector<SceneObject *> m_objects;
};

class SceneList : public SpatialAccel {
public:
    SceneList(SceneObjectList *objects) {
        for (unsigned i = 0; i < objects->size(); ++i)
            m_objects.push_back(objects->at(i));
    };

    SceneObject *getIntersection(const Ray *ray, SurfacePoint *sp, RenderThread *thread);
};


