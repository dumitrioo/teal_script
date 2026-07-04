QT       -= core
QT       -= gui
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++20 -march=native
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

# DEFINES += TEALSCRIPT_NO_EIGEN
# DEFINES += USE_CUSTOM_MEMORY_ALLOCATION

# DEFINES += TEAL_USE_CUSTOM_SHARED_MUTEX
# DEFINES += RW_MUTEX_PRIORITIES
# DEFINES += RW_MUTEX_UPGRADEABLE

LIBS += -lraylib

INCLUDEPATH += ../../src \
    ../../3rd/bullet3/src

SOURCES += main.cpp \
    ../../3rd/bullet3/src/btBulletCollisionAll.cpp \
    ../../3rd/bullet3/src/btBulletDynamicsAll.cpp \
    ../../3rd/bullet3/src/btLinearMathAll.cpp

DISTFILES += \
    ../pidreg.teal
