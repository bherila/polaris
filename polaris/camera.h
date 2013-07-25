#ifndef CAMERA_H
#define CAMERA_H

#include "pmath.h"
#include <iostream>

class RenderThread;

struct Ray {

    /// Constructs a ray from a start position and a direction.
    Ray () {}
    Ray (const Vector4 p, const Vector4 d);
    Ray (const Vector4 p, const Vector4 d, int id);

    /// Copy constructor
    inline Ray (const Ray &r);

    inline Ray& operator  = (const Ray& rhs) {
        memcpy(this, &rhs, sizeof(Ray));
        return *this;
    }

    const Vector4  origin;
    const Vector4  direction;
    const Vector4  inv_direction;
    const Vector4i sign;

    int id; // unique id (per thread)

    char m_normalCase;

    void reset(const Vector4 p, const Vector4 d);

private:
    void init();
};

class Camera
{
public:
    Camera() {}
    virtual ~Camera() {}
    const Matrix4& getFilmToWorldMat()  { return m_filmToWorld;  };
    const Matrix4& getWorldToFilmMat()  { return m_worldToFilm;  };

    virtual Vector4& getPosition() {
        return m_position;
    }
    virtual Vector3& getLook() {
        return m_look;
    }
    virtual Vector3& getUp() {
        return m_up;
    }
    virtual void init() = 0;

    virtual void setLook(const Vector3 &look) {
        m_look = look;
    }

    virtual void setPosition(const Vector4 &position) {
        m_position = position;
    }

    virtual void setUp(const Vector3 &up) {
        m_up = up;
    }

    virtual void setHeightAngle(real_t angle){
        m_heightAngle = PI_180 * angle;
    }
    virtual void setAspectRatio(real_t aspect){
        m_aspect = aspect;
    }

    virtual void setNear(real_t near) {
        m_near = near;
    }
    virtual void setFar(real_t far) {
        m_far = far;
    }
    virtual void orient(const Vector4 &pos, const Vector3 &look, const Vector3 &up) {
        assert(pos[3] == 1);
        m_position = pos;
        m_look = look;
        m_up = up;
        m_w = -m_look.getNormalized();
        m_v = m_up.cross(m_w).getNormalized();
        m_u = m_w.cross(m_v);
    }

    /// Sets the look vector to point towards the target vector. Requires position to be set.
    virtual void lookAt(const Vector4 &target);

protected:
    Vector4 m_position;
    Vector3 m_look, m_up, m_u, m_v, m_w; //look, up, camera coords
    Matrix4 m_filmToWorld, m_worldToFilm;
    real_t m_heightAngle, m_aspect; //Defined in radians
    real_t m_near, m_far;
};

class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera();
    virtual void init();
};

#endif // CAMERA_H
