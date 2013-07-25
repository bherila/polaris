/* 
 * File:   sampler.h
 * Author: psastras
 *
 * Created on April 27, 2010, 12:51 PM
 */

#ifndef _SAMPLER_H
#define	_SAMPLER_H

#include "mtwist.h"
#include "vector.h"

#ifdef	__cplusplus
extern "C" {
#endif

    //Structures

    typedef struct sampler {
        mt_state mState;
    } Sampler;

    //Constructors & destructors

    Sampler *createSampler();
    void releaseSampler(Sampler *s);
    
    //Methods

    inline float urand(Sampler *s);
    inline void urand_sphere(Sampler *s, float r, Vector4 *out);
    inline void urand_hemisphere(Sampler *s, Vector4 *out);
    inline void cosrand_sphere(Sampler *s, Vector4 *out);
    inline void cosrand_hemisphere(Sampler *s, Vector4 *out);

#ifdef	__cplusplus
}
#endif

#endif	/* _SAMPLER_H */

