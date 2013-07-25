#include "renderer.h"
#include <float.h>
#include <stdio.h>
#include "kdtree.h"
#include <math.h>
#include <stdlib.h>

#define MAX_DEPTH 8

//Forward Declares
void calc_direct_illumination(const SurfacePoint *intersect, const Scene *scene,
        Sampler *s, Color3 *color_out);
unsigned generate_path(const Scene *scene, Ray *r, Sampler *s, PathVertex *vertexes, unsigned n);
void evaluate_path(const Scene *scene, Sampler *s, const PathVertex *vertexes, unsigned n, Color3 *color_out);

unsigned generate_path(const Scene *scene, Ray *r, Sampler *s, PathVertex *vertexes, unsigned maxDepth) {
    float pAbsorb = 0.01f;
    unsigned i = 0;
    for (; i < maxDepth; ++i) {
        vec4_vec4_scalar_add_weighted(&r->direction, EPSILON, &r->origin);
        if (!calc_nearest_intersection((Scene*) scene, r, &vertexes[i].mIntersect)) break;
        float pCont = 1.0f;
        if (reflect_ray_lambertian(&vertexes[i].mIntersect, r, r, &pCont, s)) {
            --i;
            continue;
        }
        if (pCont == 0 || urand(s) < pAbsorb || (pCont < 1 && urand(s) > pCont)) break;
        vertexes[i].mProb = 1.0 / (pCont * (1 - pAbsorb));
        vertexes[i].mWout = r->direction;
    }
    return i;
}

void evaluate_path(const Scene *scene, Sampler *s, const PathVertex *vertexes, unsigned n, Color3 * color_out) {
    int i = n - 1;
    Color3 temp_color = {0.0f, 0.0f, 0.0f};
    for (; i >= 0; --i) {

        float coeff = vec4_vec4_dot(&vertexes[i].mIntersect.normal, &vertexes[i].mWout) * vertexes[i].mProb;
        vec3_scalar_mult(&temp_color, coeff, &temp_color);

        vec3_vec3_mult(&temp_color, &vertexes[i].mIntersect.object->mat.mDiffuse, &temp_color);
        calc_direct_illumination(&vertexes[i].mIntersect, scene, s, &temp_color);
    }
    vec3_vec3_add(&temp_color, color_out, color_out);
}

void calc_direct_illumination(const SurfacePoint *intersect, const Scene *scene,
        Sampler *s, Color3 * color_out) {
    if (is_emitter(intersect->object)) {
        vec3_vec3_add(color_out, &intersect->object->mat.mEmitterpower, color_out);
        return;
    }
    unsigned num_samples = 1;
    SceneObject **emitters = scene->mEmitters;
    unsigned i = scene->mNumEmitters - 1, j = 0;
    Ray temp_ray;
    SurfacePoint light_intersect;


    for (; i < scene->mNumEmitters; ++i) {
        for (; j < num_samples; ++j) {
            Color3 temp_color = {0.0f, 0.0f, 0.0f};
            scene->mSampleFuncs[emitters[i]->type](emitters[i], (Sampler*) s, &temp_ray.direction);
            mat4_vec4_mult(&emitters[i]->ctm_inv, &temp_ray.direction, &temp_ray.direction); //ctm^-1 * sample pt
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
                vec3_scalar_mult(&temp_color, cosWoWi * attenuation * 1 / (float)num_samples, &temp_color);

                vec3_vec3_add(&temp_color, color_out, color_out);
            } else {
              //  printf("fail at life: %f",t);
            }
        }
    }
}

void path_trace(const Scene *scene, Sampler *s, Color3 *image_out, const RenderSettings * settings) {
    unsigned x, y, i, j, k, l;
    float u, v;
    float total = (settings->mROI.mYBottom - settings->mROI.mYTop) * settings->mAASamples;
    Ray world_ray;
    SurfacePoint intersect_point;
    unsigned max_vertices = MAX_DEPTH;
    PathVertex *path_vertices = (PathVertex*) malloc(sizeof (PathVertex) * max_vertices);
    for (i = 0, j = 0; i < settings->mAASamples; ++i) {
        for (y = settings->mROI.mYTop; y <= settings->mROI.mYBottom; ++y, ++j) {
            for (x = settings->mROI.mXLeft; x <= settings->mROI.mXRight; ++x) {
                u = urand(s);
                v = urand(s);
                calc_world_ray(scene, (x + u) / (float) (settings->mWidth),
                        (y + v) / (float) (settings->mHeight), &world_ray);
#ifdef USE_KDTREE
                if (kdtree_intersect((Scene*)scene, scene->mTree, &world_ray, &intersect_point)) {
#else
                if (calc_nearest_intersection((Scene*)scene, &world_ray, &intersect_point)) {
#endif
                    unsigned len = generate_path(scene, &world_ray, s, path_vertices, max_vertices);
                    evaluate_path(scene, s, path_vertices, len, &image_out[y * settings->mWidth + x]);
                }
            }
#ifndef USE_MPI
            int complete = (int) (j / total * 100) + 1;
            printf("[\033[01m");
            for (l = 0; l < complete / 2; l++) printf("=");
            printf(">");
            for (k = l; k < 50; k++) printf(" ");
            printf("\033[0m] %d%\r", complete);
            fflush(stdout);
#endif
        }
    }
#ifndef USE_MPI
    printf("\n\n");
#endif
    for (y = settings->mROI.mYTop; y < settings->mROI.mYBottom; ++y) {
        for (x = settings->mROI.mXLeft; x < settings->mROI.mXRight; ++x) {
            vec3_scalar_mult(&image_out[y * settings->mWidth + x],
                    1 / (float) settings->mAASamples, &image_out[y * settings->mWidth + x]);
        }
    }
    free(path_vertices);
}

void ray_trace(const Scene *scene, Sampler *s, Color3 *image_out, const RenderSettings * settings) {
    unsigned x, y, i, j, k, l;
    float u, v;
    float total = (settings->mROI.mYBottom - settings->mROI.mYTop) * settings->mAASamples;
    Ray world_ray;
    SurfacePoint intersect_point;
    for (i = 0, j = 0; i < settings->mAASamples; ++i) {
        for (y = settings->mROI.mYTop; y <= settings->mROI.mYBottom; ++y, ++j) {
            for (x = settings->mROI.mXLeft; x <= settings->mROI.mXRight; ++x) {
                u = urand(s);
                v = urand(s);
                calc_world_ray(scene, (x + u) / (float) (settings->mWidth),
                        (y + v) / (float) (settings->mHeight), &world_ray);
#ifdef USE_KDTREE
                if (kdtree_intersect((Scene*)scene, scene->mTree, &world_ray, &intersect_point)) {
#else
                if (calc_nearest_intersection((Scene*) scene, &world_ray, &intersect_point)) {
#endif
                    //image_out[y * settings->mWidth + x] = intersect_point.object->mat.mDiffuse;
                    calc_direct_illumination(&intersect_point, scene, s, &image_out[y * settings->mWidth + x]);
                }
            }
#ifndef USE_MPI
            int complete = (int) (j / total * 100) + 1;
            printf("[\033[01m");
            for (l = 0; l < complete / 2; l++) printf("=");
            printf(">");
            for (k = l; k < 50; k++) printf(" ");
            printf("\033[0m] %d%\r", complete);
            fflush(stdout);
#endif
        }
    }
#ifndef USE_MPI
    printf("\n\n");
#endif
    for (y = settings->mROI.mYTop; y < settings->mROI.mYBottom; ++y) {
        for (x = settings->mROI.mXLeft; x < settings->mROI.mXRight; ++x) {
            vec3_scalar_mult(&image_out[y * settings->mWidth + x],
                    1 / (float) settings->mAASamples, &image_out[y * settings->mWidth + x]);
        }
    }
}
