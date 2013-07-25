#ifndef RENDERER_H
#define RENDERER_H

#include "scene.h"
#include "pmath.h"
#include "stats.h"
#include "sceneobject.h"
#include <stdlib.h>

class RenderThread;

struct RenderSettings{
    unsigned width, height, threads, aasamples;
};

struct ROI {
    union{
        struct {unsigned left, right, top, bottom;};
        unsigned data[4];
    };
    void print() { printf("[%d %d][%d %d]", left, top, right, bottom); }
};
inline std::ostream& operator<<(std::ostream& os, const ROI& f) {
        os << "[" << f.left << "," << f.top << "] to [" << f.right << "," << f.bottom <<"]";
        return os;
}

class Renderer
{
public:
    Renderer(Scene *scene = NULL, RenderSettings *settings = NULL) : m_scene(scene),
    m_settings(settings) {}

    virtual void render(Color3 *image, RenderThread *thread, const ROI *roi = NULL) = 0;

protected:
    Scene *m_scene;
    RenderSettings *m_settings;
    virtual void calcDirectIllumination(const SurfacePoint &pt, RenderThread *thread, Color3 &c, int level=0);
};

class PathTracer : public Renderer {
public:
    PathTracer(Scene *scene = NULL, RenderSettings *settings = NULL) : Renderer(scene, settings),
    mMaxDepth(100) {}
    void render(Color3 *image, RenderThread *thread, const ROI *roi = NULL);

    unsigned mMaxDepth;
protected:
    Color3 tracePath(Ray &r, unsigned depth, RenderThread *thread);
};

class RayTracer : public Renderer {
public:
    RayTracer(Scene *scene = NULL, RenderSettings *settings = NULL) : Renderer(scene, settings) {}
    void render(Color3 *image, RenderThread *thread, const ROI *roi = NULL);
};

class BiDiPathTracer : public Renderer{
public:
    BiDiPathTracer(Scene *scene = NULL, RenderSettings *settings = NULL) : Renderer(scene, settings),
    mMaxDepth(100){}
    void render(Color3 *image, RenderThread *thread,const  ROI *roi = NULL);
protected:
    struct PathVertex {
        SurfacePoint pt;
        real_t rw, tw, pCont;
        Vector4 wo;
    };
    inline real_t G(PathVertex &v0, PathVertex &v1)  {
        Vector4 w = (v1.pt.intersection - v0.pt.intersection);
        w.normalize();
        if(v0.pt.normal.dot(w) < 0 || v1.pt.normal.dot(-w) < 0) return 0;
        return (v0.pt.normal.dot(w)) * (v1.pt.normal.dot(-w))  / v0.pt.intersection.getDistance2(v1.pt.intersection);
    }

    inline real_t weightPath(PathVertex* /*eye*/, int nEye, PathVertex* /*light*/, int nLight)  {
           return real_t(nEye + nLight);
    }

    int generatePath(Ray &r, PathVertex *vertices, int maxVertices, RenderThread *thread);
    Color3 evalPath(PathVertex *eye, int nEye, PathVertex *light, int nLight,  RenderThread *thread);
    Color3 Li(Ray &eyeRay, Ray &lightRay, real_t lightpdf, real_t lightweight, RenderThread *thread);
    int generatePath(const Ray &r, PathVertex *vertices, int maxVertices);

    unsigned mMaxDepth;
};

class PhotonMapper : public Renderer {

};

class MetropolisLightTransporter : public Renderer {

};

#endif // RENDERER_H
