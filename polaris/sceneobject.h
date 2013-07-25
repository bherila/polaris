#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "AABB.h"
#include "pmath.h"
#include "camera.h"
#include <stdlib.h>
#include  "stats.h"
#include "material.h"

struct Texture{
    Color3 *data;
};



//struct Material {
//    Material() : emitter(NULL) { }
//    Color3 kd;
//    Color3 ks;
//    Emitter *emitter;
//    real_t shine;
//    //Texture tex;
//};

class SceneObject
{
public:
    SceneObject(Matrix4 cumulativeTransform = NULL)
        : material(NULL), material_shared(true), ctm(cumulativeTransform), surface_area(-1), m_aabb(NULL) {}

    virtual ~SceneObject() {
        delete m_aabb;
        if (!material_shared) delete material;
    }

    /**
      Returns the t value of the intersection of the ray in object space to this
      object.  If there is no intersection return t < 0.
      **/
    virtual real_t getIntersection(const Ray &r) const = 0;
    /**
      Returns the world space normal vector of the given intersection point with
      this object.
      **/
    virtual void getNormal(const Vector4 &intersection, Vector4 &normal) const = 0;

    virtual void init() = 0;

    virtual Vector4 getSamplePoint(Sampler &sampler)  = 0;

    inline bool isEmitter() {
        return material->emitter.isEmissive;
    }

    Material *material; ///Material of the object
    bool material_shared;

    Matrix4  ctm; ///Cumulative transformation matrix of the object
    Matrix4  ctmI; //Inverse ctm of the object
    real_t   surface_area; //Surface area of  the object

    AABB *m_aabb;

    inline void setMaterial(Material *material, bool autoDelete) { this->material = material; this->material_shared = !autoDelete; }

protected:

};

class Cube : public SceneObject {
public:
    Cube() {}
    virtual real_t getIntersection(const Ray &r) const;
    virtual void getNormal(const Vector4 &intersection, Vector4 &normal) const;
    virtual void init();
    virtual Vector4 getSamplePoint(Sampler &sampler) ;
};

class Sphere : public SceneObject {
public:
    Sphere() : mRadius(-1.0) {}
    virtual real_t getIntersection(const Ray &r) const;
    virtual void getNormal(const Vector4 &intersection, Vector4 &normal) const;
    virtual void init();
    virtual Vector4 getSamplePoint(Sampler &sampler) ;
protected:
    real_t mRadius;
};

class Cone : public SceneObject {
public:
    Cone() {}
    virtual real_t getIntersection(const Ray &r) const;
    virtual void getNormal(const Vector4 &intersection, Vector4 &normal) const;
    virtual void init();
    virtual Vector4 getSamplePoint(Sampler &sampler) ;
};

class Cylinder : public SceneObject {
public:
    Cylinder() {}
    virtual real_t getIntersection(const Ray &r) const;
    virtual void getNormal(const Vector4 &intersection, Vector4 &normal) const;
    virtual void init();
    virtual Vector4 getSamplePoint(Sampler &sampler) ;
};

class Plane : public SceneObject {
    Plane() {}
    virtual real_t getIntersection(const Ray &r) const;
    virtual void getNormal(const Vector4 &intersection, Vector4 &normal) const;
    virtual real_t getSurfaceArea();
    virtual Vector4 getSamplePoint(Sampler &sampler) ;
};

inline std::ostream& operator<<(std::ostream& os, const Color3& f) {
        os <<"[";
        for (unsigned i = 0; i < 3; ++i) {
            os << f.data[i] << ",";
        }
        os << "]";
        return os;
}



#endif // SCENEOBJECT_H
