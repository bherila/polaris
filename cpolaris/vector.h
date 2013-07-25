/* 
 * File:   vector.h
 * Author: psastras
 *
 * Created on April 26, 2010, 4:34 PM
 */

#define EPSILON 0.0001
#define PI 3.14159265
#define TWO_PI 6.28318531
#define EQ(x, y) (((x) <= ((y) + EPSILON)) && ((x) >= ((y) - EPSILON)))
#ifndef _VECTOR_H
#define	_VECTOR_H

#ifdef	__cplusplus
extern "C" {
#endif

    //Structures

    typedef struct vector3 {

        union {

            struct {
                float x, y, z;
            };
            float data[4];
        };
    } Vector3;

    typedef Vector3 Color3;

    typedef struct vector4 {

        union {

            struct {
                float x, y, z, w;
            };
            float data[4];
        };
    } Vector4;

    //Constructors & Destructors

    //Methods
    inline void vec3_vec3_add(const Vector3 *lhs, const Vector3 *rhs, Vector3 *out);
    inline void vec3_scalar_mult(const Vector3 *vec, float s, Vector3 *out);
    inline void vec3_vec3_mult(const Vector3 *lhs, const Vector3 *rhs, Vector3 *out);
    inline void vec3_set(Vector3 *vec, float x, float y, float z);
    inline void vec3_printf(const Vector3 *vec);

    inline void vec4_vec4_scalar_add_weighted(const Vector4 *vec_1, float alpha, Vector4 *out); //In place
    inline void vec4_vec4_lin_combine(const Vector4 *vec_1, float alpha, const Vector4 *vec_2, float beta, Vector4 *out);
    inline void vec4_vec4_add(const Vector4 *lhs, const Vector4 *rhs, Vector4 *out);
    inline void vec4_vec4_sub(const Vector4 *lhs, const Vector4 *rhs, Vector4 *out);
    inline float vec4_vec4_dot(const Vector4 *lhs, const Vector4 *rhs);
    inline void vec4_vec4_cross(const Vector4 *lhs, const Vector4 *rhs, Vector4 *out);
    inline void vec4_scalar_mult(const Vector4 *vec, float s, Vector4 *out);
    inline float vec4_magnitude(const Vector4 *vec);
    inline float vec4_magnitude2(const Vector4 *vec);
    inline float vec4_distance(const Vector4 *lhs, const Vector4 *rhs);
    inline float vec4_distance2(const Vector4 *lhs, const Vector4 *rhs);
    inline float vec4_normalize(const Vector4 *vec, Vector4 *out);
    inline void vec4_set(Vector4 *vec, float x, float y, float z, float w);
    inline void vec4_printf(const Vector4 *vec);
    inline void vec4_cos_random(const Vector4 *normal, float u, float v, Vector4 *direction_out);

#ifdef	__cplusplus
}
#endif

#endif	/* _VECTOR_H */

