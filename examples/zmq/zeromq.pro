QT       -= core
QT       -= gui

TEMPLATE = lib

###############################################################################
# DEFINES += TEAL_DEBUGGING
###############################################################################

###############################################################################
#add_definitions(-DTEAL_USE_ASYNC_CONSOLE)
###############################################################################

###############################################################################
# DEFINES += TEAL_USE_CUSTOM_MUTEX
###############################################################################

###############################################################################
# DEFINES += TEAL_USE_CUSTOM_SHARED_MUTEX
DEFINES += RW_MUTEX_PRIORITIES
# DEFINES += RW_MUTEX_UPGRADEABLE
DEFINES += RW_MUTEX_COPYABLE_WITHOUT_ACTUAL_COPYING
# DEFINES += RW_MUTEX_ATOMIC_SM_SLEEP_NANOS=100000
###############################################################################

###############################################################################
DEFINES += TEAL_USE_EMHASH8_MAP
###############################################################################

###############################################################################
DEFINES += STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
###############################################################################

QMAKE_CXXFLAGS += -std=c++20 -fPIC -Wno-unused-function
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

SOURCES += \
    libmain.cpp

LIBS += -lzmq
