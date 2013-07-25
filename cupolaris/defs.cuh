/* 
 * File:   defs.cuh
 * Author: psastras
 *
 * Created on May 12, 2010, 12:53 AM
 */

#ifndef _DEFS_CUH
#define	_DEFS_CUH
#include <cuda.h>
#include <cuda_runtime.h>
#include <cutil.h>
#include <math.h>
#include <string.h>
#include <float.h>


#define EPSILON 0.0001
#define PI 3.14159265
#define TWO_PI 6.28318531
#define EQ(x, y) (((x) <= ((y) + EPSILON)) && ((x) >= ((y) - EPSILON)))

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct ray {
        float4 origin, direction;
    } Ray;

    typedef struct rendersettings {
        int width, height, samples;
        char *output_file;
    } RenderSettings;

    typedef struct matrix {
        float4 r0, r1, r2, r3;
    } Matrix4;

    typedef struct camera {
        Matrix4 mFilmToWorld, mWorldToFilm;
        float4 mPosition;
    } Camera;

    typedef struct roi {
    	int startIdx, length;
    } ROI;

    typedef enum shape {
        Sphere, Cube, Cylinder, Cone
    } SHAPE;

    typedef struct material {
        float3 mDiffuse;
        float3 mEmitterpower;
        float3 mAttenuation;
        float ks;
    } Material;

    typedef struct sceneobject {
        SHAPE type;
        Material mat;
        Matrix4 ctm, ctm_inv;
        float mSurfaceArea, mSize; //Only valid for spheres at the moment size stores radius
    } SceneObject;

    typedef struct surfacepoint {
        float4 intersection, normal;
        SceneObject *object;
    } SurfacePoint;

    typedef struct scene {
        SceneObject *mObjects, *mEmitters;
        unsigned mNumObjects, mNumEmitters;
        Camera mCamera;
        float (*mSurfaceAreaFuncs[4])(SceneObject *);
        unsigned mNumrays;
    } Scene;

    typedef struct pathvertex {
        SurfacePoint mIntersect;
        float4 mWout;
        float mProb;
        unsigned isSpecular; //is end of path

    } PathVertex;

    __constant__ unsigned int shift1[4] = {6, 2, 13, 3};
    __constant__ unsigned int shift2[4] = {13, 27, 21, 12};
    __constant__ unsigned int shift3[4] = {18, 2, 7, 13};
    __constant__ unsigned int offset[4] = {4294967294, 4294967288, 4294967280, 4294967168};

    __device__ unsigned int randStates[32];

    __device__ unsigned int TausStep(unsigned int &z, int S1, int S2, int S3, unsigned int M) {
        unsigned int b = (((z << S1) ^ z) >> S2);
        return z = (((z & M) << S3) ^ b);
    }

    __device__ unsigned int randInt(int idx) {
        TausStep(randStates[idx & 31], shift1[idx & 3], shift2[idx & 3], shift3[idx & 3], offset[idx & 3]);
        return (randStates[(idx)&31]^randStates[(idx + 1)&31]^randStates[(idx + 2)&31]^randStates[(idx + 3)&31]);
    }

    __device__ float urand(int idx) {
        return (randInt(idx) % 10000) / 10000.0f;
    }

    __host__ inline void vec4_printf(const float4 *vec) {
        printf("[%'.4f %'.4f %'.4f %'.4f]\n", vec->x, vec->y, vec->z, vec->w);
    }

    __host__ inline void vec3_printf(const float3 *vec) {
        printf("[%'.4f %'.4f %'.4f]\n", vec->x, vec->y, vec->z);
    }

    __host__ inline void mat4_printf(const Matrix4 *m) {
        vec4_printf(&m->r0);
        vec4_printf(&m->r1);
        vec4_printf(&m->r2);
        vec4_printf(&m->r3);
        printf("\n");
    }

    __device__ __host__ inline void vec4_vec4_add(const float4 *lhs, const float4 *rhs, float4 *out) {
        out->x = lhs->x + rhs->x;
        out->y = lhs->y + rhs->y;
        out->z = lhs->z + rhs->z;
        out->w = lhs->w + rhs->w;
    }

    __device__ __host__ inline void vec4_vec4_sub(const float4 *lhs, const float4 *rhs, float4 *out) {
        out->x = lhs->x - rhs->x;
        out->y = lhs->y - rhs->y;
        out->z = lhs->z - rhs->z;
        out->w = lhs->w - rhs->w;
    }

    __device__ __host__ inline float vec4_vec4_dot(const float4 *lhs, const float4 *rhs) {
        return (lhs->x * rhs->x) + (lhs->y * rhs->y) + (lhs->z * rhs->z) + (lhs->w * rhs->w);
    }

    __device__ __host__ inline void vec4_vec4_cross(const float4 *lhs, const float4 *rhs, float4 *out) {
        out->x = lhs->y * rhs->z - lhs->z * rhs->y;
        out->y = lhs->z * rhs->x - lhs->x * rhs->z;
        out->z = lhs->x * rhs->y - lhs->y * rhs->x;
        out->w = 0;
    }

    __device__ __host__ inline void vec4_scalar_mult(const float4 *vec, float s, float4 *out) {
        out->x = vec->x * s;
        out->y = vec->y * s;
        out->z = vec->z * s;
        out->w = vec->w * s;
    }

    __device__ __host__ inline void vec4_vec4_lin_combine(const float4 *vec_1, float alpha, const float4 *vec_2, float beta, float4 *out) {
        out->x = vec_1->x * alpha + vec_2->x * beta;
        out->y = vec_1->y * alpha + vec_2->y * beta;
        out->z = vec_1->z * alpha + vec_2->z * beta;
        out->w = vec_1->w * alpha + vec_2->w * beta;
    }

    __device__ __host__ inline void vec4_vec4_scalar_add_weighted(const float4 *vec_1, float alpha, float4 *out) {
        out->x += vec_1->x * alpha;
        out->y += vec_1->y * alpha;
        out->z += vec_1->z * alpha;
        out->w += vec_1->w * alpha;
    }

    __device__ __host__ inline float vec4_magnitude2(const float4 *vec) {
        return vec->x * vec->x + vec->y * vec->y + vec->z * vec->z + vec->w * vec->w;
    }

    __device__ __host__ inline float vec4_magnitude(const float4 *vec) {
        return sqrtf(vec4_magnitude2(vec));
    }

    __device__ __host__ inline float vec4_normalize(const float4 *vec, float4 *out) {
        float m = vec4_magnitude(vec);
        vec4_scalar_mult(vec, 1.0f / m, out);
        return m;
    }

    __device__ __host__ inline void vec4_set(float4 *vec, float x, float y, float z, float w) {
        vec->x = x;
        vec->y = y;
        vec->z = z;
        vec->w = w;
    }

    __device__ __host__ inline void vec3_vec3_add(const float3 *lhs, const float3 *rhs, float3 *out) {
        out->x = lhs->x + rhs->x;
        out->y = lhs->y + rhs->y;
        out->z = lhs->z + rhs->z;
    }

    __device__ __host__ inline void vec3_vec3_mult(const float3 *lhs, const float3 *rhs, float3 *out) {
        out->x = lhs->x * rhs->x;
        out->y = lhs->y * rhs->y;
        out->z = lhs->z * rhs->z;
    }

    __device__ __host__ inline void vec3_scalar_mult(const float3 *vec, float s, float3 *out) {
        out->x = vec->x * s;
        out->y = vec->y * s;
        out->z = vec->z * s;
    }

    __device__ __host__ inline void vec3_set(float3 *vec, float x, float y, float z) {
        vec->x = x;
        vec->y = y;
        vec->z = z;
    }

    __device__ __host__ inline Matrix4 create_identity_mat4() {
        Matrix4 to_return = {1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f};
        return to_return;
    }

    __device__ __host__ inline Matrix4 create_translation_mat4(float x, float y, float z) {
        Matrix4 to_return = {1.0f, 0.0f, 0.0f, x,
            0.0f, 1.0f, 0.0f, y,
            0.0f, 0.0f, 1.0f, z,
            0.0f, 0.0f, 0.0f, 1.0f};
        return to_return;
    }

    __device__ __host__ inline Matrix4 create_scale_mat4(float s_1, float s_2, float s_3) {
        Matrix4 to_return = {s_1, 0.0f, 0.0f, 0,
            0.0f, s_2, 0.0f, 0,
            0.0f, 0.0f, s_3, 0,
            0.0f, 0.0f, 0.0f, 1.0f};
        return to_return;
    }

    __device__ __host__ inline Matrix4 create_rotx_mat4(const float radians) {
        Matrix4 to_return = {1, 0, 0, 0,
            0, cos(radians), -sin(radians), 0,
            0, sin(radians), cos(radians), 0,
            0, 0, 0, 1};
        return to_return;
    }

    __device__ __host__ inline Matrix4 create_roty_mat4(const float radians) {
        Matrix4 to_return = {cos(radians), 0, sin(radians), 0,
            0, 1, 0, 0,
            -sin(radians), 0, cos(radians), 0,
            0, 0, 0, 1};
        return to_return;
    }

    __device__ __host__ inline void mat4_vec4_mult(const Matrix4 *lhs, const float4 *rhs, float4 *out) {
        float data[4];
        data[0] = vec4_vec4_dot(&lhs->r0, rhs);
        data[1] = vec4_vec4_dot(&lhs->r1, rhs);
        data[2] = vec4_vec4_dot(&lhs->r2, rhs);
        data[3] = vec4_vec4_dot(&lhs->r3, rhs);
        memcpy(out, &data, sizeof (float) * 4);
    }

    __device__ __host__ inline float mat4_determinant(const Matrix4 *m) {
        return m->r0.x * m->r1.y * m->r2.z * m->r3.w - m->r0.x * m->r1.y * m->r2.w * m->r3.z - m->r0.x * m->r1.z * m->r2.y * m->r3.w + m->r0.x *
                m->r1.z * m->r2.w * m->r3.y + m->r0.x * m->r1.w * m->r2.y * m->r3.z - m->r0.x * m->r1.w * m->r2.z * m->r3.y - m->r0.y * m->r1.x *
                m->r2.z * m->r3.w + m->r0.y * m->r1.x * m->r2.w * m->r3.z + m->r0.y * m->r1.z * m->r2.x * m->r3.w - m->r0.y * m->r1.z * m->r2.w *
                m->r3.x - m->r0.y * m->r1.w * m->r2.x * m->r3.z + m->r0.y * m->r1.w * m->r2.z * m->r3.x + m->r0.z * m->r1.x * m->r2.y * m->r3.w -
                m->r0.z * m->r1.x * m->r2.w * m->r3.y - m->r0.z * m->r1.y * m->r2.x * m->r3.w + m->r0.z * m->r1.y * m->r2.w * m->r3.x + m->r0.z *
                m->r1.w * m->r2.x * m->r3.y - m->r0.z * m->r1.w * m->r2.y * m->r3.x - m->r0.w * m->r1.x * m->r2.y * m->r3.z + m->r0.w * m->r1.x *
                m->r2.z * m->r3.y + m->r0.w * m->r1.y * m->r2.x * m->r3.z - m->r0.w * m->r1.y * m->r2.z * m->r3.x - m->r0.w * m->r1.z * m->r2.x *
                m->r3.y + m->r0.w * m->r1.z * m->r2.y * m->r3.x;
    }

    __device__ __host__ inline void mat4_mat4_mult(const Matrix4 *lhs, const Matrix4 *rhs, Matrix4 *out) {
        float data[16];
        data[0] = lhs->r0.x * rhs->r0.x + lhs->r0.y * rhs->r1.x +
                lhs->r0.z * rhs->r2.x + lhs->r0.w * rhs->r3.x;
        data[1] = lhs->r0.x * rhs->r0.y + lhs->r0.y * rhs->r1.y +
                lhs->r0.z * rhs->r2.y + lhs->r0.w * rhs->r3.y;
        data[2] = lhs->r0.x * rhs->r0.z + lhs->r0.y * rhs->r1.z +
                lhs->r0.z * rhs->r2.z + lhs->r0.w * rhs->r3.z;
        data[3] = lhs->r0.x * rhs->r0.w + lhs->r0.y * rhs->r1.w +
                lhs->r0.z * rhs->r2.w + lhs->r0.w * rhs->r3.w;

        data[4] = lhs->r1.x * rhs->r0.x + lhs->r1.y * rhs->r1.x +
                lhs->r1.z * rhs->r2.x + lhs->r1.w * rhs->r3.x;
        data[5] = lhs->r1.x * rhs->r0.y + lhs->r1.y * rhs->r1.y +
                lhs->r1.z * rhs->r2.y + lhs->r1.w * rhs->r3.y;
        data[6] = lhs->r1.x * rhs->r0.z + lhs->r1.y * rhs->r1.z +
                lhs->r1.z * rhs->r2.z + lhs->r1.w * rhs->r3.z;
        data[7] = lhs->r1.x * rhs->r0.w + lhs->r1.y * rhs->r1.w +
                lhs->r1.z * rhs->r2.w + lhs->r1.w * rhs->r3.w;

        data[8] = lhs->r2.x * rhs->r0.x + lhs->r2.y * rhs->r1.x +
                lhs->r2.z * rhs->r2.x + lhs->r2.w * rhs->r3.x;
        data[9] = lhs->r2.x * rhs->r0.y + lhs->r2.y * rhs->r1.y +
                lhs->r2.z * rhs->r2.y + lhs->r2.w * rhs->r3.y;
        data[10] = lhs->r2.x * rhs->r0.z + lhs->r2.y * rhs->r1.z +
                lhs->r2.z * rhs->r2.z + lhs->r2.w * rhs->r3.z;
        data[11] = lhs->r2.x * rhs->r0.w + lhs->r2.y * rhs->r1.w +
                lhs->r2.z * rhs->r2.w + lhs->r2.w * rhs->r3.w;

        data[12] = lhs->r3.x * rhs->r0.x + lhs->r3.y * rhs->r1.x +
                lhs->r3.z * rhs->r2.x + lhs->r3.w * rhs->r3.x;
        data[13] = lhs->r3.x * rhs->r0.y + lhs->r3.y * rhs->r1.y +
                lhs->r3.z * rhs->r2.y + lhs->r3.w * rhs->r3.y;
        data[14] = lhs->r3.x * rhs->r0.z + lhs->r3.y * rhs->r1.z +
                lhs->r3.z * rhs->r2.z + lhs->r3.w * rhs->r3.z;
        data[15] = lhs->r3.x * rhs->r0.w + lhs->r3.y * rhs->r1.w +
                lhs->r3.z * rhs->r2.w + lhs->r3.w * rhs->r3.w;

        memcpy(out, &data, sizeof (float) * 16);
    }

    __device__ __host__ inline void mat4_inv(const Matrix4 *in, Matrix4 *out) {
        float det = mat4_determinant(in);
        float a = in->r0.x, b = in->r0.y, c = in->r0.z, d = in->r0.w;
        float e = in->r1.x, f = in->r1.y, g = in->r1.z, h = in->r1.w;
        float i = in->r2.x, j = in->r2.y, k = in->r2.z, l = in->r2.w;
        float m = in->r3.x, n = in->r3.y, o = in->r3.z, p = in->r3.w;
        float data[16];
        data[0] = (f * k * p + g * l * n + h * j * o - f * l * o - g * j * p - h * k * n) / det,
                data[1] = (b * l * o + c * j * p + d * k * n - b * k * p - c * l * n - d * j * o) / det,
                data[2] = (b * g * p + c * h * n + d * f * o - b * h * o - c * f * p - d * g * n) / det,
                data[3] = (b * h * k + c * f * l + d * g * j - b * g * l - c * h * j - d * f * k) / det,
                data[4] = (e * l * o + h * k * m + g * i * p - e * k * p - g * l * m - h * i * o) / det,
                data[5] = (a * k * p + c * l * m + d * i * o - a * l * o - c * i * p - d * k * m) / det,
                data[6] = (a * h * o + c * e * p + d * g * m - a * g * p - c * h * m - d * e * o) / det,
                data[7] = (a * g * l + c * h * i + d * e * k - a * h * k - c * e * l - d * g * i) / det,
                data[8] = (e * j * p + f * l * m + h * i * n - e * l * n - f * i * p - h * j * m) / det,
                data[9] = (a * l * n + b * i * p + d * j * m - a * j * p - b * l * m - d * i * n) / det,
                data[10] = (a * f * p + b * h * m + d * e * n - a * h * n - b * e * p - d * f * m) / det,
                data[11] = (a * h * j + b * e * l + d * f * i - a * f * l - b * h * i - d * e * j) / det,
                data[12] = (e * k * n + f * i * o + g * j * m - e * j * o - f * k * m - g * i * n) / det,
                data[13] = (a * j * o + b * k * m + c * i * n - a * k * n - b * i * o - c * j * m) / det,
                data[14] = (a * g * n + b * e * o + c * f * m - a * f * o - b * g * m - c * e * n) / det,
                data[15] = (a * f * k + b * g * i + c * e * j - a * g * j - b * e * k - c * f * i) / det;
        //        data[0] = in->r0.x, data[1] = in->r0.y, data[2] = in->r0.z, data[3] = in->r0.w;
        //        data[4] = in->r1.x, data[5] = in->r1.y, data[6] = in->r1.z, data[7] = in->r1.w;
        //        data[8] = in->r2.x, data[9] = in->r2.y, data[10] = in->r2.z, data[11] = in->r2.w;
        //        data[12] = in->r3.x, data[13] = in->r3.y, data[14] = in->r3.z, data[15] = in->r3.w;

        //        data[0] = a, data[1] = b, data[2] = c, data[3] = d;
        //        data[4] = e, data[5] = f, data[6] = g, data[7] = h;
        //        data[8] = i, data[9] = j, data[10] = k, data[11] = l;
        //        data[12] = m, data[13] = n, data[14] = o, data[15] = p;
        memcpy(out, &data, sizeof (float) * 16);


    }

    __host__ inline Camera *createCamera(const float4 *position, const float4 *look,
            const float4 *up, float heightangle, float aspect,
            float near, float far) {
        Camera *to_return = (Camera *) malloc(sizeof (Camera));
        memcpy(&to_return->mPosition, position, sizeof (float4));

        float4 u, v, w;
        vec4_normalize(look, &w);
        vec4_scalar_mult(&w, -1.0f, &w);
        vec4_vec4_cross(up, &w, &v);
        vec4_normalize(&v, &v);
        vec4_vec4_cross(&w, &v, &u);

        Matrix4 d_persp = {1.0f / far, 0, 0, 0,
            0, 1.0f / far, 0, 0,
            0, 0, 1.0f / far, 0,
            0, 0, 0, 1};

        Matrix4 scale = {1.0f / tan(heightangle / 2.0) / aspect, 0, 0, 0,
            0, 1.0f / tan(heightangle / 2.0), 0, 0,
            0, 0, 1.0f, 0,
            0, 0, 0, 1.0f};
        Matrix4 rotate = {v.x, v.y, v.z, 0,
            u.x, u.y, u.z, 0,
            w.x, w.y, w.z, 0,
            0, 0, 0, 1};
        Matrix4 translate = {1, 0, 0, -position->x,
            0, 1, 0, -position->y,
            0, 0, 1, -position->z,
            0, 0, 0, 1};
        mat4_mat4_mult(&d_persp, &scale, &scale);
        mat4_mat4_mult(&scale, &rotate, &rotate);
        mat4_mat4_mult(&rotate, &translate, &to_return->mWorldToFilm);
        mat4_inv(&to_return->mWorldToFilm, &to_return->mFilmToWorld);
        return to_return;
    }

    __host__ inline void releaseCamera(Camera* camera) {

        free(camera);
    }

    __host__ __device__ inline unsigned is_emitter(const SceneObject* object) {
        return (object->mat.mEmitterpower.x > 0 || object->mat.mEmitterpower.y > 0
                || object->mat.mEmitterpower.z > 0) ? 1 : 0;
    }

    __host__ inline void releaseSceneObject(SceneObject *object) {
        free(object);
    }

    __host__ inline void releaseScene(Scene *scene) {
        //free(scene->mEmitters);
        //free(scene->mObjects);
        cudaFree(scene->mEmitters);
        cudaFree(scene->mObjects);
    }

    __host__ inline SceneObject createSceneObject(SHAPE type, Matrix4 translation,
            Matrix4 rotation, Matrix4 scale, Material m) {
        SceneObject host_object; // = (SceneObject*) malloc(sizeof (SceneObject));
        host_object.type = type;
        host_object.mat = m;
        mat4_mat4_mult(&translation, &rotation, &host_object.ctm);
        mat4_mat4_mult(&host_object.ctm, &scale, &host_object.ctm_inv);
        mat4_inv(&host_object.ctm_inv, &host_object.ctm);
        return host_object;
    }

     __device__ inline float calc_intersection_sphere(const float4 *origin, const float4 *direction) {
        float a = 2 * (direction->x * direction->x + direction->y *
                direction->y + direction->z * direction->z),
                b = 2 * (direction->x * origin->x + direction->y * origin->y
                + direction->z * origin->z),
                c = origin->x * origin->x + origin->y * origin->y +
                origin->z * origin->z - 0.25f,
                discriminant = b * b - 2 * a * c;
        if (discriminant < 0) return -1.0f;
        return (-b - sqrtf(discriminant)) / a;

    }

    __device__ inline float calc_intersection_cube(const float4 *origin, const float4 *direction) {

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

    __device__ inline void calc_normal_sphere(const float4 *obj_intersect, SurfacePoint *intersection) {
        intersection->normal.x = intersection->object->ctm.r0.x *
                obj_intersect->x + intersection->object->ctm.r1.x *
                obj_intersect->y + intersection->object->ctm.r2.x *
                obj_intersect->z;
        intersection->normal.y = intersection->object->ctm.r0.y *
                obj_intersect->x + intersection->object->ctm.r1.y *
                obj_intersect->y + intersection->object->ctm.r2.y *
                obj_intersect->z;
        intersection->normal.z = intersection->object->ctm.r0.z *
                obj_intersect->x + intersection->object->ctm.r1.z *
                obj_intersect->y + intersection->object->ctm.r2.z *
                obj_intersect->z;
        intersection->normal.w = 0;
    }

    __host__ __device__ inline void calc_normal_cube(const float4 *obj_intersect, SurfacePoint *intersection) {
        vec4_set(&intersection->normal, 0.0f, 0.0f, 0.0f, 0.0f);
        if (EQ(obj_intersect->y, 0.5)) intersection->normal.y = 1;
        else if (EQ(obj_intersect->y, -0.5)) intersection->normal.y = -1;
        else if (EQ(obj_intersect->z, 0.5)) intersection->normal.z = 1;
        else if (EQ(obj_intersect->z, -0.5)) intersection->normal.z = -1;
        else if (EQ(obj_intersect->x, 0.5)) intersection->normal.x = 1;
        else if (EQ(obj_intersect->x, -0.5)) intersection->normal.x = -1;
    }

    __host__ __device__ inline float calc_surface_area_sphere(SceneObject* obj) {
        float4 dir = {0.5, 0, 0, 0};
        mat4_vec4_mult(&obj->ctm_inv, &dir, &dir);
        float radius = vec4_magnitude(&dir);
        obj->mSize = radius;
        return 4 * PI * radius*radius;
    }

    __host__ __device__ inline float calc_surface_area_cube(SceneObject *obj) {
        float4 dir = {1.0, 0, 0, 0};
        mat4_vec4_mult(&obj->ctm_inv, &dir, &dir);
        float edgeLength = vec4_magnitude(&dir);
        obj->mSize = edgeLength;
        return edgeLength * edgeLength * 6.0f;
    }

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

    __device__ inline void vec4_cos_random(const float4 *normal, float u, float v, float4 *direction_out) {
        const float theta = acosf(sqrtf(u)), phi = TWO_PI * v;
        vec4_set(direction_out, sinf(theta) * cosf(phi), cosf(theta), sinf(theta) * sinf(phi), 0.0f);
        vec4_normalize(direction_out, direction_out);
        float4 U, V, up = {0.0f, 1.0f, 0.0f, 0.0f};
        if (fabs(vec4_vec4_dot(&up, normal)) > 0.8f) vec4_set(&up, 1.0f, 0.0f, 0.0f, 0.0f);
        vec4_vec4_cross(&up, normal, &U);
        vec4_normalize(&U, &U);
        vec4_vec4_cross(normal, &U, &V);
        vec4_normalize(&V, &V);
        vec4_scalar_mult(&U, direction_out->x, &U);
        vec4_scalar_mult(&V, direction_out->z, &V);
        vec4_vec4_add(&U, &V, &U);
        vec4_scalar_mult(normal, direction_out->y, &V);
        vec4_vec4_add(&U, &V, direction_out);
    }

    __device__ inline void urand_sphere(float u, float v, float r, float4 *out) {
        float theta = TWO_PI * u;
        
        float phi = acosf(2 * v - 1);
        float sphi = sinf(phi);
        vec4_set(out, r * __cosf(theta) * sphi, r * cosf(phi), r * sinf(theta) * sphi, 1.0f);
        //vec4_set(out, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    __device__ inline void reflect_ray(const float4 *normal, const Ray *wi, Ray *r) {
        vec4_scalar_mult(normal, vec4_vec4_dot(normal, &wi->direction) * 2.0f, &r->origin);
        vec4_vec4_sub(&wi->direction, &r->origin, &r->direction);
    }

    __device__ inline unsigned reflect_ray_lambertian(const SurfacePoint *intersect, const Ray *wi, Ray *wo, float *prob, int idx) {
        if (vec4_vec4_dot(&intersect->normal, &wi->direction) > 0)
            vec4_scalar_mult(&intersect->normal, -1.0f, &wo->origin);
        else
            wo->origin = intersect->normal;
        *prob = 1.0f;
        float phi = urand(idx), psi = urand(idx);
        float r = urand(idx);
        if (r < intersect->object->mat.ks) {
            reflect_ray(&wo->origin, wi, wo);
            wo->origin = intersect->intersection;
            return 1;
        } else {
            vec4_cos_random(&wo->origin, phi, psi, &wo->direction);
            wo->origin = intersect->intersection;
            return 0; //not spec
        }
    }

    __device__ inline void refract_ray(const float4 *normal, const float in, const float out, const Ray *wi, Ray *r) {
        float4 norm = *normal;
        float4 w = wi->direction;
        vec4_scalar_mult(&w, -1.0f, &w);
        float n = in / out;
        if (vec4_vec4_dot(&wi->direction, &norm) >= 0) {
            vec4_scalar_mult(&norm, -1.0f, &norm);
            n = out / in;
        }
        const float d = vec4_vec4_dot(&norm, &w);
        const float det = 1.0 - n * n * (1.0 - d * d);
        vec4_scalar_mult(&norm, n * d - sqrtf(det), &norm);
        vec4_scalar_mult(&w, -n, &r->direction);
        vec4_vec4_add(&r->direction, &norm, &r->direction);
        r->direction.w = 0;
        vec4_normalize(&r->direction, &r->direction);
    }

    __device__ inline void calc_sample_point_sphere(const SceneObject *obj, float u, float v, float4 *point_out) {
        urand_sphere(u, v, .5f - EPSILON, point_out);
       // urvec4_set(point_out, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    //    __host__ __device__ inline void calc_sample_point_cube(const SceneObject *obj, Sampler *s, float4 *point_out) {
    //        float edge = 0.5f;
    //        float u = urand(s) - edge, v = urand(s) - edge;
    //        int r = urand(s) * 6;
    //        switch (r) {
    //            case 0:
    //                vec4_set(point_out, -edge, u, v, 1.0f);
    //                break;
    //            case 1:
    //                vec4_set(point_out, edge, u, v, 1.0f);
    //                break;
    //            case 2:
    //                vec4_set(point_out, u, -edge, v, 1.0f);
    //                break;
    //            case 3:
    //                vec4_set(point_out, u, edge, v, 1.0f);
    //                break;
    //            case 4:
    //                vec4_set(point_out, u, v, -edge, 1.0f);
    //                break;
    //            case 5:
    //                vec4_set(point_out, u, v, edge, 1.0f);
    //                break;
    //        }
    //    }

    __device__ inline unsigned calc_nearest_intersection(Scene *scene, const Ray *wi, SurfacePoint *intersect_out) {
        unsigned i = 0;
        float min_t = FLT_MAX;
        SceneObject *objects = scene->mObjects, *near_object;
        scene->mNumrays += scene->mNumObjects;
        for (; i < scene->mNumObjects; ++i) {

            mat4_vec4_mult(&objects[i].ctm, &wi->direction, &intersect_out->normal);
            mat4_vec4_mult(&objects[i].ctm, &wi->origin, &intersect_out->intersection);
            float t;
            switch (objects[i].type) {
                case Sphere:
                    t = calc_intersection_sphere(&intersect_out->intersection, &intersect_out->normal);
                    break;
                case Cube:
                    t = calc_intersection_cube(&intersect_out->intersection, &intersect_out->normal);
                    break;
            }
            if (t >= 0 && t < min_t) {
                min_t = t;
                near_object = &objects[i];
            }
        }
        if (min_t < FLT_MAX) {
            float4 obj_intersect, ob_direction;
            intersect_out->object = near_object;
            vec4_scalar_mult(&wi->direction, min_t, &intersect_out->intersection);
            vec4_vec4_add(&wi->origin, &intersect_out->intersection, &intersect_out->intersection);
            mat4_vec4_mult(&near_object->ctm, &wi->origin, &obj_intersect);
            mat4_vec4_mult(&near_object->ctm, &wi->direction, &ob_direction);
            vec4_scalar_mult(&ob_direction, min_t, &ob_direction);
            vec4_vec4_add(&obj_intersect, &ob_direction, &obj_intersect);
            switch (near_object->type) {
                case Sphere:
                    calc_normal_sphere(&obj_intersect, intersect_out);
                    break;
                case Cube:
                    calc_normal_cube(&obj_intersect, intersect_out);
                    break;
            }

            vec4_normalize(&intersect_out->normal, &intersect_out->normal);
            return 1;
        }
        return 0;
    }

    __device__ inline void calc_world_ray(const Scene *scene, const float ndc_x, const float ndc_y, Ray *ray_out) {
        ray_out->direction.x = 2.0f * ndc_x - 1.0f;
        ray_out->direction.y = 1.0f - 2.0f * ndc_y;
        ray_out->direction.z = -1.0f;
        ray_out->direction.w = 1.0f;
        mat4_vec4_mult(&scene->mCamera.mFilmToWorld, &ray_out->direction, &ray_out->direction);
        vec4_vec4_sub(&ray_out->direction, &scene->mCamera.mPosition, &ray_out->direction);
        ray_out->origin = scene->mCamera.mPosition;
    }

    __host__ inline Scene *createScene(SceneObject *objects, const unsigned num_objects, Camera *camera) {
        Scene *sc = (Scene*) malloc(sizeof (Scene));

        sc->mObjects = objects;
        sc->mNumObjects = num_objects;
        sc->mNumrays = 0;

        //Initialize the emitters in the scene
        unsigned i, j;
        sc->mNumEmitters = 0;
        for (i = 0; i < num_objects; ++i) {
            if (is_emitter(&objects[i])) {
                sc->mNumEmitters++;
            }
        }
        SceneObject *emitters = (SceneObject*) malloc(sizeof (SceneObject) * sc->mNumEmitters);

        for (i = 0, j = 0; i < num_objects; ++i) {
            if (is_emitter(&objects[i])) {
                emitters[j++] = sc->mObjects[i];
            }
        }
        sc->mCamera = *camera;
        sc->mSurfaceAreaFuncs[0] = &calc_surface_area_sphere;
        sc->mSurfaceAreaFuncs[1] = &calc_surface_area_cube;

        for (i = 0; i < num_objects; ++i)
            objects[i].mSurfaceArea = sc->mSurfaceAreaFuncs[sc->mObjects[i].type](&sc->mObjects[i]);
        cudaMalloc((void**) & sc->mObjects, sizeof (SceneObject) * num_objects);
        cudaMalloc((void**) & sc->mEmitters, sizeof (SceneObject) * sc->mNumEmitters);
        CUDA_SAFE_CALL(cudaMemcpy(sc->mObjects, objects, sizeof (SceneObject) * num_objects, cudaMemcpyHostToDevice));
        CUDA_SAFE_CALL(cudaMemcpy(sc->mEmitters, emitters, sizeof (SceneObject) * sc->mNumEmitters, cudaMemcpyHostToDevice));
        free(emitters);
        return sc;
    }

    __device__ inline void calc_direct_illumination(const SurfacePoint *intersect, const Scene *scene,
            float u, float v, float3 * color_out) {
        if (is_emitter(intersect->object)) {
            vec3_vec3_add(color_out, &intersect->object->mat.mEmitterpower, color_out);
            return;
        }

        SceneObject *emitters = scene->mEmitters;
        unsigned i = 0;
        Ray temp_ray;
        SurfacePoint light_intersect;
        for (; i < scene->mNumEmitters; ++i) {
            float3 temp_color = {0.0f, 0.0f, 0.0f};
            switch (emitters[i].type) {
                case Sphere:
                    calc_sample_point_sphere(&emitters[i], u, v, &temp_ray.direction);
            }
           //temp_ray.direction.x = temp_ray.direction.y = temp_ray.direction.z = 0.0f;
            //temp_ray.direction.w = 1.0f;
            mat4_vec4_mult(&emitters[i].ctm_inv, &temp_ray.direction, &temp_ray.direction); //ctm^-1 * sample pt
            vec4_vec4_sub(&temp_ray.direction, &intersect->intersection, &temp_ray.direction); //pt -= intersect
            float t = vec4_normalize(&temp_ray.direction, &temp_ray.direction);
            vec4_scalar_mult(&temp_ray.direction, EPSILON, &temp_ray.origin);
            vec4_vec4_add(&intersect->intersection, &temp_ray.origin, &temp_ray.origin);
            if (calc_nearest_intersection((Scene*) scene, &temp_ray, &light_intersect) &&
                    is_emitter(light_intersect.object)) {
                float cosWoWi = (vec4_vec4_dot(&intersect->normal, &temp_ray.direction));
                if (cosWoWi < 0) continue;
                vec4_scalar_mult(&temp_ray.direction, -1.0f, &temp_ray.direction);
                cosWoWi *= (vec4_vec4_dot(&light_intersect.normal, &temp_ray.direction));
                if (cosWoWi < 0) continue;
                float attenuation = light_intersect.object->mSurfaceArea /
                        (light_intersect.object->mat.mAttenuation.x +
                        light_intersect.object->mat.mAttenuation.y * t +
                        light_intersect.object->mat.mAttenuation.z * t * t);

                vec3_vec3_mult(&light_intersect.object->mat.mEmitterpower,
                        &intersect->object->mat.mDiffuse, &temp_color);
                vec3_scalar_mult(&temp_color, cosWoWi * attenuation, &temp_color);

                vec3_vec3_add(&temp_color, color_out, color_out);
            } 
        }
       // color_out->x = color_out->y =color_out->z = 1.0f;
    }

        __device__ unsigned generate_path(const Scene *scene, Ray *r, int idx, PathVertex *vertexes, unsigned maxDepth) {
            float pAbsorb = 0.01f;
            unsigned i = 0;
            for (; i < maxDepth; ++i) {
                vec4_vec4_scalar_add_weighted(&r->direction, EPSILON, &r->origin);
                if (!calc_nearest_intersection((Scene*) scene, r, &vertexes[i].mIntersect)) break;
                float pCont = 1.0f;
                if (reflect_ray_lambertian(&vertexes[i].mIntersect, r, r, &pCont, idx)) {
                    --i;
                    continue;
                }
                if (pCont == 0 || urand(idx) < pAbsorb || (pCont < 1 && urand(idx) > pCont)) break;
                vertexes[i].mProb = 1.0 / (pCont * (1 - pAbsorb));
                vertexes[i].mWout = r->direction;
            }
            return i;
        }
    
        __device__ void evaluate_path(const Scene *scene, int idx, const PathVertex *vertexes, unsigned n, float3 * color_out) {
            int i = n - 1;
            float3 temp_color = {0.0f, 0.0f, 0.0f};
            for (; i >= 0; --i) {
    
                float coeff = vec4_vec4_dot(&vertexes[i].mIntersect.normal, &vertexes[i].mWout) * vertexes[i].mProb;
                vec3_scalar_mult(&temp_color, coeff, &temp_color);
                vec3_vec3_mult(&temp_color, &vertexes[i].mIntersect.object->mat.mDiffuse, &temp_color);
                calc_direct_illumination(&vertexes[i].mIntersect, scene, urand(idx), urand(idx), &temp_color);
            }
            if(temp_color.x >= 0 && temp_color.y >= 0 && temp_color.z >=0)
	            vec3_vec3_add(&temp_color, color_out, color_out);
        }
#ifdef	__cplusplus
}
#endif

#endif	/* _DEFS_CUH */

