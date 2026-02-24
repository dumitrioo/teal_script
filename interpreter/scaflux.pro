QT       -= core
QT       -= gui
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# DEFINES += SINGLE_THREADED_SCFX
# DEFINES += DEBUG_SCFX_RUN_CYCLE

DEFINES += USE_FILE_MAGIC
DEFINES += SCFX_USE_ZMQ
DEFINES += SCFX_USE_RAYLIB

INCLUDEPATH += ../src \
    ../include \
    ../ext

SOURCES += main.cpp

HEADERS += \
    ../ext/ray_ext.hpp \
    ../ext/zmq_ext.hpp \
    ../include/base16.hpp \
    ../include/base64.hpp \
    ../include/base85.hpp \
    ../include/bit_util.hpp \
    ../include/commondefs.hpp \
    ../include/containers/circular_buffer.hpp \
    ../include/containers/static_buffer.hpp \
    ../include/crypto/keccak.hpp \
    ../include/crypto/sha256.hpp \
    ../include/crypto/sha3_512.hpp \
    ../include/crypto/sha512.hpp \
    ../include/dlib.hpp \
    ../include/file_util.hpp \
    ../include/file_util_magic_mgc.hpp \
    ../include/fsm_tokenizer.hpp \
    ../include/hash/adler.hpp \
    ../include/json.hpp \
    ../include/lzari.hpp \
    ../include/math/math_util.hpp \
    ../include/math/matrix4.hpp \
    ../include/math/vector4.hpp \
    ../include/sequence_generator.hpp \
    ../include/serialization.hpp \
    ../include/str_util.hpp \
    ../include/sys_util.hpp \
    ../include/timespec_wrapper.hpp \
    ../include/unicode_operations.hpp \
    ../src/ext/cpu_ext.hpp \
    ../src/ext/crypto_ext.hpp \
    ../src/ext/file_ext.hpp \
    ../src/ext/math_ext.hpp \
    ../src/ext/rand_ext.hpp \
    ../src/ext/time_ext.hpp \
    ../src/scaflux_cells.hpp \
    ../src/scaflux_codegen.hpp \
    ../src/scaflux_exec_ctx.hpp \
    ../src/scaflux_expr.hpp \
    ../src/scaflux_interfaces.hpp \
    ../src/scaflux_lexer.hpp \
    ../src/scaflux_parser.hpp \
    ../src/scaflux_runtime.hpp \
    ../src/scaflux_statement.hpp \
    ../src/scaflux_token.hpp \
    ../src/scaflux_util.hpp \
    ../src/scaflux_value.hpp

QMAKE_CXXFLAGS += -march=znver4 -std=c++20 -Wno-unused-function -Wl,-rpath,.
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

LIBS += -lpthread -ldl
LIBS += -lz-ng
LIBS += -lmagic
LIBS += -lraylib
LIBS += -lzmq

DISTFILES += \
    ../examples/alu74181.scfx \
    ../examples/ex_cli.scfx \
    ../examples/ex_srv.scfx \
    ../examples/example.scfx \
    ../examples/alu74181.png \
    ../examples/fractal/fractal.scfx \
    ../examples/quad_eq.scfx \
    ../examples/scp.scfx \
    ../examples/tbbt_2cola.scfx \
    ../examples/tests.scfx
