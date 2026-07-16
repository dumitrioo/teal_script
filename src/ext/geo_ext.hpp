#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/math/math_util.hpp"
#include "../inc/geocoordinates.hpp"

#include "../tealscript_value.hpp"
#include "../tealscript_util.hpp"
#include "../tealscript_interfaces.hpp"

namespace teal {

    class geo_ext: public extension_interface {
    public:
        geo_ext() = default;
        ~geo_ext() {
            unregister_runtime();
        }
        geo_ext(geo_ext const &) = delete;
        geo_ext &operator=(geo_ext const &) = delete;
        geo_ext(geo_ext &&) = delete;
        geo_ext &operator=(geo_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) { return; }
            rt_ = rt;
            if(rt_ == nullptr) { return; }

            rt_->add_function("LLA", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 0, 7)
                if(args.size() == 0) {
                    return valbox{LLA{}, "LLA"};
                } else if(args.size() == 1) {
                    if(args[0].is_string()) {
                        return valbox{LLA{args[0].as_string()}, "LLA"};
                    } else if(args[0].is_wstring()) {
                        return valbox{LLA{args[0].cast_to_string()}, "LLA"};
                    } else if(args[0].is_numeric()) {
                        return valbox{LLA{args[0].cast_to_double()}, "LLA"};
                    } else {
                        throw std::runtime_error{"wrong actual function arguments number"};
                    }
                } else if(args.size() == 2) {
                    return valbox{LLA{args[0].cast_to_double(), args[1].cast_to_double()}, "LLA"};
                } else if(args.size() == 3) {
                    return valbox{LLA{args[0].cast_to_double(), args[1].cast_to_double(), args[2].cast_to_double()}, "LLA"};
                } else if(args.size() == 6) {
                    return valbox{LLA{args[0].cast_to_double(), args[1].cast_to_double(), args[2].cast_to_double(),
                                      args[3].cast_to_double(), args[4].cast_to_double(), args[5].cast_to_double()}, "LLA"};
                } else if(args.size() == 7) {
                    return valbox{LLA{args[0].cast_to_double(), args[1].cast_to_double(), args[2].cast_to_double(),
                                      args[3].cast_to_double(), args[4].cast_to_double(), args[5].cast_to_double(),
                                      args[6].cast_to_double()}, "LLA"};
                } else {
                    throw std::runtime_error{"wrong actual function arguments number"};
                }
            });
            rt_->add_object_stringifier("LLA",
                [](valbox const &v) -> valbox {
                    if(v.class_name() == "LLA") {
                        auto lla{v.as_class<teal::LLA>()};
                        std::stringstream ss{};
                        ss << lla.to_string_traditional();
                        return ss.str();
                    }
                    throw std::runtime_error{"invalid operand"};
                }
            );

            rt_->add_method("LLA", "latitude", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, LLA).latitude();
            });
            rt_->add_method("LLA", "longitude", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, LLA).longitude();
            });
            rt_->add_method("LLA", "altitude", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, LLA).altitude();
            });
            rt_->add_method("LLA", "to_string_traditional", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2)
                if(args.size() == 2) {
                    return TEALTHIS(args, LLA).to_string_traditional(args[1].cast_to_size_t());
                } else {
                    return TEALTHIS(args, LLA).to_string_traditional();
                }
            });
            rt_->add_method("LLA", "to_string_decimal_degrees", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, LLA).to_string_decimal_degrees();
            });
            rt_->add_method("LLA", "to_string_iso_6709", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, LLA).to_string_iso_6709();
            });
            rt_->add_method("LLA", "distance_to", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return TEALTHIS(args, LLA).distance_to(TEALCLASSARG(args, 1, LLA));
            });

            rt_->add_function("ECEF", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 0, 3)
                if(args.size() == 0) {
                    return valbox{ECEF{}, "ECEF"};
                } else if(args.size() == 1) {
                    return valbox{ECEF{args[0].cast_to_double()}, "ECEF"};
                } else if(args.size() == 2) {
                    return valbox{ECEF{args[0].cast_to_double(), args[1].cast_to_double()}, "ECEF"};
                } else if(args.size() == 3) {
                    return valbox{ECEF{args[0].cast_to_double(), args[1].cast_to_double(), args[2].cast_to_double()}, "ECEF"};
                } else {
                    throw std::runtime_error{"wrong actual function arguments number"};
                }
            });
            rt_->add_object_stringifier("ECEF",
                [](valbox const &v) -> valbox {
                    if(v.class_name() == "ECEF") {
                        auto ecef{v.as_class<ECEF>()};
                        std::stringstream ss{};
                        ss << "ECEF{" << std::fixed << ecef.x() << ", " << ecef.y() << ", " << ecef.z() << "}";
                        return ss.str();
                    }
                    throw std::runtime_error{"invalid operand"};
                }
            );
            rt_->add_method("ECEF", "x", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, ECEF).x();
            });
            rt_->add_method("ECEF", "y", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, ECEF).y();
            });
            rt_->add_method("ECEF", "z", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, ECEF).z();
            });

            rt_->add_function("ENU", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 0, 3)
                if(args.size() == 0) {
                    return valbox{ENU{}, "ENU"};
                } else if(args.size() == 1) {
                    return valbox{ENU{args[0].cast_to_double()}, "ENU"};
                } else if(args.size() == 2) {
                    return valbox{ENU{args[0].cast_to_double(), args[1].cast_to_double()}, "ENU"};
                } else if(args.size() == 3) {
                    return valbox{ENU{args[0].cast_to_double(), args[1].cast_to_double(), args[2].cast_to_double()}, "ECEF"};
                } else {
                    throw std::runtime_error{"wrong actual function arguments number"};
                }
            });
            rt_->add_object_stringifier("ENU",
                [](valbox const &v) -> valbox {
                    if(v.class_name() == "ENU") {
                        auto enu{v.as_class<ENU>()};
                        std::stringstream ss{};
                        ss << "ENU{" << std::fixed << enu.east() << ", " << enu.north() << ", " << enu.up() << "}";
                        return ss.str();
                    }
                    throw std::runtime_error{"invalid operand"};
                }
            );
            rt_->add_method("ENU", "east", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, ENU).east();
            });
            rt_->add_method("ENU", "north", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, ENU).north();
            });
            rt_->add_method("ENU", "up", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, ENU).up();
            });

            rt_->add_function("LLAtoECEF", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return valbox{LLAtoECEF(TEALCLASSARG(args, 0, LLA)), "ECEF"};
            });
            rt_->add_function("ECEFtoLLA", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return valbox{ECEFtoLLA(TEALCLASSARG(args, 0, ECEF)), "LLA"};
            });
            rt_->add_function("ECEFtoENU", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return valbox{ECEFtoENU(TEALCLASSARG(args, 0, ECEF), TEALCLASSARG(args, 1, LLA)), "ENU"};
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }

            // TODO:

            rt_ = nullptr;
        }

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
