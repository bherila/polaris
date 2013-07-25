#include "pmath.h"
#include "stats.h"
#include "camera.h"

#ifndef MATERIAL_H
#define MATERIAL_H

class SurfacePoint;
class RenderThread;

// Portions adapted from http://rendering.aspone.cz/

struct Emitter {
    Emitter() : isEmissive(false) {}
    bool isEmissive;
    Color3 attenuation;
    Color3 power;
};

class Material {
public:
    virtual void f(const SurfacePoint *sp, const Vector4 *outDir, Color3 &color) const = 0;
    virtual bool reflectRay(const SurfacePoint *sp, RenderThread *thread, Ray *outRay, real_t *weight) = 0;
    virtual Color3 getColor()  const  = 0;
    Emitter emitter;
};

class LambertianMaterial : public Material {

    Color3 kd;

public:
    LambertianMaterial(Color3 reflectance)  {
        kd = reflectance;// * M_1_PI;//TODO: This constant might be wrong.
    }

    virtual void f(const SurfacePoint *sp, const Vector4 *outDir, Color3 &color) const {
        color = kd;
    }

    virtual bool reflectRay(const SurfacePoint *sp, RenderThread *thread, Ray *outRay, real_t *weight);
    virtual Color3 getColor() const { return kd; }
};

class PhongMaterial : public Material {
    bool lastType;
    Color3 BSDFdiffuse;
public:
    PhongMaterial(Color3 reflectance) {
        BSDFdiffuse = reflectance;// * 0.31830988618379067153776752674503;
    }

    real_t kd, ks, spec_n;

    void f(const SurfacePoint *sp, const Vector4 *outDir, Color3 &color) const;
    bool reflectRay(const SurfacePoint *sp, RenderThread *thread, Ray *outRay, real_t *weight);
    virtual Color3 getColor() const  { return BSDFdiffuse; }
};

class MirroredMaterial : public Material {

public:
    void f(const SurfacePoint *sp, const Vector4 *outDir, Color3 &color) const;
    bool reflectRay(const SurfacePoint *sp, RenderThread *thread, Ray *outRay, real_t *weight);
    virtual Color3 getColor() const  { return Color3(1.0, 1.0, 1.0); }
};

class CombinedMaterial : public Material {
    real_t ks, kd, spec_n;
    bool lastType;
    Vector3 refFdiffuse;

public:
    void f(const SurfacePoint *sp, const Vector4 *outDir, Color3 &color) const;
    bool reflectRay(const SurfacePoint *sp, RenderThread *thread, Ray *outRay, real_t *weight);
    virtual Color3 getColor()  const { return Color3(refFdiffuse.x, refFdiffuse.y, refFdiffuse.z); }
};

#endif // MATERIAL_H
