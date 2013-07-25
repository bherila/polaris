#ifndef RAYSLOPE_H
#define RAYSLOPE_H

#include "kdtree.h"
typedef aabb aabox;

enum CLASSIFICATION
{ MMM, MMP, MPM, MPP, PMM, PMP, PPM, PPP, POO, MOO, OPO, OMO, OOP, OOM,
        OMM,OMP,OPM,OPP,MOM,MOP,POM,POP,MMO,MPO,PMO,PPO};

typedef struct sray
{
        //common variables
        float x, y, z;		// ray origin
        float i, j, k;		// ray direction
        float ii, ij, ik;	// inverses of direction components

        // ray slope
        int classification;
        float ibyj, jbyi, kbyj, jbyk, ibyk, kbyi; //slope
        float c_xy, c_xz, c_yx, c_yz, c_zx, c_zy;
} sray;


void ray_to_sray(ray *r, sray *out);
void make_ray(float x, float y, float z, float i, float j, float k, sray *r);
int slope(sray *r, aabox *b);
int slopeint_mul(sray *r, aabox *b, float *t);
int slopeint_div(sray *r, aabox *b, float *t);

#endif // RAYSLOPE_H
