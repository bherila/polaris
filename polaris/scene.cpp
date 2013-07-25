#include "scene.h"
#include "spatialdatastructure.h"
#include "CS123XmlSceneParser.h"
#include "material.h"

#include <assert.h>
#include <iostream>
#include <string>

using namespace std;
SurfacePoint::SurfacePoint() :
    incoming(Ray(Vector4(0,0,0,0), Vector4(0,0,0,0)))
{

}

Scene::Scene() :m_spatialaccel(NULL), m_camera(NULL) {

}

Scene::~Scene() {
    for (unsigned i = 0; i < m_objects.size(); ++i) {
        delete m_objects[i];
    }
    delete m_spatialaccel;
}

void makeSceneItem() {

}

void parseCS123Tree(Scene *scene, Matrix4 xtm, const CS123SceneNode* root) {
    Matrix4 ctm(xtm);
    unsigned N;

    N = root->transformations.size();
    for (unsigned i = 0; i < N; ++i) {
        Matrix4 nextMatrix;
        const CS123SceneTransformation *t = root->transformations[i];
        switch (t->type) {
            case TRANSFORMATION_TRANSLATE: nextMatrix = Matrix4::Translation(t->translate); break;
            case TRANSFORMATION_SCALE:     nextMatrix = Matrix4::Scale(VEC_4TO3(t->scale)); break;
            case TRANSFORMATION_ROTATE:    nextMatrix = Matrix4::Rotation(Point3(0, 0, 0, 1),VEC_4TO3(t->rotate), t->angle); break;
            case TRANSFORMATION_MATRIX:    nextMatrix = t->matrix; break;
        }
        ctm = ctm * nextMatrix;
    }

    N = root->primitives.size();
    for (unsigned i = 0; i < N; ++i) {
        SceneObject *o;
        CS123ScenePrimitive &p = *(root->primitives[i]);
        switch (p.type) {
        case PRIMITIVE_CONE:
            o = new Cone();
            break;
        case PRIMITIVE_CUBE:
            o = new Cube();
            break;
        case PRIMITIVE_CYLINDER:
            cout << "Warning: PRIMITIVE_CYLINDER not supported." << endl;
            break;
        case PRIMITIVE_MESH:
            cout << "Warning: PRIMITIVE_MESH not supported." << endl;
            break;
        case PRIMITIVE_SPHERE:
            o = new Sphere();
            break;
        case PRIMITIVE_TORUS:
            cout << "Warning: PRIMITIVE_TORUS not supported." << endl;
            break;
        default:
            cout << "Warning: Unknown primitive type." << endl;
            break;
        }
        o->ctmI  = ctm;
        o->ctm   = ctm.getInverse();

        Material *m = new LambertianMaterial(CONV_COLOR3(p.material.cDiffuse));
        o->setMaterial(m, true);

//        // is this object an emitter?
//        CS123SceneColor ke = p.material.cEmissive;
//        if (ke.r > 0 || ke.g > 0 || ke.b > 0) {
//            Emitter *em     = new Emitter(); // TOOD: Make sure this is eventually deleted
//            em->attenuation = COLOR3_BLACK; //TODO: attenuation
//            em->power       = CONV_COLOR3(ke);
//            o->m->emitter   = em;
//        }

        if (o) scene->addObject(o);
    }
    
    N = root->children.size();
    for (unsigned i = 0; i < N; ++i) {
        const CS123SceneNode* child = root->children[i];
        parseCS123Tree(scene, ctm, child);
    }
}

void Scene::parseFile(const string &filename){
//    Sphere *s1 = new Sphere();
//    s1->ctm = (Matrix4::Translation(Vector4(-0.3, 0, -1.5, 0)) * Matrix4::Scale(Vector3(.25,.25,.25))).getInverse();
//    s1->setMaterial(new LambertianMaterial(Color3(0,0,1)), true);
//    m_objects.push_back(s1);
//
//    Sphere *s2 = new Sphere();
//    s2->ctm = (Matrix4::Translation(Vector4(0.15, .2, -1.5, 0)) * Matrix4::Scale(Vector3(.25,.25,.25))).getInverse();
//    s2->setMaterial(new LambertianMaterial(Color3(1,1,0)), true);
//    m_objects.push_back(s2);
//
//    Sphere *s3 = new Sphere();
//    s3->ctm = (Matrix4::Translation(Vector4(.3, 0, -1.5, 0)) * Matrix4::Scale(Vector3(.25,.25,.25))).getInverse();
//    s3->setMaterial(new LambertianMaterial(Color3(0,1,0)), true);
//    m_objects.push_back(s3);
//
//    Sphere *s4 = new Sphere();
//    s4->ctm = (Matrix4::Translation(Vector4(0, .4, -1.5, 0)) * Matrix4::Scale(Vector3(.25,.25,.25))).getInverse();
//    s4->setMaterial(new LambertianMaterial(Color3(1,0,1)), true);
//    m_objects.push_back(s4);
//
//    Sphere *s5 = new Sphere();
//    s5->ctm = (Matrix4::Translation(Vector4(-0.15, .2, -1.5, 0)) * Matrix4::Scale(Vector3(.25,.25,.25))).getInverse();
//    //s5->setMaterial(new LambertianMaterial(Color3(1,0,0)), true);
//    {
//        PhongMaterial *p = new PhongMaterial(Color3(1,0,0));
//        p->kd = 0.5;
//        p->ks = 0.5;
//        s5->setMaterial(p, true);
//    }
//    m_objects.push_back(s5);
//
//    Cube *c1 = new Cube(); //floor
//    c1->ctm = (Matrix4::Translation(Vector4(0, -.125, -3, 0)) * Matrix4::Scale(Vector3(5,.01,10))).getInverse();
//    c1->setMaterial(new LambertianMaterial(Color3(1,1,1)), true);
//    m_objects.push_back(c1);
//
//    Cube *c2 = new Cube(); //front wall
//    c2->ctm = (Matrix4::Translation(Vector4(0, 0, -2.5, 0)) * Matrix4::Scale(Vector3(20,20,.01))).getInverse();
//    c2->setMaterial(new LambertianMaterial(Color3(1,1,1)), true);
//    m_objects.push_back(c2);
//
///*
//    Cube *c3 = new Cube(); //right wall
//    c3->ctm = (Matrix4::Translation(Vector4(3.5, 0, 0, 0)) * Matrix4::Scale(Vector3(.01,20,20))).getInverse();
//    c3->m->kd = Color3(1, 1, 1);
//    m_objects.push_back(c3);
//
//    Cube *c4 = new Cube(); //left wall
//    c4->ctm = (Matrix4::Translation(Vector4(-3.5, 0, 0, 0)) * Matrix4::Scale(Vector3(.01,20,20))).getInverse();
//    c4->m->kd = Color3(1, 1, 1);
//    m_objects.push_back(c4);
//
//    Cube *c5 = new Cube(); //back wall
//    c5->ctm = (Matrix4::Translation(Vector4(0, 0, 0, 0)) * Matrix4::Scale(Vector3(20,20,.01))).getInverse();
//    c5->m->kd = Color3(1, 1, 1);
//    m_objects.push_back(c5);
//*/
//    Cube *c6 = new Cube(); //Ceiling
//    c6->ctm = (Matrix4::Translation(Vector4(0, 2, -3, 0)) * Matrix4::Scale(Vector3(5,.01,10))).getInverse();
//    //c6->material->kd = Color3(1, 1, 1);
//    c6->setMaterial(new LambertianMaterial(Color3(1,1,1)), true);
//    m_objects.push_back(c6);
//
//    Sphere *emitter = new Sphere();
//    {
//        LambertianMaterial *m = new LambertianMaterial(COLOR3_WHITE);
//        m->emitter.attenuation = Color3(.45, 0.00002, 0.00001);
//        m->emitter.power       = Color3(2,2,2);
//        m->emitter.isEmissive = true;
//
//        emitter->material = m;
//        emitter->ctm = (Matrix4::Translation(Vector4(0, 0, -0.5, 0))* Matrix4::Scale(Vector3(.25,.25,.25))).getInverse();
//    }
//    m_objects.push_back(emitter);


    CS123XmlSceneParser sp(filename);
    if (sp.parse()) {
        { // load objects
            CS123SceneNode *root = sp.getRootNode();
            parseCS123Tree(this, Matrix4::Identity(), root);
        }
        { // load camera
          // note: camera is filled in now, but initialized later in Scene::init
            CS123SceneCameraData cd;
            if (sp.getCameraData(cd)) {
                delete m_camera;
                m_camera = new PerspectiveCamera();
                m_camera->setAspectRatio(cd.aspectRatio);
                m_camera->setFar(cd.focalLength);
                m_camera->setNear(0.01);
                m_camera->setFar(100);
                m_camera->orient(cd.pos, Vector3(cd.look.data), Vector3(cd.up.data));
            }
        }
        { // load lights
            unsigned NL = sp.getNumLights();
            CS123SceneLightData ld;
            for (unsigned i = 0; i < NL; ++i) {
                sp.getLightData(i, ld);
                if (ld.type == LIGHT_POINT) {
                    Sphere *s = new Sphere();
                    LambertianMaterial *m = new LambertianMaterial(COLOR3_WHITE);
                    m->emitter.attenuation = Color3(.45, 0.1, 0.001);
                    m->emitter.power       = CONV_COLOR3(ld.color);
                    m->emitter.isEmissive  = true;
                    s->setMaterial(m, true);
                    s->ctmI =  Matrix4::Translation(ld.pos);
                    s->ctm  = s->ctmI.getInverse();
                    m_objects.push_back(s);
                }
                else {
                    cout << "WARNING: Unsupported light type." << endl;
                }
            }
        }
    }


}

void Scene::addObject(SceneObject* object) {
    m_objects.push_back(object);
}

void Scene::init(){
    //m_filmToWorld = m_camera->getFilmToWorldMat();
    assert(m_camera);
    m_camera->init();
    for(unsigned i = 0; i < m_objects.size(); ++i) {
        SceneObject *obj = m_objects[i];
        obj->ctmI = obj->ctm.getInverse();
        if(obj->isEmitter()) m_emitters.push_back(obj);
        obj->init();
    }

    //TODO: rebuild kd-tree
    delete m_spatialaccel;
    m_spatialaccel = new SceneList(&m_objects); //kdTree(&m_objects);
}

bool Scene::getNearestObject(const Ray &r, SurfacePoint &intersect, RenderThread *thread) {
    //return m_spatialaccel->findIntersection(r, intersect);
    return m_spatialaccel->getIntersection(&r, &intersect, thread);
}

Ray Scene::getWorldRay(real_t filmx, real_t filmy) {
    assert(m_camera);
    //assert(filmx >= 0 && filmx <= 1);
    //assert(filmy >= 0 && filmy <= 1);
    assert(m_camera->getPosition().data[3] == 1);

    const Vector4 ndc(2 * filmx - 1,  1 - 2 * filmy,  -1,  1);
    const Vector4 direction = m_camera->getFilmToWorldMat() * ndc - m_camera->getPosition();
    assert(direction[3] == 0);
    return Ray(m_camera->getPosition(),direction);
}

