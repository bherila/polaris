#include <stdlib.h>
#include "matrix.h"
#include "vector.h"
#include "scene.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#ifdef USE_KDTREE
#include "kdtree.h"
#endif

//Forward declares
float calc_intersection_sphere(const Vector4 *origin, const Vector4 *direction);
float calc_intersection_cube(const Vector4 *origin, const Vector4 *direction);
void calc_normal_sphere(const Vector4 *obj_intersect, SurfacePoint *intersection);
void calc_normal_cube(const Vector4 *obj_intersect, SurfacePoint *intersection);
float calc_surface_area_sphere(SceneObject *obj);
float calc_surface_area_cube(SceneObject *obj);
void calc_sample_point_sphere(const SceneObject *obj, Sampler *s, Vector4 *point_out);
void calc_sample_point_cube(const SceneObject *obj, Sampler *s, Vector4 *point_out);
void refract_ray(const Vector4 *normal, const float in, const float out, const Ray *wi, Ray *r);


Scene *createScene(SceneObject **objects, const unsigned num_objects, Camera *camera) {
    Scene *sc = (Scene*) malloc(sizeof (Scene));
    sc->mObjects = objects;
    sc->mNumObjects = num_objects;
    sc->mNumrays = 0;
#ifdef USE_KDTREE
    sc->mTree = createKdTree(objects, num_objects);
#endif

    //Initialize the emitters in the scene
    unsigned i, j;
    sc->mNumEmitters = 0;
    for (i = 0; i < num_objects; ++i) {
        if (is_emitter(objects[i])) {
            sc->mNumEmitters++;
        }
    }
    sc->mEmitters = (SceneObject**) malloc(sizeof (SceneObject *) * sc->mNumEmitters);
    for (i = 0, j = 0; i < num_objects; ++i) {
        if (is_emitter(objects[i])) {
            sc->mEmitters[j++] = sc->mObjects[i];
        }
    }
    sc->mCamera = camera;

    //initialize function pointer arrays
    sc->mIntersectFuncs[0] = &calc_intersection_sphere;
    sc->mIntersectFuncs[1] = &calc_intersection_cube;

    sc->mNormalFuncs[0] = &calc_normal_sphere;
    sc->mNormalFuncs[1] = &calc_normal_cube;

    sc->mSurfaceAreaFuncs[0] = &calc_surface_area_sphere;
    sc->mSurfaceAreaFuncs[1] = &calc_surface_area_cube;

    sc->mSampleFuncs[0] = &calc_sample_point_sphere;
    sc->mSampleFuncs[1] = &calc_sample_point_cube;

    //Calculate surface area
    for (i = 0; i < num_objects; ++i)
        sc->mObjects[i]->mSurfaceArea = sc->mSurfaceAreaFuncs[sc->mObjects[i]->type](sc->mObjects[i]);

    return sc;
}

unsigned is_emitter(const SceneObject* object) {
    return (object->mat.mEmitterpower.x > 0 || object->mat.mEmitterpower.y > 0
            || object->mat.mEmitterpower.z > 0) ? 1 : 0;
}

void releaseScene(Scene *scene) {
    unsigned i = 0;
    for (; i < scene->mNumObjects; ++i) releaseSceneObject(scene->mObjects[i]);
    releaseCamera(scene->mCamera);
#ifdef USE_KDTREE
    releaseKdTree(&scene->mTree);
#endif
    free(scene->mEmitters);
    free(scene->mObjects);
}

void releaseSceneObject(SceneObject *object) {
    free(object);
}

SceneObject *createSceneObject(SHAPE type, Matrix4 translation,
        Matrix4 rotation, Matrix4 scale, Material m) {
    SceneObject *to_return = (SceneObject*) malloc(sizeof (SceneObject));
    to_return->type = type;
    to_return->mat = m;
    mat4_mat4_mult(&translation, &rotation, &to_return->ctm);
    mat4_mat4_mult(&to_return->ctm, &scale, &to_return->ctm_inv);
    mat4_inv(&to_return->ctm_inv, &to_return->ctm);
    return to_return;
}

float calc_intersection_sphere(const Vector4 *origin, const Vector4 *direction) {
    float a = 2 * (direction->x * direction->x + direction->y *
            direction->y + direction->z * direction->z),
            b = 2 * (direction->x * origin->x + direction->y * origin->y
            + direction->z * origin->z),
            c = origin->x * origin->x + origin->y * origin->y +
            origin->z * origin->z - 0.25f,
            discriminant = b * b - 2 * a * c;
    if (discriminant < 0) return -1.0f;
    return (-b - sqrt(discriminant)) / a;

}

float calc_intersection_cube(const Vector4 *origin, const Vector4 *direction) {

    float t1, t2, tmp;
    float tfar = FLT_MAX;
    float tnear = -FLT_MAX;
    // check X slab
    if (direction->x == 0 && (origin->x > 0.5f || origin->x < -0.5f)) {
        return -1.0f; // ray is parallel to the planes & outside slab
    } else {
        tmp = 1.0 / direction->x;
        t1 = (-0.5f - origin->x) * tmp;
        t2 = (0.5f - origin->x) * tmp;
        if (t1 > t2) {
            tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        if (t1 > tnear) tnear = t1;
        if (t2 < tfar) tfar = t2;
        if (tnear > tfar || tfar < 0.0) return -1.0f; // ray missed box or box is behind ray
    }
    // check Y slab
    if (direction->y == 0 && (origin->y > 0.5f || origin->y < -0.5f)) {
        return -1.0f; // ray is parallel to the planes & outside slab
    } else {
        tmp = 1.0 / direction->y;
        t1 = (-0.5f - origin->y) * tmp;
        t2 = (0.5f - origin->y) * tmp;
        if (t1 > t2) {
            tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        if (t1 > tnear) tnear = t1;
        if (t2 < tfar) tfar = t2;
        if (tnear > tfar || tfar < 0) return -1.0f; // ray missed box or box is behind ray
    }
    // check Z slab
    if (direction->z == 0 && (origin->z > 0.5f || origin->z < -0.5f)) {
        return -1.0f; // ray is parallel to the planes & outside slab
    } else {
        tmp = 1.0 / direction->z;
        t1 = (-0.5f - origin->z) * tmp;
        t2 = (0.5f - origin->z) * tmp;
        if (t1 > t2) {
            tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        if (t1 > tnear) tnear = t1;
        if (t2 < tfar) tfar = t2;
        if (tnear > tfar || tfar < 0) return -1.0f; // ray missed box or box is behind ray
    }
    return (tnear > 0) ? tnear : tfar;
}

void calc_normal_sphere(const Vector4 *obj_intersect, SurfacePoint *intersection) {
    intersection->normal.x = intersection->object->ctm.data[0] *
            obj_intersect->data[0] + intersection->object->ctm.data[4] *
            obj_intersect->data[1] + intersection->object->ctm.data[8] *
            obj_intersect->data[2];
    intersection->normal.y = intersection->object->ctm.data[1] *
            obj_intersect->data[0] + intersection->object->ctm.data[5] *
            obj_intersect->data[1] + intersection->object->ctm.data[9] *
            obj_intersect->data[2];
    intersection->normal.z = intersection->object->ctm.data[2] *
            obj_intersect->data[0] + intersection->object->ctm.data[6] *
            obj_intersect->data[1] + intersection->object->ctm.data[10] *
            obj_intersect->data[2];
    intersection->normal.w = 0;
}

void calc_normal_cube(const Vector4 *obj_intersect, SurfacePoint *intersection) {
    vec4_set(&intersection->normal, 0.0f, 0.0f, 0.0f, 0.0f);
    if (EQ(obj_intersect->data[1], 0.5)) intersection->normal.y = 1;
    else if (EQ(obj_intersect->data[1], -0.5)) intersection->normal.y = -1;
    else if (EQ(obj_intersect->data[2], 0.5)) intersection->normal.z = 1;
    else if (EQ(obj_intersect->data[2], -0.5)) intersection->normal.z = -1;
    else if (EQ(obj_intersect->data[0], 0.5)) intersection->normal.x = 1;
    else if (EQ(obj_intersect->data[0], -0.5)) intersection->normal.x = -1;
}

float calc_surface_area_sphere(SceneObject* obj) {
    Vector4 dir = {0.5, 0, 0, 0};
    mat4_vec4_mult(&obj->ctm_inv, &dir, &dir);
    float radius = vec4_magnitude(&dir);
    obj->mSize = radius;
    return 4 * PI * radius*radius;
}

float calc_surface_area_cube(SceneObject *obj) {
    Vector4 dir = {1.0, 0, 0, 0};
    mat4_vec4_mult(&obj->ctm_inv, &dir, &dir);
    float edgeLength = vec4_magnitude(&dir);
    obj->mSize = edgeLength;
    return edgeLength * edgeLength * 6.0f;
    /*
        float size_x = obj->mSize, size_y = obj->mSize, size_z = obj->mSize;
        size_x = obj->ctm_inv.a + obj->ctm_inv.e + obj->ctm_inv.i + obj->ctm_inv.m; //length of edge x
        size_y = obj->ctm_inv.b + obj->ctm_inv.f + obj->ctm_inv.j + obj->ctm_inv.n;
        size_z = obj->ctm_inv.c + obj->ctm_inv.g + obj->ctm_inv.k + obj->ctm_inv.o;
        float surface_area = (size_x * size_y) * 2 + (size_x * size_z) * 2 + (size_y * size_z) * 2;
        return surface_area;
     */
}

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

void calc_aabb(const SceneObject *obj, Vector4 *min, Vector4 *max) {
    Vector4 v[8] = {
        { -.5, .5, -.5, 1 },
        { -.5, .5,  .5, 1 },
        { -.5,-.5,  .5, 1 },
        { -.5,-.5, -.5, 1 },
        {  .5,-.5, -.5, 1 },
        {  .5,-.5,  .5, 1 },
        {  .5, .5,  .5, 1 },
        {  .5, .5, -.5, 1 }
    };
    mat4_vec4_mult(&obj->ctm_inv, v, v);
    *min = *v;
    *max = *v;
    unsigned i;
    for (i = 1; i < 8; ++i) {
        mat4_vec4_mult(&obj->ctm_inv, v + i, v + i);
        min->x = MIN(v[i].x, min->x);
        min->y = MIN(v[i].y, min->y);
        min->z = MIN(v[i].z, min->z);
        max->x = MAX(v[i].x, max->x);
        max->y = MAX(v[i].y, max->y);
        max->z = MAX(v[i].z, max->z);
    }
};

void calc_aabb_from_list(SceneObject * const* obj, unsigned nObj, Vector4 *min, Vector4 *max) {
    Vector4 v[8] = {
        { -.5, .5, -.5, 1 },
        { -.5, .5,  .5, 1 },
        { -.5,-.5,  .5, 1 },
        { -.5,-.5, -.5, 1 },
        {  .5,-.5, -.5, 1 },
        {  .5,-.5,  .5, 1 },
        {  .5, .5,  .5, 1 },
        {  .5, .5, -.5, 1 }
    };
    Vector4 vt;
    mat4_vec4_mult(&obj[0]->ctm_inv, v, &vt);
    *min = vt;
    *max = vt;
    unsigned h, i;
    for (h = 0; h < nObj; ++h) {
    for (i = 0; i < 8; ++i) {
        mat4_vec4_mult(&obj[h]->ctm_inv, v+i, &vt);
        min->x = MIN( vt.x , min->x );
        min->y = MIN( vt.y , min->y );
        min->z = MIN( vt.z , min->z );
        max->x = MAX( vt.x , max->x );
        max->y = MAX( vt.y , max->y );
        max->z = MAX( vt.z , max->z );
    }
    }
};

unsigned reflect_ray_lambertian(const SurfacePoint *intersect, const Ray *wi, Ray *wo, float *prob, Sampler *s) {

    if (vec4_vec4_dot(&intersect->normal, &wi->direction) > 0)
        vec4_scalar_mult(&intersect->normal, -1.0f, &wo->origin);
    else
        wo->origin = intersect->normal;
    *prob = 1.0f;
    float phi = urand(s), psi = urand(s);
    float r = urand(s);
    if (r < intersect->object->mat.ks) {
        reflect_ray(&wo->origin, wi, wo);
        wo->origin = intersect->intersection;
        return 1;
        /*
            } else if (r < fabs(intersect->object->mat.ks)) {
                refract_ray(&wo->origin, 1.0f, 1.0f, wi, wo);
                wo->origin = intersect->intersection;
                return 1;
         */
    } else {
        vec4_cos_random(&wo->origin, phi, psi, &wo->direction);
        wo->origin = intersect->intersection;
        return 0; //not spec
    }
}

void refract_ray(const Vector4 *normal, const float in, const float out, const Ray *wi, Ray *r) {
    Vector4 norm = *normal;
    Vector4 w = wi->direction;
    vec4_scalar_mult(&w, -1.0f, &w);
    float n = in / out;
    if (vec4_vec4_dot(&wi->direction, &norm) >= 0) {
        vec4_scalar_mult(&norm, -1.0f, &norm);
        n = out / in;
    }
    const float d = vec4_vec4_dot(&norm, &w);
    const float det = 1.0 - n * n * (1.0 - d * d);
    vec4_scalar_mult(&norm, n * d - sqrt(det), &norm);
    vec4_scalar_mult(&w, -n, &r->direction);
    vec4_vec4_add(&r->direction, &norm, &r->direction);
    r->direction.w = 0;
    vec4_normalize(&r->direction, &r->direction);
}

void reflect_ray(const Vector4 *normal, const Ray *wi, Ray *r) {
    vec4_scalar_mult(normal, vec4_vec4_dot(normal, &wi->direction) * 2.0f, &r->origin);
    vec4_vec4_sub(&wi->direction, &r->origin, &r->direction);
}

void calc_sample_point_sphere(const SceneObject *obj, Sampler *s, Vector4 *point_out) {
  //  printf("%f", obj->mSize);
    urand_sphere(s, .5f - EPSILON, point_out);
}

void calc_sample_point_cube(const SceneObject *obj, Sampler *s, Vector4 *point_out) {
    float edge = 0.5f;
    float u = urand(s) - edge, v = urand(s) - edge;
    int r = urand(s) * 6;
    switch (r) {
        case 0:
            vec4_set(point_out, -edge, u, v, 1.0f);
            break;
        case 1:
            vec4_set(point_out, edge, u, v, 1.0f);
            break;
        case 2:
            vec4_set(point_out, u, -edge, v, 1.0f);
            break;
        case 3:
            vec4_set(point_out, u, edge, v, 1.0f);
            break;
        case 4:
            vec4_set(point_out, u, v, -edge, 1.0f);
            break;
        case 5:
            vec4_set(point_out, u, v, edge, 1.0f);
            break;
    }
    /*
        float length_x = obj->mSize, length_y = obj->mSize, length_z = obj->mSize;
        length_x = obj->ctm_inv.a + obj->ctm_inv.e + obj->ctm_inv.i + obj->ctm_inv.m; //length of edge x
        length_y = obj->ctm_inv.b + obj->ctm_inv.f + obj->ctm_inv.j + obj->ctm_inv.n;
        length_z = obj->ctm_inv.c + obj->ctm_inv.g + obj->ctm_inv.k + obj->ctm_inv.o;

        float surface_area = (length_x * length_y) * 2 + (length_x * length_z) * 2 + (length_y * length_z) * 2;
        float p[6] = {(length_x * length_y) / surface_area, ((length_x * length_y) * 2) / surface_area, ((length_x * length_y) * 2 + (length_x * length_z)) / surface_area,
            ((length_x * length_y) * 2 + (length_x * length_z) * 2) / surface_area,
            ((length_x * length_y) * 2 + (length_x * length_z) * 2 + (length_y * length_z)) / surface_area, 1.0f};
        float u = urand(s) - 0.5f, v = urand(s) - 0.5f;
        unsigned i = 0;
        float r = urand(s);
        for (; i < 6; ++i) if (r < p[i]) break;
        switch (i) {
            case 0:
                vec4_set(point_out, -0.5f, u, v, 1.0f);
                break;
            case 1:
                vec4_set(point_out, 0.5f, u, v, 1.0f);
                break;
            case 2:
                vec4_set(point_out, u, -0.5f, v, 1.0f);
                break;
            case 3:
                vec4_set(point_out, u, 0.5f, v, 1.0f);
                break;
            case 4:
                vec4_set(point_out, u, v, -0.5f, 1.0f);
                break;
            case 5:
                vec4_set(point_out, u, v, 0.5f, 1.0f);
                break;
        }
     */

}

unsigned calc_nearest_intersection(Scene *scene, const Ray *wi, SurfacePoint *intersect_out) {
    return calc_nearest_intersection2(scene, scene->mObjects, scene->mNumObjects, wi, intersect_out);
}

unsigned calc_nearest_intersection2(Scene *scene, SceneObject **objects, unsigned nObjects, const Ray *wi, SurfacePoint *intersect_out) {

    unsigned i = 0;
    float min_t = FLT_MAX;
    SceneObject *near_object;
    scene->mNumrays += nObjects;
    for (; i < nObjects; ++i) {
        mat4_vec4_mult(&objects[i]->ctm, &wi->direction, &intersect_out->normal);
        mat4_vec4_mult(&objects[i]->ctm, &wi->origin, &intersect_out->intersection);
        float t = scene->mIntersectFuncs[objects[i]->type](&intersect_out->intersection, &intersect_out->normal);
        if (t >= 0 && t < min_t) {
            min_t = t;
            near_object = objects[i];
        }
    }
    if (min_t < FLT_MAX) {
        Vector4 obj_intersect, ob_direction;
        intersect_out->object = near_object;
        vec4_scalar_mult(&wi->direction, min_t, &intersect_out->intersection);
        vec4_vec4_add(&wi->origin, &intersect_out->intersection, &intersect_out->intersection);
        mat4_vec4_mult(&near_object->ctm, &wi->origin, &obj_intersect);
        mat4_vec4_mult(&near_object->ctm, &wi->direction, &ob_direction);
        vec4_scalar_mult(&ob_direction, min_t, &ob_direction);
        vec4_vec4_add(&obj_intersect, &ob_direction, &obj_intersect);
        scene->mNormalFuncs[near_object->type](&obj_intersect, intersect_out);
        vec4_normalize(&intersect_out->normal, &intersect_out->normal);
        return 1;
    }
    return 0;
}

void calc_world_ray(const Scene *scene, const float ndc_x, const float ndc_y, Ray *ray_out) {
    ray_out->direction.x = 2.0f * ndc_x - 1.0f;
    ray_out->direction.y = 1.0f - 2.0f * ndc_y;
    ray_out->direction.z = -1.0f;
    ray_out->direction.w = 1.0f;
    mat4_vec4_mult(scene->mCamera->mFilmToWorld, &ray_out->direction, &ray_out->direction);
    vec4_vec4_sub(&ray_out->direction, scene->mCamera->mPosition, &ray_out->direction);
    ray_out->origin = *scene->mCamera->mPosition;
}

