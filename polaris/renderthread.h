#include <QRunnable>
#include "stats.h"

#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

class Renderer;
struct ROI;
struct Color3;

class RenderThread
{
public:
    RenderThread(Renderer *renderer, Color3 *target_img) : m_renderer(renderer), m_ROI(0), m_image(target_img) {};
    virtual ~RenderThread() { delete m_ROI; };

    virtual void run() = 0;
    virtual void setROI(unsigned left, unsigned right, unsigned top, unsigned bottom);
    
    const ROI* getROI() { return m_ROI; };
    Sampler sampler;

    // used internally by rendering process
    unsigned ray_count;

    // avoid reallocations for quick things in this thread
    Vector3 tmp_vec3;
    Vector4 tmp_vec4;

protected:
    Renderer *m_renderer;
    ROI *m_ROI;
    Color3 *m_image;


};

class RenderThreadMPI : public RenderThread {
public:
    RenderThreadMPI(Renderer *renderer, Color3 *target_img) :
            RenderThread(renderer,target_img) {};
    void run();

protected:
    void run_master();
    void run_slave();

};


class RenderThreadQt  : public QRunnable, public RenderThread {
public:
    RenderThreadQt(Renderer *renderer, Color3 *target_img) : RenderThread(renderer,target_img) {};

    void run();

};

#endif // RENDERTHREAD_H
