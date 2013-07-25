#include "kdtree.h"
#include "ray.h"
#include "rayslope.h"
#include <string.h>
#include <vector>

// =====================================================================================================

//void list_init(List* lst, int size) {
//    lst->nAllocatedSize = size;
//    lst->objects        = ARRAY(void*, size);
//    lst->nCount         = 0;
//};
//
//int list_pushBack(List* heap, void* ptr) {
//    assert(heap->nCount < heap->nAllocatedSize);
//    heap->objects[heap->nCount] = ptr;
//    return heap->nCount++;
//};
//
//void list_removeAt(List* heap, int index) {
//    heap->objects[index] = NULL;
//};

// =====================================================================================================

void insertSplitPos(FloatList *splitPositions, float pos) {
    unsigned i;
    for (i = 0; i < splitPositions->size(); ++i)
        if (splitPositions->at(i) == pos)
            return;
    splitPositions->push_back(pos);
}

void subdivideNode(kdTree *tree, kdNode *a_node, aabb *a_Box, unsigned depth) {
    assert(a_node->isLeaf);
    ObjectList &l = tree->objectLists->at(a_node->value);
    assert(l.size() > 0);

    unsigned i, axis;
    Vector4 aabb_min = a_Box->min, aabb_max = a_Box->max, s;

    for (i = 0; i < 4; ++i)
        s.data[i] = abs(aabb_min.data[i] - aabb_max.data[i]);

    if (s.x >= s.y && s.x >= s.z)
        axis = 0;
    else if (s.y >= s.x && s.y >= s.z)
        axis = 1;
    else
        axis = 2;

    float p1, p2;
    float pos1 = aabb_min.data[axis];
    float pos2 = aabb_max.data[axis];
    bool *pright = new bool[l.size()];
    float *eleft = new float[l.size()];
    float *eright = new float[l.size()];
    ObjectList parray(l.size());
    int aidx = 0;

    FloatList *splist = new FloatList();

    // determine split positions
    for (i = 0; i < l.size(); ++i) {
        SceneObject *p = parray[aidx] = l.at(i);
        pright[aidx] = true;
        calc_aabb(p, &aabb_min, &aabb_max);
        eleft[aidx] = aabb_min.data[axis];
        eright[aidx] = aabb_max.data[axis];
        //TODO: We could compute p1 and p2 differently if sphere or mesh, etc.
        p1 = eleft[aidx];
        p2 = eright[aidx];
        aidx++;
        if (p1 >= pos1 && p1 <= pos2) insertSplitPos(splist, p1);
        if (p2 >= pos1 && p2 <= pos2) insertSplitPos(splist, p2);
    }

    // determine n1count / n2count for each split position
    int *n1counts = new int[splist->size()];
    int *n2counts = new int[splist->size()];
    memset(n1counts, 0, sizeof(int) * splist->size()); // reset all to zero
    memset(n2counts, 0, sizeof(int) * splist->size());

    aabb b1, b2, b3, b4;
    b1 = b2 = b3 = b4 = *a_Box;
    float b3p1 = b3.min.data[axis];
    float b4p2 = b4.max.data[axis];

    for (i = 0; i < splist->size(); ++i) {
        float splitpos = splist->at(i);
        b4.min.data[axis] = splitpos;
        b4.max.data[axis] = pos2;
        b3.max.data[axis] = splitpos + b3.min.data[axis];
        float b3p2 = b3.max.data[axis];
        float b4p1 = b4.min.data[axis];
        for (unsigned j = 0; j < l.size(); ++j) {
            if (pright[i]) {
                const SceneObject *p = l.at(j);
                if (eleft[j] <= b3p2 && eright[j] >= b3p1 && b3.intersectsPrimitive(p))
                        n1counts[i]++;
                if (eleft[j] <= b4p2 && eright[i] >= b4p1 && b4.intersectsPrimitive(p))
                        n2counts[i]++;
                else
                    pright[i] = false;
            }
            else
                n1counts[i]++;

        }//splist = splist->next;

    }
    delete[] pright;

    // calculate surface area for current node
    float SAV = 0.5f / ( a_Box->w() * a_Box->d() + a_Box->w() * a_Box->h() + a_Box->d() * a_Box->h());

    // calculate cost for not splitting
    float Cleaf = l.size() * 1.0f;

    // determine optimal split plane position
    float lowcost = 10000;
    float bestpos = 0;
    for (i = 0; i < splist->size(); ++i) {
        // calculate child node extends
        float splitpos = splist->at(i);
        b4.min.data[axis] = splitpos;
        b4.max.data[axis] = pos2;
        b3.max.data[axis] = splitpos;

        // calculate child node cost
        float SA1 = 2 * (b3.w() * b3.d() + b3.w() * b3.h() + b3.d() * b3.h());
        float SA2 = 2 * (b4.w() * b4.d() + b4.w() * b4.h() + b4.d() * b4.h());
        float splitcost = 0.3f + 1.0f * (SA1 * SAV * n1counts[i] + SA2 * SAV * n2counts[i]);

        // update best cost tracking variables
        if (splitcost < lowcost) {
            lowcost = splitcost;
            bestpos = splitpos;
            b1 = b3, b2 = b4;
        }
    }

    // construct child nodes only if it's worth it
    ObjectList left, right;
    int n1count = 0, n2count = 0, total = 0;

    if (lowcost >= Cleaf) goto ret;

    { // determine which objects go in which child *********************
        float b1p1 = b1.min.data[axis];
        float b2p2 = b2.max.data[axis];
        float b1p2 = b1.max.data[axis];
        float b2p1 = b2.min.data[axis];
        for (i = 0; i < l.size(); ++i) {
            SceneObject *p = l.at(i);
            ++total;
            if ((eleft[i] < b1p2) && (eright[i] > b1p1) && (b1.intersectsPrimitive(p))) {
                left.push_back(p);
                n1count++;
            }
            else if ((eleft[i] < b2p2) && (eright[i] > b2p1) && (b2.intersectsPrimitive(p))) {
                right.push_back(p);
                n2count++;
            }
            else {
                assert(0);
            }
        }
    }

    for (unsigned i = 0; i < depth*2; ++i) printf(" ");
    printf("[ %d  %d ] split axis = %d, aabb = [%f, %f], bestpos = %f\n", n1count, n2count, axis,
           a_Box->min.data[axis], a_Box->max.data[axis], bestpos);
    for (unsigned i = 0; i < depth*2; ++i) printf(" "); printf("Stuff in this node: \n");
    for (unsigned j = 0; j < l.size(); ++j) {
        for (unsigned i = 0; i < depth*2+4; ++i) printf(" ");
        calc_aabb(l.at(j), &b3.min, &b3.max);
        vec4_vec4_add(&b3.min, &b3.max, &b3.min);
        vec4_scalar_mult(&b3.min, 0.5, &b3.min);
        vec4_printf(&b3.min);
    }

    { // create children **********************************************
        int l1 = tree->objectLists->size();
        int l2 = l1+1;
        tree->objectLists->push_back(left);
        tree->objectLists->push_back(right);

        l.clear(); // the list is no longer needed but we still need its index reserved

        a_node->isLeaf = false;
        a_node->value  = tree->numNodes; // the left child we're about to add
        assert(a_node->value > 0);

        kdNode *child1 = tree->nodes + tree->numNodes++; // new node
        kdNode *child2 = tree->nodes + tree->numNodes++; // new node

        child1->isLeaf = child2->isLeaf = true;
        child1->value = l1;
        child2->value = l2;

        // recur on the newly-added children ****************************
        assert(child1 != a_node && child2 != a_node);
        assert(a_node->value > 0);
        if (!child1->isLeaf) { printf("child 1 (index %d) is not leaf! depth = %d", a_node->value  , depth); exit(1); }
        if (!child1->isLeaf) { printf("child 2 (index %d) is not leaf! depth = %d", a_node->value+1, depth); exit(1); }

        if (depth < 3) {
            if (n1count > 2) subdivideNode(tree, child1, &b1, depth+1);
            if (n2count > 2) subdivideNode(tree, child2, &b2, depth+1);
        }
    } // **************************************************************

ret:
    delete[] eleft;
    delete[] eright;
    delete   splist;

}

#include <list>
unsigned kdtree_intersect(Scene *scene, kdTree *tree, ray *wo, SurfacePoint *out) {
    kdNode *cur = tree->nodes;
    sray ray;
    aabb bbox = tree->m_aabb;
    ray_to_sray(wo, &ray); //TODO: PERF!!!
    assert(slope(&ray, &bbox));
    std::list<kdNode*> nodes_to_check;
    //while(!cur->isLeaf) {
    while (nodes_to_check.size() > 0) {
        cur = nodes_to_check.pop_front();

        // intersect with the children and recur
        int axis = cur->splitAxis;
        float sp = cur->splitPos;
        kdNode *lc = tree->nodes + cur->value; //&tree->nodes->at(cur->value);
        kdNode *rc = tree->nodes + cur->value + 1;

        aabb rbox = bbox;
        aabb lbox = bbox;
        lbox.max.data[axis] = sp;
        rbox.min.data[axis] = sp;

        if (slope(&ray, &lbox)) {
            cur = lc;
        }
        else if (slope(&ray, &rbox)) {
            cur = rc;
        }
        else {
            return 0; // no intersection
        }
    }
    // we're in the leaf node, now look for an object that we intersect with
    ObjectList &list = tree->objectLists->at(cur->value);
    return calc_nearest_intersection2(scene, list.data(), list.size(), wo, out);
}

kdTree::kdTree(SceneObject **objects, unsigned nObjects) {
    max_numNodes = 1000000;
    nodes = new kdNode[max_numNodes];
    objectLists = new ObjectListList();

    // create the root node
    kdNode *root = nodes; // get root node
    root->isLeaf = 1;

    ObjectList lst;
    for (unsigned i = 0; i < nObjects; ++i)
        lst.push_back(objects[i]);
    this->objectLists->push_back(lst);
    root->value  = 0; // object list 0

    numNodes = 1;

    calc_aabb_from_list(objects, nObjects, &m_aabb.min, &m_aabb.max);

    //subdivideNode(this, root, &m_aabb, 0);
}

kdTree::~kdTree() {
    delete nodes;
    delete objectLists;
}



// C Functions ===========================

kdTree* createKdTree(SceneObject **objects, unsigned nObjects) {
    kdTree *tree = new kdTree(objects, nObjects);
    return tree;
};

void releaseKdTree(kdTree** tree) {
    delete *tree;
    *tree = 0;
};


kdNode* kdnode_getLeftChild(kdNode* node, kdTree* parent) {
    return parent->nodes + node->value;
};

kdNode* kdnode_getRightChild(kdNode* node, kdTree* parent) {
    // &parent->nodes->at(node->value + 1);
    return parent->nodes + node->value + 1;
};


