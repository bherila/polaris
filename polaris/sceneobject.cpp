#include "stdafx.h"
#include "sceneobject.h"
#include "pmath.h"
#include <limits>
#include <iostream>

class Sampler;

/*
 * Ray-box intersection using IEEE numerical properties to ensure that the
 * test is both robust and efficient, as described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */
static inline bool intersect_bbox(const Vector3 bbox[2],const Ray& ray, real_t  &t0, real_t  &t1)
{
  const real_t *ray_o    = ray.origin.data;
  const real_t *ray_di   = ray.inv_direction.data;
  const int    *ray_sign = ray.sign.data;

  real_t  tmin  = (bbox[  ray_sign[0]].data[0] - ray_o[0]) * ray_di[0];
  real_t  tmax  = (bbox[1-ray_sign[0]].data[0] - ray_o[0]) * ray_di[0];

  real_t  tymin = (bbox[  ray_sign[1]].data[1] - ray_o[1]) * ray_di[1];
  real_t  tymax = (bbox[1-ray_sign[1]].data[1] - ray_o[1]) * ray_di[1];

  if ( (tmin > tymax) || (tymin > tmax) ) return false;
  if (tymin > tmin) tmin = tymin;
  if (tymax < tmax) tmax = tymax;

  real_t  tzmin = (bbox[  ray_sign[2]].data[2] - ray_o[2]) * ray_di[2];
  real_t  tzmax = (bbox[1-ray_sign[2]].data[2] - ray_o[2]) * ray_di[2];

  if ( (tmin > tzmax) || (tzmin > tmax) )    return false;
  if (tzmin > tmin) tmin = tzmin;
  if (tzmax < tmax) tmax = tzmax;

  if (!( (tmin < t1) && (tmax > t0) )) return false;

  t0 = tmin;
  t1 = tmax;
  return true;
}

static const Vector3 unit_min(-.5,-.5,-.5);
static const Vector3 unit_max( .5, .5, .5);
static const Vector3 unit_box[2] = {unit_min, unit_max};

real_t Cube::getIntersection(const Ray &r) const {
    real_t tmin = -1;
    real_t tmax = std::numeric_limits<real_t>::max();
    intersect_bbox(unit_box, r, tmin, tmax);
    return tmin;
}

real_t Sphere::getIntersection(const Ray &r) const {
    real_t a = 2 * (r.direction.x * r.direction.x + r.direction.y * r.direction.y + r.direction.z * r.direction.z);
    real_t b = 2 * (r.direction.x * r.origin.x + r.direction.y * r.origin.y + r.direction.z * r.origin.z);
    real_t c = r.origin.x * r.origin.x + r.origin.y * r.origin.y + r.origin.z * r.origin.z - 0.25;
    real_t discriminant = b * b - 2 * a * c;
    if (discriminant < 0) return -1;
    return (-b - sqrt(discriminant)) / a;
}

real_t Cone::getIntersection(const Ray &r) const {
    assert(0);
    return 0;
}

real_t Cylinder::getIntersection(const Ray &r) const {
    assert(0);
    return 0;
}

void Cube::getNormal(const Vector4 &intersection, Vector4 &normal) const {
    if (EQ(intersection.data[1], 0.5)) normal.y = 1;
    else if (EQ(intersection.data[1], -0.5)) normal.y = -1;
    else if (EQ(intersection.data[2], 0.5)) normal.z = 1;
    else if (EQ(intersection.data[2], -0.5)) normal.z = -1;
    else if (EQ(intersection.data[0], 0.5)) normal.x = 1;
    else if (EQ(intersection.data[0], -0.5)) normal.x = -1;
}

void Sphere::getNormal(const Vector4 &intersection, Vector4 &normal) const {
     normal.x = ctm.data[0] * intersection.data[0] + ctm.data[4] * intersection.data[1] + ctm.data[8] * intersection.data[2];
     normal.y = ctm.data[1] * intersection.data[0] + ctm.data[5] * intersection.data[1] + ctm.data[9] * intersection.data[2];
     normal.z = ctm.data[2] * intersection.data[0] + ctm.data[6] * intersection.data[1] + ctm.data[10] * intersection.data[2];
     normal.w = 0;
}

void Cone::getNormal(const Vector4 &intersection, Vector4 &normal) const {

}

void Cylinder::getNormal(const Vector4 &intersection, Vector4 &normal) const {

}

void Cube::init() {

}

void Cylinder::init() {

}

void Cone::init() {

}

void Sphere::init() {
    if(mRadius < 0) mRadius = (ctmI * Vector4(0.5, 0, 0, 0)).getMagnitude();
    surface_area = 4*PI*mRadius*mRadius;
}

Vector4 Sphere::getSamplePoint(Sampler &sampler){
    if(mRadius < 0) mRadius = (ctm.getInverse() * Vector4(0.50, 0, 0, 0)).getMagnitude();
    return sampler.urandSphere(mRadius - 0.01); //Epsilon
}

Vector4 Cube::getSamplePoint(Sampler &sampler) {
    assert(0);
    return Vector4::zero();
}
Vector4 Cone::getSamplePoint(Sampler &sampler) {
    assert(0);
    return Vector4::zero();
}
Vector4 Cylinder::getSamplePoint(Sampler &sampler)  {
    assert(0);
    return Vector4::zero();
}

