#pragma once

#include "../include/commondefs.hpp"

#include "../scaflux_value.hpp"
#include "../scaflux_util.hpp"
#include "../scaflux_interfaces.hpp"

namespace scfx {

    class time_ext: public extension_interface {
    public:
        time_ext() = default;
        ~time_ext() {
            unregister_runtime();
        }
        time_ext(time_ext const &) = delete;
        time_ext &operator=(time_ext const &) = delete;
        time_ext(time_ext &&) = delete;
        time_ext &operator=(time_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) { return; }
            rt_ = rt;
            if(rt_ == nullptr) { return; }

            rt->add_function("timestamp", SCFXFUN(/*fname*/, args) {
                std::string s{};
                if(args.size() > 0) {
                    if(args[0].is_string_ref()) {
                        return scfx::valbox{scfx::timespec_wrapper{args[0].as_string()}, "timespec_wrapper"};
                    } else if(args[0].is_wstring_ref()) {
                        return scfx::valbox{scfx::timespec_wrapper{args[0].as_wstring()}, "timespec_wrapper"};
                    } else if(args[0].is_any_fp_number()) {
                        return scfx::valbox{scfx::timespec_wrapper{args[0].cast_to_long_double()}, "timespec_wrapper"};
                    } else if(args[0].is_any_int_number()) {
                        return scfx::valbox{scfx::timespec_wrapper{args[0].cast_to_s64()}, "timespec_wrapper"};
                    }
                }
                return scfx::valbox{scfx::timespec_wrapper::now(), "timespec_wrapper"};
            });
            rt->add_function("gmtimestamp", SCFXFUN(/*fname*/, /*args*/) { return scfx::valbox{scfx::timespec_wrapper::gmtnow(), "timespec_wrapper"}; });
            rt->add_method("timespec_wrapper", "year", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).year(); });
            rt->add_method("timespec_wrapper", "month", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).month(); });
            rt->add_method("timespec_wrapper", "day", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).day(); });
            rt->add_method("timespec_wrapper", "weekday", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).weekday(); });
            rt->add_method("timespec_wrapper", "hour", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).hour(); });
            rt->add_method("timespec_wrapper", "min", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).min(); });
            rt->add_method("timespec_wrapper", "sec", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).sec_with_subsec(); });
            rt->add_method("timespec_wrapper", "seconds", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).seconds(); });
            rt->add_method("timespec_wrapper", "fseconds", SCFXFUN(/*fname*/, args) { return SCFXTHIS(scfx::timespec_wrapper).fseconds(); });
            rt->add_method("timespec_wrapper", "as_iso_8601", SCFXFUN(/*fname*/, args) {
                std::size_t prec{static_cast<std::size_t>(9)};
                if(args.size() > 1) { prec = args[1].cast_num_to_num<std::size_t>(); }
                return SCFXTHIS(scfx::timespec_wrapper).as_iso_8601_str(prec);
            });
            rt->add_method("timespec_wrapper", "as_gmt_iso_8601", SCFXFUN(/*fname*/, args) {
                std::size_t prec{static_cast<std::size_t>(9)};
                if(args.size() > 1) { prec = args[1].cast_num_to_num<std::size_t>(); }
                return SCFXTHIS(scfx::timespec_wrapper).as_gmt_iso_8601_str(prec);
            });
            rt->add_method("timespec_wrapper", "from_string", SCFXFUN(/*fname*/, args) {
                std::string s{}; if(args.size() > 1) { s = args[1].cast_to_string(); }
                SCFXTHIS(scfx::timespec_wrapper).from_iso_8601(s);
                return args[0];
            });
            rt->add_function("steady_clock", SCFXFUN(/*fname*/, /*args*/) {
                return static_cast<long double>(std::chrono::steady_clock::now().time_since_epoch().count()) / 1e9L;
            });
            rt->add_function("time", SCFXFUN(/*fname*/, /*args*/) {
                return scfx::timespec_wrapper::now().fseconds();
            });
            rt->add_function("gmtime", SCFXFUN(/*fname*/, /*args*/) {
                return scfx::timespec_wrapper::gmtnow().fseconds();
            });
        }
        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("timestamp");
            rt_->remove_function("gmtimestamp");
            rt_->remove_function("time");
            rt_->remove_function("gmtime");
            rt_->remove_method("timespec_wrapper", "year");
            rt_->remove_method("timespec_wrapper", "month");
            rt_->remove_method("timespec_wrapper", "day");
            rt_->remove_method("timespec_wrapper", "weekday");
            rt_->remove_method("timespec_wrapper", "hour");
            rt_->remove_method("timespec_wrapper", "min");
            rt_->remove_method("timespec_wrapper", "sec");
            rt_->remove_method("timespec_wrapper", "seconds");
            rt_->remove_method("timespec_wrapper", "fseconds");
            rt_->remove_method("timespec_wrapper", "as_iso_8601");
            rt_->remove_method("timespec_wrapper", "as_gmt_iso_8601");
            rt_->remove_method("timespec_wrapper", "from_string");
            rt_ = nullptr;
        }

    private:
        std::shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
