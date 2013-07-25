#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/sampler.o \
	${OBJECTDIR}/mtwist.o \
	${OBJECTDIR}/matrix.o \
	${OBJECTDIR}/renderer.o \
	${OBJECTDIR}/vector.o \
	${OBJECTDIR}/camera.o \
	${OBJECTDIR}/kdtree.o \
	${OBJECTDIR}/scenes.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/scene.o

# C Compiler Flags
CFLAGS=-pg

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/cpolaris

dist/Debug/GNU-Linux-x86/cpolaris: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cpolaris ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/sampler.o: nbproject/Makefile-${CND_CONF}.mk sampler.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/sampler.o sampler.c

${OBJECTDIR}/mtwist.o: nbproject/Makefile-${CND_CONF}.mk mtwist.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/mtwist.o mtwist.c

${OBJECTDIR}/matrix.o: nbproject/Makefile-${CND_CONF}.mk matrix.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/matrix.o matrix.c

${OBJECTDIR}/renderer.o: nbproject/Makefile-${CND_CONF}.mk renderer.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/renderer.o renderer.c

${OBJECTDIR}/vector.o: nbproject/Makefile-${CND_CONF}.mk vector.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/vector.o vector.c

${OBJECTDIR}/camera.o: nbproject/Makefile-${CND_CONF}.mk camera.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/camera.o camera.c

${OBJECTDIR}/kdtree.o: nbproject/Makefile-${CND_CONF}.mk kdtree.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/kdtree.o kdtree.c

${OBJECTDIR}/scenes.o: nbproject/Makefile-${CND_CONF}.mk scenes.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/scenes.o scenes.c

${OBJECTDIR}/main.o: nbproject/Makefile-${CND_CONF}.mk main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/scene.o: nbproject/Makefile-${CND_CONF}.mk scene.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/scene.o scene.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/cpolaris

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
