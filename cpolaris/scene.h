/* 
 * File:   scene.h
 * Author: psastras
 *
 * Created on April 26, 2010, 4:14 PM
 */

#define USE_MPI

#ifndef _SCENE_H
#define	_SCENE_H

#include "vector.h"
#include "ray.h"
#include "matrix.h"
#include "camera.h"
#include "sampler.h"

#ifdef USE_KDTREE
struct kdTree;
#endif

#ifdef	__cplusplus
extern "C" {
#endif
    //Structures

    typedef enum shape {
        Sphere, Cube, Cylinder, Cone
    } SHAPE;

    typedef struct material {
        Color3 mDiffuse;
        Color3 mEmitterpower;
        Color3 mAttenuation;
        float ks;
    } Material;

    typedef struct sceneobject {
        SHAPE type;
        Material mat;
        Matrix4 ctm, ctm_inv;
        float mSurfaceArea, mSize; //Only valid for spheres at the moment size stores radius
    } SceneObject;

    typedef struct surfacepoint {
        Vector4 intersection, normal;
        SceneObject *object;
    } SurfacePoint;

    typedef struct scene {
#ifdef USE_KDTREE
        struct kdTree *mTree;
#endif
        SceneObject **mObjects, **mEmitters;
        unsigned mNumObjects, mNumEmitters;
        Camera *mCamera;
        float (*mIntersectFuncs[4])(const Vector4 *origin, const Vector4 * direction);
        void (*mNormalFuncs[4])(const Vector4 *, SurfacePoint *);
        float (*mSurfaceAreaFuncs[4])(SceneObject *);
        void (*mSampleFuncs[4])(const SceneObject *, Sampler *, Vector4 *);
        unsigned mNumrays;
    } Scene;


    //Constructors & Destructors

    Scene *createScene(SceneObject **objects, const unsigned num_objects, Camera *camera);
    void releaseScene(Scene *scene);
    SceneObject *createSceneObject(SHAPE type, Matrix4 translation,
            Matrix4 rotation, Matrix4 scale, Material m);
    void releaseSceneObject(SceneObject *object);

    //Methods
    unsigned is_emitter(const SceneObject *object);
    void calc_world_ray(const Scene *scene, const float ndc_x, const float ndc_y, Ray *ray_out);
    void reflect_ray(const Vector4 *normal, const Ray *wi, Ray *r);
    unsigned calc_nearest_intersection(Scene *scene, const Ray *wi, SurfacePoint *intersect_out);
    unsigned calc_nearest_intersection2(Scene *scene, SceneObject **objects, unsigned nObjects, const Ray *wi, SurfacePoint *intersect_out);
    unsigned reflect_ray_lambertian(const SurfacePoint *intersect, const Ray *wi, Ray *wo, float *prob, Sampler *s);
    void calc_aabb(const SceneObject *obj, Vector4 *min, Vector4 *max);
    void calc_aabb_from_list(SceneObject * const * obj, unsigned nObj, Vector4 *min, Vector4 *max);


#ifdef	__cplusplus
}
#endif

#endif	/* _SCENE_H */

