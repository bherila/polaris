HEADERS += matrix.h \
    camera.h \
    ray.h \
    renderer.h \
    image.h \
    vector.h \
    sampler.h \
    mtwist.h \
    scene.h \
    scenes.h \
    rayslope.h
SOURCES += scene.c \
    matrix.c \
    camera.c \
    renderer.c \
    main.c \
    vector.c \
    sampler.c \
    mtwist.c \
    scenes.c \
    rayslope.cpp
INCLUDEPATH += /usr/include/lam
INCLUDEPATH += tinyxml \
    cs123 \
    /usr/include/mpi \
    thirdparty/gd-2.0.34
LIBS += -lm

QMAKE_CC = mpicc
QMAKE_CXX = mpiCC
QMAKE_LINK = mpiCC
QMAKE_CXXFLAGS_RELEASE -= -O2 -O1
QMAKE_CXXFLAGS_RELEASE += -O3 \
    -funroll-loops \
    -funswitch-loops
QMAKE_CFLAGS_RELEASE -= -O2 -O1
QMAKE_CFLAGS_RELEASE += -O3 \
    -funroll-loops \
    -funswitch-loops
