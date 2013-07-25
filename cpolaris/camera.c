#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "camera.h"

Camera *createCamera(const Vector4 *position, const Vector4 *look,
        const Vector4 *up, float heightangle, float aspect,
           float near, float far) {
    Camera *to_return = (Camera*)malloc(sizeof(Camera));
    to_return->mWorldToFilm = (Matrix4*)malloc(sizeof(Matrix4));
    to_return->mFilmToWorld = (Matrix4*)malloc(sizeof(Matrix4));
    to_return->mPosition = (Vector4*)malloc(sizeof(Vector4));
    memcpy(to_return->mPosition, position, sizeof(Vector4));

    Vector4 u, v, w;
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
    mat4_mat4_mult(&rotate, &translate, to_return->mWorldToFilm);
    mat4_inv(to_return->mWorldToFilm, to_return->mFilmToWorld);
    return to_return;
}


void releaseCamera(Camera* camera) {
    free(&camera->mFilmToWorld->data);
    free(&camera->mWorldToFilm->data);
    free(&camera->mPosition->data);
    free(camera);
}
