QT       -= core
QT       -= gui

TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++20 -fPIC -march=znver4 -Wno-unused-function
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

SOURCES += \
    libmain.cpp
