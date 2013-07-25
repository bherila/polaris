/* 
 * File:   scenes.cuh
 * Author: psastras
 *
 * Created on May 12, 2010, 1:33 AM
 */

#ifndef _SCENES_CUH
#define	_SCENES_CUH

#ifdef	__cplusplus
extern "C" {
#endif

#include "defs.cuh"
#include <stdlib.h>

inline  Scene * generate_cornell_box(RenderSettings settings) {

    float4 position = {0, 0, .8, 1}, look = {0, 0, -1, 0},
    up = {0, 1, 0, 0};
    Camera *scene_camera = createCamera(&position, &look, &up, 45.0f * PI / 180.0f,
            (float) settings.width / (float) settings.height, 0.1f, 100.0f);

    Material m;
    m.ks = 0.0f;
    vec3_set(&m.mDiffuse, 5.0f, 5.0f, 5.0f);
    vec3_set(&m.mAttenuation, 0.4f, 0.2f, 0.1f);
    const unsigned numObjects = 9;
    SceneObject *scene_obects = (SceneObject*) malloc(sizeof (SceneObject) * numObjects);
    vec3_set(&m.mEmitterpower, 10.0f, 10.0f, 10.0f);
    scene_obects[0] = createSceneObject(Cube, create_translation_mat4(0, .25f, 0.0f), //Emitter
            create_identity_mat4(), create_scale_mat4(.1f, .1f, .1f), m);
    vec3_set(&m.mEmitterpower, 0.0f, 0.0f, 0.0f);
    vec3_set(&m.mAttenuation, 0.0f, 0.0f, 0.0f);
    vec3_set(&m.mDiffuse, 0.5f, 1.0f, 0.5f);
    scene_obects[1] = createSceneObject(Cube, create_translation_mat4(.5f, 0, 0.0f),
            create_identity_mat4(), create_scale_mat4(.5f, .5f, .5f), m);
    vec3_set(&m.mDiffuse, 1.0f, 0.5f, 0.5f);
    scene_obects[2] = createSceneObject(Cube, create_translation_mat4(-.5f, 0.0f, 0.0f),
            create_identity_mat4(), create_scale_mat4(.5f, .5f, .5f), m);
    vec3_set(&m.mDiffuse, 0.7f, 0.7f, 0.7f);
    scene_obects[3] = createSceneObject(Cube, create_translation_mat4(0.0f, .5f, 0.0f),
            create_identity_mat4(), create_scale_mat4(.5f, .5f, .5f), m);
    vec3_set(&m.mDiffuse, 0.35f, 0.35f, 0.35f);
    scene_obects[4] = createSceneObject(Cube, create_translation_mat4(0.0f, -.5f, 0.0f),
            create_identity_mat4(), create_scale_mat4(.5f, .5f, .5f), m);
    vec3_set(&m.mDiffuse, 0.5f, 0.5f, 0.5f);
    scene_obects[5] = createSceneObject(Cube, create_translation_mat4(0.0f, 0.0f, -.5f),
            create_identity_mat4(), create_scale_mat4(.5f, .5f, .5f), m);
    vec3_set(&m.mDiffuse, 0.5f, 0.5f, 0.5f);
    scene_obects[6] = createSceneObject(Cube, create_translation_mat4(0.0f, 0.0f, 1.0f),
            create_identity_mat4(), create_scale_mat4(5.0f, 5.0f, .10f), m);
    vec3_set(&m.mDiffuse, 0.0f, 0.0f, 1.0f);
    scene_obects[7] = createSceneObject(Cube, create_translation_mat4(-.10f, -.19f, -.10f),
            create_identity_mat4(), create_scale_mat4(.12f, .12f, .12f), m);
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 0.0f);
    scene_obects[8] = createSceneObject(Cube, create_translation_mat4(.11f, -.15f, 0.03f),
            create_identity_mat4(), create_scale_mat4(.10f, .20f, .10f), m);
    Scene *scene = createScene(scene_obects, numObjects, scene_camera);
    return scene;
}

inline Scene * generate_diffuse_cube(RenderSettings settings) {

    float4 position = {0, 0.275, -0.5, 1}, look = {0, 0, -1, 0},
    up = {0, 1, 0, 0};
    Camera *scene_camera = createCamera(&position, &look, &up, 45.0f * PI / 180.0f,
            (float) settings.width / (float) settings.height, 0.1f, 100.0f);


    Material m;
    m.ks = 0.0f;
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 1.0f);
    vec3_set(&m.mAttenuation, 0.45, 0.00002, 0.00001);
    const unsigned numObjects = 9;
    SceneObject *scene_obects = (SceneObject*) malloc(sizeof (SceneObject) * numObjects);
    vec3_set(&m.mEmitterpower, 5.5f, 5.5f,5.5f);
    scene_obects[0] = createSceneObject(Sphere, create_translation_mat4(0, 1, -1.0f), //Emitter
            create_identity_mat4(), create_scale_mat4(.15f, .15f, .15f), m);

    vec3_set(&m.mEmitterpower, 0.0f, 0.0f, 0.0f);
    vec3_set(&m.mAttenuation, 0.0f, 0.0f, 0.0f);
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 1.0f);
    m.ks = 0.1f;
    scene_obects[1] = createSceneObject(Cube, create_translation_mat4(-0.3, .05, -1.7f),
            create_identity_mat4(), create_scale_mat4(.35f, .35f, .35f), m);
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 1.0f);
    scene_obects[2] = createSceneObject(Cube, create_translation_mat4(0.3, .2, -1.8f),
            create_roty_mat4(PI * .05f), create_scale_mat4(.35f, .65f, .35f), m);
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 1.0f);
    m.ks = 0.0f;
    scene_obects[3] = createSceneObject(Cube, create_translation_mat4(0.0, -.125f, -3.0f), //floor
            create_identity_mat4(), create_scale_mat4(5.0f, 0.01f, 10.0f), m);
    scene_obects[4] = createSceneObject(Cube, create_translation_mat4(0.0, 0.0, -2.5f), //front wall
            create_identity_mat4(), create_scale_mat4(20.0f, 20.0f, .01f), m);
    scene_obects[5] = createSceneObject(Cube, create_translation_mat4(0.0, 2.0, -3.0f), //ceeling
            create_identity_mat4(), create_scale_mat4(5.0f, 0.01f, 10.0f), m);

    scene_obects[6] = createSceneObject(Cube, create_translation_mat4(0.0, 0.0, 0.0f), //back wall
            create_identity_mat4(), create_scale_mat4(20.0f, 20.0f, .01f), m);
    vec3_set(&m.mDiffuse, 1.0f, 0.0f, 0.0f);
    scene_obects[7] = createSceneObject(Cube, create_translation_mat4(-.8f, 0.0, 0.0f), //left wall
            create_identity_mat4(), create_scale_mat4(0.01f, 20.0f, 20.0f), m);
    vec3_set(&m.mDiffuse, 0.25f, 0.75f, 1.0f);
    scene_obects[8] = createSceneObject(Cube, create_translation_mat4(0.8f, 0.0, 0.0f), //right wall
            create_identity_mat4(), create_scale_mat4(0.01f, 20.0f, 20.0f), m);
    Scene *scene = createScene(scene_obects, numObjects, scene_camera);
    return scene;
}


inline Scene * generate_diffuse_spheres(RenderSettings settings) {

    float4 position = {0, 0.185, -0.5, 1}, look = {0, 0, -1, 0},
    up = {0, 1, 0, 0};
    Camera *scene_camera = createCamera(&position, &look, &up, 45.0f * PI / 180.0f,
            (float) settings.width / (float) settings.height, 0.1f, 100.0f);


    Material m;
    m.ks = 0.0f;
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 1.0f);
    vec3_set(&m.mAttenuation, 0.45, 0.00002, 0.00001);
    const unsigned numObjects = 12;
    SceneObject *scene_obects = (SceneObject*) malloc(sizeof (SceneObject) * numObjects);
    vec3_set(&m.mEmitterpower, 3.0f, 3.0f, 3.0f);
    scene_obects[0] = createSceneObject(Sphere, create_translation_mat4(0, 0, -1.5f), //Emitter
            create_identity_mat4(), create_scale_mat4(.25f, .25f, .25f), m);
    /*
        vec3_set(&m.mEmitterpower, 1.0f, 1.0f, 1.0f);
        scene_obects[0] = createSceneObject(Cube, create_translation_mat4(0, .05, -1.5f), //Emitter
                create_identity_mat4(), create_scale_mat4(.25f, .25f, .25f), m);
     */
    vec3_set(&m.mEmitterpower, 0.0f, 0.0f, 0.0f);
    vec3_set(&m.mAttenuation, 0.0f, 0.0f, 0.0f);
    vec3_set(&m.mDiffuse, 0.0f, 0.0f, 1.0f);

    scene_obects[1] = createSceneObject(Sphere, create_translation_mat4(-0.3, 0, -1.5f),
            create_identity_mat4(), create_scale_mat4(.25f, .25f, .25f), m);
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 0.0f);
    scene_obects[2] = createSceneObject(Sphere, create_translation_mat4(0.15, .2, -1.5f),
            create_identity_mat4(), create_scale_mat4(.25f, .25f, .25f), m);
    vec3_set(&m.mDiffuse, 0.0f, 1.0f, 0.0f);
    scene_obects[3] = createSceneObject(Sphere, create_translation_mat4(0.3, 0, -1.5f),
            create_identity_mat4(), create_scale_mat4(.25f, .25f, .25f), m);
    vec3_set(&m.mDiffuse, 1.0f, 0.0f, 0.0f);
    scene_obects[4] = createSceneObject(Sphere, create_translation_mat4(-.15f, .2, -1.5f),
            create_identity_mat4(), create_scale_mat4(.25f, .25f, .25f), m);
    vec3_set(&m.mDiffuse, 1.0f, 0.0f, 1.0f);

    scene_obects[5] = createSceneObject(Sphere, create_translation_mat4(0.0, .4, -1.5f),
            create_identity_mat4(), create_scale_mat4(.25f, .25f, .25f), m);

    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 1.0f);

    scene_obects[6] = createSceneObject(Cube, create_translation_mat4(0.0, -.125f, -3.0f), //floor
            create_identity_mat4(), create_scale_mat4(5.0f, 0.01f, 10.0f), m);
    scene_obects[7] = createSceneObject(Cube, create_translation_mat4(0.0, 0.0, -2.5f), //front wall
            create_identity_mat4(), create_scale_mat4(20.0f, 20.0f, .01f), m);
    scene_obects[8] = createSceneObject(Cube, create_translation_mat4(0.0, 2.0, -3.0f), //ceeling
            create_identity_mat4(), create_scale_mat4(5.0f, 0.01f, 10.0f), m);

    scene_obects[9] = createSceneObject(Cube, create_translation_mat4(0.0, 0.0, 0.0f), //back wall
            create_identity_mat4(), create_scale_mat4(20.0f, 20.0f, .01f), m);
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 1.0f);
    scene_obects[10] = createSceneObject(Cube, create_translation_mat4(-3.0f, 0.0, 0.0f), //left wall
            create_identity_mat4(), create_scale_mat4(0.01f, 20.0f, 20.0f), m);
    vec3_set(&m.mDiffuse, 1.0f, 1.0f, 1.0f);
    scene_obects[11] = createSceneObject(Cube, create_translation_mat4(3.0f, 0.0, 0.0f), //right wall
            create_identity_mat4(), create_scale_mat4(0.01f, 20.0f, 20.0f), m);
    Scene *scene = createScene(scene_obects, numObjects, scene_camera);
    return scene;
}




#ifdef	__cplusplus
}
#endif

#endif	/* _SCENES_CUH */

