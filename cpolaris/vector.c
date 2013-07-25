#include <math.h>
#include <stdio.h>
#include "vector.h"

inline void vec4_vec4_add(const Vector4 *lhs, const Vector4 *rhs, Vector4 *out) {
    out->x = lhs->x + rhs->x;
    out->y = lhs->y + rhs->y;
    out->z = lhs->z + rhs->z;
    out->w = lhs->w + rhs->w;
}

inline void vec4_vec4_sub(const Vector4 *lhs, const Vector4 *rhs, Vector4 *out) {
    out->x = lhs->x - rhs->x;
    out->y = lhs->y - rhs->y;
    out->z = lhs->z - rhs->z;
    out->w = lhs->w - rhs->w;
}

inline float vec4_vec4_dot(const Vector4 *lhs, const Vector4 *rhs) {
    return (lhs->x * rhs->x) + (lhs->y * rhs->y) + (lhs->z * rhs->z) + (lhs->w * rhs->w);
}

inline void vec4_vec4_cross(const Vector4 *lhs, const Vector4 *rhs, Vector4 *out) {
    out->x = lhs->y * rhs->z - lhs->z * rhs->y;
    out->y = lhs->z * rhs->x - lhs->x * rhs->z;
    out->z = lhs->x * rhs->y - lhs->y * rhs->x;
    out->w = 0;
}

inline void vec4_scalar_mult(const Vector4 *vec, float s, Vector4 *out) {
    out->x = vec->x * s;
    out->y = vec->y * s;
    out->z = vec->z * s;
    out->w = vec->w * s;
}

inline void vec4_vec4_lin_combine(const Vector4 *vec_1, float alpha, const Vector4 *vec_2, float beta, Vector4 *out) {
    out->x = vec_1->x * alpha + vec_2->x * beta;
    out->y = vec_1->y * alpha + vec_2->y * beta;
    out->z = vec_1->z * alpha + vec_2->z * beta;
    out->w = vec_1->w * alpha + vec_2->w * beta;
}

inline void vec4_vec4_scalar_add_weighted( const Vector4 *vec_1, float alpha, Vector4 *out) {
    out->x += vec_1->x * alpha;
    out->y += vec_1->y * alpha;
    out->z += vec_1->z * alpha;
    out->w += vec_1->w * alpha;
}

inline float vec4_normalize(const Vector4 *vec, Vector4 *out) {
    float m = vec4_magnitude(vec);
    vec4_scalar_mult(vec, 1.0f / m, out);
    return m;
}

inline float vec4_magnitude(const Vector4 *vec) {
    return sqrt(vec4_magnitude2(vec));
}

inline float vec4_magnitude2(const Vector4 *vec) {
    return vec->x * vec->x + vec->y * vec->y + vec->z * vec->z + vec->w * vec->w;
}

inline void vec4_set(Vector4 *vec, float x, float y, float z, float w) {
    vec->x = x;
    vec->y = y;
    vec->z = z;
    vec->w = w;
}

inline void vec4_printf(const Vector4 *vec) {
    printf("[%'.4f %'.4f %'.4f %'.4f]\n", vec->x, vec->y, vec->z, vec->w);
}

inline void vec3_vec3_add(const Vector3 *lhs, const Vector3 *rhs, Vector3 *out) {
    out->x = lhs->x + rhs->x;
    out->y = lhs->y + rhs->y;
    out->z = lhs->z + rhs->z;
}

inline void vec3_scalar_mult(const Vector3 *vec, float s, Vector3 *out) {
    out->x = vec->x * s;
    out->y = vec->y * s;
    out->z = vec->z * s;
}

inline void vec3_set(Vector3 *vec, float x, float y, float z) {
    vec->x = x;
    vec->y = y;
    vec->z = z;
}

inline void vec3_printf(const Vector3 *vec) {
    printf("[%'.4f %'.4f %'.4f]\n", vec->x, vec->y, vec->z);
}

inline void vec3_vec3_mult(const Vector3 *lhs, const Vector3 *rhs, Vector3 *out) {
    out->x = lhs->x * rhs->x;
    out->y = lhs->y * rhs->y;
    out->z = lhs->z * rhs->z;
}

void vec4_cos_random(const Vector4 *normal, float u, float v, Vector4 *direction_out) {
    const float theta = acos(sqrt(u)), phi = TWO_PI * v;
    vec4_set(direction_out, sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi), 0.0f);
    vec4_normalize(direction_out, direction_out);
    Vector4 U, V, up = {0.0f, 1.0f, 0.0f, 0.0f};
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
