#include <commondefs.hpp>
#include <dlib.hpp>
#include <timespec_wrapper.hpp>
#include <file_util.hpp>
#include <bit_util.hpp>
#include <sys_util.hpp>
#include <sequence_generator.hpp>
#include <base85.hpp>
#include <math/math_util.hpp>
#include <containers/circular_buffer.hpp>

#include <scaflux_runtime.hpp>
#ifdef SCFX_USE_ZMQ
#include <zmq_ext.hpp>
#endif
#ifdef SCFX_USE_RAYLIB
#include <ray_ext.hpp>
#endif

int main(int argc, char **argv) {
    std::vector<std::string> args{argv, argv + argc};
    std::setlocale(LC_ALL, "en_US.UTF-8");
    std::signal(SIGPIPE, SIG_IGN);

    scfx::runtime rt{};

#ifdef SCFX_USE_ZMQ
    scfx::zmq_ext zmq{};
    zmq.register_runtime(&rt);
#endif
#ifdef SCFX_USE_RAYLIB
    scfx::ray_ext ray{};
    ray.register_runtime(&rt);
#endif

    if(args.size() >= 2 && scfx::file_util::file_exists(args[1])) {
#ifndef DEBUG_SCFX_RUN_CYCLE
        try {
#endif
            for(std::size_t i = 1; i < args.size(); ++i) {
                rt.load_file(args[i]);
            }
            rt.loading_complete();

            try { rt.set_input("command_line_args", args); } catch(...) {}

#ifdef SINGLE_THREADED_SCFX
            while(!rt.termination_requested()) {
                rt.run_cycle();
            }
#else
            rt.run_mt(std::thread::hardware_concurrency());
            while(!rt.termination_requested()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
#endif

#ifndef DEBUG_SCFX_RUN_CYCLE
        } catch(std::exception const &e) {
            std::cerr << "error: " << e.what() << std::endl;
        }
#endif
    }

    return rt.exit_status();
}
