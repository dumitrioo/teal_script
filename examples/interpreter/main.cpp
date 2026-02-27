#include <clocale>
#include <vector>
#include <thread>
#include <filesystem>

#include "../../src/scaflux_runtime.hpp"

#ifdef SCFX_USE_ZMQ
#include "../ext/zmq_ext.hpp"
#endif
#ifdef SCFX_USE_RAYLIB
#include "../ext/ray_ext.hpp"
#endif

int main(int argc, char **argv) {
    std::vector<std::string> args{argv, argv + argc};
    std::setlocale(LC_ALL, "en_US.UTF-8");

    scfx::runtime rt{};

#ifdef SCFX_USE_ZMQ
    zmq_ext zmq{};
    zmq.register_runtime(&rt);
#endif
#ifdef SCFX_USE_RAYLIB
    ray_ext ray{};
    ray.register_runtime(&rt);
#endif

    if(args.size() >= 2) {
#ifndef DEBUG_SCFX_RUN_CYCLE
        try {
#endif
            for(std::size_t i{1}; i < args.size(); ++i) {
                if(std::filesystem::is_regular_file(args[i])) {
                    rt.load_file(args[i]);
                } else if(std::filesystem::is_directory(args[i])) {
                    for(auto const &dir_entry: std::filesystem::recursive_directory_iterator{args[i]}) {
                        if(dir_entry.is_regular_file()) {
                            rt.load_file(dir_entry.path());
                        }
                    }
                }
            }
            rt.loading_complete();

            try {
                // in some scripts, the input cell for command line args is defined
                // under the  "command_line_args" identifier...
                rt.set_input("command_line_args", args);
            } catch(...) {
                // but if not, we don't care
            }

#ifdef SINGLE_THREADED_SCFX
            while(!rt.termination_requested()) {
                rt.run_cycle();
            }
#else
        rt.run_mt(std::thread::hardware_concurrency());
        while(!rt.wait(0.1));
#endif

#ifndef DEBUG_SCFX_RUN_CYCLE
        } catch(std::exception const &e) {
            std::cerr << "error: " << e.what() << std::endl;
        }
#endif
    }

    return rt.exit_status();
}
