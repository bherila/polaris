/* ---------------------------------------------- *\
   file: kdTree.C
   auth: Travis Fischer
   acct: tfischer
   date: 10/7/2006

   A kd-Tree is an axis-aligned binary space
   partitioning data structure.  Each internal
   node is split along one of the three principle
   axes at a heuristically-determined splitting
   axis in an attempt to maximize the volume of
   the empty cells.  This kd-Tree uses a Surface
   Area Heuristic (SAH) to estimate the best
   local splitting plane.
\* ---------------------------------------------- */

#include "stdafx.h"
#include "kdtree.h"
#include "pmath.h"
#include "sceneobject.h"
#include "AABB.h"
#include "camera.h"
#include "scene.h"

// static intersection routines
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#define DEBUG
#define vecdata(vec, i) ((vec).data[(i)])

kdTree::kdTree(SceneObjectList *SceneObjects) : m_bounds(NULL)
{
   m_kdStack = new kdStack();
   IAPoint min(INFINITY, INFINITY, INFINITY, 1), max(-INFINITY, -INFINITY, -INFINITY, 1);
   const int numSceneObjects = SceneObjects->size();
   m_leftBranchAABB = NULL;
   m_rightBranchAABB= NULL;
   //   for(int i = NO_BUILD_THREADS; i--;)
   //      m_branchAABB[i] = NULL;
#ifdef DEBUG
   cout << "Building kdTree..\n";
#endif
   time_t beginBuildTime = time(0);

   //SceneObject **primArray = new SceneObject*[numSceneObjects];
   const SceneObjectList &primList = *SceneObjects;
   for(int a = numSceneObjects; a--;) {
      SceneObject *curPrim = primList[a];
      //primArray[a] = curPrim;

      // ----------------------------
      // Initialize current SceneObject
      // ----------------------------
      curPrim->m_aabb = new AABB(curPrim->ctm);

#if 0
      // Scale SceneObject color intensities only once rather than once per light per intersection
      curPrim->m_SceneObjectData.ambient_r *= globalAmbientCoeff;
      curPrim->m_SceneObjectData.ambient_g *= globalAmbientCoeff;
      curPrim->m_SceneObjectData.ambient_b *= globalAmbientCoeff;

      curPrim->m_SceneObjectData.specular_r *= globalSpecularCoeff;
      curPrim->m_SceneObjectData.specular_g *= globalSpecularCoeff;
      curPrim->m_SceneObjectData.specular_b *= globalSpecularCoeff;

      curPrim->m_SceneObjectData.reflect_r *= globalReflectCoeff;
      curPrim->m_SceneObjectData.reflect_g *= globalReflectCoeff;
      curPrim->m_SceneObjectData.reflect_b *= globalReflectCoeff;

      _prim_ = curPrim;

      real_t diffuseScale = globalDiffuseCoeff;

      // Smart-loading of texture maps (if several objects are textured with
      // the same image, only load that image once by keeping a map of
      // texture names to QImage that have already been loaded)
      if (curPrim->m_SceneObjectData.textureName != "") {
         const TextureMapIterator cIterator = m_textureMap.find(curPrim->m_SceneObjectData.textureName);

         if (cIterator == m_textureMap.end()) {
            curPrim->m_texture = new CS123Image(curPrim->m_SceneObjectData.textureName.c_str());
            /*         curPrim->m_texture = new QImage();

                       if (!curPrim->m_texture->load(QString(curPrim->m_SceneObjectData.textureName.c_str()))) {
                       cerr << "Error loading texture: " << curPrim->m_SceneObjectData.textureName << endl;
                       delete curPrim->m_texture;
                       }

            //            cerr << curPrim->m_SceneObjectData.textureName << ":   " << curPrim->m_texture->width() << ", " << curPrim->m_texture->height() << endl;

            if (curPrim->m_texture->format() != QImage::Format_RGB32) {
            QImage *old = curPrim->m_texture;

            curPrim->m_texture = new QImage(old->convertToFormat(QImage::Format_RGB32));
            delete old;
            }*/
            //cerr << curPrim->m_texture->format() << " vs " << QImage::Format_RGB32 << endl;

            m_textureMap[curPrim->m_SceneObjectData.textureName] = curPrim->m_texture;
         } else {
            curPrim->m_texture = cIterator->second;
         }

         diffuseScale *= (1 - curPrim->m_SceneObjectData.blend);
         curPrim->m_SceneObjectData.blend = (curPrim->m_SceneObjectData.blend * globalDiffuseCoeff) / 255;
      }

      curPrim->m_SceneObjectData.diffuse_r *= diffuseScale;
      curPrim->m_SceneObjectData.diffuse_g *= diffuseScale;
      curPrim->m_SceneObjectData.diffuse_b *= diffuseScale;

#endif

      assert(curPrim->m_aabb);
      const real_t minX = curPrim->m_aabb->m_min[0]; // ->getMin(0);
      if (minX < vecdata(min,0)) vecdata(min,0) = minX;
      const real_t minY = curPrim->m_aabb->m_min[1];
      if (minY < vecdata(min,1)) vecdata(min,1) = minY;
      const real_t minZ = curPrim->m_aabb->m_min[2];
      if (minZ < vecdata(min,2)) vecdata(min,2) = minZ;

      const real_t maxX = curPrim->m_aabb->m_max[0];
      if (maxX > vecdata(max,0)) vecdata(max,0) = maxX;
      const real_t maxY = curPrim->m_aabb->m_max[1];
      if (maxY > vecdata(max,1)) vecdata(max,1) = maxY;
      const real_t maxZ = curPrim->m_aabb->m_max[2];
      if (maxZ > vecdata(max,2)) vecdata(max,2) = maxZ;
   }

   // Allocate a large chunk of memory in the hopes that memory allocations during the build process
   // will be avoided
   unsigned long defaultSize = numSceneObjects * 6;
   if (numSceneObjects > 50000) { // larger scenes have proportionately less kdNodes because of MAX_DEPTH
      defaultSize >>= 1;
      defaultSize -= 80000;

      if (defaultSize > 300000)
         defaultSize = 300000;
   }

   //    cout << "No nodes preallocated: " << defaultSize << '\n';
   //defaultSize <<= 3;  // sizeof(kdNode) is 8
   m_root = (kdNode*)malloc(sizeof(kdNode) * defaultSize);
   m_kdNodes = m_root;
   m_lastkdNode = m_root + (defaultSize - 4);

   m_bounds = new AABB(min, max);
   //m_root = (kdNode*)malloc(sizeof(kdNode));

   const int dummySplitAxis = !m_bounds->getLongestExtent();
   m_root->m_noSceneObjects = (numSceneObjects << 2) | dummySplitAxis;
   m_root->m_SceneObjects   = SceneObjects;//primArray;


#ifdef DEBUG
   m_maxDepth = 0;
   m_noNodes  = 0;
   m_avgLeafSceneObjects = 0;
   m_noLeafs  = 0;
   m_avgDepth = 0;

   m_noIntersectionTests = 0;
   m_noHits = 0;
   m_noBacktracks = 0;
   m_numTraversals = 0;
#endif // DEBUG

   buildTree(m_root, *m_bounds, 0);

   real_t duration = difftime(time(0), beginBuildTime);

   std::cout << "Done Building kdTree in " << duration << ((duration != 1) ? " seconds" : " second");
   std::cout << endl;
#ifdef DEBUG
   std::cout << "\n  Num SceneObjects:     " << numSceneObjects
      << "\n  Num Nodes:          " << m_noNodes
      << "\n  Maximum Depth:      " << m_maxDepth
      << "\n  Average Depth:      " << m_avgDepth / (real_t)m_noLeafs
      << "\n  Num Leaf Nodes:     " << m_noLeafs
      << "\n  Average Prims/Leaf: " << m_avgLeafSceneObjects / (real_t)m_noLeafs << '\n';
#endif // DEBUG

   //cerr << "LeftEmpty: " << leftEmpty << "  RightEmpty: " << rightEmpty << '\n';
   //m_bounds->m_min.print();
   //m_bounds->m_max.print();
}

// curNode should have the following (compact) format:
//   * m_flag denotes the previous split plane (necessary because you shouldn't split
//    the same plane twice in a row)
//   * m_SceneObjects stores SceneObjects located within the node
//   * m_noSceneObjects stores the number of SceneObjects within this node
void kdTree::buildTree(kdNode *__restrict__ curNode, AABB &aabb, unsigned int depth) {
   int noSceneObjects = KD_NO_SceneObjectS(curNode);

#ifdef DEBUG
   if (depth > m_maxDepth)
      m_maxDepth = depth;

   m_noNodes++;
#endif // DEBUG


   // Termination Criteria for leaves
   if (noSceneObjects <= KD_MIN_SceneObjectS || depth >= KD_MAX_DEPTH) {
      curNode->m_flag |= KD_LEAF_FLAG;

#ifdef DEBUG
      m_avgLeafSceneObjects += noSceneObjects;
      m_noLeafs++;
      m_avgDepth += depth;
#endif

      return;
   }



   real_t bestCost = INFINITY;
   real_t bestPos  = 0;
   const real_t myArea   = aabb.getSurfaceArea();
   //const real_t areaCOST = KD_COST_INTERSECT / myArea;
#ifdef KD_EMPTY_BIAS
   real_t bestRealCost = 0;
#endif


   // -----------------------------
   // Determine Axis to split along
   // -----------------------------

   // Note, this method of axis determination is non-optimal but is fast and
   // yields good results.  Optimally, we'd be able to try each of the two
   // possible split axes, and take the best cost between the two.
   // (only two of the three axes are possible for non-root nodes because
   // nodes cannot be split along the same axis as their parent)
   //int splitAxis = KD_SPLIT_AXIS(curNode) + 1;
   //if (splitAxis > 2) splitAxis = 0;

   /*real_t *primMin = (real_t*) alloca(sizeof(real_t) * noSceneObjects * 2);
   real_t *primMax = primMin + noSceneObjects;
   unsigned int *minIndices = (unsigned int*) alloca(sizeof(unsigned int) * noSceneObjects * 2);
   unsigned int *maxIndices = minIndices + noSceneObjects;*/

   const SceneObjectList &primList = *curNode->m_SceneObjects;
#define CHECK_ALL_SPLITS
#ifndef CHECK_ALL_SPLITS
   unsigned int splitAxis = aabb.getLongestExtent();
   if (splitAxis == KD_SPLIT_AXIS(curNode))
      splitAxis = aabb.getSecondExtent(splitAxis);
#else
   real_t prevBest = INFINITY;
   unsigned int bestAxis  = 0;
   unsigned int splitAxis = 3;
   for(; splitAxis--;) {
      if (splitAxis == KD_SPLIT_AXIS(curNode))
         continue;
#endif

      /*for(int p = noSceneObjects; p--;) {
         const SceneObject *splitPrim = primList[p];
         primMin[p] = splitPrim->getMin(splitAxis);
         primMax[p] = splitPrim->getMax(splitAxis);
      }

      radixSortIndices<real_t>(primMin, minIndices, noSceneObjects);
      radixSortIndices<real_t>(primMax, maxIndices, noSceneObjects);*/

      SplitHashMap splitHash(noSceneObjects * 2 + 1);
      //splitHash.insert((float)aabb.m_min[splitAxis]);
      //splitHash.insert((float)aabb.m_max[splitAxis]);
      splitHash[(float)vecdata(aabb.m_min, splitAxis)] = true;
      splitHash[(float)vecdata(aabb.m_max, splitAxis)] = true;

      // ----------------------------------------
      // Determine Possible Split-Plane Positions
      // ----------------------------------------

      //SceneObject **primList = curNode->m_SceneObjects;
      for(int p = noSceneObjects; p--;) {
         const SceneObject *splitPrim = primList[p];

#define SPLIT_OFFSET       (0.0000004)
         const real_t minSplit = splitPrim->m_aabb->m_min[splitAxis] - SPLIT_OFFSET;
         const real_t maxSplit = splitPrim->m_aabb->m_max[splitAxis] + SPLIT_OFFSET;

         // Check for duplicate split location possibilities
         if (splitHash.count((float)minSplit) <= 0) {
            // We haven't come across this splitPos yet, so add it to the list of
            // possibles
            unsigned int noPrimLeft = 0, noPrimRight = 0;
            for(int a = noSceneObjects; a--;) {
               const SceneObject *curPrim = primList[a];
               const real_t bbMin = curPrim->m_aabb->m_min[splitAxis];
               const real_t bbMax = curPrim->m_aabb->m_max[splitAxis];

               noPrimRight += (bbMax > minSplit);  // bbMax >= minSplit works, but degrades quality?
               noPrimLeft  += (bbMin <= minSplit);
            }

            //splitHash.insert((float)minSplit);
            splitHash[(float)minSplit] = true;
            //splitHash[(float)minSplit] = (noPrimRight << 16) | (noPrimLeft & 0x0000FFFF);// | (1 << 31);

#define USE_ACCURATE_SURFACE_AREA
#ifndef USE_ACCURATE_SURFACE_AREA
            const real_t leftArea  = aabb.getMinSurfaceArea(splitAxis, minSplit);
            const real_t rightCost = (myArea - leftArea) * noPrimRight; // div myArea
            const real_t leftCost  = (leftArea * noPrimLeft);  // div myArea
#else
            //          const real_t leftCost  = ((minSplit - aabb.m_min[splitAxis]) * noPrimLeft);  // div myArea
            //          const real_t rightCost = ((aabb.m_max[splitAxis] - minSplit) * noPrimRight); // div myArea

            real_t leftArea, rightArea;
            aabb.getMinMaxSurfaceArea(splitAxis, minSplit, leftArea, rightArea);
            const real_t leftCost  = (leftArea  * noPrimLeft);  // div myArea
            const real_t rightCost = (rightArea * noPrimRight); // div myArea
#endif
            /*const real_t leftArea  = aabb.getMinSurfaceArea(splitAxis, minSplit);
              const real_t rightArea = (myArea - leftArea);
              real_t splitCost = KD_COST_TRAVERSAL + (KD_COST_INTERSECT * leftArea * noPrimLeft / myArea) +
              (KD_COST_INTERSECT * rightArea * noPrimRight / myArea);*/

#ifdef KD_EMPTY_BIAS
            const real_t realCost = /*KD_COST_TRAVERSAL + areaCOST * */(leftCost + rightCost);
            real_t splitCost = realCost;

            // Favor split positions which produce empty voxels
            if (noPrimLeft == 0 || noPrimRight == 0)
               splitCost *= KD_EMPTY_BIAS;

            if (splitCost < bestCost) {
               bestCost = splitCost;
               bestPos  = minSplit;
               bestRealCost = realCost;
            }
#else
            // Surface Area Heuristic (SAH) Equation for determining the local,
            // relative cost of splitting a node
            //Cost(cell) = C_Trav + SA(L) * PrimCount(L) + SA(R) * PrimCount(L);
            const real_t splitCost = /*KD_COST_TRAVERSAL + areaCOST * */(leftCost + rightCost);

            if (splitCost < bestCost) {
               bestCost = splitCost;
               bestPos  = minSplit;
            }
#endif // KD_EMPTY_BIAS
         }

         // Check for duplicate split location possibilities
         if (splitHash.count((float)maxSplit) <= 0) {
            unsigned int noPrimLeft = 0, noPrimRight = 0;
            for(int a = noSceneObjects; a--;) {
               const SceneObject *curPrim = primList[a];
               const real_t bbMin = curPrim->m_aabb->m_min[splitAxis];
               const real_t bbMax = curPrim->m_aabb->m_max[splitAxis];

               noPrimRight += (bbMax > maxSplit);
               noPrimLeft  += (bbMin <= maxSplit);
            }

            //splitHash.insert((float)maxSplit);
            splitHash[(float)maxSplit] = true;
            //splitHash[(float)maxSplit] = (noPrimRight << 16) | (noPrimLeft & 0x0000FFFF);

#ifndef USE_ACCURATE_SURFACE_AREA
            const real_t leftArea  = aabb.getMinSurfaceArea(splitAxis, maxSplit);
            const real_t rightCost = (myArea - leftArea) * noPrimRight; // div myArea
            const real_t leftCost  = (leftArea  * noPrimLeft);  // div myArea
#else
            //          const real_t leftCost  = ((maxSplit - aabb.m_min[splitAxis]) * noPrimLeft);  // div myArea
            //          const real_t rightCost = ((aabb.m_max[splitAxis] - maxSplit) * noPrimRight); // div myArea

            real_t leftArea, rightArea;
            aabb.getMinMaxSurfaceArea(splitAxis, maxSplit, leftArea, rightArea);
            const real_t leftCost  = (leftArea  * noPrimLeft);  // div myArea
            const real_t rightCost = (rightArea * noPrimRight); // div myArea
#endif

            /*const real_t leftArea  = aabb.getMinSurfaceArea(splitAxis, maxSplit);
              const real_t rightArea = (myArea - leftArea); // div myArea
              real_t splitCost = KD_COST_TRAVERSAL + (KD_COST_INTERSECT * leftArea * noPrimLeft / myArea) +
              (KD_COST_INTERSECT * rightArea * noPrimRight / myArea);*/

#ifdef KD_EMPTY_BIAS
            const real_t realCost = /*KD_COST_TRAVERSAL + areaCOST * */(leftCost + rightCost);
            real_t splitCost = realCost;

            // Favor split positions which produce empty voxels
            if (noPrimLeft == 0 || noPrimRight == 0)
               splitCost *= KD_EMPTY_BIAS;

            if (splitCost < bestCost) {
               bestCost = splitCost;
               bestPos  = maxSplit;
               bestRealCost = realCost;
            }
#else
            // Surface Area Heuristic (SAH) Equation for determining the local,
            // relative cost of splitting a node
            //Cost(cell) = C_Trav + SA(L) * PrimCount(L) + SA(R) * PrimCount(L);
            const real_t splitCost = /*KD_COST_TRAVERSAL + areaCOST * */(leftCost + rightCost);

            if (splitCost < bestCost) {
               bestCost = splitCost;
               bestPos  = maxSplit;
            }
#endif // KD_EMPTY_BIAS

         }
      }

#ifdef CHECK_ALL_SPLITS
      if (bestCost != prevBest)
         bestAxis = splitAxis;
      prevBest = bestCost;
   }
   splitAxis = bestAxis;
#endif

   // ----------------------------------------------
   // Done Selecting the Position of the Split-Plane
   // ----------------------------------------------

   // If the estimated cost of slitting is greater than the cost
   // of intersecting this node as-is, then don't split and instead,
   // make this node into a leaf  (splitting is too costly)
#ifdef KD_EMPTY_BIAS
   // same as: if (costTrav + costIntersect * bestRealCost / myArea > noSceneObjects * costIntersect)
   if (/*KD_COST_RATIO + */bestRealCost > noSceneObjects * myArea)
      //if (bestRealCost > noSceneObjects * (aabb.m_max[splitAxis] - aabb.m_min[splitAxis]))
#else
      if (KD_COST_TRAVERSAL + KD_COST_INTERSECT * bestCost / myArea > noSceneObjects * KD_COST_INTERSECT)
#endif
      {

         curNode->m_flag |= KD_LEAF_FLAG;

#ifdef DEBUG
         m_avgLeafSceneObjects += noSceneObjects;
         m_noLeafs++;
#endif

         return;
      } else {  // Make this node is an internal node
         /*const unsigned int hashFlag = splitHash[(float)bestPos];
           unsigned int numRightPrims  = (hashFlag >> 16);
           unsigned int numLeftPrims   = (unsigned int)(hashFlag & 0x0000FFFF);
         //const bool minSplit = ((hashFlag & (1 << 31)) != 0);

         //#warning "combine two memory allocs into one!"

         SceneObject **leftList  = new SceneObject*[numLeftPrims+200];
         SceneObject **rightList = new SceneObject*[numRightPrims+200];
         //SceneObject **rightList = leftList + numLeftPrims;
         SceneObject **left  = leftList;
         SceneObject **right = rightList;

         unsigned int numLeft = 0, numRight = 0;*/

         SceneObjectList *leftList  = new SceneObjectList();
         SceneObjectList *rightList = new SceneObjectList();

         // Determine which SceneObjects belong in which sides of the splitting plane
         for(int a = noSceneObjects; a--;) {
            SceneObject *curPrim = primList[a];
            const real_t bbMin = curPrim->m_aabb->m_min[splitAxis];
            const real_t bbMax = curPrim->m_aabb->m_max[splitAxis];

            //if (minSplit ? (bbMax >= bestPos) : (bbMax > bestPos)) {// - EPSILON){
            if (bbMax > bestPos)
               //*right++ = curPrim;
               //numRight++;
               rightList->push_back(curPrim);

            if (bbMin <= bestPos)
               //*left++  = curPrim;
               //numLeft++;
               leftList->push_back(curPrim);
         }

         //assert(numLeft == numLeftPrims && numRight == numRightPrims);

         /*if (numLeft != numLeftPrims || numRightPrims != numRight) {
           cerr << numLeft << "  vs  " << numLeftPrims << "   " <<
           numRight << "  vs  " << numRightPrims << '\n';
           }*/

         //numLeftPrims = numLeft; numRightPrims = numRight;


         if (curNode != m_root) {
            // Free up SceneObject array since this node is not a leaf
            delete curNode->m_SceneObjects;
         }


         curNode->m_splitPos = (float)bestPos;// - EPSILON;

         //curNode->m_flag = (curNode->m_flag & ~KD_FLAG_BITS) | splitAxis;


         // Split this node into two children and recurse
         // Allocate children on 16 byte boundary (side-by-side)

         //       kdNode *leftChild;
         //       if (m_kdNodes >= m_lastkdNode) {
         //          leftChild  = (kdNode*)malloc(sizeof(kdNode) * 2);
         //       } else {
         //          m_kdNodes += 2;
         //          leftChild = m_kdNodes;
         //       }
         kdNode *leftChild = ((m_kdNodes < m_lastkdNode) ? (m_kdNodes += 2) : (kdNode*)malloc(sizeof(kdNode) * 2));

         kdNode *rightChild = leftChild + 1;
         const unsigned int numLeftPrims   = (leftList->size() << 2);
         const unsigned int numRightPrims  = (rightList->size() << 2);
         //numLeftPrims  <<= 2;
         //numRightPrims <<= 2;
         //curNode->m_flag = ((unsigned int)leftChild) | splitAxis;


#ifdef DEBUG
         /*cout << "Depth: " << depth <<
           "  numSceneObjects: " << noSceneObjects <<
           "  numLeft: " << (numLeftPrims>>2) <<
           "  numRight: " << (numRightPrims>>2) <<
           "  SplitAxis: " << splitAxis <<
           "  SplitPos: " << bestPos <<
           endl;*/
#endif

         ++depth;

         // ------------------
         // Recurse Left Child
         // ------------------

         if (numLeftPrims > 0) {
            leftChild->m_SceneObjects   = leftList;
            leftChild->m_noSceneObjects = numLeftPrims | splitAxis;

            const real_t prevMax = aabb.m_max[splitAxis];
            aabb.m_max[splitAxis] = bestPos;

            // NOTE:  cannot use pthreads with Qt for TA Demo

            /*         if (curNode == m_root) {
                       pthread_t pThread;
                       int error;

                       m_leftBranchAABB = new AABB(*m_bounds);

                       buildTree(leftChild, *aabb, depth);

#if 0
BuildkdTreeThread t = { this, leftChild, m_leftBranchAABB, depth };
if ((error = pthread_create(&pThread, 0, startBuildThread, &t))) {
cerr << "Error in pthread_create: " << strerror(error) << '\n' << "(kdTree::branchThreads)\n";
exit(1);
}

pthread_join(pThread, 0);
#endif
} else {*/
            buildTree(leftChild, aabb, depth);
            //}

            aabb.m_max[splitAxis] = prevMax;
            } else { // leftChild is empty
               delete leftList;

#ifdef DEBUG
               m_noLeafs++;
               m_avgDepth += depth;
#endif
               //leftEmpty++;

               leftChild->m_SceneObjects = NULL;
               leftChild->m_flag = KD_LEAF_FLAG; // denotes a leaf node
            }

// -------------------
// Recurse Right Child
// -------------------

if (numRightPrims > 0) {
   rightChild->m_SceneObjects   = rightList;
   rightChild->m_noSceneObjects = numRightPrims | splitAxis;

   const real_t prevMin = aabb.m_min[splitAxis];
   aabb.m_min[splitAxis]  = bestPos;

   // NOTE:  cannot use pthreads with Qt for TA Demo

   /*if (curNode == m_root) {
     pthread_t pThread;
     int error;

     m_rightBranchAABB = new AABB(*m_bounds);

     BuildkdTreeThread t = { this, rightChild, m_rightBranchAABB, depth };
     if ((error = pthread_create(&pThread, 0, startBuildThread, &t))) {
     cerr << "Error in pthread_create: " << strerror(error) << '\n' << "(kdTree::branchThreads)\n";
     exit(1);
     }

     pthread_join(pThread, 0);
     } else {*/
   buildTree(rightChild, aabb, depth);
   //}

   aabb.m_min[splitAxis] = prevMin;
} else { // rightChild is empty
   delete rightList;

#ifdef DEBUG
   m_noLeafs++;
   m_avgDepth += depth;
#endif

   //rightEmpty++;

   rightChild->m_SceneObjects = NULL;
   rightChild->m_flag = KD_LEAF_FLAG; // denotes a leaf node
}
}
}


// Returns the information regarding the closest object (if any)
// of intersection with the given Ray in the scene
SceneObject *kdTree::getIntersection(const Ray *ray, SurfacePoint *sp, unsigned threadId) {
   real_t tMin, tMax; // close to +infinity

   /*SceneObject *curPrim = _prim_;
     curPrim->m_rayID = ray->m_rayID;

     const IAPoint  &P = curPrim->ctmI * ray->origin;
     const IAVector &D = curPrim->ctmI * ray-ir;
     char norm = 0;

     const real_t t = (*primIntersects[curPrim->m_type])(P, D, norm);

     if (t < MAXreal_t) {
     ray->m_tHit = t;
     ray->m_pHit = P + t * D;
     ray->m_normalCase = norm;

     return curPrim;
     }

     return NULL;*/


   // Check if Ray completely misses bounding box of scene
   if (!m_bounds->intersectsRay(ray, tMin, tMax)) {
      return NULL;
   }
#ifdef DEBUG
   m_numTraversals++;
#endif

   //cerr << "tMin: " << tMin << "; tMax: " << tMax << endl;
   //TODO: const unsigned int rayID = ray->m_rayID;
   // "Empty" kdStack
   m_kdStack->reset();
   kdNode *curNode = m_root;
   kdNode *far;

   while(1) {
      // Traverse until we get to a leaf
      while(KD_INTERNAL_NODE(curNode)) {
         const unsigned char splitAxis = KD_SPLIT_AXIS(curNode);
         register const real_t diff = (KD_SPLIT_POS(curNode) - ray->origin[splitAxis]);
         register real_t d = diff * ray->inv_direction[splitAxis];

         /*if (d == 0) {
         //if (ray->inv_direction[splitAxis] > 0) //fabs(ray->inv_direction[splitAxis]) < EPSILON) {
         //   return (SceneObject*)ray;//_staticPrim;

         m_kdStack->push(far, tMax); // { far, d, tMax }
         //m_kdStack->push(near, tMax); // { far, d, tMax }
         //cerr << "(" << ray-ir[splitAxis] << ", " << diff << ") ";

         // continue with { near child, tMin, d }
         curNode = near;
         tMax = d;
         continue;
         //cerr << tMin << "  " << tMax << " -- ";
         }*/

         // Order near/far children relative to ray's origin
         register const bool reverseOrder = (diff <= 0) ^ (d < 0);

         //far  = near + 1 - 2 * reverseOrder;
#if 1

         curNode =  KD_LEFT_CHILD(curNode) + reverseOrder;

         if (d >= tMax)
            continue;

         far     = curNode + (reverseOrder ? -1 : 1);

         if (d <= tMin) {
            curNode = far;
         } else if (curNode->m_SceneObjects == NULL) {
            curNode = far;
            tMin = d;
         } else {
            m_kdStack->push(far, tMax); // { far, d, tMax }
            tMax = d;
         }


         //          near = KD_LEFT_CHILD(curNode) + reverseOrder;
         //          far  = near + 1 - 2 * reverseOrder;
         //          if (d <= tMin) {
         //             curNode = far;
         //          } else if (d >= tMax) {
         //             curNode = near;
         //          } else {
         //             if (near->m_SceneObjects == NULL) {//(near->m_flag) == 3 &&
         //                curNode = far;
         //                tMin = d;
         //             } else {
         //    m_kdStack->push(far, tMax); // { far, d, tMax }
         //
         //             // continue with { near child, tMin, d }
         //             curNode = near;
         //             tMax = d;
         //             }
         //          }
#else

         if (d >= tMax || d < 0) {
            // continue with { near child, tMin, tMax }
            curNode = near;
         } else if (d <= tMin) {
            // continue with { far child, tMin, tMax }
            curNode = far;
         } else {

            // Could handle empty voxels
            /*if (near->m_SceneObjects == NULL) {//(near->m_flag) == 3 &&
              curNode = far;
              tMin = d;
              } else {*/

            // Ray intersects both children, so push far
            // child and continue with near
            m_kdStack->push(far, tMax); // { far, d, tMax }

            // continue with { near child, tMin, d }
            curNode = near;
            tMax = d;
         }
#endif
         }

         // curNode must be a leaf, so check ray for intersections
         // with all SceneObjects within this leaf node
         real_t closestT_Hit = INFINITY; // close to +infinity
         SceneObject *primHit = NULL;
         //char normalCase, bestNormalCase = 0;
         IAPoint bestP;
         IAVector bestD;

         //SceneObject **primList = curNode->m_SceneObjects;
         const SceneObjectList &primList = *curNode->m_SceneObjects;
         for(int i = KD_NO_SceneObjectS(curNode); i--;) {
            SceneObject *curPrim = primList[i];

            // Only test Ray-SceneObject intersection once per ray (using a mailbox "ID")
            // if (curPrim->m_rayID != rayID) { // mailboxing
#ifdef DEBUG
               m_noIntersectionTests++;
#endif
               //const real_t t = curPrim->intersectsRay(ray);
               //curPrim->m_rayID = rayID; // mailboxing

               //if (curPrim->m_aabb->intersectsRay(ray)) {
               const IAPoint  &P = curPrim->ctmI * ray->origin; //TODO: Might be wrong.
               const IAVector &D = curPrim->ctmI * ray->direction;
               const Ray ray(P, D);
               const real_t t = curPrim->getIntersection(ray); //(*primIntersects[curPrim->m_type])(P, D, normalCase);

               if (t < closestT_Hit) {

                  // Only remember closest intersection to Ray origin:  smallest t-value greater
                  // than a predefined threshold (T_THRESHOLD); See SceneObject.h for details
                  //if (isValidT(t, closestT_Hit))
                  closestT_Hit = t;
                  primHit = curPrim;

                  //primHit->m_P = P;
                  //primHit->m_D = D;
                  //primHit->m_normalCase = normalCase;
                  bestP = P;
                  bestD = D;
                  //bestNormalCase = normalCase;
               }
               //}
            // } // mailboxing
         }

         // Early termination!
         if (primHit) {
            //primHit->m_rayID = !rayID;

            if (closestT_Hit <= tMax) {
               /*if ((closestT_Hit < tMin && fabs(closestT_Hit - tMin) > EPSILON) ||
                 (closestT_Hit > tMax && fabs(closestT_Hit - tMax) > EPSILON))
                 cout << tMin << "  <=  " << closestT_Hit << "  <=  " << tMax << endl;*/

                sp->t = closestT_Hit;
                sp->intersection = bestP + closestT_Hit * bestD;
                primHit->getNormal(sp->intersection, sp->normal);

                //sp->normal = bestNormalCase;
               //ray->m_tHit = closestT_Hit;
               //ray->m_pHit = bestP + closestT_Hit * bestD;
               //ray->m_normalCase = bestNormalCase;
#ifdef DEBUG
               m_noHits++;
#endif

               return primHit;
            }
         }

         if (m_kdStack->isEmpty())
            return NULL;

#ifdef DEBUG
         m_noBacktracks++;
#endif

         // No valid intersection found, so traverse back up tree and
         // check an alternate path

         tMin = tMax;
         m_kdStack->pop(curNode, tMax);
      }

      return NULL;
      }

      /* Predicate which returns true if the given ShadowRay intersects
         any object in the scene */
      SceneObject *kdTree::intersects(ShadowRay *__restrict__ ray, kdStack *__restrict__ m_kdStack, real_t lightDist) {
         real_t tMin, tMax;

         // Check if Ray completely misses bounding box of scene
         m_bounds->intersectsShadowRay(ray, tMin, tMax);

         if (lightDist < tMax)
            tMax = lightDist;

         // "Empty" kdStack
         m_kdStack->reset();
         //const unsigned int rayID = ray->m_rayID; //mailboxing
         kdNode *curNode = m_root;
         kdNode *near, *far;

         while(1) {
            // Traverse until we get to a leaf
            while(KD_INTERNAL_NODE(curNode)) {
               const unsigned char splitAxis = KD_SPLIT_AXIS(curNode);
               register const real_t diff = (KD_SPLIT_POS(curNode) - ray->origin[splitAxis]);
               register const real_t d = diff * ray->inv_direction[splitAxis];


               // Order near/far children relative to ray's origin
               register const bool reverseOrder = (diff <= 0) ^ (d < 0);
               near = KD_LEFT_CHILD(curNode) + reverseOrder;
               far  = near + 1 - 2 * reverseOrder;
               //far  = near + (reverseOrder ? -1 : 1);
#if 1
               if (d <= tMin) {
                  curNode = far;
               } else if (d >= tMax) {
                  curNode = near;
               } else {
                  if (near->m_SceneObjects == NULL) {//(near->m_flag) == 3 &&
                     curNode = far;
                     tMin = d;
                  } else {
                     m_kdStack->push(far, tMax); // { far, d, tMax }

                     // continue with { near child, tMin, d }
                     curNode = near;
                     tMax = d;
                  }
               }
#else
               if (d >= tMax || d < 0) {
                  // continue with { near child, tMin, tMax }
                  curNode = near;
               } else if (d <= tMin) {
                  // continue with { far child, tMin, tMax }
                  curNode = far;
               } else {
                  // Ray intersects both children, so push far
                  // child and continue with near
                  m_kdStack->push(far, tMax); // { far, d, tMax }

                  // continue with { near child, tMin, d }
                  curNode = near;
                  tMax = d;
               }
#endif
            }

            // curNode must be a leaf, so check ShadowRay for intersections
            //SceneObjectList *primList = curNode->m_SceneObjects;
            //SceneObject **primList = curNode->m_SceneObjects;
            const SceneObjectList &primList = *curNode->m_SceneObjects;
            for(int i = KD_NO_SceneObjectS(curNode); i--;) {
               SceneObject *curPrim = primList[i];//(*primList)[i];

               // Only test Ray-SceneObject intersection once per ray (using a mailbox "ID")
               //if (curPrim->m_rayID != rayID) { // mailboxing
                  //if (curPrim->intersectsRay(ray) < lightDist)
                  //   return curPrim;
                  //if (curPrim->m_aabb->intersectsShadowRay(ray)) {

                  const IAPoint  &P = curPrim->ctmI * ray->origin;
                  const IAVector &D = curPrim->ctmI * ray->direction;
                  const Ray ray(P,D);

                  if (curPrim->getIntersection(ray) < 999999) //TODO: speed
                      return curPrim;
                  //if ((*primOccludes[curPrim->m_type])(P, D, lightDist))
                  //   return curPrim;

                  //}

                  // curPrim->m_rayID = rayID; // mailbox
               // } //mailbox
            }

            // ShadowRay did not hit any SceneObjects in current leaf, so we need to backtrack
            if (m_kdStack->isEmpty())
               return NULL;

            // No valid intersection found, so traverse back up tree and
            // check an alternate path
            tMin = tMax;
            m_kdStack->pop(curNode, tMax);
         }

         return NULL;
      }

      kdTree::~kdTree()
      {
         if (m_bounds)
            delete m_bounds;
         if (m_leftBranchAABB)
            delete m_leftBranchAABB;
         if (m_rightBranchAABB)
            delete m_rightBranchAABB;

         // Delete all texture maps
         TextureMapIterator cIter = m_textureMap.begin();
         for(; cIter != m_textureMap.end(); cIter++) {
            //const string &masterName = compIter->first;
            CS123Image *texture = cIter->second;
            delete texture;
         }

         // delete all kdNodes starting with m_root
         stack<kdNode*> s;
         s.push(m_root);

         // Since child node pairs are allocated together, we have to be
         // careful to only free the left children
         stack<bool> isLeftChild;
         isLeftChild.push(false);

         do {
            kdNode *curNode = s.top();
            s.pop();
            bool shouldFree = isLeftChild.top();
            isLeftChild.pop();

            if (curNode != NULL) {
               if (KD_IS_PARTITIONED(curNode)) {
                  s.push(KD_LEFT_CHILD(curNode));
                  isLeftChild.push(true);

                  s.push(KD_RIGHT_CHILD(curNode));
                  isLeftChild.push(false);
               }
               else if (curNode != m_root) {
                  // root's SceneObjects are deleted in Scene::cleanupShapeList()
                  if (curNode->m_SceneObjects)
                     delete curNode->m_SceneObjects;
               }
               if (shouldFree && (curNode < m_root || curNode > m_lastkdNode)) {
                  if (curNode) {
                     free(curNode);
                  }
               }
            }
         } while(!s.empty());

         if (m_root)
            free(m_root);

         delete m_kdStack;
      }

#if 0
      register const bool reverseOrder = (diff <= 0);
      curNode = KD_LEFT_CHILD(curNode) + reverseOrder;

      if (d >= tMax || d < 0)
         continue;

      far = curNode + 1 - 2 * reverseOrder;
      if (d <= tMin) {
         curNode = far;
         continue;
      }

      m_kdStack.push(far, tMax);
      tMax = d;


#include "SceneObject.h"
#include "RayCanvas.h"
#include "Scene.h"
      //#ifndef USE_KD_TREE
      SceneObject *RayCanvas::getIntersectionNaive(Ray *ray) {
         const unsigned int rayID = ray->m_rayID;
         real_t closestT_Hit = MAXreal_t; // close to +infinity
         SceneObject *primHit = NULL;
         char normalCase, bestNormalCase;
         IAPoint bestP;
         IAVector bestD;

         ShapeList *primList = m_scene->m_shapeList;
         for(int i = primList->size(); i--;) {
            SceneObject *curPrim = (*primList)[i];

            // Only test Ray-SceneObject intersection once per ray (using a mailbox "ID")
            if (curPrim->m_rayID != rayID) {
               const IAPoint  &P = curPrim->ctmI * ray->origin;
               const IAVector &D = curPrim->ctmI * ray-ir;
               const real_t t = (*primIntersects[curPrim->m_type])(P, D, normalCase);
               curPrim->m_rayID = rayID;

               // Only remember closest intersection to Ray origin:  smallest t-value greater
               // than a predefined threshold (T_THRESHOLD); See Intersect.C for details
               if (t < closestT_Hit && t > EPSILON) {//isValidT(t, closestT_Hit)) {
                  closestT_Hit = t;
                  primHit = curPrim;
                  bestP = P;
                  bestD = D;
                  bestNormalCase = normalCase;
               }
               }
            }

            if (primHit) {

               ray->m_tHit = closestT_Hit;
               ray->m_pHit = bestP + closestT_Hit * bestD;
               ray->m_normalCase = bestNormalCase;
            }

            ray->m_tHit = closestT_Hit;
            //(-10.5,-2.5,-4.5,1)
            //(7.5,2.5,4.5,1)

            /*if (primHit) {
            //cout.precision(2);

            if ((closestT_Hit < _tMin && fabs(closestT_Hit - _tMin) > EPSILON) ||
            (closestT_Hit > _tMax && fabs(closestT_Hit - _tMax) > EPSILON)) {
            //if (closestT_Hit < _tMin || closestT_Hit > _tMax) {
            //ray->origin.print();
            //ray-ir.print();
            static unsigned int __n = 0;
            __n++;
            cout << __n << ")   " << _tMin << "  <=  " << closestT_Hit << "  <=  " << _tMax << endl;

            }
            }*/

            return primHit;
         }

#endif

