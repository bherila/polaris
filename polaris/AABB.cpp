/* ---------------------------------------------- *\
   FILE: AABB.C
   AUTH: Travis Fischer
   ACCT: tfischer
   DATE: 10/7/2006

   An AABB is an Axis-Aligned Bounding Box,
   defined by its minimum and maximum coordinates
   in space.
\* ---------------------------------------------- */
#include "stdafx.h"
#include "AABB.h"
#include "pmath.h"
#include "camera.h"

// Axis Aligned Bounding Box (AABB), defined by its min and max coordinates in space
AABB::AABB(const IAPoint &min, const IAPoint &max) {
   m_min = min;
   m_max = max;
}

#define COMPARE_POINT(p)   if ((p).data[a] < m_min.data[a]) m_min.data[a] = (p).data[a]; \
                           else if ((p).data[a] > m_max.data[a]) m_max.data[a] = (p).data[a];

// Transforms an object space AABB into a world space OBB and then
// finds the smallest AABB which surrounds that OBB
AABB::AABB(const IAMatrix &trans) {
   m_min = trans * IAPoint(-.5f, -.5f,-.5f, 0);
   const IAPoint &p0 = trans * IAPoint(-.5f, -.5f, .5f, 0);
   const IAPoint &p2 = trans * IAPoint(.5f,  -.5f,-.5f, 0);
   const IAPoint &p1 = trans * IAPoint(.5f,  -.5f, .5f, 0);

   const IAPoint &p3 = trans * IAPoint(-.5f, .5f,-.5f, 0);
   const IAPoint &p4 = trans * IAPoint(-.5f, .5f, .5f, 0);
   const IAPoint &p5 = trans * IAPoint(.5f,  .5f,-.5f, 0);
   m_max = trans * IAPoint(.5f,  .5f, .5f, 0);

   for(int a = 3; a--;) {
      COMPARE_POINT(p0);
      COMPARE_POINT(p1);
      COMPARE_POINT(p2);
      COMPARE_POINT(p3);
      COMPARE_POINT(p4);
      COMPARE_POINT(p5);
   }
   /*const IAPoint localMin(-.5f, -.5f, -.5f);
   const IAPoint localMax(.5f, .5f, .5f);

   m_min = trans * localMin;
   m_max = m_min;
   const IAVector &x = trans * IAPoint(localMax.data[0], localMin.data[1], localMin.data[2], 0) - m_min;
   const IAVector &y = trans * IAPoint(localMin.data[0], localMax.data[1], localMin.data[2], 0) - m_min;
   const IAVector &z = trans * IAPoint(localMin.data[0], localMin.data[1], localMax.data[2], 0) - m_min;

   const IAPoint &p0 = m_min + x, &p1 = m_min + y, &p2 = m_min + z,
         &p3 = p0 + y, &p4 = p1 + z, &p5 = p2 + x, &p6 = p5 + y;

   for(int a = 3; a--;) {
      COMPARE_POINT(p0);
      COMPARE_POINT(p1);
      COMPARE_POINT(p2);
      COMPARE_POINT(p3);
      COMPARE_POINT(p4);
      COMPARE_POINT(p5);
      COMPARE_POINT(p6);
   }*/
}


// Determines whether this AABB intersects another AABB
bool AABB::intersectsAABB(const AABB& box) const {
   const IAPoint &min2 = box.m_min, &max2 = box.m_max;

   // Check if x, y, and z planes are inside of the other AABB's x, y, and z planes
   return ((X_COORD(m_max) > X_COORD(min2)) && (X_COORD(m_min) < X_COORD(max2)) &&
           (Y_COORD(m_max) > Y_COORD(min2)) && (Y_COORD(m_min) < Y_COORD(max2)) &&
           (Z_COORD(m_max) > Z_COORD(min2)) && (Z_COORD(m_min) < Z_COORD(max2)));
}

/*bool AABB::intersectsRay(const Ray *ray, real_t &tMin, real_t &tMax) const {
   // Detect trivial non-intersections

   //tMin = 0;
   //tMax = 200;
#if 0
   for(int i = 0; i < 3; i++) {
      if (ray->datair[i] < 0) {
         if (ray->origin[i] < m_min[i])
            return 0;
      } else if (ray->origin[i] > m_max[i])
         return 0;
   }
#endif

   for(int i = 0; i < 3; i++) {
      //const real_t t = (m_min.data[i] - ray->origin[i]) * ray->inv_direction[i];
      //const real_t t2 = (m_max.data[i] - ray->origin[i]) * ray->inv_direction[i];
      const real_t t[2] = {
         (m_min[i] - ray->origin[i]) * ray->inv_direction[i],
         (m_max[i] - ray->origin[i]) * ray->inv_direction[i],
      };

      int a = (t[0] < t[1]);

      if (t[a] < tMax) {
         tMax = t[a];
         if(tMax < 0)
            return false;
      }

      if (t[1 - a] > tMin)
         tMin = t[1 - a];
   }

   return (tMax > tMin);
}*/

/*
 * Ray-box intersection using IEEE numerical properties to ensure that the
 * test is both robust and efficient, as described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 * Modified slightly by tfischer for the purposes of this program
 */

bool AABB::intersectsRay(const Ray * r, real_t &tmin, real_t &tmax) const {
   real_t tymin, tymax, tzmin, tzmax;

   const IAPoint parameters[2] = { m_min, m_max };
   const bool xSign = (r->inv_direction[0] < 0);
   const bool ySign = (r->inv_direction[1] < 0);

   tmin = (parameters[xSign][0] - r->origin[0]) * r->inv_direction[0];
   tmax = (parameters[!xSign][0] - r->origin[0]) * r->inv_direction[0];
   tymin = (parameters[ySign][1] - r->origin[1]) * r->inv_direction[1];
   tymax = (parameters[!ySign][1] - r->origin[1]) * r->inv_direction[1];
   if ( (tmin > tymax) || (tymin > tmax) )
      return false;
   if (tymin > tmin)
      tmin = tymin;
   if (tymax < tmax)
      tmax = tymax;

   const bool zSign = (r->inv_direction[2] < 0);
   tzmin = (parameters[zSign][2] - r->origin[2]) * r->inv_direction[2];
   tzmax = (parameters[!zSign][2] - r->origin[2]) * r->inv_direction[2];
   if ( (tmin > tzmax) || (tzmin > tmax) )
      return false;
   if (tzmin > tmin)
      tmin = tzmin;
   if (tzmax < tmax)
      tmax = tzmax;

  return (tmin < tmax);
}

bool AABB::intersectsShadowRay(const ShadowRay * r, real_t &tmin, real_t &tmax) const {
   real_t tymin, tymax, tzmin, tzmax;

   const IAPoint parameters[2] = { m_min, m_max };
   const bool xSign = (r->inv_direction[0] < 0);
   const bool ySign = (r->inv_direction[1] < 0);

   tmin = (parameters[xSign][0] - r->origin[0]) * r->inv_direction[0];
   tmax = (parameters[!xSign][0] - r->origin[0]) * r->inv_direction[0];
   tymin = (parameters[ySign][1] - r->origin[1]) * r->inv_direction[1];
   tymax = (parameters[!ySign][1] - r->origin[1]) * r->inv_direction[1];
   if ( (tmin > tymax) || (tymin > tmax) )
      return false;
   if (tymin > tmin)
      tmin = tymin;
   if (tymax < tmax)
      tmax = tymax;

   const bool zSign = (r->inv_direction[2] < 0);
   tzmin = (parameters[zSign][2] - r->origin[2]) * r->inv_direction[2];
   tzmax = (parameters[!zSign][2] - r->origin[2]) * r->inv_direction[2];
   if ( (tmin > tzmax) || (tzmin > tmax) )
      return false;
   if (tzmin > tmin)
      tmin = tzmin;
   if (tzmax < tmax)
      tmax = tzmax;

   return (tmin < tmax);
}

bool AABB::intersectsRay(const Ray * r) const {
   const IAPoint parameters[2] = { m_min, m_max };
   const bool xSign = (r->inv_direction[0] < 0);
   const bool ySign = (r->inv_direction[1] < 0);

   real_t tmin = (parameters[xSign][0] - r->origin[0]) * r->inv_direction[0];
   real_t tmax = (parameters[!xSign][0] - r->origin[0]) * r->inv_direction[0];
   const real_t tymin = (parameters[ySign][1] - r->origin[1]) * r->inv_direction[1];
   const real_t tymax = (parameters[!ySign][1] - r->origin[1]) * r->inv_direction[1];
   if ( (tmin > tymax) || (tymin > tmax) )
      return false;
   if (tymin > tmin)
      tmin = tymin;
   if (tymax < tmax)
      tmax = tymax;

   const bool zSign = (r->inv_direction[2] < 0);
   const real_t tzmin = (parameters[zSign][2] - r->origin[2]) * r->inv_direction[2];
   const real_t tzmax = (parameters[!zSign][2] - r->origin[2]) * r->inv_direction[2];
   if ( (tmin > tzmax) || (tzmin > tmax) )
      return false;
   if (tzmin > tmin)
      tmin = tzmin;
   if (tzmax < tmax)
      tmax = tzmax;

  return (tmin < tmax);
}

bool AABB::intersectsShadowRay(const ShadowRay * r) const {
   const IAPoint parameters[2] = { m_min, m_max };
   const bool xSign = (r->inv_direction[0] < 0);
   const bool ySign = (r->inv_direction[1] < 0);

   real_t tmin = (parameters[xSign][0] - r->origin[0]) * r->inv_direction[0];
   real_t tmax = (parameters[!xSign][0] - r->origin[0]) * r->inv_direction[0];
   const real_t tymin = (parameters[ySign][1] - r->origin[1]) * r->inv_direction[1];
   const real_t tymax = (parameters[!ySign][1] - r->origin[1]) * r->inv_direction[1];
   if ( (tmin > tymax) || (tymin > tmax) )
      return false;
   if (tymin > tmin)
      tmin = tymin;
   if (tymax < tmax)
      tmax = tymax;

   const bool zSign = (r->inv_direction[2] < 0);
   const real_t tzmin = (parameters[zSign][2] - r->origin[2]) * r->inv_direction[2];
   const real_t tzmax = (parameters[!zSign][2] - r->origin[2]) * r->inv_direction[2];
   if ( (tmin > tzmax) || (tzmin > tmax) )
      return false;
   if (tzmin > tmin)
      tmin = tzmin;
   if (tzmax < tmax)
      tmax = tzmax;

  return (tmin < tmax);
}


// Determines whether this AABB contains the given point
bool AABB::contains(const IAPoint &p) const {
   // Check if x, y, and z coords are inside of the planes defined by my six x, y, and z sides
   return ((X_COORD(m_max) > X_COORD(p)) && (X_COORD(m_min) < X_COORD(p)) &&
           (Y_COORD(m_max) > Y_COORD(p)) && (Y_COORD(m_min) < Y_COORD(p)) &&
           (Z_COORD(m_max) > Z_COORD(p)) && (Z_COORD(m_min) < Z_COORD(p)));
}

// Returns the surface area of this AABB
real_t AABB::getSurfaceArea() const {
   const IAVector &diag = (m_max - m_min);

   return 2 * (diag.data[0] * diag.data[1] +
               diag.data[0] * diag.data[2] +
               diag.data[1] * diag.data[2]);
}


int AABB::getShortestExtent() const {
   const real_t m_extantX = m_max.data[0] - m_min.data[0];
   const real_t m_extantY = m_max.data[1] - m_min.data[1];
   const real_t m_extantZ = m_max.data[2] - m_min.data[2];

   // m_extantX is the smallest:  return 0
   // m_extantY is the smallest:  return 1
   // m_extantZ is the smallest:  return 2
   return (m_extantX < m_extantY) ? 2 * (m_extantZ < m_extantX) : 1 + (m_extantZ < m_extantY);
}

int AABB::getLongestExtent() const {
   const real_t m_extantX = m_max.data[0] - m_min.data[0];
   const real_t m_extantY = m_max.data[1] - m_min.data[1];
   const real_t m_extantZ = m_max.data[2] - m_min.data[2];

   // m_extantX is the largest:  return 0
   // m_extantY is the largest:  return 1
   // m_extantZ is the largest:  return 2
   return (m_extantX > m_extantY) ? 2 * (m_extantZ > m_extantX) : 1 + (m_extantZ > m_extantY);
}

// Return the second Largest extent, given the longest extent
int AABB::getSecondExtent(const int longestExtent) const {
   const real_t m_extantX = m_max.data[0] - m_min.data[0];
   const real_t m_extantY = m_max.data[1] - m_min.data[1];
   const real_t m_extantZ = m_max.data[2] - m_min.data[2];


   if (longestExtent == 0)
      return 1 + (m_extantZ > m_extantY);

   if (longestExtent == 1)
      return 2 * (m_extantZ > m_extantX);

   //if (longestExtent == 2)
   return (m_extantY > m_extantX);
}

real_t AABB::getMinSurfaceArea(int splitAxis, real_t splitPos) const {
   IAVector diag = (m_max - m_min);
   diag.data[splitAxis] = splitPos - m_min.data[splitAxis];

   return 2 * (diag.data[0] * diag.data[1] +
               diag.data[0] * diag.data[2] +
               diag.data[1] * diag.data[2]);
}


/*void AABB::operator << (ostream &out) {
   out << "{ MIN: (" << m_min.data << "), MAX: (" << m_max.data << ')';
}*/


#if 0

// Return the second Smallest extent, given the shortest extent
/*int AABB::getSecondExtent(const int shortestExtent) const {
   const real_t m_extantX = m_max.data[0] - m_min.data[0];
   const real_t m_extantY = m_max.data[1] - m_min.data[1];
   const real_t m_extantZ = m_max.data[2] - m_min.data[2];

   if (shortestExtent == 0)
      return 1 + (m_extantZ < m_extantY);

   if (shortestExtent == 1)
      return 2 * (m_extantZ < m_extantX);

   //if (longestExtent == 2)
   return (m_extantY < m_extantX);
}*/


/*inline AABB AABB::getMinAABB(int splitAxis, real_t splitPos) const {
   IAPoint max = m_max;
   max.data[splitAxis] = splitPos;

   return AABB(m_min, max);
}

inline AABB AABB::getMaxAABB(int splitAxis, real_t splitPos) const {
   IAPoint min = m_min;
   min.data[splitAxis] = splitPos;

   return AABB(min, m_max);
}*/

// It is up to the caller to ensure that the given OBB is really an AABB
AABB::AABB(const OBB &obb) {
   m_min = obb.m_pos, m_max = obb.m_pos;
   const IAPoint &p0 = obb.m_pos + obb.m_x, &p1 = obb.m_pos + obb.m_y,
         &p2 = obb.m_pos + obb.m_z, &p3 = p0 + obb.m_y, &p4 = p1 + obb.m_z,
         &p5 = p2 + obb.m_x, &p6 = p5 + obb.m_y;

   for(int a = 3; a--;) {
      COMPARE_POINT(p0);
      COMPARE_POINT(p1);
      COMPARE_POINT(p2);
      COMPARE_POINT(p3);
      COMPARE_POINT(p4);
      COMPARE_POINT(p5);
      COMPARE_POINT(p6);
   }

   /*const IAPoint &pos = obb.obb.m_pos;
   const IAVector &x  = obb.obb.m_x;
   const IAVector &y  = obb.obb.m_y;
   const IAVector &z  = obb.obb.m_z;
   int a;

   for(a = 3; a--;) {
      m_min.data[a] = pos.data[a];
      m_max.data[a] = pos.data[a];
      const real_t diagonal = diag.data[a];

      if (diagonal > 0)
         m_max.data[a] += diagonal;
      else m_min.data[a] += diagonal;
   }*/
}

// Returns the volume of this AABB
/*inline real_t AABB::getVolume() const {
   const IAVector &diag = (m_max - m_min);

   return diag.data[0] * diag.data[1] * diag.data[2];
}*/

#endif

