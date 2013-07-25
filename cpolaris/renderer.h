/* 
 * File:   renderer.h
 * Author: psastras
 *
 * Created on April 27, 2010, 1:03 PM
 */

#ifndef _RENDERER_H
#define	_RENDERER_H

#include "scene.h"
#include "sampler.h"


#ifdef	__cplusplus
extern "C" {
#endif

    //Structures

    typedef struct roi {
        union { struct { unsigned mXLeft, mXRight, mYTop, mYBottom;} ;
                struct { unsigned left, right, top, bottom; };
                unsigned data[4];
        };
    } ROI;

    typedef struct rendersettings {
        unsigned mAASamples, mThreads;
        unsigned mWidth, mHeight;
        ROI mROI;
    } RenderSettings;

    typedef struct pathvertex {
        SurfacePoint mIntersect;
        Vector4 mWout;
        float mProb;
        unsigned isSpecular; //is end of path

    } PathVertex;

    //Methods
    void ray_trace(const Scene *scene, Sampler *s, Color3 *image_out, const RenderSettings *settings);
    void path_trace(const Scene *scene, Sampler *s, Color3 *image_out, const RenderSettings *settings);
#ifdef	__cplusplus
}
#endif

#endif	/* _RENDERER_H */

