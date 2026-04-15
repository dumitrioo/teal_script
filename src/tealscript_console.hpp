#pragma once

#include "inc/commondefs.hpp"
#include "inc/file_util.hpp"
#include "inc/str_util.hpp"
#include "inc/containers/concurrentqueue.h"
#include "inc/net/url.hpp"
#include "inc/net/net_utils.hpp"
#include "inc/sys_util.hpp"
#include "inc/base16.hpp"
#include "inc/base64.hpp"
#include "inc/base85.hpp"
#include "inc/so.hpp"
#include "tealscript_util.hpp"
#include "tealscript_interfaces.hpp"

namespace teal {

#if defined(TEAL_USE_ASYNC_CONSOLE)
    class console {
    public:
        console(runtime_interface *rt = nullptr):
            rt_{rt},
            out_buffers_{std::make_unique<buff>(), std::make_unique<buff>()},
            out_thread_{
                [this]() {
                    while(!termination_) {
                        auto opt_pair{fetch_string_from_buffer()};
                        if(opt_pair) {
                            if(opt_pair->second) {
                                std::cerr << opt_pair->first << std::flush;
                            } else {
                                std::cout << opt_pair->first << std::flush;
                            }
                        }
                    }
                }
            }
        {
        }

        ~console() {
            termination_ = true;
            if(out_thread_.joinable()) { out_thread_.join(); }
        }

        void print(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[32mprint\033[0m" : "print"), args);
        }
        void info(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[34minfo\033[0m" : "info"), args);
        }
        void log(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[32mlog\033[0m" : "log"), args);
        }
        void warn(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[35mwarning\033[0m" : "warning"), args);
        }
        void debug(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[93mdebug\033[0m" : "debug"), args);
        }
        void error(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[91merror\033[0m" : "error"), args);
        }

        void rawprint(std::vector<valbox> const &args) {
            std::stringstream out{};
            if(setfill_) { out << std::setfill(fill_char_); }
            if(setw_) { out << std::setw(w_); }
            if(setprec_) { out << std::setprecision(prec_); }
            if(fk_ != flt_kind::def) {
                switch(fk_) {
                    case flt_kind::fix: out << std::fixed; break;
                    case flt_kind::sci: out << std::scientific; break;
                    case flt_kind::hex: out << std::hex; break;
                    default: out << std::defaultfloat; break;
                }
            }
            for(auto &&v: args) {
                auto printed{false};
                if(v.is_class_ref()) {
                    obj_services const *osvc{rt_->get_object_services(v.class_name())};
                    if(osvc != nullptr) {
                        auto srgngr{osvc->stringify};
                        auto s{srgngr(v)};
                        out << s;
                        printed = true;
                    }
                }
                if(!printed) out << v;
            }
            put_string_to_buffer(out.str(), false);
        }

        void println(std::vector<valbox> const &args) {
            std::stringstream out{};
            if(setfill_) { out << std::setfill(fill_char_); }
            if(setw_) { out << std::setw(w_); }
            if(setprec_) { out << std::setprecision(prec_); }
            if(fk_ != flt_kind::def) {
                switch(fk_) {
                    case flt_kind::fix: out << std::fixed; break;
                    case flt_kind::sci: out << std::scientific; break;
                    case flt_kind::hex: out << std::hex; break;
                    default: out << std::defaultfloat; break;
                }
            }
            for(auto &&v: args) {
                auto printed{false};
                if(v.is_class_ref()) {
                    obj_services const *osvc{rt_->get_object_services(v.class_name())};
                    if(osvc != nullptr) {
                        auto srgngr{osvc->stringify};
                        auto s{srgngr(v)};
                        out << s;
                        printed = true;
                    }
                }
                if(!printed) out << v;
            }
            out << '\n';
            put_string_to_buffer(out.str(), false);
        }

        void flush() {}
        void fixed() { fk_ = flt_kind::fix; }
        void scientific() { fk_ = flt_kind::sci; }
        void hexfloat() { fk_ = flt_kind::hex; }
        void defaultfloat() { fk_ = flt_kind::def; }
        void setprecision(int prec) { setprec_ = true; prec_ = prec; }
        int precision() { return prec_; }
        void setw(int w) { setw_ = true; w_ = w; }
        void setfill(char arg) { setfill_ = true; fill_char_ = arg; }
        char fill() { return fill_char_; }
        bool colors_enabled() const { return terminal_colours_; }
        void enable_colors(bool v) { terminal_colours_ = v; }
        bool setsync(bool v) const { return std::ios::sync_with_stdio(v); }

    private:
        void cerr_out(std::string const &type, std::vector<valbox> const &args) {
            std::stringstream out{};
            if(setfill_) { out << std::setfill(fill_char_); }
            if(setw_) { out << std::setw(w_); }
            if(setprec_) { out << std::setprecision(prec_); }
            if(fk_ != flt_kind::def) {
                switch(fk_) {
                case flt_kind::fix: out << std::fixed; break;
                case flt_kind::sci: out << std::scientific; break;
                case flt_kind::hex: out << std::hex; break;
                default: out << std::defaultfloat; break;
                }
            }
            out << str_util::from_utf8(timespec_wrapper::now().as_iso_8601_str()) << " " << type << ": ";
            for(auto &&v: args) {
                auto printed{false};
                if(v.is_class_ref()) {
                    obj_services const *osvc{rt_->get_object_services(v.class_name())};
                    if(osvc != nullptr) {
                        auto srgngr{osvc->stringify};
                        auto s{srgngr(v)};
                        out << s;
                        printed = true;
                    }
                }
                if(!printed) out << v;
            }
            out << '\n';
            put_string_to_buffer(out.str(), true);
        }

        void cout_out(std::string const &type, std::vector<valbox> const &args) {
            std::stringstream out{};
            if(setfill_) { out << std::setfill(fill_char_); }
            if(setw_) { out << std::setw(w_); }
            if(setprec_) { out << std::setprecision(prec_); }
            if(fk_ != flt_kind::def) {
                switch(fk_) {
                    case flt_kind::fix: out << std::fixed; break;
                    case flt_kind::sci: out << std::scientific; break;
                    case flt_kind::hex: out << std::hex; break;
                    default: out << std::defaultfloat; break;
                }
            }
            out << str_util::from_utf8(timespec_wrapper::now().as_iso_8601_str()) << " " << type << ": ";
            for(auto &&v: args) {
                auto printed{false};
                if(v.is_class_ref()) {
                    obj_services const *osvc{rt_->get_object_services(v.class_name())};
                    if(osvc != nullptr) {
                        auto srgngr{osvc->stringify};
                        auto s{srgngr(v)};
                        out << s;
                        printed = true;
                    }
                }
                if(!printed) out << v;
            }
            out << '\n';
            put_string_to_buffer(out.str(), false);
        }

        class buff {
        public:
            void put_string(std::string const &s, bool is_err) {
                out_buffer_.enqueue({s, is_err});
            }

            std::optional<std::pair<std::string, bool>> fetch_string() {
                std::pair<std::string, bool> res{};
                if(out_buffer_.try_dequeue(res)) {
                    return res;
                }
                return {};
            }

            std::size_t size() const {
                return out_buffer_.size_approx();
            }

        private:
            moodycamel::ConcurrentQueue<std::pair<std::string, bool>> out_buffer_{};
        };

        void put_string_to_buffer(std::string const &s, bool is_err) {
            {
                std::shared_lock l{out_index_mtp_};
                out_buffers_[(out_index_.load(std::memory_order::acquire) + 1) % 2]->put_string(s, is_err);
            }
            {
                std::unique_lock l{out_mtp_};
                out_cvar_.notify_one();
            }
        }

        std::optional<std::pair<std::string, bool>> fetch_string_from_buffer() {
            std::optional<std::pair<std::string, bool>> res{out_buffers_[out_index_.load(std::memory_order::acquire) % 2]->fetch_string()};
            if(res) {
                return res;
            } else {
                bool need_switch_index{true};
                std::unique_lock l{out_mtp_};
                if(out_buffers_[(out_index_.load(std::memory_order::acquire) + 1) % 2]->size() == 0) {
                    std::cv_status wst{out_cvar_.wait_for(l, std::chrono::milliseconds{100})};
                    if(wst == std::cv_status::timeout) {
                        need_switch_index = false;
                    }
                }
                if(need_switch_index) {
                    std::unique_lock l{out_index_mtp_};
                    out_index_ = (out_index_ + 1) % 2;
                    out_index_switches_++;
                }
            }
            return out_buffers_[out_index_.load(std::memory_order::acquire) % 2]->fetch_string();
        }

        runtime_interface *rt_{nullptr};

        std::mutex out_mtp_{};
        std::condition_variable out_cvar_{};
        std::atomic<std::size_t> out_index_switches_{0};
        mutable shared_mutex out_index_mtp_{};
        std::atomic<std::size_t> out_index_{0};
        std::array<std::unique_ptr<buff>, 2> out_buffers_{};
        std::thread out_thread_{};

        bool setfill_{false};
        char fill_char_{};
        bool setw_{false};
        int w_{};
        bool setprec_{false};
        int prec_{};
        enum class flt_kind{def, sci, fix, hex};
        flt_kind fk_{flt_kind::def};
        bool terminal_colours_{false};
        bool termination_{false};
    };

#else

    class console {
    public:
        console(runtime_interface *rt = nullptr): rt_{rt} {
        }

        void print(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[32mprint\033[0m" : "print"), args);
        }
        void info(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[34minfo\033[0m" : "info"), args);
        }
        void log(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[32mlog\033[0m" : "log"), args);
        }
        void warn(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[35mwarning\033[0m" : "warning"), args);
        }
        void debug(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[93mdebug\033[0m" : "debug"), args);
        }
        void error(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[91merror\033[0m" : "error"), args);
        }

        void rawprint(std::vector<valbox> const &args) {
            std::stringstream out{};
            if(setfill_) { out << std::setfill(fill_char_); }
            if(setw_) { out << std::setw(w_); }
            if(setprec_) { out << std::setprecision(prec_); }
            if(fk_ != flt_kind::def) {
                switch(fk_) {
                    case flt_kind::fix: out << std::fixed; break;
                    case flt_kind::sci: out << std::scientific; break;
                    case flt_kind::hex: out << std::hex; break;
                    default: out << std::defaultfloat; break;
                }
            }
            for(auto &&v: args) {
                auto printed{false};
                if(v.is_class_ref()) {
                    obj_services const *osvc{rt_->get_object_services(v.class_name())};
                    if(osvc != nullptr) {
                        auto srgngr{osvc->stringify};
                        auto s{srgngr(v)};
                        out << s;
                        printed = true;
                    }
                }
                if(!printed) out << v;
            }
            std::unique_lock l{out_mtp_};
            std::cout << out.str() << std::flush;
        }

        void println(std::vector<valbox> const &args) {
            std::stringstream out{};
            if(setfill_) { out << std::setfill(fill_char_); }
            if(setw_) { out << std::setw(w_); }
            if(setprec_) { out << std::setprecision(prec_); }
            if(fk_ != flt_kind::def) {
                switch(fk_) {
                    case flt_kind::fix: out << std::fixed; break;
                    case flt_kind::sci: out << std::scientific; break;
                    case flt_kind::hex: out << std::hex; break;
                    default: out << std::defaultfloat; break;
                }
            }
            for(auto &&v: args) {
                auto printed{false};
                if(v.is_class_ref()) {
                    obj_services const *osvc{rt_->get_object_services(v.class_name())};
                    if(osvc != nullptr) {
                        auto srgngr{osvc->stringify};
                        auto s{srgngr(v)};
                        out << s;
                        printed = true;
                    }
                }
                if(!printed) out << v;
            }
            std::unique_lock l{out_mtp_};
            std::cout << out.str() << std::endl;
        }

        void flush() {
            std::unique_lock l{out_mtp_};
            std::cout.flush();
        }
        void fixed() { fk_ = flt_kind::fix; }
        void scientific() { fk_ = flt_kind::sci; }
        void hexfloat() { fk_ = flt_kind::hex; }
        void defaultfloat() { fk_ = flt_kind::def; }
        void setprecision(int prec) { setprec_ = true; prec_ = prec; }
        int precision() { return prec_; }
        void setw(int w) { setw_ = true; w_ = w; }
        void setfill(char arg) { setfill_ = true; fill_char_ = arg; }
        char fill() { return fill_char_; }
        bool colors_enabled() const { return terminal_colours_; }
        void enable_colors(bool v) { terminal_colours_ = v; }
        bool setsync(bool v) const { return std::ios::sync_with_stdio(v); }

    private:
        void cerr_out(std::string const &type, std::vector<valbox> const &args) {
            std::stringstream out{};
            if(setfill_) { out << std::setfill(fill_char_); }
            if(setw_) { out << std::setw(w_); }
            if(setprec_) { out << std::setprecision(prec_); }
            if(fk_ != flt_kind::def) {
                switch(fk_) {
                case flt_kind::fix: out << std::fixed; break;
                case flt_kind::sci: out << std::scientific; break;
                case flt_kind::hex: out << std::hex; break;
                default: out << std::defaultfloat; break;
                }
            }
            out << str_util::from_utf8(timespec_wrapper::now().as_iso_8601_str()) << " " << type << ": ";
            for(auto &&v: args) {
                auto printed{false};
                if(v.is_class_ref()) {
                    obj_services const *osvc{rt_->get_object_services(v.class_name())};
                    if(osvc != nullptr) {
                        auto srgngr{osvc->stringify};
                        auto s{srgngr(v)};
                        out << s;
                        printed = true;
                    }
                }
                if(!printed) out << v;
            }
            std::unique_lock l{out_mtp_};
            std::cerr << out.str() << std::endl;
        }

        void cout_out(std::string const &type, std::vector<valbox> const &args) {
            std::stringstream out{};
            if(setfill_) { out << std::setfill(fill_char_); }
            if(setw_) { out << std::setw(w_); }
            if(setprec_) { out << std::setprecision(prec_); }
            if(fk_ != flt_kind::def) {
                switch(fk_) {
                    case flt_kind::fix: out << std::fixed; break;
                    case flt_kind::sci: out << std::scientific; break;
                    case flt_kind::hex: out << std::hex; break;
                    default: out << std::defaultfloat; break;
                }
            }
            out << str_util::from_utf8(timespec_wrapper::now().as_iso_8601_str()) << " " << type << ": ";
            for(auto &&v: args) {
                auto printed{false};
                if(v.is_class_ref()) {
                    obj_services const *osvc{rt_->get_object_services(v.class_name())};
                    if(osvc != nullptr) {
                        auto srgngr{osvc->stringify};
                        auto s{srgngr(v)};
                        out << s;
                        printed = true;
                    }
                }
                if(!printed) out << v;
            }
            std::unique_lock l{out_mtp_};
            std::cout << out.str() << std::endl;
        }

    private:
        runtime_interface *rt_{nullptr};
        bool setfill_{false};
        char fill_char_{};
        bool setw_{false};
        int w_{};
        bool setprec_{false};
        int prec_{};
        shared_mutex out_mtp_{};
        enum class flt_kind{def, sci, fix, hex};
        flt_kind fk_{flt_kind::def};
        bool terminal_colours_{false};
    };
#endif

}
