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

// Just a regular C++ class to be added as an <<object type>> to the scripting runtime
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

    if(args.size() < 2) {
        return 0;
    }

    // The runtime
    scfx::runtime rt{};


    // The host part of scripting language possibilities extending example.
    // For usage, see script "examples/extending_example.scfx".

    // -----------------------------------------------------------------------------------
    // Example of adding function to the runtime
    rt.add_function("hello_from_cpp",
        SCFXFUN(args) {
            std::cout << "C++ extension function hello_from_cpp() called with arguments:" << std::endl;
            for(auto &&a: args) {
                std::cout << "\t" << a << std::endl;
            }
            return args.size();
        }
    );
    // -----------------------------------------------------------------------------------

    // -----------------------------------------------------------------------------------
    // Example of adding named value to the runtime
    rt.add_var("The_Answer_to_the_Ultimate_Question_of_Life_the_Universe_and_Everything", 42);
    // -----------------------------------------------------------------------------------

    // -----------------------------------------------------------------------------------
    // Example of adding object type to the runtime
    rt.add_function("example_object",
        SCFXFUN(args) {
            if(args.size() > 0) {
                return scfx::valbox{example_object{args[0].cast_to_s32()}, "example_object"};
            }
            return scfx::valbox{example_object{}, "example_object"};
        }
    );
    rt.add_method("example_object", "set_val", SCFXFUN(args) {
        // check number of arguments, when needed, including
        // implicit object reference as the first arg
        SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
        SCFXTHIS(args, example_object).set_val(args[1].cast_to_s32());
        return 0;
    });
    rt.add_method("example_object", "get_val", SCFXFUN(args) {
        SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
        return SCFXTHIS(args, example_object).get_val();
    });
    // -----------------------------------------------------------------------------------

    // -----------------------------------------------------------------------------------
    // Example of adding extension to the runtime
#ifdef SCFX_USE_ZMQ
    zmq_ext zmq{};
    // The extension for ZeroMQ is used in "examples/ex_srv.scfx" and "examples/ex_cli.scfx" scripts
    zmq.register_runtime(&rt);
#endif
    // One more extension
#ifdef SCFX_USE_RAYLIB
    ray_ext ray{};
    // The extension for RayLib is used in "examples/alu74181.scfx" script
    ray.register_runtime(&rt);
#endif
    // -----------------------------------------------------------------------------------


#ifndef SCFX_DEBUGGING
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
            } else {
                throw std::runtime_error{args[i] + " - no such file or directory"};
            }
        }
        rt.loading_complete();

        if(rt.worker_cells_count() == 0) {
            throw std::runtime_error{"nothing to do - no working elements"};
        }

#ifdef SINGLE_THREADED_SCFX
        while(!rt.termination_requested()) {
            rt.run_cycle();
        }
#else
        rt.run_mt(std::thread::hardware_concurrency());
        while(!rt.wait(0.1)) {}
        if(rt.failure()) { throw std::runtime_error{rt.failure_description()}; }
#endif

#ifndef SCFX_DEBUGGING
    } catch(std::exception const &e) {
        std::cerr << "error: " << e.what() << std::endl;
    }
#endif

    return rt.exit_status();
}
