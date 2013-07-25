#include "stdafx.h"
#include <iostream>
#include "renderthread.h"
#include "camera.h"

using namespace std;

inline void Ray::init() {
    // set up inverse direction
    real_t *s = (real_t*)(inv_direction.data);
#pragma vector align
    for (unsigned i = 0; i < 4; ++i)
    s[i] = 1.0 / direction.data[i];

    // set up sign
    int *t = (int*)(sign.data);
#pragma vector align
    for (unsigned i = 0; i < 4; ++i)
    t[i] = inv_direction.data[i] < 0;

#ifdef DEBUG
    if (!(direction[3] < EPSILON && direction[3] > -EPSILON)) {
        cout << "FAIL: " << direction[3] << endl;
        assert(0);
    }
#endif

}

Ray::Ray(const Vector4 p, const Vector4 d) : origin(p.data), direction(d.data)
{
    init();
}

Ray::Ray(const Vector4 p, const Vector4 d, int id) : origin(p.data), direction(d.data), id(id)
{
    init();
}

void Ray::reset(Vector4 p, Vector4 d) {
    memcpy((void*)&direction, &d, sizeof(Vector4));
    memcpy((void*)&origin, &p, sizeof(Vector4));
    init();
}


Ray::Ray(const Ray &r)  :
        origin(r.origin),
        direction(r.direction),
        inv_direction(r.inv_direction),
        sign(r.sign)
{
}

PerspectiveCamera::PerspectiveCamera() : Camera() {

}


void PerspectiveCamera::init() {
    printf("Far: %f, HeightAngle: %f, Aspect ratio: %f, Position: [%f, %f, %f] \n",
           m_far, m_heightAngle, m_aspect, m_position[0], m_position[1], m_position[2]);
    Matrix4 B = Matrix4(1 / m_far, 0, 0, 0,
                        0, 1 / m_far, 0, 0,
                        0, 0, 1 / m_far, 0,
                        0, 0, 0, 1);
    Matrix4 C = Matrix4(1 / tan(m_heightAngle / 2.0) / m_aspect, 0, 0, 0,
                        0, 1 / tan(m_heightAngle / 2.0), 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1);
    Matrix4 D = Matrix4(m_v.data[0], m_v.data[1], m_v.data[2], 0,
                        m_u.data[0], m_u.data[1], m_u.data[2], 0,
                        m_w.data[0], m_w.data[1], m_w.data[2], 0,
                        0, 0, 0, 1);
    Matrix4 E = Matrix4(1, 0, 0, -m_position.data[0],
                        0, 1, 0, -m_position.data[1],
                        0, 0, 1, -m_position.data[2],
                        0, 0, 0, 1);

    m_worldToFilm = B * C * D * E;
    m_filmToWorld = m_worldToFilm.getInverse();
}

void Camera::lookAt(const Vector4 &target) {
    m_look = Vector3((target - m_position).data);
    m_w = -m_look.getNormalized();
    m_v = m_up.cross(m_w).getNormalized();
    m_u = m_w.cross(m_v);
}
