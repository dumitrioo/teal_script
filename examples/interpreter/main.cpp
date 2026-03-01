#include <clocale>
#include <vector>
#include <thread>
#include <filesystem>
#include <iostream>

#include <scaflux_runtime.hpp>

#ifdef SCFX_USE_ZMQ
#include "optext/zmq_ext.hpp"
#endif
#ifdef SCFX_USE_RAYLIB
#include "optext/ray_ext.hpp"
#endif

class example_object {
public:
    example_object() = default;
    example_object(int v): v_{v} {}
    void set_val(int v) { v_ = v; }
    int get_val() const { return v_; }

private:
    int v_{};
};

int main(int argc, char **argv) {
    std::vector<std::string> args{argv, argv + argc};
    std::setlocale(LC_ALL, "en_US.UTF-8");

    if(args.size() == 1) {
        return 0;
    }

    scfx::runtime rt{};

    // example of adding a function to the runtime
    rt.add_function("hello_from_cpp", [](scfx::valbox const &/*fname*/, std::vector<scfx::valbox> &args) -> scfx::valbox {
        std::cout << "C++ extension function hello_from_cpp() called" << std::endl
                  << "the function arguments:" << std::endl;
        for(auto &&a: args) {
            std::cout << "\t" << a << std::endl;
        }
        return args.size();
    });
    // ----------------------------------------------

    // example of adding a named value to the runtime
    rt.add_var("The_Answer_to_the_Ultimate_Question_of_Life_the_Universe_and_Everything", 42);
    // ----------------------------------------------

    // example of adding an object type to the runtime
    rt.add_function("example_object", [](scfx::valbox const &/*fname*/, std::vector<scfx::valbox> &args) -> scfx::valbox {
        if(args.size() > 0) {
            return scfx::valbox{example_object{args[0].cast_to_s32()}, "example_object"};
        }
        return scfx::valbox{example_object{}, "example_object"};
    });
    rt.add_method("example_object", "set_val", SCFXFUN(/*fname*/, args) {
        SCFX_CHCK_FUN_PARMS_NUM_EQ(2) // check number of arguments, if needed, including implicit object reference as the first arg
        SCFXTHIS(example_object).set_val(args[1].cast_to_s32());
        return 0;
    });
    rt.add_method("example_object", "get_val", SCFXFUN(/*fname*/, args) {
        SCFX_CHCK_FUN_PARMS_NUM_EQ(1)
        return SCFXTHIS(example_object).get_val();
    });
    // ----------------------------------------------

    // examples of adding an extension to the runtime
#ifdef SCFX_USE_ZMQ
    zmq_ext zmq{};
    zmq.register_runtime(&rt);
#endif
#ifdef SCFX_USE_RAYLIB
    ray_ext ray{};
    ray.register_runtime(&rt);
#endif
    // ----------------------------------------------

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

        if(rt.worker_cells_count() == 0) {
            std::cerr << "warning: nothing to do - none of working elements" << std::endl;
            return 0;
        }

        try {
            // in some of the example scripts, there input cell for the host application's
            // command line argsuments is defined under the "command_line_args" identifier
            // so let's set it
            rt.set_input("command_line_args", args);
        } catch(...) {
            // but if is not defined, we don't care
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

    return rt.exit_status();
}
