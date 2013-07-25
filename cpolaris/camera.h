/* 
 * File:   camera.h
 * Author: psastras
 *
 * Created on April 26, 2010, 5:26 PM
 */

#ifndef _CAMERA_H
#define	_CAMERA_H

#include "matrix.h"

#ifdef	__cplusplus
extern "C" {
#endif

    //Structures

    typedef struct camera {
        Matrix4 *mFilmToWorld, *mWorldToFilm;
        Vector4 *mPosition;
    } Camera;

    //Constructors & Destructors

    Camera *createCamera(const Vector4 *position, const Vector4 *look,
        const Vector4 *up, float heightangle, float aspect,
        float near, float far);

    void releaseCamera(Camera *camera);

    //Methods

    void updateCamera(const Vector4 *position, const Vector4 *look,
        const Vector4 *up, float heightangle, float aspect,
        float near, float far);

#ifdef	__cplusplus
}
#endif

#endif	/* _CAMERA_H */

