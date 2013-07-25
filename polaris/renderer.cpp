#include "stdafx.h"
#include "renderthread.h"
#include "renderer.h"
#include "sceneobject.h"
#include <iostream>
#include <stdio.h>
#include "stats.h"
#include <assert.h>
using namespace std;

ROI setup_ROI(const ROI *region_of_interest, const RenderSettings *m_settings) {
    ROI roi;
    if (region_of_interest == NULL) {
        roi.top = roi.left = 0;
        roi.right = m_settings->width - 1;
        roi.bottom = m_settings->height - 1;
    }
    else roi = *region_of_interest;
    return roi;
}


//        if (mat->reflectRay(sp, /*ref*/ m_sampler, /*ref*/ ray)) { // modifies ray
//        if(m_scene->getNearestObject(r, pt)) { // try intersecting the reflected ray
//            calcDirectIllumination(pt, m_sampler, image[i]);
//            image[i] /= (real_t)m_settings->aasamples;
//        }
//        }


//            c += (emitter->material->emitter->power);
//            c *= (cosWoWi) * attenuation;
//            c *= sp.object->material->kd;
//
//            if (sp.object->isSpecular() && !sp.object->isEmitter()) {
//                // (R dot V)^n    where n is the Phong "specular exponent"
//                // TODO: might want to randomly sample again?
//                //       or maybe we don't have to since the rays are already randomly sampled
//                SurfacePoint other_pt;
//                Vector4 lightReflected = sp.incoming.direction;
//                lightReflected.normalize();
//                lightReflected.reflect(sp.normal);
//                float spec = lightReflected.dot(ray.direction);
//                spec = (spec > 0 ? pow(spec, sp.object->material->shine) : 0);
//                c += sp.object->material->ks * spec;
//            }
//        }

void Renderer::calcDirectIllumination(const SurfacePoint &sp, RenderThread *thread, Color3 &c, int level){
    if (level > 3) return;
    const Material *mat = sp.object->material;
    if (mat->emitter.isEmissive) {
        c += mat->emitter.power;
        return;
    }
    const std::vector<SceneObject *> &emitters = m_scene->getEmitters();
    SurfacePoint lightPt;
    for(unsigned i = 0; i < emitters.size(); ++i) { //Get contribution from each emitter
        SceneObject *emitter =  emitters[i];
        const Matrix4 &ctmI = emitter->ctmI;
        const Vector4& samplept = emitter->getSamplePoint(thread->sampler);
        Vector4 wo = ctmI * samplept; //This is really the sampled light position in obj space
        real_t t = (wo-= sp.intersection).normalize(); //Ray from intersect to light source
        // now wo is the incoming light vector normalized

        Ray ray(sp.intersection + (EPSILON * wo), wo);
        if(m_scene->getNearestObject(ray, lightPt, thread) && lightPt.object == emitter) { //If the nearest object is the light source

            real_t cosWoWi = sp.normal.dot(wo);
            if(cosWoWi < 0 || (cosWoWi *= (lightPt.normal.dot(-wo))) < 0) continue;
            real_t attenuation = lightPt.object->surface_area/(emitter->material->emitter.attenuation[0] +
                                 emitter->material->emitter.attenuation[1] * t +
                                 emitter->material->emitter.attenuation[2] * t * t); //1 / (t * t);*/
            //real_t attenuation = lightPt.object->surface_area/(t * t);

            Color3 directLight = (emitter->material->emitter.power  * (cosWoWi * attenuation) * mat->getColor());

            // specular
            // kSpec = (r dot v)^n
            // Where R is the mirror reflection of the light vector off the surface, and
            // V is the viewpoint vector. The number n is called the Phong exponent
            wo.reflect(sp.normal); // now wo is the reflection (i.e. R)
            real_t nDotL = dot(wo, sp.incoming.direction);
            if (nDotL > 0) {
                real_t kSpec = pow(nDotL, 0.5);
                ray.reset(ray.origin, wo);
                if (m_scene->getNearestObject(ray, lightPt, thread)) {
                    Color3 cRefl;
                    calcDirectIllumination(lightPt, thread, cRefl, level+1);
                    directLight += cRefl * 0.5 * kSpec;
                }
            }
            // end spec

            c += directLight;


        }
    }
}

void RayTracer::render(Color3 *image, RenderThread *thread, const ROI *region_of_interest){
    const ROI roi = setup_ROI(region_of_interest, m_settings);
    SurfacePoint pt;
    for(unsigned y = roi.top; y <= roi.bottom; ++y) {
        unsigned i = y*m_settings->width+roi.left;
        image[i] = Color3(0,0,0);
        for(unsigned x = roi.left; x <= roi.right; ++x, ++i) {
            int cc = 0;
            for(unsigned j = 0; j < m_settings->aasamples; ++j) {
                Ray r = m_scene->getWorldRay((x + thread->sampler.urand()) / (real_t)m_settings->width,
                                             (y + thread->sampler.urand()) / (real_t)m_settings->height);
                if (!m_scene->getNearestObject(r, pt, thread))
                    continue;

                Color3 directLight;
                calcDirectIllumination(pt, thread, directLight);
                ++cc;

                image[i] += directLight;

//                // suppose this is specular:
//                Vector4 refl = pt.incoming.direction;
//                refl.normalize();
//                refl.reflect(pt.normal);
//                refl.normalize();
//
//                real_t nDotL = dot(-pt.incoming.direction, refl)+0.5;
//                if (nDotL > 0) {
//                    Ray specRay(pt.intersection, refl);
//                    if (m_scene->getNearestObject(specRay, pt, thread)) {
//                        Color3 spec;
//                        calcDirectIllumination(pt, thread, spec);
//                        spec *= pow(nDotL, 0.5 /* material shininess */);
//                        spec *= 0.1; // global specular coeff
//
//                        image[i] += spec;
//                    }
//                }
//                // end specular

            }
            image[i] /= (real_t)cc;
        }

        cout << (int)(y / real_t(m_settings->height) * 100) << "%\r";
    }
}


Color3 PathTracer::tracePath(Ray &r, unsigned depth, RenderThread *thread) {
    if(depth == mMaxDepth) return Color3(0, 0, 0);
    SurfacePoint sp;
    if(!m_scene->getNearestObject(r, sp, thread)) return Color3(0, 0, 0);
    Color3 c;
    if (depth == 0 && sp.object->material->emitter.isEmissive) c += sp.object->material->emitter.power;
    real_t pAbsorb = 0.01; //Absorb probability  - noise vs speed-> big means speed...,lower this when doing a real render
    if(thread->sampler.urand() < pAbsorb) return c;
    calcDirectIllumination(sp, thread, c);
    Ray outRay;
    real_t pCont = 1;
    sp.object->material->reflectRay(&sp, thread, &outRay, &pCont);
    if(pCont == 0) return c;
    //if(thread->sampler.urand() > pCont) return c;
    else {
        Ray rr(sp.intersection + EPSILON * outRay.direction, outRay.direction);
        Color3 kd;
        sp.object->material->f(&sp, &outRay.direction/*TODO*/, kd);
        real_t coeff = (sp.normal.dot(rr.direction)) / (pCont) / (1 - pAbsorb);
        c += ((tracePath(rr, depth+1, thread) *= coeff)) *= kd;
        return c;
    }
}

void PathTracer::render(Color3 *image, RenderThread *thread, const ROI *region_of_interest) {
    Sampler m_sampler;
    const ROI roi = setup_ROI(region_of_interest, m_settings);
    SurfacePoint pt;
    for(unsigned y = roi.top; y <= roi.bottom; ++y) {
        for(unsigned x = roi.left; x <= roi.right; ++x) {
            Color3 c;
            for(unsigned j = 0; j < m_settings->aasamples; ++j) {
                Ray r = m_scene->getWorldRay((x + m_sampler.urand()) / (real_t)m_settings->width,
                                             (y + m_sampler.urand()) / (real_t)m_settings->height);

                if(m_scene->getNearestObject(r, pt, thread) )
                    c += tracePath(r, 0, thread);
            }
            image[y*m_settings->width+x] = (c /= (real_t)m_settings->aasamples);
        }
        cout << (int)(y / real_t(m_settings->height) * 100) << "%\n";
    }
}

int BiDiPathTracer::generatePath(Ray &r, PathVertex *vertices, int maxVertices, RenderThread *thread) {
    Sampler &sampler = thread->sampler;
    int num_vertices = 0;
    real_t pAbsorb = 0.01;
    while(num_vertices < maxVertices) {
        PathVertex &v = vertices[num_vertices];
        SurfacePoint &sp = vertices[num_vertices].pt;
        if(!m_scene->getNearestObject(r, sp, thread)) break;
        v.rw = 1.0 / (1 - pAbsorb);
        sp.object->material->reflectRay(&sp, thread, &r, &v.pCont);
        if (v.pCont == 0) break; // no contribution
        v.wo = r.direction;
        if(num_vertices > 11 && pAbsorb > 0 && sampler.urand() <  pAbsorb)  break;
        if(v.pCont < 1 && thread->sampler.urand() > v.pCont) break;
        ++num_vertices;
    }
    /*for(int i = 0; i < num_vertices - 1; ++i) {
        vertices[i].tw = vertices[i].pCont * fabs((vertices[i].wo).dot(vertices[i+1].pt.normal)) /
                         vertices[i].pt.intersection.G(vertices[i+1].pt.intersection);
    }*/
    return num_vertices;
}


Color3 BiDiPathTracer::Li(Ray &eyeRay, Ray &lightRay, real_t lightpdf, real_t lightweight, RenderThread *thread) {
    Color3 L(0,0,0);
    //PathVertex eyePath[mMaxDepth], lightPath[mMaxDepth];
    PathVertex *eyePath = new PathVertex[mMaxDepth];
    PathVertex *lightPath = new PathVertex[mMaxDepth];
    int nEye = generatePath(eyeRay, eyePath, mMaxDepth, thread);
    real_t Le = lightweight / lightpdf;
    int nLight = generatePath(lightRay, lightPath, mMaxDepth, thread);
    Color3 directWt(1.0, 1.0, 1.0);
    for(int i = 1; i <= nEye; ++i) {
        real_t t = (eyePath[i-1].wo.dot(eyePath[i-1].pt.normal));
        if(t < 0) continue;
        directWt /= eyePath[i - 1].rw;
        Color3 c =  eyePath[i-1].pt.object->material->getColor(), c1, c2;
        calcDirectIllumination(eyePath[i-1].pt,thread, c1);
        L += directWt * c *= c1 /= weightPath(eyePath, i, lightPath, 0);
        eyePath[i-1].pt.object->material->f(&eyePath[i-1].pt, &eyePath[i-1].wo,c2) ;
        directWt *= c2 *= c * t  / eyePath[i-1].pCont;
        for(int j = 1; j  <= nLight; ++j)  L +=  evalPath(eyePath, i, lightPath, j, thread) / weightPath(eyePath, i , lightPath, j) * Le;
    }
    delete[] eyePath;
    delete[] lightPath;
    return L;
}

Color3 BiDiPathTracer::evalPath(PathVertex *eye, int nEye, PathVertex *light, int nLight, RenderThread *thread) {
       Color3 L(1.0, 1.0, 1.0);
       Color3 c1;
       for (int i = 0; i < nEye-1; ++i) {
           real_t t = eye[i].wo.dot(eye[i].pt.normal);
           if(t < 0) continue;
           eye[i].pt.object->material->f(&eye[i].pt, &eye[i].wo, c1);
           L *=  c1 *= eye[i].pt.object->material->getColor() * t / (eye[i].pCont * eye[i].rw);
       }
       Vector4 w = light[nLight-1].pt.intersection - eye[nEye-1].pt.intersection;
       eye[nEye-1].pt.object->material->f(&eye[nEye-1].pt, &w, c1);
       L *=  c1 *= eye[nEye-1].pt.object->material->getColor() * G(eye[nEye-1], light[nLight-1]) /= (eye[nEye-1].rw * light[nLight-1].rw);
       light[nLight-1].pt.object->material->f(&light[nLight-1].pt, &(w *= -1), c1);
       L *= c1;
       for (int i = nLight-2; i >= 0; --i) {
           real_t t = light[i].wo.dot(light[i].pt.normal);
           if(t < 0) continue;
             light[i].pt.object->material->f(&light[i].pt, &light[i].wo, c1);
             L *=  c1 *= light[i].pt.object->material->getColor() * t /
                   (light[i].pCont * light[i].rw);
       }
       if (L.r == 0. && L.g == 0 && L.b == 0) return L;
       const Vector4 &dir = light[nLight-1].pt.intersection - eye[nEye-1].pt.intersection;
       SurfacePoint sp; //Visibility test
       m_scene->getNearestObject(Ray(eye[nEye-1].pt.intersection + EPSILON * dir, dir), sp,thread);
       if (sp.object != light[nLight-1].pt.object) return Color3(0, 0, 0);
       return L;
};

void BiDiPathTracer::render(Color3 *image, RenderThread *thread, const ROI *region_of_interest) {
    Sampler m_sampler;
    const ROI roi = setup_ROI(region_of_interest, m_settings);
    //cout << "BEGIN" << endl;
    for(unsigned y = roi.top; y <= roi.bottom; ++y) {
        for(unsigned x = roi.left; x <= roi.right; ++x) {
            Color3 c;
            for(unsigned j = 0; j < m_settings->aasamples; ++j) {
                Ray r1 = m_scene->getWorldRay((x + m_sampler.urand()) / (real_t)m_settings->width,
                                             (y + m_sampler.urand()) / (real_t)m_settings->height);
                SceneObject *emitter =  m_scene->getEmitters()[0];
                const Matrix4 &ctmI = emitter->ctmI;
                const Vector4 &samplept = emitter->getSamplePoint(thread->sampler);
                const Vector4 &pos = ctmI * samplept; //This is really the sampled light position in obj space
                const Vector4 &dir = pos - ctmI * Vector4(0,0,0,1);
                Ray r2(pos + EPSILON * dir, dir);
                c += Li(r1, r2, 1, 1, thread);

            }
            image[y*m_settings->width+x] = (c /= (real_t)m_settings->aasamples);
        }
        //cout << (int)(y / real_t(m_settings->height) * 100) << endl;
    }
}
