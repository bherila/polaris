#include "stdafx.h"
#include "material.h"
#include "scene.h"
#include "pmath.h"
#include "renderthread.h"

bool LambertianMaterial::reflectRay(const SurfacePoint *sp, RenderThread *thread, Ray *outRay, real_t *weight) {
    Sampler *ss = &thread->sampler;
    Vector4 normal = sp->normal; // inRay.hit->GetNormal(inRay);
    // turn normal to front
    if (dot(normal, sp->incoming.direction) > 0)
       normal = -normal;
    // init random numbers
    real_t fi = ss->urand(), psi = ss->urand();
    const Vector4& wo =  Vector4::cosRandom(normal, fi, psi);
    outRay->reset(sp->intersection, wo);
    *weight = 1; //kd.max();
    // not specular
    return false;
}

void PhongMaterial::f(const SurfacePoint *sp, const Vector4 *outDir, Color3 &color) const {
      if (lastType)
      {
             color += 1.;
      }
      else
      {
             color += kd;
      }
}

bool PhongMaterial::reflectRay(const SurfacePoint *sp, RenderThread *thread, Ray *outRay, real_t *weight) {
    // set reflection as diffuse
    lastType = false;

    // get shading normal
    Vector4 normal = sp->normal; //inRay.hit->GetNormal(inRay);

    // turn normal to front
    if (dot(normal, sp->incoming.direction) > 0)
        normal *= -1;

     // initiate outgoing ray
//     outRay.hit = NULL;
//     outRay.t = Infinity;
//     outRay.u = outRay.v = 0.0;
//     outRay.org = inRay.org + inRay.t * inRay.dir;
     const Vector4 outPt = sp->intersection;

     real_t val, fi, psi;

     val = thread->sampler.urand(); //frand();
     fi  = thread->sampler.urand(); // frand();
     psi = thread->sampler.urand(); // frand();

     // this ray is diffuse reflection
     if (val < kd)
     {
            // random vector sampled with pdf(x) = 1/PI * cos(x)
            // no need to return weigh with of cos, already included in sampling techniques
            //ss.ImportanceSampleHemisphere(outRay.dir, weight, normal);

            const Vector4 &outDir = Vector4::cosRandom(normal, fi, psi);
            outRay->reset(outPt, outDir);
            //*weight *= dot(sp->normal, outDir) * kd;
            *weight = 1;
     }
     // this is specular reflection
     else if (val < kd + ks)
     {
            // random vector sampled with pdf(x) = cos (alpha) (n + 1) / (2 * PI), where alpha is angle from reflected ray to ideal mirror reflection
            Vector4 outDir;
            outDir[0] = sqrt(1-pow(fi, 2/(spec_n+1)))*cos(2*M_PI*psi);
            outDir[1] = sqrt(1-pow(fi, 2/(spec_n+1)))*sin(2*M_PI*psi);
            outDir[2] = pow(fi, 2/(spec_n+1));
            outDir[3] = 0;

            // compute reflection vector
            Vector4 reflect = sp->incoming.direction - 2*dot(normal,sp->incoming.direction)*normal;

            //outRay.dir = RotateByAngle(Vector3(0,0,1), reflect, outRay.dir);
            outDir.reflect(sp->normal);
            *weight =  ks;

            // reflection is specular
            lastType = true;

            outRay->reset(outPt, outDir);
     }
     else
     {
            // nothing reflected, contribution is 0
            *weight = 0;
            *weight = 1 - ks - kd;
     }

     return lastType;
}
