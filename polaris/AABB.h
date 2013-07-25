/* ---------------------------------------------- *\
   file: AABB.h
   auth: travis fischer
   acct: tfischer
   date: 10/7/2006

      an aabb is an axis-aligned bounding box,
   defined by its minimum and maximum coordinates
   in space.
\* ---------------------------------------------- */
#ifndef __AABB_H__
#define __AABB_H__

#include "pmath.h"

#define X_COORD(point)   ((point).data[0])
#define Y_COORD(point)	 ((point).data[1])
#define Z_COORD(point)	 ((point).data[2])

class Ray;

typedef Ray ShadowRay;
typedef Vector4 IAPoint;
typedef Matrix4 IAMatrix;
typedef Vector4 IAVector;

class AABB
{
public:
   // public instance variables to mimic struct appearance for Speed reasons
   IAPoint m_min;
   IAPoint m_max;

   // Axis Aligned Bounding Box (AABB), defined by its min and max coordinates in space
   AABB(const IAPoint &min, const IAPoint &max);
   // Construct an Axis-Aligned Bounding Box from an arbitrarily rotated Bounding Box
   //AABB(const OBB &obb);
   AABB(/*const IAPoint &localMin, const IAPoint &localMax, */const IAMatrix &trans);
   ~AABB() { }

   bool intersectsAABB(const AABB &box) const;
   bool contains(const IAPoint &p) const;
   //bool intersectsOBB(const OBB &obb) const;
   //bool intersectsRay(Ray *ray, real_t &tMin, real_t &tMax) const;
   bool intersectsRay(const Ray * r, real_t &tmin, real_t &tmax) const;
   bool intersectsShadowRay(const ShadowRay * r, real_t &tmin, real_t &tmax) const;

   bool intersectsRay(const Ray * r) const;
   bool intersectsShadowRay(const ShadowRay * r) const;

   real_t getSurfaceArea() const;
   int getShortestExtent() const;
   int getLongestExtent() const;
   int getSecondExtent(const int shortestExtent) const;
   real_t getMinSurfaceArea(int splitAxis, real_t splitPos) const;


   inline void getMinMaxSurfaceArea(const int splitAxis, const real_t splitPos, real_t &leftArea, real_t &rightArea) const {
      IAVector diag = (m_max - m_min);

      diag.data[splitAxis] = splitPos - m_min.data[splitAxis];
      leftArea = 2 * (X_COORD(diag) * Y_COORD(diag) +
                      X_COORD(diag) * Z_COORD(diag) +
                      Y_COORD(diag) * Z_COORD(diag) );

      diag.data[splitAxis] = m_max.data[splitAxis] - splitPos;
      rightArea = 2 * (X_COORD(diag) * Y_COORD(diag) +
                       X_COORD(diag) * Z_COORD(diag) +
                       Y_COORD(diag) * Z_COORD(diag));
   }

   inline void include(Vector3 &vec) {
       if (vec.x < m_min.x) m_min.x = vec.x;
       else if (vec.x > m_max.x) m_max.x = vec.x;
       if (vec.y < m_min.y) m_min.y = vec.y;
       else if (vec.y > m_max.y) m_max.y = vec.y;
       if (vec.z < m_min.z) m_min.z = vec.z;
       else if (vec.z > m_max.z) m_max.z = vec.z;
   }


//   inline AABB getMinAABB(int splitAxis, real_t splitPos) const;
//   inline AABB getMaxAABB(int splitAxis, real_t splitPos) const;
//   void operator<<(ostream &out);
};

#endif //__AABB_H__

