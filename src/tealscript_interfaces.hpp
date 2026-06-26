#pragma once

#include "inc/commondefs.hpp"

#include "tealscript_util.hpp"
#include "tealscript_value.hpp"

namespace teal {

    constexpr std::string_view OPERATOR_PLUS                 = "+";
    constexpr std::string_view OPERATOR_MINUS                = "-";
    constexpr std::string_view OPERATOR_MUL                  = "*";
    constexpr std::string_view OPERATOR_DIV                  = "/";
    constexpr std::string_view OPERATOR_MOD                  = "%";
    constexpr std::string_view OPERATOR_EQUAL                = "==";
    constexpr std::string_view OPERATOR_NOT                  = "!";
    constexpr std::string_view OPERATOR_NOTEQUAL             = "!=";
    constexpr std::string_view OPERATOR_LESSEQUAL            = "<=";
    constexpr std::string_view OPERATOR_LESS                 = "<";
    constexpr std::string_view OPERATOR_GREATER              = ">";
    constexpr std::string_view OPERATOR_GREATEREQUAL         = ">=";
    constexpr std::string_view OPERATOR_SPACESHIP            = "<=>";
    constexpr std::string_view OPERATOR_ASSIGN               = "=";
    constexpr std::string_view OPERATOR_ADDASSIGN            = "+=";
    constexpr std::string_view OPERATOR_SUBASSIGN            = "-=";
    constexpr std::string_view OPERATOR_MULASSIGN            = "*=";
    constexpr std::string_view OPERATOR_DIVASSIGN            = "/=";
    constexpr std::string_view OPERATOR_MODASSIGN            = "%=";
    constexpr std::string_view OPERATOR_BITANDASSIGN         = "&=";
    constexpr std::string_view OPERATOR_XORASSIGN            = "^=";
    constexpr std::string_view OPERATOR_BITORASSIGN          = "|=";
    constexpr std::string_view OPERATOR_LSHIFTASSIGN         = "<<=";
    constexpr std::string_view OPERATOR_RSHIFTASSIGN         = ">>=";
    constexpr std::string_view OPERATOR_PREFIX_INCREMENT     = "prefix++";
    constexpr std::string_view OPERATOR_POSTFIX_INCREMENT    = "postfix++";
    constexpr std::string_view OPERATOR_PREFIX_DECREMENT     = "prefix--";
    constexpr std::string_view OPERATOR_POSTFIX_DECREMENT    = "postfix--";
    constexpr std::string_view OPERATOR_LSHIFT               = "<<";
    constexpr std::string_view OPERATOR_RSHIFT               = ">>";
    constexpr std::string_view OPERATOR_BITNOT               = "~";
    constexpr std::string_view OPERATOR_XOR                  = "^";
    constexpr std::string_view OPERATOR_BITOR                = "|";
    constexpr std::string_view OPERATOR_OR                   = "||";
    constexpr std::string_view OPERATOR_BITAND               = "&";
    constexpr std::string_view OPERATOR_AND                  = "&&";

    constexpr std::string_view OPERATOR_BRACKETS             = "[]";

    constexpr std::string_view OPERATOR_CAST_TO_BOOL         = "(bool)";
    constexpr std::string_view OPERATOR_CAST_TO_CHAR         = "(char)";
    constexpr std::string_view OPERATOR_CAST_TO_S8           = "(s8)";
    constexpr std::string_view OPERATOR_CAST_TO_U8           = "(u8)";
    constexpr std::string_view OPERATOR_CAST_TO_S16          = "(s16)";
    constexpr std::string_view OPERATOR_CAST_TO_U16          = "(u16)";
    constexpr std::string_view OPERATOR_CAST_TO_WCHAR        = "(wchar)";
    constexpr std::string_view OPERATOR_CAST_TO_S32          = "(s32)";
    constexpr std::string_view OPERATOR_CAST_TO_U32          = "(u32)";
    constexpr std::string_view OPERATOR_CAST_TO_S64          = "(s64)";
    constexpr std::string_view OPERATOR_CAST_TO_U64          = "(u64)";
    constexpr std::string_view OPERATOR_CAST_TO_F32          = "(f32)";
    constexpr std::string_view OPERATOR_CAST_TO_F64          = "(f64)";
    constexpr std::string_view OPERATOR_CAST_TO_FLOAT        = "(float)";
    constexpr std::string_view OPERATOR_CAST_TO_VEC4         = "(vec4)";
    constexpr std::string_view OPERATOR_CAST_TO_MAT4         = "(mat4)";
    constexpr std::string_view OPERATOR_CAST_TO_ARRAY        = "(array)";
    constexpr std::string_view OPERATOR_CAST_TO_OBJECT       = "(object)";
    constexpr std::string_view OPERATOR_CAST_TO_STRING       = "(string)";
    constexpr std::string_view OPERATOR_CAST_TO_WSTRING      = "(wstring)";

    struct obj_services {
        std::function<std::optional<std::string>(valbox const &)> serializer{nullptr};
        std::function<valbox(std::string const &, std::string const &)> deserializer{nullptr};
        std::function<valbox(valbox const &)> stringify{nullptr};
        str_map_t<std::function<valbox(valbox &, valbox &)>> binops{};
        str_map_t<std::function<valbox(valbox &)>> unops{};
    };

    enum class network_address_family: int {
        unspecified,
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        unix_socket = 1,
#endif
        inet4 = 2,
        inet6 = 10,
    };

    class runtime_interface {
    public:
        virtual ~runtime_interface() = default;
        virtual str_map_t<valbox> const *global_constants_dictionary() const = 0;
        virtual str_map_t<valbox> const *global_functions_dictionary() const = 0;
        virtual str_map_t<str_map_t<valbox>> const *global_methods_dictionary() const = 0;
        virtual std::function<bool(std::string)> const &user_functions_search() const = 0;
        virtual std::function<valbox(std::vector<valbox> &)> const &user_function_selector() const = 0;
        virtual valbox get_input(std::string const &) = 0;
        virtual valbox const &get_output(std::string const &) = 0;
        virtual void set_input(std::string const &, valbox const &) = 0;
        virtual void set_output(std::string const &, valbox const &) = 0;
        virtual void clear_input(std::string const &) = 0;
        virtual void clear_output(std::string const &) = 0;
        virtual void clear_inputs() = 0;
        virtual void clear_outputs() = 0;
        virtual bool programmatic_termination_enabled() const = 0;
        virtual void set_exit_status(int) = 0;
        virtual void terminate() = 0;
        virtual void unterminate() = 0;
        virtual bool termination_requested() const = 0;
        virtual bool undefined_inputs_enabled() const = 0;
        virtual bool except_on_out_of_range_or_field() const = 0;

        virtual void add_object_serializer(
            std::string const &,
            std::function<std::optional<std::string>(valbox const &)> const &
        ) = 0;
        virtual void add_object_deserializer(
            std::string const &,
            std::function<valbox(std::string const &, std::string const &)> const &
        ) = 0;
        virtual void add_object_stringifier(
            std::string const &,
            std::function<valbox(valbox const &)> const &
        ) = 0;
        virtual void add_object_unary_operation(
            std::string const &, std::string const &,
            std::function<valbox(valbox &)> const &
        ) = 0;
        virtual void add_object_binary_operation(
            std::string const &, std::string const &,
            std::function<valbox(valbox &, valbox &)> const &
        ) = 0;
        virtual void remove_object_services(std::string const &) = 0;
        virtual obj_services const *get_object_services(std::string const &) const = 0;

        virtual void add_function(
            std::string const &,
            std::function<valbox(std::vector<valbox> &)>
        ) = 0;
        virtual void remove_function(std::string const &) = 0;
        virtual void add_var(std::string const &, valbox const &) = 0;
        virtual void remove_var(std::string const &) = 0;
        virtual void add_method(
            std::string const &/*class_name*/,
            std::string const &/*method_name*/,
            std::function<valbox(std::vector<valbox> &)>
        ) = 0;
        virtual void remove_method(
            std::string const &/*class_name*/,
            std::string const &/*method_name*/
        ) = 0;

        // expose values to network directly
        virtual bool start_net_server(
            network_address_family /*af*/,
            std::string const &/*bind_addr*/,
            std::uint16_t /*port*/,
            long double /*stale_connections_removal_timeout*/
        ) = 0;
        virtual void stop_net_server() = 0;
        virtual bool net_server_running() const = 0;
        virtual void set_external_cells_update_interval(long double /*seconds*/) = 0;
        virtual long double external_cells_update_interval() const = 0;

        // connect to a common network for exchanging values (alternative to the above)
        virtual void net_hub_connect(
            std::string const &/*host_addr*/,
            std::uint16_t /*port*/, std::string const &/*unique_net_name*/
        ) = 0;
    };

    class extension_interface {
    public:
        virtual ~extension_interface() {}
        virtual void register_runtime(runtime_interface *rt) = 0;
        virtual void unregister_runtime() = 0;
    };

}

extern "C" teal::extension_interface *create_teal_extension();
extern "C" void remove_teal_extension(teal::extension_interface *);
