
#include "stdafx.h"
#include "spatialdatastructure.h"
#include "scene.h"
#include "camera.h"
#include "renderthread.h"
#include <iostream>
#include <limits>

using namespace std;


//bool SceneList::findIntersection(const Ray &r, SurfacePoint &pt) {
SceneObject* SceneList::getIntersection(const Ray *r, SurfacePoint *sp, RenderThread *thread) {
    assert(thread);
    SurfacePoint &pt = *sp;
    SceneObject *min_o = NULL;
    const Vector4 &ray_d = r->direction; //assert(ray_d[3] == 0);
    const Vector4 &ray_o = r->origin;
    real_t min_t =std::numeric_limits<real_t>::max();
    Vector4 p, d;
    for (unsigned i = 0; i < m_objects.size(); ++i) {
        SceneObject *so = m_objects[i];
        so->ctm.mulVec4(ray_d, d); //eqivalent to d = so->ctm * ray_d;
        so->ctm.mulVec4(ray_o, p); //so->ctm * ray_o
        Ray r_obj(p, d, thread->ray_count++); //move ray into object space
        const real_t t = so->getIntersection(r_obj);
        if (t > 0 && t < min_t) {
            min_o = so;
            min_t = t;
        }
    }
    if (!min_o) return NULL;
    pt.t = min_t;
    pt.incoming = *r;
    pt.intersection = ray_o + min_t*ray_d; //intersection in world space
    pt.object = min_o;
    //@TODO: OPTIMIZE
    min_o->ctm.mulVec4(ray_o, p); // p = min_o->ctm * ray_o;
    min_o->ctm.mulVec4(ray_d, d); // d = min_o->ctm * ray_d
    min_o->getNormal(p + d * min_t, pt.normal);
    pt.normal.normalize();
    //pt.normal = min_o->getNormal(min_o->ctm * pt.intersection).normalize(); //convert the intersection to object soace
    return min_o;
}
