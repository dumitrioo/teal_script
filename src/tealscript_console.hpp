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

    class console {
    public:
        console(runtime_interface *rt = nullptr): rt_{rt} {
        }

        void print(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[32mprint\033[0m" : "print"), args, false);
        }
        void println(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[32mprint\033[0m" : "print"), args, true);
        }
        void info(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[34minfo\033[0m" : "info"), args, false);
        }
        void log(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[32mlog\033[0m" : "log"), args, false);
        }
        void warn(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[35mwarning\033[0m" : "warning"), args, false);
        }
        void debug(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[93mdebug\033[0m" : "debug"), args, false);
        }
        void error(std::vector<valbox> const &args) {
            cout_out((terminal_colours_ ? "\033[91merror\033[0m" : "error"), args, false);
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
                if(v.is_class()) {
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
            std::cout << out.str();
        }

        void rawprintln(std::vector<valbox> const &args) {
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
                if(v.is_class()) {
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
        void cerr_out(std::string const &type, std::vector<valbox> const &args, bool flush_out) {
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
                if(v.is_class()) {
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
            if(flush_out) {
                std::cerr << out.str() << std::endl;
            } else {
                std::cerr << out.str() << '\n';
            }
        }

        void cout_out(std::string const &type, std::vector<valbox> const &args, bool flush_out) {
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
                if(v.is_class()) {
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
            if(flush_out) {
                std::cout << out.str() << std::endl;
            } else {
                std::cout << out.str() << '\n';
            }
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

}
