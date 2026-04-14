#pragma once

#include "../inc/commondefs.hpp"

#include "../tealscript_value.hpp"
#include "../tealscript_util.hpp"
#include "../tealscript_interfaces.hpp"

namespace teal {

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

            rt->add_function("timestamp", TEALFUN(args) {
                std::string s{};
                if(args.size() > 0) {
                    if(args[0].is_string_ref()) {
                        return teal::valbox{teal::timespec_wrapper{args[0].as_string()}, "timespec_wrapper"};
                    } else if(args[0].is_wstring_ref()) {
                        return teal::valbox{teal::timespec_wrapper{args[0].as_wstring()}, "timespec_wrapper"};
                    } else if(args[0].is_any_fp_number()) {
                        return teal::valbox{teal::timespec_wrapper{args[0].cast_to_long_double()}, "timespec_wrapper"};
                    } else if(args[0].is_any_int_number()) {
                        return teal::valbox{teal::timespec_wrapper{args[0].cast_to_s64()}, "timespec_wrapper"};
                    }
                }
                return teal::valbox{teal::timespec_wrapper::now(), "timespec_wrapper"};
            });

            rt->add_object_serializer("timespec_wrapper",
                [](valbox const &vb) -> std::optional<std::string> {
                    if(vb.is_class_ref() && vb.class_name() == "timespec_wrapper") {
                        return vb.as_class<teal::timespec_wrapper>().as_iso_8601_str();
                    }
                    return {};
                }
            );

            rt->add_object_deserializer("timespec_wrapper",
                [](std::string const &class_name, std::string const &serial_form) -> valbox {
                    if(class_name == "timespec_wrapper") {
                        return teal::valbox{teal::timespec_wrapper{serial_form}, "timespec_wrapper"};
                    }
                    return teal::valbox{valbox_no_initialize::dont_do_it};
                }
            );

            rt->add_object_comparator("timespec_wrapper",
                [](valbox const &l, valbox const &r) -> valbox {
                    if(l.class_name() == "timespec_wrapper" && r.class_name() == "timespec_wrapper") {
                        return
                            l.as_class<teal::timespec_wrapper>() < r.as_class<teal::timespec_wrapper>() ?
                                -1:
                                l.as_class<teal::timespec_wrapper>() > r.as_class<teal::timespec_wrapper>() ?
                                    1:
                                    0
                        ;
                    }
                    return teal::valbox{valbox_no_initialize::dont_do_it};
                }
            );

            rt->add_object_stringifier("timespec_wrapper",
                [](valbox const &v) -> valbox {
                    if(v.class_name() == "timespec_wrapper") {
                        return v.as_class<teal::timespec_wrapper>().as_iso_8601_str();
                    }
                    return std::string{};
                }
            );

            rt->add_function("gmtimestamp", TEALFUN() { return teal::valbox{teal::timespec_wrapper::gmtnow(), "timespec_wrapper"}; });
            rt->add_method("timespec_wrapper", "year", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).year(); });
            rt->add_method("timespec_wrapper", "month", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).month(); });
            rt->add_method("timespec_wrapper", "day", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).day(); });
            rt->add_method("timespec_wrapper", "weekday", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).weekday(); });
            rt->add_method("timespec_wrapper", "hour", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).hour(); });
            rt->add_method("timespec_wrapper", "min", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).min(); });
            rt->add_method("timespec_wrapper", "sec", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).sec_with_subsec(); });
            rt->add_method("timespec_wrapper", "seconds", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).seconds(); });
            rt->add_method("timespec_wrapper", "fseconds", TEALFUN(args) { return TEALTHIS(args, teal::timespec_wrapper).fseconds(); });
            rt->add_method("timespec_wrapper", "to_string", TEALFUN(args) {
                std::size_t prec{static_cast<std::size_t>(9)};
                if(args.size() > 1) { prec = args[1].cast_num_to_num<std::size_t>(); }
                return TEALTHIS(args, teal::timespec_wrapper).as_iso_8601_str(prec);
            });
            rt->add_method("timespec_wrapper", "as_iso_8601", TEALFUN(args) {
                std::size_t prec{static_cast<std::size_t>(9)};
                if(args.size() > 1) { prec = args[1].cast_num_to_num<std::size_t>(); }
                return TEALTHIS(args, teal::timespec_wrapper).as_iso_8601_str(prec);
            });
            rt->add_method("timespec_wrapper", "as_gmt_iso_8601", TEALFUN(args) {
                std::size_t prec{static_cast<std::size_t>(9)};
                if(args.size() > 1) { prec = args[1].cast_num_to_num<std::size_t>(); }
                return TEALTHIS(args, teal::timespec_wrapper).as_gmt_iso_8601_str(prec);
            });
            rt->add_method("timespec_wrapper", "from_string", TEALFUN(args) {
                std::string s{}; if(args.size() > 1) { s = args[1].cast_to_string(); }
                TEALTHIS(args, teal::timespec_wrapper).from_iso_8601(s);
                return args[0];
            });
            rt->add_function("steady_clock", TEALFUN() {
                return static_cast<long double>(std::chrono::steady_clock::now().time_since_epoch().count()) / 1e9L;
            });
            rt->add_function("time", TEALFUN() {
                return teal::timespec_wrapper::now().fseconds();
            });
            rt->add_function("gmtime", TEALFUN() {
                return teal::timespec_wrapper::gmtnow().fseconds();
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_object_services("timespec_wrapper");
            rt_->remove_function("timestamp");
            rt_->remove_function("gmtimestamp");
            rt_->remove_function("steady_clock");
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
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
