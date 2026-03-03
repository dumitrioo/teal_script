#pragma once

#include "include/commondefs.hpp"
#include "include/emhash/hash_set8.hpp"

#include "scaflux_util.hpp"
#include "scaflux_value.hpp"

namespace scfx {

    class runtime_interface {
    public:
        virtual ~runtime_interface() = default;
        virtual dict_map_t<std::string, valbox> *global_constants_dictionary() = 0;
        virtual dict_map_t<std::string, valbox> *global_functions_dictionary() = 0;
        virtual dict_map_t<std::string, dict_map_t<std::string, valbox>> *global_methods_dictionary() = 0;
        virtual std::function<bool(std::string)> const &user_functions_search() = 0;
        virtual std::function<valbox(std::vector<valbox> &)> const &user_function_selector() = 0;
        virtual valbox get_input(std::string const &) = 0;
        virtual valbox const &get_output(std::string const &) = 0;
        virtual void set_input(std::string const &, valbox const &) = 0;
        virtual void set_output(std::string const &, valbox const &) = 0;
        virtual void clear_input(std::string const &) = 0;
        virtual void clear_output(std::string const &) = 0;
        virtual void clear_inputs() = 0;
        virtual void clear_outputs() = 0;

        virtual void add_function(std::string const &, std::function<valbox(std::vector<valbox> &)>) = 0;
        virtual void remove_function(std::string const &) = 0;
        virtual void add_var(std::string const &, valbox const &) = 0;
        virtual void remove_var(std::string const &) = 0;
        virtual void add_method(std::string const &/*class_name*/, std::string const &/*method_name*/, std::function<valbox(std::vector<valbox> &)>) = 0;
        virtual void remove_method(std::string const &/*class_name*/, std::string const &/*method_name*/) = 0;
    };

    class extension_interface {
    public:
        virtual ~extension_interface() {}
        virtual void register_runtime(runtime_interface *rt) = 0;
        virtual void unregister_runtime() = 0;
    };

}

extern "C" scfx::extension_interface *create_scfx_extension();
extern "C" void remove_scfx_extension(scfx::extension_interface *);
