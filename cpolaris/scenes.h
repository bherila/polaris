/* 
 * File:   scenes.h
 * Author: psastras
 *
 * Created on April 29, 2010, 4:39 PM
 */

#ifndef _SCENES_H
#define	_SCENES_H
#include "renderer.h"

#ifdef	__cplusplus
extern "C" {
#endif

    Scene * generate_diffuse_spheres(RenderSettings settings, float emitter_z);
    Scene * generate_9_spheres(RenderSettings settings);
    Scene * generate_cornell_box(RenderSettings settings);
    Scene * generate_diffuse_cube(RenderSettings settings);
    Scene * generate_recursive_spheres(RenderSettings settings, int n);


#ifdef	__cplusplus
}
#endif

#endif	/* _SCENES_H */

