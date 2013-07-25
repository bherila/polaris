/******************************************************************************

  This source code accompanies the Journal of Graphics Tools paper:

  "Fast Ray / Axis-Aligned Bounding Box Overlap Tests using Ray Slopes"
  by Martin Eisemann, Thorsten Grosch, Stefan Müller and Marcus Magnor
  Computer Graphics Lab, TU Braunschweig, Germany and
  University of Koblenz-Landau, Germany

  This source code is public domain, but please mention us if you use it.

******************************************************************************/
#include "ray.h"
#include "rayslope.h"


void ray_to_sray(ray *r, sray *out) {
    make_ray(r->origin.x, r->origin.y, r->origin.z, r->direction.x, r->direction.y, r->direction.z, out);
}



void make_ray(float x, float y, float z, float i, float j, float k, sray *r)
{
        //common variables
        r->x = x;
        r->y = y;
        r->z = z;
        r->i = i;
        r->j = j;
        r->k = k;

        r->ii = 1.0f/i;
        r->ij = 1.0f/j;
        r->ik = 1.0f/k;

        //ray slope
        r->ibyj = r->i * r->ij;
        r->jbyi = r->j * r->ii;
        r->jbyk = r->j * r->ik;
        r->kbyj = r->k * r->ij;
        r->ibyk = r->i * r->ik;
        r->kbyi = r->k * r->ii;
        r->c_xy = r->y - r->jbyi * r->x;
        r->c_xz = r->z - r->kbyi * r->x;
        r->c_yx = r->x - r->ibyj * r->y;
        r->c_yz = r->z - r->kbyj * r->y;
        r->c_zx = r->x - r->ibyk * r->z;
        r->c_zy = r->y - r->jbyk * r->z;

        //ray slope classification
        if(i < 0)
        {
                if(j < 0)
                {
                        if(k < 0)
                        {
                                r->classification = MMM;
                        }
                        else if(k > 0){
                                r->classification = MMP;
                        }
                        else//(k >= 0)
                        {
                                r->classification = MMO;
                        }
                }
                else//(j >= 0)
                {
                        if(k < 0)
                        {
                                r->classification = MPM;
                                if(j==0)
                                        r->classification = MOM;
                        }
                        else//(k >= 0)
                        {
                                if((j==0) && (k==0))
                                        r->classification = MOO;
                                else if(k==0)
                                        r->classification = MPO;
                                else if(j==0)
                                        r->classification = MOP;
                                else
                                        r->classification = MPP;
                        }
                }
        }
        else//(i >= 0)
        {
                if(j < 0)
                {
                        if(k < 0)
                        {
                                r->classification = PMM;
                                if(i==0)
                                        r->classification = OMM;
                        }
                        else//(k >= 0)
                        {
                                if((i==0) && (k==0))
                                        r->classification = OMO;
                                else if(k==0)
                                        r->classification = PMO;
                                else if(i==0)
                                        r->classification = OMP;
                                else
                                        r->classification = PMP;
                        }
                }
                else//(j >= 0)
                {
                        if(k < 0)
                        {
                                if((i==0) && (j==0))
                                        r->classification = OOM;
                                else if(i==0)
                                        r->classification = OPM;
                                else if(j==0)
                                        r->classification = POM;
                                else
                                        r->classification = PPM;
                        }
                        else//(k > 0)
                        {
                                if(i==0)
                                {
                                        if(j==0)
                                                r->classification = OOP;
                                        else if(k==0)
                                                r->classification = OPO;
                                        else
                                                r->classification = OPP;
                                }
                                else
                                {
                                        if((j==0) && (k==0))
                                                r->classification = POO;
                                        else if(j==0)
                                                r->classification = POP;
                                        else if(k==0)
                                                r->classification = PPO;
                                        else
                                                r->classification = PPP;
                                }
                        }
                }
        }
}

int slope(sray *r, aabox *b){

    switch (r->classification)
    {
        case MMM:

                if ((r->x < b->x0) || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                return true;

        case MMP:

                if ((r->x < b->x0) || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                return true;

        case MPM:

                if ((r->x < b->x0) || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                return true;

        case MPP:

                if ((r->x < b->x0) || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                return true;

        case PMM:

                if ((r->x > b->x1) || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                return true;

        case PMP:

                if ((r->x > b->x1) || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                return true;

        case PPM:

                if ((r->x > b->x1) || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                return true;

        case PPP:

                if ((r->x > b->x1) || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                return true;

        case OMM:

                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        )
                        return false;

                return true;

        case OMP:

                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        )
                        return false;

                return true;

        case OPM:

                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        )
                        return false;

                return true;

        case OPP:

                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        )
                        return false;

                return true;

        case MOM:

                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x < b->x0) || (r->z < b->z0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                return true;

        case MOP:

                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x < b->x0) || (r->z > b->z1)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                return true;

        case POM:

                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x > b->x1) || (r->z < b->z0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                return true;

        case POP:

                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x > b->x1) || (r->z > b->z1)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                return true;

        case MMO:

                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x < b->x0) || (r->y < b->y0)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        )
                        return false;

                return true;

        case MPO:

                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x < b->x0) || (r->y > b->y1)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        )
                        return false;

                return true;

        case PMO:

                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x > b->x1) || (r->y < b->y0)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        )
                        return false;

                return true;

        case PPO:

                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x > b->x1) || (r->y > b->y1)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        )
                        return false;

                return true;

        case MOO:

                if((r->x < b->x0)
                        || (r->y < b->y0) || (r->y > b->y1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                return true;

        case POO:

                if((r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                return true;

        case OMO:

                if((r->y < b->y0)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

        case OPO:

                if((r->y > b->y1)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

        case OOM:

                if((r->z < b->z0)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        )
                        return false;

        case OOP:

                if((r->z > b->z1)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        )
                        return false;

                return true;

        }

        return false;
}

int slopeint_mul(sray *r, aabox *b, float *t){

        switch (r->classification)
        {
        case MMM:
                {
                if ((r->x < b->x0) || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                // compute the intersection distance

                        *t = (b->x1 - r->x) * r->ii;
                        float t1 = (b->y1 - r->y) * r->ij;
                        if(t1 > *t)
                                *t = t1;
                        float t2 = (b->z1 - r->z) * r->ik;
                        if(t2 > *t)
                                *t = t2;

                        return true;
                }


        case MMP:
                {
                if ((r->x < b->x0) || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) * r->ii;
                        float t1 = (b->y1 - r->y) * r->ij;
                        if(t1 > *t)
                                *t = t1;
                        float t2 = (b->z0 - r->z) * r->ik;
                        if(t2 > *t)
                                *t = t2;

                        return true;
                }

        case MPM:
                {
                if ((r->x < b->x0) || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) * r->ii;
                float t1 = (b->y0 - r->y) * r->ij;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z1 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case MPP:
                {
                if ((r->x < b->x0) || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) * r->ii;
                        float t1 = (b->y0 - r->y) * r->ij;
                        if(t1 > *t)
                                *t = t1;
                        float t2 = (b->z0 - r->z) * r->ik;
                        if(t2 > *t)
                                *t = t2;

                        return true;
                }

        case PMM:
                {
                if ((r->x > b->x1) || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                        float t1 = (b->y1 - r->y) * r->ij;
                        if(t1 > *t)
                                *t = t1;
                        float t2 = (b->z1 - r->z) * r->ik;
                        if(t2 > *t)
                                *t = t2;

                        return true;
                }


        case PMP:
                {
                if ((r->x > b->x1) || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                        float t1 = (b->y1 - r->y) * r->ij;
                        if(t1 > *t)
                                *t = t1;
                        float t2 = (b->z0 - r->z) * r->ik;
                        if(t2 > *t)
                                *t = t2;

                        return true;
                }

        case PPM:
                {
                if ((r->x > b->x1) || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                        float t1 = (b->y0 - r->y) * r->ij;
                        if(t1 > *t)
                                *t = t1;
                        float t2 = (b->z1 - r->z) * r->ik;
                        if(t2 > *t)
                                *t = t2;

                        return true;
                }

        case PPP:
                {
                if ((r->x > b->x1) || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                        float t1 = (b->y0 - r->y) * r->ij;
                        if(t1 > *t)
                                *t = t1;
                        float t2 = (b->z0 - r->z) * r->ik;
                        if(t2 > *t)
                                *t = t2;

                        return true;
                }

        case OMM:
                {
                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        )
                        return false;

                *t = (b->y1 - r->y) * r->ij;
                float t2 = (b->z1 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case OMP:
                {
                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        )
                        return false;

                *t = (b->y1 - r->y) * r->ij;
                float t2 = (b->z0 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case OPM:
                {
                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        )
                        return false;

                *t = (b->y0 - r->y) * r->ij;
                float t2 = (b->z1 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case OPP:
                {
                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        )
                        return false;

                *t = (b->y0 - r->y) * r->ij;
                float t2 = (b->z0 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }


        case MOM:
                {
                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x < b->x0) || (r->z < b->z0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) * r->ii;
                float t2 = (b->z1 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }


        case MOP:
                {
                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x < b->x0) || (r->z > b->z1)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) * r->ii;
                float t2 = (b->z0 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case POM:
                {
                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x > b->x1) || (r->z < b->z0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                float t2 = (b->z1 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }


        case POP:
                {
                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x > b->x1) || (r->z > b->z1)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                float t2 = (b->z0 - r->z) * r->ik;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case MMO:
                {
                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x < b->x0) || (r->y < b->y0)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) * r->ii;
                float t1 = (b->y1 - r->y) * r->ij;
                if(t1 > *t)
                        *t = t1;

                return true;
                }

        case MPO:
                {
                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x < b->x0) || (r->y > b->y1)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) * r->ii;
                float t1 = (b->y0 - r->y) * r->ij;
                if(t1 > *t)
                        *t = t1;

                return true;
                }


        case PMO:
                {
                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x > b->x1) || (r->y < b->y0)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                float t1 = (b->y1 - r->y) * r->ij;
                if(t1 > *t)
                        *t = t1;

                return true;
                }

        case PPO:
                {
                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x > b->x1) || (r->y > b->y1)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                float t1 = (b->y0 - r->y) * r->ij;
                if(t1 > *t)
                        *t = t1;

                return true;
                }


        case MOO:
                {
                if((r->x < b->x0)
                        || (r->y < b->y0) || (r->y > b->y1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                *t = (b->x1 - r->x) * r->ii;
                return true;
                }

        case POO:
                {
                if((r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                *t = (b->x0 - r->x) * r->ii;
                return true;
                }

        case OMO:
                {
                if((r->y < b->y0)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                *t = (b->y1 - r->y) * r->ij;
                return true;
                }

        case OPO:
                {
                if((r->y > b->y1)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                *t = (b->y0 - r->y) * r->ij;
                return true;
                }


        case OOM:
                {
                if((r->z < b->z0)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        )
                        return false;

                *t = (b->z1 - r->z) * r->ik;
                return true;
                }

        case OOP:
                {
                if((r->z > b->z1)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        )
                        return false;

                *t = (b->z0 - r->z) * r->ik;
                return true;
                }
        }

        return false;
};

int slopeint_div(sray *r, aabox *b, float *t){

        switch (r->classification)
        {
        case MMM:
                {
                if ((r->x < b->x0) || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) / r->i;
                float t1 = (b->y1 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z1 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }


        case MMP:
                {
                if ((r->x < b->x0) || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;
                *t = (b->x1 - r->x) / r->i;
                float t1 = (b->y1 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z0 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }


        case MPM:
                {
                if ((r->x < b->x0) || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) / r->i;
                float t1 = (b->y0 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z1 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case MPP:
                {
                if ((r->x < b->x0) || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) / r->i;
                float t1 = (b->y0 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z0 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case PMM:
                {
                if ((r->x > b->x1) || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                float t1 = (b->y1 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z1 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case PMP:
                {
                if ((r->x > b->x1) || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                float t1 = (b->y1 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z0 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case PPM:
                {
                if ((r->x > b->x1) || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                float t1 = (b->y0 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z1 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case PPP:
                {
                if ((r->x > b->x1) || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                float t1 = (b->y0 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;
                float t2 = (b->z0 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case OMM:
                {
                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->z < b->z0)
                        || (r->jbyk * b->z0 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z1 + r->c_yz > 0)
                        )
                        return false;

                *t = (b->y1 - r->y) / r->j;
                float t2 = (b->z1 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case OMP:
                {
                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->z > b->z1)
                        || (r->jbyk * b->z1 - b->y1 + r->c_zy > 0)
                        || (r->kbyj * b->y0 - b->z0 + r->c_yz < 0)
                        )
                        return false;

                *t = (b->y1 - r->y) / r->j;
                float t2 = (b->z0 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case OPM:
                {
                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y > b->y1) || (r->z < b->z0)
                        || (r->jbyk * b->z0 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z1 + r->c_yz > 0)
                        )
                        return false;

                *t = (b->y0 - r->y) / r->j;
                float t2 = (b->z1 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case OPP:
                {
                if((r->x < b->x0) || (r->x > b->x1)
                        || (r->y > b->y1) || (r->z > b->z1)
                        || (r->jbyk * b->z1 - b->y0 + r->c_zy < 0)
                        || (r->kbyj * b->y1 - b->z0 + r->c_yz < 0)
                        )
                        return false;

                *t = (b->y0 - r->y) / r->j;
                float t2 = (b->z0 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case MOM:
                {
                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x < b->x0) || (r->z < b->z0)
                        || (r->kbyi * b->x0 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) / r->i;
                float t2 = (b->z1 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case MOP:
                {
                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x < b->x0) || (r->z > b->z1)
                        || (r->kbyi * b->x0 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x1 + r->c_zx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) / r->i;
                float t2 = (b->z0 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case POM:
                {
                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x > b->x1) || (r->z < b->z0)
                        || (r->kbyi * b->x1 - b->z1 + r->c_xz > 0)
                        || (r->ibyk * b->z0 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                float t2 = (b->z1 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case POP:
                {
                if((r->y < b->y0) || (r->y > b->y1)
                        || (r->x > b->x1) || (r->z > b->z1)
                        || (r->kbyi * b->x1 - b->z0 + r->c_xz < 0)
                        || (r->ibyk * b->z1 - b->x0 + r->c_zx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                float t2 = (b->z0 - r->z) / r->k;
                if(t2 > *t)
                        *t = t2;

                return true;
                }

        case MMO:
                {
                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x < b->x0) || (r->y < b->y0)
                        || (r->jbyi * b->x0 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x1 + r->c_yx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) / r->i;
                float t1 = (b->y1 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;

                return true;
                }

        case MPO:
                {
                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x < b->x0) || (r->y > b->y1)
                        || (r->jbyi * b->x0 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x1 + r->c_yx > 0)
                        )
                        return false;

                *t = (b->x1 - r->x) / r->i;
                float t1 = (b->y0 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;

                return true;
                }

        case PMO:
                {
                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x > b->x1) || (r->y < b->y0)
                        || (r->jbyi * b->x1 - b->y1 + r->c_xy > 0)
                        || (r->ibyj * b->y0 - b->x0 + r->c_yx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                float t1 = (b->y1 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;

                return true;
                }

        case PPO:
                {
                if((r->z < b->z0) || (r->z > b->z1)
                        || (r->x > b->x1) || (r->y > b->y1)
                        || (r->jbyi * b->x1 - b->y0 + r->c_xy < 0)
                        || (r->ibyj * b->y1 - b->x0 + r->c_yx < 0)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                float t1 = (b->y0 - r->y) / r->j;
                if(t1 > *t)
                        *t = t1;

                return true;
                }
        case MOO:
                {
                if((r->x < b->x0)
                        || (r->y < b->y0) || (r->y > b->y1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                *t = (b->x1 - r->x) / r->i;
                return true;
                }

        case POO:
                {
                if((r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                *t = (b->x0 - r->x) / r->i;
                return true;
                }

        case OMO:
                {
                if((r->y < b->y0)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                *t = (b->y1 - r->y) / r->j;
                return true;
                }

        case OPO:
                {
                if((r->y > b->y1)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->z < b->z0) || (r->z > b->z1)
                        )
                        return false;

                *t = (b->y0 - r->y) / r->j;
                return true;
        }


        case OOM:
                {
                if((r->z < b->z0)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        )
                        return false;

                *t = (b->z1 - r->z) / r->k;
                return true;
                }

        case OOP:
                {
                if((r->z > b->z1)
                        || (r->x < b->x0) || (r->x > b->x1)
                        || (r->y < b->y0) || (r->y > b->y1)
                        )
                        return false;

                *t = (b->z0 - r->z) / r->k;
                return true;
                }

        }

        return false;
};
