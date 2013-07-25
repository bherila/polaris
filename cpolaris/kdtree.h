#ifndef KD_TREE_DEF
#define KD_TREE_DEF

#include <assert.h>
#include <stdlib.h>
#include "scene.h"
#include "ray.h"

#ifdef __cplusplus
#include <vector>
#endif

//struct List {
//    int    nAllocatedSize;
//    int    nCount;
//    void **objects; // array of pointers
//};
//typedef struct List List;


#define BETWEEN(x, a, b) (a <= x && x <= b)
#define aabb_diff(box, dimension) box->max.data[dimension] - box->min.data[dimension]

typedef struct aabb {
    union {
        struct {
            Vector4 min;
            Vector4 max;
        };
        struct {
            float x0, y0, z0, w0, x1, y1, z1, w1;
        };
    };

#ifdef __cplusplus
    bool intersectsPrimitive(const SceneObject *prim) {
        Vector4 p0, p1;
        calc_aabb(prim, &p0, &p1);
        return  BETWEEN(p0.x, min.x, max.x) &&
                BETWEEN(p0.y, min.y, max.y) &&
                BETWEEN(p0.z, min.z, max.z);
    }
    float w() { return aabb_diff(this, 0); };
    float h() { return aabb_diff(this, 1); };
    float d() { return aabb_diff(this, 2); };
#endif

} aabb;


// =====================================================================================================

// warning - the kd-nodes must be on an 8 byte boundary!
typedef struct kdNode
{
    float splitPos;
    unsigned isLeaf  : 1;
    short splitAxis  : 2;
    unsigned value      : 29; // either index of left child OR index of list of items
} kdNode;

// =====================================================================================================

#ifdef __cplusplus
    typedef std::vector<SceneObject*> ObjectList;
    typedef std::vector<ObjectList>   ObjectListList;
    typedef std::vector<float>        FloatList;
#endif
#ifndef __cplusplus
    typedef void NodeList;
    typedef void ObjectListList;
    typedef void FloatList;
#endif

typedef struct kdTree {
#ifdef __cplusplus
    kdTree(SceneObject **objects, unsigned nObjects);
    ~kdTree();
#endif
    aabb m_aabb;

    int max_numNodes;
    int numNodes;
    kdNode         *nodes;        // array of kd-nodes
    ObjectListList *objectLists;  // array of pointers to objects

} kdTree;

// =====================================================================================================

#ifdef __cplusplus
extern "C" {
#endif

    kdTree* createKdTree(SceneObject **objects, unsigned nObjects);
    void releaseKdTree(kdTree** tree);
    kdNode* kdnode_getLeftChild(kdNode* node, kdTree* parent);
    kdNode* kdnode_getRightChild(kdNode* node, kdTree* parent);
    unsigned kdtree_intersect(Scene *scene, kdTree *tree, Ray *wo, SurfacePoint *out);

#ifdef __cplusplus
}
#endif




// =====================================================================================================


///*! Adds a node to the specified kd-tree and returns the index of the newly added node.
//    @note the node is COPIED into the kd-tree */
//int kdtree_addNode(kdNode node, kdTree *parent) {
//    if (parent->nodeCount >= parent->nodeAllocation - 1) {
//        kdNode* NODES = NEWAR(kdNode, parent->nodeAllocation * 2);
//        memcpy(NODES, parent->nodes, sizeof(kdNode)*parent->nodeAllocation); //TODO: Use realloc instead?
//        parent->nodeAllocation *= 2;
//        DELAR(parent->nodes);
//        parent->nodes = NODES;
//    }
//    parent->nodes[parent->nodeCount] = node;
//    return parent->nodeCount++;
//};

#endif
