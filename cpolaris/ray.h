/* 
 * File:   ray.h
 * Author: psastras
 *
 * Created on April 26, 2010, 4:36 PM
 */

#ifndef _RAY_H
#define	_RAY_H

#include "vector.h"

#ifdef	__cplusplus
extern "C" {
#endif

    //Structures

    typedef struct ray{
        Vector4 origin, direction;
    }Ray;

    //Constructors & Destructors

    //Methods

#ifdef	__cplusplus
}
#endif

#endif	/* _RAY_H */

