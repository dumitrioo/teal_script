QT       -= core
QT       -= gui

TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++20
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

INCLUDEPATH += ../../src \
    ../../include

SOURCES += \
    libmain.cpp

