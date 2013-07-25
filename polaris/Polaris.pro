# -------------------------------------------------
# Project created by QtCreator 2010-04-16T10:32:56
# -------------------------------------------------
TARGET = Polaris
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += spatialdatastructure.cpp \
    sceneobject.cpp \
    scene.cpp \
    renderer.cpp \
    main.cpp \
    camera.cpp \
    stats.cpp \
    tinyxml/tinyxmlparser.cpp \
    tinyxml/tinyxml.cpp \
    tinyxml/tinystr.cpp \
    cs123/CS123XmlSceneParser.cpp \
    renderthread.cpp \
    randistrs.c \
    mtwist.c \
    AABB.cpp \
    kdtree.cpp \
    material.cpp
HEADERS += spatialdatastructure.h \
    sceneobject.h \
    scene.h \
    renderer.h \
    pmath.h \
    camera.h \
    stats.h \
    tinyxml/tinyxml.h \
    tinyxml/tinystr.h \
    cs123/CS123XmlSceneParser.h \
    cs123/CS123SceneData.h \
    cs123/CS123ISceneParser.h \
    cs123/CS123Compatibility.h \
    renderthread.h \
    randistrs.h \
    mtwist.h \
    AABB.h \
    kdtree.h \
    material.h \
    stdafx.h
DEFINES += TIXML_USE_STL QT DEBUG
INCLUDEPATH += tinyxml \
    cs123 \
    /usr/include/mpi \
    thirdparty/gd-2.0.34
LIBS += -Lthirdparty/gd-2.0.34/.libs/ -lgd

#QMAKE_CC = mpicc
#QMAKE_CXX = mpiCC
#QMAKE_LINK = mpiCC

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 \
    -funroll-loops \
    -funswitch-loops
