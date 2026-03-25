QT       -= core
QT       -= gui
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

###############################################################################
# DEFINES += SINGLE_THREADED_SCFX
# DEFINES += SCFX_DEBUGGING
###############################################################################

###############################################################################
# DEFINES += SCFX_USE_ASYNC_CONSOLE
###############################################################################

###############################################################################
# DEFINES += SCFX_USE_CUSTOM_MUTEX
DEFINES += MUTEX_COPYABLE_WITHOUT_ACTUAL_COPYING
DEFINES += MUTEX_ATOMIC_SM_SLEEP_NANOS=100000
###############################################################################

###############################################################################
# DEFINES += SCFX_USE_CUSTOM_SHARED_MUTEX
DEFINES += RW_MUTEX_PRIORITIES
# DEFINES += RW_MUTEX_UPGRADEABLE
DEFINES += RW_MUTEX_COPYABLE_WITHOUT_ACTUAL_COPYING
DEFINES += RW_MUTEX_ATOMIC_SM_SLEEP_NANOS=100000
###############################################################################

###############################################################################
DEFINES += USE_FILE_MAGIC
DEFINES += SCFX_USE_ZMQ
DEFINES += SCFX_USE_RAYLIB
###############################################################################

###############################################################################
DEFINES += SCFX_USE_EMHASH8_MAP
###############################################################################

###############################################################################
#DEFINES += STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
###############################################################################

INCLUDEPATH += ../../src

SOURCES += main.cpp

HEADERS += \
    ../../src/ext/array_buffer_ext.hpp \
    ../../src/ext/containers_ext.hpp \
    ../../src/ext/socket_ext.hpp \
    ../../src/inc/base16.hpp \
    ../../src/inc/base64.hpp \
    ../../src/inc/base85.hpp \
    ../../src/inc/bit_util.hpp \
    ../../src/inc/commondefs.hpp \
    ../../src/inc/containers/circular_buffer.hpp \
    ../../src/inc/containers/static_buffer.hpp \
    ../../src/inc/crypto/keccak.hpp \
    ../../src/inc/crypto/sha256.hpp \
    ../../src/inc/crypto/sha3_512.hpp \
    ../../src/inc/crypto/sha512.hpp \
    ../../src/inc/emhash/hash_set2.hpp \
    ../../src/inc/emhash/hash_set3.hpp \
    ../../src/inc/emhash/hash_set4.hpp \
    ../../src/inc/emhash/hash_set8.hpp \
    ../../src/inc/emhash/hash_table5.hpp \
    ../../src/inc/emhash/hash_table6.hpp \
    ../../src/inc/emhash/hash_table7.hpp \
    ../../src/inc/emhash/hash_table8.hpp \
    ../../src/inc/emhash/lru_size.h \
    ../../src/inc/emhash/lru_time.h \
    ../../src/inc/file_util.hpp \
    ../../src/inc/fsm_tokenizer.hpp \
    ../../src/inc/hash/adler.hpp \
    ../../src/inc/json.hpp \
    ../../src/inc/lzari.hpp \
    ../../src/inc/math/math_util.hpp \
    ../../src/inc/math/matrix4.hpp \
    ../../src/inc/math/vector4.hpp \
    ../../src/inc/mt_synchro.hpp \
    ../../src/inc/net/net_utils.hpp \
    ../../src/inc/net/socket_poller.hpp \
    ../../src/inc/net/socket_wrapper.hpp \
    ../../src/inc/sequence_generator.hpp \
    ../../src/inc/serialization.hpp \
    ../../src/inc/so.hpp \
    ../../src/inc/str_util.hpp \
    ../../src/inc/sys_util.hpp \
    ../../src/inc/timespec_wrapper.hpp \
    ../../src/inc/unicode_operations.hpp \
    ../../src/ext/cpu_ext.hpp \
    ../../src/ext/crypto_ext.hpp \
    ../../src/ext/file_ext.hpp \
    ../../src/ext/math_ext.hpp \
    ../../src/ext/rand_ext.hpp \
    ../../src/ext/time_ext.hpp \
    ../../src/scaflux_cells.hpp \
    ../../src/scaflux_codegen.hpp \
    ../../src/scaflux_exec_ctx.hpp \
    ../../src/scaflux_expr.hpp \
    ../../src/scaflux_interfaces.hpp \
    ../../src/scaflux_lexer.hpp \
    ../../src/scaflux_parser.hpp \
    ../../src/scaflux_runtime.hpp \
    ../../src/scaflux_statement.hpp \
    ../../src/scaflux_token.hpp \
    ../../src/scaflux_util.hpp \
    ../../src/scaflux_value.hpp \
    optext/ray_ext.hpp \
    optext/zmq_ext.hpp

QMAKE_CXXFLAGS += -std=c++20 -march=native -Wno-unused-parameter -Wno-unused-function -Wl,-rpath,.
QMAKE_CXXFLAGS += -ftree-vectorize -mavx2 -ftree-vectorizer-verbose=5
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

LIBS += -lpthread -ldl
LIBS += -lz
LIBS += -lmagic
LIBS += -lraylib
LIBS += -lzmq

DISTFILES += \
    ../alu74181.scfx \
    ../array_buffer_test.scfx \
    ../draft.scfx \
    ../ex_cli.scfx \
    ../ex_srv.scfx \
    ../example.scfx \
    ../alu74181.png \
    ../extending_example.scfx \
    ../pid_regulator.scfx \
    ../quad_eq.scfx \
    ../sockets_server.scfx \
    ../tbbt_2cola.scfx \
    ../tests.scfx \
    ../unix_socket.scfx
