#ifndef __kdTree_H__
#define __kdTree_H__

#include <stack>
#include <vector>
#include <hash_map>
#include <string>
#include <map>
#include "AABB.h"
#include "scene.h"

class SceneObject;
class AABB;
class CS123Image;

enum KD_flags { KD_SPLIT_X_FLAG = 0, KD_SPLIT_Y_FLAG = 1,
                KD_SPLIT_Z_FLAG = 2, KD_LEAF_FLAG = 3 };

// Parameters affecting construction and layout of tree
#define KD_MIN_SceneObjectS           (3)
#define KD_MAX_DEPTH                (24)
// Parameters affecting the Surface Area Heuristic (SAH)
#define KD_COST_TRAVERSAL           (0.0f)
#define KD_COST_INTERSECT           (120.0f) // 0 gives tree w/ one node cause intersecting
#define KD_COST_RATIO               (0.0f)  // C0ST_TRAVERSAL / COST_INTERSECT
//static const real_t KD_COST_RATIO = KD_COST_TRAVERSAL / KD_COST_INTERSECT;
// any no SceneObjects is 0
#define KD_EMPTY_BIAS               (0.9)

using namespace std;

struct HashFloat {
   size_t operator()(float val) const {
      unsigned long h = 0;
      char *s = (char*)&val;
      for(unsigned i = sizeof(float); i--;)
         h = 5 * h + s[i];
      return size_t(h);
   }
};

//typedef hash_map<float, bool/*unsigned int*/, HashFloat > SplitHashMap;
typedef __gnu_cxx::hash_map<float, bool, HashFloat> SplitHashMap;
typedef SplitHashMap::const_iterator SplitHashMapIterator;

typedef map<string, CS123Image*, less<string> > TextureMap;
typedef TextureMap::const_iterator TextureMapIterator;

#define KD_FLAG_BITS                (3)
#define KD_FLAG(node)               ((node)->m_flag & KD_FLAG_BITS)
#define KD_LEAF_NODE(node)          (KD_FLAG(node) == 3)
#define KD_INTERNAL_NODE(node)      (KD_FLAG(node) < 3)
#define KD_IS_PARTITIONED(node)     (KD_INTERNAL_NODE(node))

#define KD_SPLIT_POS(node)          ((node)->m_splitPos)
#define KD_SPLIT_AXIS(node)         (KD_FLAG(node))
#define KD_NO_SceneObjectS(node)      ((node)->m_noSceneObjects >> 2)

#define KD_LEFT_CHILD(node)         \
((kdNode*)((node)->m_flag & ~KD_FLAG_BITS))
// Left/Right Children stored next to each other on 16 byte boundary
// Right child is stored at Left child + 8
#define KD_RIGHT_CHILD(node)        (KD_LEFT_CHILD(node) + 1)

// 8 byte packed struct optimized for cache performance
struct kdNode {
   union {  // 4 bytes
      kdNode *m_child;
      unsigned int m_noSceneObjects;
      unsigned int m_flag;
   };
   union {  // 4 bytes
      SceneObjectList *m_SceneObjects;
      //SceneObject **m_SceneObjects;
      float m_splitPos;
   };
};

class kdTree;
struct BuildkdTreeThread {
    kdTree *m_kdTree;
    kdNode *curNode;
    AABB *aabb;
    unsigned int depth;
};

/* Stack for kd tree */

struct kdParametricNode {
    //kdParametricNode *next;
    kdNode *node;
    //real_t tMin;
    real_t tMax;
};

class kdStack
{
   public:
      kdStack() : m_top(m_nodes) { }
      ~kdStack() {}

      kdParametricNode *m_top;
      // Max number of internal nodes pushed on to stack during Traversal
      // is equal to the tree depth
      kdParametricNode m_nodes[KD_MAX_DEPTH + 1];

      inline void push(kdNode *node, /*real_t tMin, */real_t tMax) {
         m_top++;
         m_top->node = node;
         m_top->tMax = tMax;
      }

      inline bool isEmpty() const {
         return (m_top <= m_nodes);
      }

      // Empty kdStack
      inline void reset() {
         m_top = m_nodes;
      }

      // assumes check for isEmpty (no error-checking, huzzah!)
      inline void pop(kdNode *&curNode, real_t &tMax) {
         curNode = m_top->node;
         tMax    = m_top->tMax;

         m_top--;
      }
};

class SurfacePoint;
class kdTree : public SpatialAccel
{
   public:
      kdTree(SceneObjectList *SceneObjects);
      ~kdTree();

      void buildTree(kdNode *curNode, AABB &aabb, unsigned int depth);

      // Returns the information regarding the closest object (if any)
      // of intersection with the given Ray in the scene
      SceneObject *getIntersection(const Ray *ray, SurfacePoint *sp, unsigned threadId);

      // Returns whether or not the given Ray hits any object in the scene
      // (for Shadow calculations where exact intersection info is unnecessary)
      SceneObject *intersects(ShadowRay *__restrict__ ray, kdStack *__restrict__ m_kdStack, real_t lightDist);

      static void *startBuildThread(void *arg) {
         BuildkdTreeThread *b = (BuildkdTreeThread*)arg;
         b->m_kdTree->buildTree(b->curNode, *b->aabb, b->depth);

         return NULL;
      }

      kdStack *m_kdStack;

#ifdef DEBUG  // Debugging Info/Statistical Variables
      unsigned int m_maxDepth;
      unsigned int m_noNodes;
      unsigned long m_avgLeafSceneObjects;
      unsigned long m_noLeafs;
      unsigned int m_avgDepth;

      unsigned long m_noIntersectionTests;
      unsigned long m_noHits;
      unsigned long m_noBacktracks;
      unsigned long m_numTraversals;
#endif // DEBUG

      // Root node of kd-Tree
      kdNode *m_root;

   private:
      kdNode *m_kdNodes;
      kdNode *m_lastkdNode;

      SplitHashMap *m_splitHash;
      TextureMap m_textureMap;

      // Coordinates defining this cube in space
      AABB *m_bounds;
      AABB *m_rightBranchAABB;
      AABB *m_leftBranchAABB;

      SceneObject *_prim_; // for debugging
};

#endif //__kdTree_H__

