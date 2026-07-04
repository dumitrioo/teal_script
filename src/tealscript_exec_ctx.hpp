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

    class valbox_pool {
    public:
        valbox get_valbox() {
            valbox res{};
            if(!vals_.empty()) {
                res = std::move(vals_.front());
                vals_.pop_front();
            }
            return res;
        }

        void put_valbox(valbox const &vb) {
            vals_.push_front(vb);
        }

        void put_valbox(valbox &&vb) {
            vals_.push_front(std::move(vb));
        }

    private:
        std::list<valbox> vals_{};
    };

    class execution_context {
    public:
        execution_context() = default;

        execution_context(execution_context const &that):
            rt_ptr_{that.rt_ptr_}
        {
        }

        runtime_error &rte() {
            return rte_;
        }

        runtime_interface *rt_interface() {
            return rt_ptr_;
        }

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


        valbox get_valbox() {
            return vals_pool_.get_valbox();
        }

        void put_valbox(valbox const &vb) {
            vals_pool_.put_valbox(vb);
        }

        void put_valbox(valbox &&vb) {
            vals_pool_.put_valbox(std::move(vb));
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
            return return_requested_ != 0 || break_requested_ != 0 || continue_requested_ != 0 || rt_ptr_->termination_requested();
        }

        bool termination_requested() const {
            return rt_ptr_->termination_requested();
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
            for(; stack_ptr_ >= 0; --stack_ptr_) { stack_[stack_ptr_].clear(); }
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

        void set_local_value(std::string const &name, valbox &val) {
            val.set_stack_placement();
            stack_[stack_ptr_].put(name, val);
        }

        bool clear_instance_variable(valbox const &v) {
            for(auto it{self_fields_->begin()}; it != self_fields_->end(); ++it) {
                if(it->second.is_same(v)) {
                    self_fields_->erase(it);
                    return true;
                }
            }
            return false;
        }

        bool clear_stack_variable_before_barrier(valbox const &v) {
            int64_t stb{-2};
            for(int64_t i{stack_ptr_}; i >= 0; --i) {
                if(stb != -2 && i <= stb) {
                    break;
                }
                if(stack_[i].clear_var(v)) {
                    return true;
                }
                if(stb == -2) {
                    stb = stack_barrier();
                }
            }
            return false;
        }

        valbox find_val_by_sym_name(std::string const &name, int64_t l, int64_t c) {
            // valbox res{};
            // int64_t stb{-2};
            // for(int64_t i{stack_ptr_}; i >= 0; --i) {
            //     if(stb != -2 && i <= stb) {
            //         break;
            //     }
            //     if(stack_[i].get(name, res)) {
            //         return res;
            //     }
            //     if(stb == -2) {
            //         stb = stack_barrier();
            //     }
            // }
            // str_map_t<valbox>::const_iterator gvd_it{rt_ptr_->global_constants_dictionary()->find(name)};
            // if(gvd_it != rt_ptr_->global_constants_dictionary()->end()) {
            //     return gvd_it->second;
            // }
            // gvd_it = rt_ptr_->global_functions_dictionary()->find(name);
            // if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
            //     return gvd_it->second;
            // }
            // if((rt_ptr_->user_functions_search())(name)) {
            //     return valbox{rt_ptr_->user_function_selector(), name, true};
            // }
            // if(create_if_not_exists()) {
            //     stack_[stack_ptr_].put(name, res);
            //     return res;
            // }
            // throw teal_identifier_not_found{l, c, std::string{"identifier \""} + name + "\" not found"};


            valbox res{};
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
                stack_[stack_ptr_].put(name, res);
                return res;
            }
            str_map_t<valbox>::const_iterator gvd_it{rt_ptr_->global_constants_dictionary()->find(name)};
            if(gvd_it != rt_ptr_->global_constants_dictionary()->end()) {
                return gvd_it->second;
            }
            gvd_it = rt_ptr_->global_functions_dictionary()->find(name);
            if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                return gvd_it->second;
            }
            if((rt_ptr_->user_functions_search())(name)) {
                return valbox{rt_ptr_->user_function_selector(), name, true};
            }
            throw teal_identifier_not_found{l, c, std::string{"identifier \""} + name + "\" not found"};
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

        valbox find_func(std::string const &name) const {
            if((rt_ptr_->user_functions_search())(name)) {
                return valbox{rt_ptr_->user_function_selector(), name, true};
            }
            auto gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
            if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                return gvd_it->second;
            }
            return {};
        }

        bool find_func(std::string const &name, valbox &fn) const {
            if((rt_ptr_->user_functions_search())(name)) {
                fn = valbox{rt_ptr_->user_function_selector(), name, true};
                return true;
            }
            auto gvd_it{rt_ptr_->global_functions_dictionary()->find(name)};
            if(gvd_it != rt_ptr_->global_functions_dictionary()->end()) {
                fn = gvd_it->second;
                return true;
            }
            return false;
        }

        valbox find_method(std::string const &class_name, std::string const &method_name) const {
            auto c_it{rt_ptr_->global_methods_dictionary()->find(class_name)};
            if(c_it != rt_ptr_->global_methods_dictionary()->end()) {
                auto m_it{c_it->second.find(method_name)};
                if(m_it != c_it->second.end()) {
                    return m_it->second;
                }
            }
            return valbox{};
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

        valbox get_or_create_self_field(std::string const &n) {
            auto it{self_fields_->find(n)};
            if(it == self_fields_->end()) {
                (*self_fields_)[n] = valbox{};
                (*self_fields_)[n].set_instance_placement();
                return (*self_fields_)[n];
            }
            return it->second;
        }

        valbox get_self_field(std::string const &n) const {
            auto it{self_fields_->find(n)};
            if(it == self_fields_->end()) {
                throw std::runtime_error{std::string{"identifier \"this."} + n + "\" not found"};
            }
            return it->second;
        }

        bool have_self_field(std::string const &n) const {
            return self_fields_->find(n) != self_fields_->end();
        }

        bool create_if_not_exists() const {
            return create_if_not_exists_ != 0;
        }

        bool set_create_if_not_exists(bool val) {
            bool res{create_if_not_exists_ != 0};
            create_if_not_exists_ = val;
            return res;
        }

    private:
        class stack_frame {
        public:
            void put(std::string const &name, valbox &value) {
                if(!value.is_stack_placement()) {
                    value.set_stack_placement();
                }
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

            bool clear_var(valbox const &v) {
                auto e{m_.end()};
                for(auto it{m_.begin()}; it != e; ++it) {
                    if(v.is_same(it->second)) {
                        m_.erase(it);
                        return true;
                    }
                }
                return false;
            }

            void clear() {
                m_.clear();
            }

        private:
            std::map<std::string, valbox> m_{};
        };

        valbox_pool vals_pool_{};
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
        runtime_error rte_{0, 0, ""};
    };

}
