/*
 *
 * Header File Containing Scene Data Structs
 *
 * @author nli
 */

#ifndef __CS123_SCENE_DATA__
#define __CS123_SCENE_DATA__

/* Includes */
#include "CS123Compatibility.h"

using namespace std;

/* Enumeration for types */
enum LightType {
   LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT, LIGHT_AREA
};

enum PrimitiveType {
   PRIMITIVE_CUBE, PRIMITIVE_CONE, PRIMITIVE_CYLINDER, 
   PRIMITIVE_TORUS, PRIMITIVE_SPHERE, PRIMITIVE_MESH
};

enum TransformationType {
   TRANSFORMATION_TRANSLATE, TRANSFORMATION_SCALE, 
   TRANSFORMATION_ROTATE, TRANSFORMATION_MATRIX
};

//! Struct to store a RGBA color in floats [0,1)
struct CS123SceneColor 
{
   float r;
   float g;
   float b;
   float a;
};

//! Scene global color coefficients
struct CS123SceneGlobalData 
{
   float ka;  // ambient
   float kd;  // diffuse
   float ks;  // specular
   float kt;  // transparent
};

//! Data for a single light
struct CS123SceneLightData 
{
   int id;
   LightType type;

   CS123SceneColor	color;
   Vector3 function;

   Point3  pos;			//Not applicable to directional lights
   Vector4 dir;			//Not applicable to point lights

   float radius;			//Only applicable to spot lights
   float penumbra;			//Only applicable to spot lights
   float angle;			//Only applicable to spot lights

   float width, height;	//Only applicable to area lights 
};

//! Data for scene camera
struct CS123SceneCameraData
{
   Point3  pos;
   Vector4 look;
   Vector4 up;

   float heightAngle;
   float aspectRatio;

   float aperture;		//Only applicable for depth of field
   float focalLength;	//Only applicable for depth of field
};

//! Data for file maps (ie: texture maps)
struct CS123SceneFileMap
{
   bool isUsed;
   string filename;
   float repeatU;
   float repeatV;
};

//! Data for scene materials
struct CS123SceneMaterial 
{
   CS123SceneColor cDiffuse;
   CS123SceneColor cAmbient;
   CS123SceneColor cReflective;
   CS123SceneColor cSpecular;
   CS123SceneColor cTransparent;
   CS123SceneColor cEmissive;

   CS123SceneFileMap* textureMap;
   float blend;

   CS123SceneFileMap* bumpMap;

   float shininess;

   float ior;				//index of refaction
};

//! Data for a single primitive.
struct CS123ScenePrimitive
{
   PrimitiveType type;
   string meshfile;		//Only applicable to meshes
   CS123SceneMaterial material;
};

//! Data for transforming a scene object.  
/*! Aside from the TransformationType, the remaining of the data in the
  struct is mutually exclusive */
struct CS123SceneTransformation
{
   TransformationType type;

   /* Translate type */
   Vector4 translate;

   /* Scale type */
   Vector4 scale;

   /* Rotate type */
   Vector4 rotate;
   float angle;		// in radians

   /* Matrix type */
   Matrix4x4 matrix;
};

//! Structure for non-primitive scene objects
struct CS123SceneNode
{

   /* Transformation at this node */
   std::vector<CS123SceneTransformation*> transformations;

   /* Primitives at this node */
   std::vector<CS123ScenePrimitive*> primitives;

   /* Children of this node */
   std::vector<CS123SceneNode*> children;
};

#endif

