#include "matrix.h"

#include <math.h>
#include <string.h>

inline Matrix4 create_identity_mat4() {
    Matrix4 to_return = {1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
    return to_return;
}

inline Matrix4 create_translation_mat4(float x, float y, float z) {
    Matrix4 to_return = {1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f};
    return to_return;
}

inline Matrix4 create_scale_mat4(float s_1, float s_2, float s_3) {
    Matrix4 to_return = {s_1, 0.0f, 0.0f, 0,
        0.0f, s_2, 0.0f, 0,
        0.0f, 0.0f, s_3, 0,
        0.0f, 0.0f, 0.0f, 1.0f};
    return to_return;
}

inline Matrix4 create_rotx_mat4(const float radians) {
    Matrix4 to_return = {1, 0, 0, 0,
        0, cos(radians), -sin(radians), 0,
        0, sin(radians), cos(radians), 0,
        0, 0, 0, 1};
    return to_return;
}

inline Matrix4 create_roty_mat4(const float radians) {
    Matrix4 to_return = {cos(radians), 0, sin(radians), 0,
                        0, 1, 0, 0,
                        -sin(radians), 0, cos(radians), 0,
                        0, 0, 0, 1};
    return to_return;
}


inline void mat4_mat4_mult(const Matrix4 *lhs, const Matrix4 *rhs, Matrix4 *out) {
    float data[16];
    data[0] = lhs->data[0] * rhs->data[0] + lhs->data[1] * rhs->data[4] +
            lhs->data[2] * rhs->data[8] + lhs->data[3] * rhs->data[12];
    data[1] = lhs->data[0] * rhs->data[1] + lhs->data[1] * rhs->data[5] +
            lhs->data[2] * rhs->data[9] + lhs->data[3] * rhs->data[13];
    data[2] = lhs->data[0] * rhs->data[2] + lhs->data[1] * rhs->data[6] +
            lhs->data[2] * rhs->data[10] + lhs->data[3] * rhs->data[14];
    data[3] = lhs->data[0] * rhs->data[3] + lhs->data[1] * rhs->data[7] +
            lhs->data[2] * rhs->data[11] + lhs->data[3] * rhs->data[15];

    data[4] = lhs->data[4] * rhs->data[0] + lhs->data[5] * rhs->data[4] +
            lhs->data[6] * rhs->data[8] + lhs->data[7] * rhs->data[12];
    data[5] = lhs->data[4] * rhs->data[1] + lhs->data[5] * rhs->data[5] +
            lhs->data[6] * rhs->data[9] + lhs->data[7] * rhs->data[13];
    data[6] = lhs->data[4] * rhs->data[2] + lhs->data[5] * rhs->data[6] +
            lhs->data[6] * rhs->data[10] + lhs->data[7] * rhs->data[14];
    data[7] = lhs->data[4] * rhs->data[3] + lhs->data[5] * rhs->data[7] +
            lhs->data[6] * rhs->data[11] + lhs->data[7] * rhs->data[15];

    data[8] = lhs->data[8] * rhs->data[0] + lhs->data[9] * rhs->data[4] +
            lhs->data[10] * rhs->data[8] + lhs->data[11] * rhs->data[12];
    data[9] = lhs->data[8] * rhs->data[1] + lhs->data[9] * rhs->data[5] +
            lhs->data[10] * rhs->data[9] + lhs->data[11] * rhs->data[13];
    data[10] = lhs->data[8] * rhs->data[2] + lhs->data[9] * rhs->data[6] +
            lhs->data[10] * rhs->data[10] + lhs->data[11] * rhs->data[14];
    data[11] = lhs->data[8] * rhs->data[3] + lhs->data[9] * rhs->data[7] +
            lhs->data[10] * rhs->data[11] + lhs->data[11] * rhs->data[15];

    data[12] = lhs->data[12] * rhs->data[0] + lhs->data[13] * rhs->data[4] +
            lhs->data[14] * rhs->data[8] + lhs->data[15] * rhs->data[12];
    data[13] = lhs->data[12] * rhs->data[1] + lhs->data[13] * rhs->data[5] +
            lhs->data[14] * rhs->data[9] + lhs->data[15] * rhs->data[13];
    data[14] = lhs->data[12] * rhs->data[2] + lhs->data[13] * rhs->data[6] +
            lhs->data[14] * rhs->data[10] + lhs->data[15] * rhs->data[14];
    data[15] = lhs->data[12] * rhs->data[3] + lhs->data[13] * rhs->data[7] +
            lhs->data[14] * rhs->data[11] + lhs->data[15] * rhs->data[15];

    memcpy(out, &data, sizeof (float) * 16);
}

inline void mat4_inv(const Matrix4 *in, Matrix4 *out) {
    float det = mat4_determinant(in);
    float data[16];
    data[0] = (in->f * in->k * in->p + in->g * in->l * in->n + in->h * in->j * in->o - in->f *
            in->l * in->o - in->g * in->j * in->p - in->h * in->k * in->n) / det,
            data[1] = (in->b * in->l * in->o + in->c * in->j * in->p + in->d * in->k * in->n - in->b *
            in->k * in->p - in->c * in->l * in->n - in->d * in->j * in->o) / det,
            data[2] = (in->b * in->g * in->p + in->c * in->h * in->n + in->d * in->f * in->o - in->b *
            in->h * in->o - in->c * in->f * in->p - in->d * in->g * in->n) / det,
            data[3] = (in->b * in->h * in->k + in->c * in->f * in->l + in->d * in->g * in->j - in->b *
            in->g * in->l - in->c * in->h * in->j - in->d * in->f * in->k) / det,
            data[4] = (in->e * in->l * in->o + in->h * in->k * in->m + in->g * in->i * in->p - in->e *
            in->k * in->p - in->g * in->l * in->m - in->h * in->i * in->o) / det,
            data[5] = (in->a * in->k * in->p + in->c * in->l * in->m + in->d * in->i * in->o - in->a *
            in->l * in->o - in->c * in->i * in->p - in->d * in->k * in->m) / det,
            data[6] = (in->a * in->h * in->o + in->c * in->e * in->p + in->d * in->g * in->m - in->a *
            in->g * in->p - in->c * in->h * in->m - in->d * in->e * in->o) / det,
            data[7] = (in->a * in->g * in->l + in->c * in->h * in->i + in->d * in->e * in->k - in->a *
            in->h * in->k - in->c * in->e * in->l - in->d * in->g * in->i) / det,
            data[8] = (in->e * in->j * in->p + in->f * in->l * in->m + in->h * in->i * in->n - in->e *
            in->l * in->n - in->f * in->i * in->p - in->h * in->j * in->m) / det,
            data[9] = (in->a * in->l * in->n + in->b * in->i * in->p + in->d * in->j * in->m - in->a *
            in->j * in->p - in->b * in->l * in->m - in->d * in->i * in->n) / det,
            data[10] = (in->a * in->f * in->p + in->b * in->h * in->m + in->d * in->e * in->n - in->a *
            in->h * in->n - in->b * in->e * in->p - in->d * in->f * in->m) / det,
            data[11] = (in->a * in->h * in->j + in->b * in->e * in->l + in->d * in->f * in->i - in->a *
            in->f * in->l - in->b * in->h * in->i - in->d * in->e * in->j) / det,
            data[12] = (in->e * in->k * in->n + in->f * in->i * in->o + in->g * in->j * in->m - in->e *
            in->j * in->o - in->f * in->k * in->m - in->g * in->i * in->n) / det,
            data[13] = (in->a * in->j * in->o + in->b * in->k * in->m + in->c * in->i * in->n - in->a *
            in->k * in->n - in->b * in->i * in->o - in->c * in->j * in->m) / det,
            data[14] = (in->a * in->g * in->n + in->b * in->e * in->o + in->c * in->f * in->m - in->a *
            in->f * in->o - in->b * in->g * in->m - in->c * in->e * in->n) / det,
            data[15] = (in->a * in->f * in->k + in->b * in->g * in->i + in->c * in->e * in->j - in->a *
            in->g * in->j - in->b * in->e * in->k - in->c * in->f * in->i) / det;
    memcpy(out, &data, sizeof (float) * 16);
}

inline void mat4_vec4_mult(const Matrix4 *lhs, const Vector4 *rhs, Vector4 *out) {
    float data[4];
    data[0] = vec4_vec4_dot(&lhs->r0, rhs);
    data[1] = vec4_vec4_dot(&lhs->r1, rhs);
    data[2] = vec4_vec4_dot(&lhs->r2, rhs);
    data[3] = vec4_vec4_dot(&lhs->r3, rhs);
    memcpy(out, &data, sizeof (float) * 4);
}

inline float mat4_determinant(const Matrix4 *m) {
    return m->a * m->f * m->k * m->p - m->a * m->f * m->l * m->o - m->a * m->g * m->j * m->p + m->a *
            m->g * m->l * m->n + m->a * m->h * m->j * m->o - m->a * m->h * m->k * m->n - m->b * m->e *
            m->k * m->p + m->b * m->e * m->l * m->o + m->b * m->g * m->i * m->p - m->b * m->g * m->l *
            m->m - m->b * m->h * m->i * m->o + m->b * m->h * m->k * m->m + m->c * m->e * m->j * m->p -
            m->c * m->e * m->l * m->n - m->c * m->f * m->i * m->p + m->c * m->f * m->l * m->m + m->c *
            m->h * m->i * m->n - m->c * m->h * m->j * m->m - m->d * m->e * m->j * m->o + m->d * m->e *
            m->k * m->n + m->d * m->f * m->i * m->o - m->d * m->f * m->k * m->m - m->d * m->g * m->i *
            m->n + m->d * m->g * m->j * m->m;
}

inline void mat4_printf(const Matrix4 *m) {
    vec4_printf(&m->r0);
    vec4_printf(&m->r1);
    vec4_printf(&m->r2);
    vec4_printf(&m->r3);
}
