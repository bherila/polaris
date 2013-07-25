/* 
 * File:   matrix.h
 * Author: psastras
 *
 * Created on April 26, 2010, 4:32 PM
 */

#ifndef _MATRIX_H
#define	_MATRIX_H

#include "vector.h"


#ifdef	__cplusplus
extern "C" {
#endif
    typedef struct matrix4 {
        union {
            struct{
                float a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
            };
            struct {
                Vector4 r0, r1, r2, r3;
            };
            float data[16];
        };
    } Matrix4;

    inline Matrix4 create_identity_mat4();
    inline Matrix4 create_translation_mat4(float x, float y, float z);
    inline Matrix4 create_scale_mat4(float s_1, float s_2, float s_3);
    inline void mat4_mat4_mult(const Matrix4 *lhs, const Matrix4 *rhs, Matrix4 *out);
    inline void mat4_inv(const Matrix4 *in, Matrix4 *out);
    inline void mat4_vec4_mult(const Matrix4 *lhs, const Vector4 *rhs, Vector4 *out);
    inline float mat4_determinant(const Matrix4 *m);
    inline void mat4_printf(const Matrix4 *m);
    inline Matrix4 create_rotx_mat4(const float radians);
    inline Matrix4 create_roty_mat4(const float radians);
#ifdef	__cplusplus
}
#endif

#endif	/* _MATRIX_H */

