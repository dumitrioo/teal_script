#pragma once

#include "inc/commondefs.hpp"
#include "inc/emhash/hash_set8.hpp"

#include "tealscript_util.hpp"
#include "tealscript_value.hpp"
#include "tealscript_token.hpp"
#include "tealscript_interfaces.hpp"

namespace teal {

    class teal_function_not_found: public runtime_error {
    public:
        teal_function_not_found(std::int64_t l, std::int64_t c, const std::string &msg):
            runtime_error{l, c, msg}
        {
        }
    };
    class teal_identifier_not_found: public runtime_error {
    public:
        teal_identifier_not_found(std::int64_t l, std::int64_t c, const std::string &msg):
            runtime_error{l, c, msg}
        {
        }
    };
    class teal_global_identifier_exists: public runtime_error {
    public:
        teal_global_identifier_exists(std::int64_t l, std::int64_t c, const std::string &msg):
            runtime_error{l, c, msg}
        {
        }
    };

    class function_definition;

    class execution_context {
    public:
        void set_runtime_interface(runtime_interface *rt_ptr) {
            rt_ptr_ = rt_ptr;
        }

        void set_return_result(valbox const &res) {
            return_result_ = res;
        }

        void request_return() {
            return_requested_ = 1;
        }

        void clear_return_request() {
            return_requested_ = 0;
        }

        bool return_requested() const {
            return return_requested_ != 0;
        }

        valbox const &return_result() const & {
            return return_result_;
        }


        void request_continue() {
            continue_requested_ = 1;
        }

        void clear_continue_request() {
            continue_requested_ = 0;
        }

        bool continue_requested() const {
            return continue_requested_ != 0;
        }


        void request_break() {
            break_requested_ = 1;
        }

        void clear_break_request() {
            break_requested_ = 0;
        }

        bool break_requested() const {
            return break_requested_ != 0;
        }

        bool some_jump_requested() const {
            return return_requested_ != 0 || break_requested_ != 0 || continue_requested_ != 0;
        }

        void clear_all_jumps_request() {
            return_requested_ = 0;
            break_requested_ = 0;
            continue_requested_ = 0;
        }

        void set_stack_barrier() {
            stack_barriers_.push_back(stack_ptr_);
        }

        void clear_stack_barrier() {
            if(stack_barriers_.empty()) {
                return;
            }
            stack_barriers_.pop_back();
        }

        int64_t stack_barrier() const {
            return stack_barriers_.empty() ? -1 : stack_barriers_.back();
        }

        void new_stack_frame() {
            while((int64_t)stack_.size() <= stack_ptr_ + 1) {
                stack_.emplace_back();
            }
            ++stack_ptr_;
        }

        void del_stack_frame() {
            if(stack_ptr_ < 0) {
                return;
            }
            stack_[stack_ptr_--].clear();
        }

        void clear_stack_hard() {
            stack_.clear();
            stack_ptr_ = -1;
            stack_barriers_.clear();
            clear_frame_ignore_stack();
        }

        void clear_stack_soft() {
            while(stack_ptr_ >= 0) {
                stack_[stack_ptr_].clear();
                --stack_ptr_;
            }
            stack_barriers_.clear();
            clear_frame_ignore_stack();
        }

        void push_frame_ignore() {
            function_depth_++;
        }

        void pop_frame_ignore() {
            function_depth_--;
        }

        void clear_frame_ignore_stack() {
            function_depth_ = 0;
        }

        bool is_inside_function() const {
            return function_depth_ > 0;
        }

        void set_local_value(std::string const &name, valbox const &val) {
            stack_[stack_ptr_].put(name, val);
        }

        enum class obj_type {
            unknown,
            stack_var,
            global_var,
            user_fun,
            global_fun,
            method,
        };
        valbox find_val_by_sym_name(std::string const &name, int64_t l, int64_t c, obj_type &objtyp) {
            switch(objtyp) {
                case obj_type::user_fun:
                    if((rt_ptr_->user_functions_search())(name)) {
                        return valbox{rt_ptr_->user_function_selector(), name, true};
                    }
                    break;
                case obj_type::global_fun: {
                    str_map_t<valbox>::iterator gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
                        if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                            return gvd_it->second.clone();
                        }
                    }
                    break;
                case obj_type::stack_var: {
                        valbox res{valbox_no_initialize::dont_do_it};
                        int64_t stb{-2};
                        for(int64_t i{stack_ptr_}; i >= 0; --i) {
                            if(stb != -2 && i <= stb) {
                                break;
                            }
                            if(stack_[i].get(name, res)) {
                                return res;
                            }
                            if(stb == -2) {
                                stb = stack_barrier();
                            }
                        }
                        if(create_if_not_exists()) {
                            valbox res{};
                            stack_[stack_ptr_].put(name, res);
                            return res;
                        }
                    }
                    break;
                case obj_type::global_var: {
                        str_map_t<valbox>::iterator gvd_it{rt_ptr_->global_constants_dictionary()->find(name)};
                        if(gvd_it != rt_ptr_->global_constants_dictionary()->end()) {
                            return gvd_it->second.clone();
                        }
                    }
                    break;
                default:
                    break;
            }

            objtyp = obj_type::unknown;
            valbox res{valbox_no_initialize::dont_do_it};
            int64_t stb{-2};
            for(int64_t i{stack_ptr_}; i >= 0; --i) {
                if(stb != -2 && i <= stb) {
                    break;
                }
                if(stack_[i].get(name, res)) {
                    objtyp = obj_type::stack_var;
                    return res;
                }
                if(stb == -2) {
                    stb = stack_barrier();
                }
            }
            if(create_if_not_exists()) {
                valbox res{};
                stack_[stack_ptr_].put(name, res);
                objtyp = obj_type::stack_var;
                return res;
            }
            str_map_t<valbox>::iterator gvd_it{rt_ptr_->global_constants_dictionary()->find(name)};
            if(gvd_it != rt_ptr_->global_constants_dictionary()->end()) {
                objtyp = obj_type::global_var;
                return gvd_it->second.clone();
            }
            gvd_it = rt_ptr_->global_functions_dictionary()->find(name);
            if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                objtyp = obj_type::global_fun;
                return gvd_it->second.clone();
            }
            if((rt_ptr_->user_functions_search())(name)) {
                objtyp = obj_type::user_fun;
                return valbox{rt_ptr_->user_function_selector(), name, true};
            }
            throw teal_identifier_not_found{l, c,
                std::string{"identifier \""} + name + "\" not found"
            };
        }

        bool is_rt_func_selector(valbox const &fn) const {
            std::function<valbox(std::vector<valbox> &)> const &f1{fn.as_func()};
            std::function<valbox(std::vector<valbox> &)> const &f2{rt_ptr_->user_function_selector()};
            auto s1{sizeof(f1)};
            auto s2{sizeof(f2)};
            bool res{false};
            if(s1 == s2) {
                res = memcmp(&f1, &f2, s1) == 0;
            }
            return res;
        }

        valbox find_func(std::string const &name, obj_type &sym_type) const {
            if(sym_type == obj_type::user_fun) {
                if((rt_ptr_->user_functions_search())(name)) {
                    return valbox{rt_ptr_->user_function_selector(), name, true};
                }
                auto gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
                if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                    sym_type = obj_type::global_fun;
                    return gvd_it->second;
                }
            } else if(sym_type == obj_type::global_fun) {
                auto gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
                if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                    return gvd_it->second;
                }
                if((rt_ptr_->user_functions_search())(name)) {
                    sym_type = obj_type::user_fun;
                    return valbox{rt_ptr_->user_function_selector(), name, true};
                }
            } else {
                if((rt_ptr_->user_functions_search())(name)) {
                    sym_type = obj_type::user_fun;
                    return valbox{rt_ptr_->user_function_selector(), name, true};
                }
                auto gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
                if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                    sym_type = obj_type::global_fun;
                    return gvd_it->second;
                }
            }
            sym_type = obj_type::unknown;
            return {};
        }

        bool find_func(std::string const &name, valbox &fn, obj_type &sym_type) const {
            if(sym_type == obj_type::user_fun) {
                if((rt_ptr_->user_functions_search())(name)) {
                    fn = valbox{rt_ptr_->user_function_selector(), name, true};
                    return true;
                }
                auto gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
                if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                    fn = gvd_it->second;
                    sym_type = obj_type::global_fun;
                    return true;
                }
            } else if(sym_type == obj_type::global_fun) {
                auto gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
                if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                    fn = gvd_it->second;
                    return true;
                }
                if((rt_ptr_->user_functions_search())(name)) {
                    sym_type = obj_type::user_fun;
                    fn = valbox{rt_ptr_->user_function_selector(), name, true};
                    return true;
                }
            } else {
                if((rt_ptr_->user_functions_search())(name)) {
                    sym_type = obj_type::user_fun;
                    fn = valbox{rt_ptr_->user_function_selector(), name, true};
                    return true;
                }
                auto gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
                if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                    fn = gvd_it->second;
                    sym_type = obj_type::global_fun;
                    return true;
                }
            }
            sym_type = obj_type::unknown;
            return false;
        }

        valbox find_method(std::string const &class_name, std::string const &method_name, int64_t l, int64_t c) const {
            auto c_it{rt_ptr_->global_methods_dictionary()->find(class_name)};
            if(c_it != rt_ptr_->global_methods_dictionary()->end()) {
                auto m_it{c_it->second.find(method_name)};
                if(m_it != c_it->second.end()) {
                    return m_it->second;
                }
            }
            std::string em{"method "}; em += class_name; em += "."; em += method_name; em += "() not found";
            throw teal_function_not_found{l, c, em};
        }

        void set_input(std::string const &name, valbox const &val) {
            rt_ptr_->set_input(name, val);
        }

        void set_output(std::string const &name, valbox const &val) {
            rt_ptr_->set_output(name, val);
        }

        void clear_input(std::string const &name) {
            rt_ptr_->clear_input(name);
        }

        void clear_output(std::string const &name) {
            rt_ptr_->clear_output(name);
        }

        void clear_inputs() {
            rt_ptr_->clear_inputs();
        }

        void clear_outputs() {
            rt_ptr_->clear_outputs();
        }

        valbox get_input(std::string const &name) const {
            return rt_ptr_->get_input(name);
        }

        valbox const &get_output(std::string const &name) const & {
            return rt_ptr_->get_output(name);
        }

        void set_self_fields(str_map_t<valbox> *sf) {
            self_fields_ = sf;
        }

        str_map_t<valbox> *self_fields() {
            return self_fields_;
        }

        bool create_if_not_exists() const {
            return create_if_not_exists_ != 0;
        }

        bool set_create_if_not_exists(bool val) {
            bool res{create_if_not_exists_ != 0};
            create_if_not_exists_ = val;
            return res;
        }

        execution_context() = default;

        execution_context(execution_context const &that):
            rt_ptr_{that.rt_ptr_}
        {
        }

    private:
        class stack_frame {
        public:
            void put(std::string const &name, valbox const &value) {
                m_[name] = value;
            }

            bool get(std::string const &name, valbox &res) const {
                auto it{m_.find(name)};
                if(it == m_.end()) {
                    return false;
                }
                res = it->second;
                return true;
            }

            void clear() {
                m_.clear();
            }

            void undefine() {
                for(auto &&p: m_) {
                    p.second.become_undefined();
                }
            }

        private:
            str_map_t<valbox> m_{};
        };

        runtime_interface *rt_ptr_{nullptr};
        std::atomic_int64_t function_depth_{0};
        std::vector<stack_frame> stack_{};
        std::vector<size_t> stack_barriers_{};
        int64_t stack_ptr_{-1};
        valbox return_result_{};
        std::uint64_t return_requested_{0};
        std::uint64_t continue_requested_{0};
        std::uint64_t break_requested_{0};
        str_map_t<valbox> *self_fields_{nullptr};
        std::uint64_t create_if_not_exists_{0};
    };

}
