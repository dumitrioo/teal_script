#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/str_util.hpp"

#include "../scaflux_value.hpp"
#include "../scaflux_util.hpp"
#include "../scaflux_interfaces.hpp"

namespace scfx {

    class array_buffer_ext: public extension_interface {
    public:
        array_buffer_ext() = default;
        ~array_buffer_ext() {
            unregister_runtime();
        }
        array_buffer_ext(array_buffer_ext const &) = delete;
        array_buffer_ext &operator=(array_buffer_ext const &) = delete;
        array_buffer_ext(array_buffer_ext &&) = delete;
        array_buffer_ext &operator=(array_buffer_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt->add_function("array_buffer", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1)
                if(args.size() == 0) {
                    return scfx::valbox{std::make_shared<array_buffer>(), "array_buffer"};
                } else if(args.size() == 1) {
                    if(args[0].is_string_ref()) {
                        return scfx::valbox{std::make_shared<array_buffer>(args[0].as_string()), "array_buffer"};
                    } else if(args[0].is_class_ref() && args[0].class_name() == "array_buffer") {
                        return scfx::valbox{std::make_shared<array_buffer>(*SCFXTHIS(args, std::shared_ptr<array_buffer>)), "array_buffer"};
                    } else if(args[0].is_numeric()) {
                        std::shared_ptr<array_buffer> res{std::make_shared<array_buffer>()};
                        res->resize(args[0].cast_to_u64());
                        return scfx::valbox{res, "array_buffer"};
                    }
                }
                throw std::runtime_error{"invalid argument(s)"};
            });
            rt->add_function("as_array_buffer", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string_ref()) {
                    return scfx::valbox{std::make_shared<array_buffer>(args[0].as_string()), "array_buffer"};
                } else if(args[0].is_class_ref() && args[0].class_name() == "array_buffer") {
                    return scfx::valbox{std::make_shared<array_buffer>(*SCFXTHIS(args, std::shared_ptr<array_buffer>)), "array_buffer"};
                } else if(args[0].is_numeric()) {
                    std::shared_ptr<array_buffer> res{std::make_shared<array_buffer>()};
                    if(args[0].is_bool_ref()) { res->assign_from_value(args[0].as_bool()); } else
                    if(args[0].is_char_ref()) { res->assign_from_value(args[0].as_char()); } else
                    if(args[0].is_wchar_ref()) { res->assign_from_value(args[0].as_wchar()); } else
                    if(args[0].is_s8_ref()) { res->assign_from_value(args[0].as_s8()); } else
                    if(args[0].is_u8_ref()) { res->assign_from_value(args[0].as_u8()); } else
                    if(args[0].is_s16_ref()) { res->assign_from_value(args[0].as_s16()); } else
                    if(args[0].is_u16_ref()) { res->assign_from_value(args[0].as_u16()); } else
                    if(args[0].is_s32_ref()) { res->assign_from_value(args[0].as_s32()); } else
                    if(args[0].is_u32_ref()) { res->assign_from_value(args[0].as_u32()); } else
                    if(args[0].is_s64_ref()) { res->assign_from_value(args[0].as_s64()); } else
                    if(args[0].is_u64_ref()) { res->assign_from_value(args[0].as_u64()); } else
                    if(args[0].is_float_ref()) { res->assign_from_value(args[0].as_float()); } else
                    if(args[0].is_double_ref()) { res->assign_from_value(args[0].as_double()); } else
                    if(args[0].is_long_double_ref()) { res->assign_from_value(args[0].as_long_double()); } else {
                        throw std::runtime_error{"invalid argument(s)"};
                    }
                    return scfx::valbox{res, "array_buffer"};
                }
                throw std::runtime_error{"invalid argument(s)"};
            });
            rt->add_method("array_buffer", "size", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return SCFXTHIS(args, std::shared_ptr<array_buffer>)->size();
            });
            rt->add_method("array_buffer", "resize", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                SCFXTHIS(args, std::shared_ptr<array_buffer>)->resize(args[1].cast_to_size_t());
                return true;
            });
            rt->add_method("array_buffer", "fill_zero", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                SCFXTHIS(args, std::shared_ptr<array_buffer>)->fill_with(0);
                return true;
            });
            rt->add_method("array_buffer", "fill_with", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                SCFXTHIS(args, std::shared_ptr<array_buffer>)->fill_with(args[1].cast_to_char());
                return true;
            });
            rt->add_method("array_buffer", "assign", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                if(args[1].is_string_ref()) {
                    SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_string());
                    return true;
                } else if(args[1].is_class_ref() && args[1].class_name() == "array_buffer") {
                    SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(*SCFXCLASSARG(args, 1, std::shared_ptr<array_buffer>));
                    return true;
                } else if(args[1].is_any_int_number()) {
                    if(args[1].is_char_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_char());
                        return true;
                    } else if(args[1].is_wchar_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_wchar());
                        return true;
                    } else if(args[1].is_s8_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_s8());
                        return true;
                    } else if(args[1].is_u8_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_u8());
                        return true;
                    } else if(args[1].is_s16_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_s16());
                        return true;
                    } else if(args[1].is_u16_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_u16());
                        return true;
                    } else if(args[1].is_s32_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_s32());
                        return true;
                    } else if(args[1].is_u32_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_u32());
                        return true;
                    } else if(args[1].is_s64_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_s64());
                        return true;
                    } else if(args[1].is_u64_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->assign(args[1].as_u64());
                        return true;
                    }
                }
                throw std::runtime_error{"invalid argument(s)"};
            });
            rt->add_method("array_buffer", "sub_buffer", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                if(args.size() == 1) {
                    return valbox{SCFXTHIS(args, std::shared_ptr<array_buffer>)->sub_buffer(), "array_buffer"};
                } else if(args.size() == 2) {
                    return valbox{SCFXTHIS(args, std::shared_ptr<array_buffer>)->sub_buffer(args[1].cast_to_size_t()), "array_buffer"};
                } else if(args.size() == 3) {
                    return valbox{SCFXTHIS(args, std::shared_ptr<array_buffer>)->sub_buffer(args[1].cast_to_size_t(), args[2].cast_to_size_t()), "array_buffer"};
                }
                throw std::runtime_error{"invalid argument(s)"};
            });
            rt->add_method("array_buffer", "to_string", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<array_buffer>)->to_string();
            });
            rt->add_method("array_buffer", "append", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                if(args[1].is_string_ref()) {
                    SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_string());
                    return true;
                } else if(args[1].is_class_ref() && args[1].class_name() == "array_buffer") {
                    SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(*SCFXCLASSARG(args, 1, std::shared_ptr<array_buffer>));
                    return true;
                } else if(args[1].is_numeric()) {
                    if(args[1].is_bool_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_bool());
                        return true;
                    } else if(args[1].is_char_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_char());
                        return true;
                    } else if(args[1].is_wchar_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_wchar());
                        return true;
                    } else if(args[1].is_s8_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_s8());
                        return true;
                    } else if(args[1].is_u8_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_u8());
                        return true;
                    } else if(args[1].is_s16_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_s16());
                        return true;
                    } else if(args[1].is_u16_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_u16());
                        return true;
                    } else if(args[1].is_s32_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_s32());
                        return true;
                    } else if(args[1].is_u32_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_u32());
                        return true;
                    } else if(args[1].is_s64_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_s64());
                        return true;
                    } else if(args[1].is_u64_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_u64());
                        return true;
                    } else if(args[1].is_float_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_float());
                        return true;
                    } else if(args[1].is_double_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_double());
                        return true;
                    } else if(args[1].is_long_double_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->append(args[1].as_long_double());
                        return true;
                    }
                }
                throw std::runtime_error{"invalid argument(s)"};
            });
            rt->add_method("array_buffer", "put_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                if(args[1].is_string_ref()) {
                    SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_string(), args[2].cast_to_size_t());
                    return true;
                } else if(args[1].is_numeric()) {
                    if(args[1].is_bool_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_bool(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_char_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_char(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_wchar_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_wchar(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_s8_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_s8(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_u8_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_u8(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_s16_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_s16(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_u16_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_u16(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_s32_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_s32(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_u32_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_u32(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_s64_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_s64(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_u64_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_u64(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_float_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_float(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_double_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_double(), args[2].cast_to_size_t());
                        return true;
                    } else if(args[1].is_long_double_ref()) {
                        SCFXTHIS(args, std::shared_ptr<array_buffer>)->put_at(args[1].as_long_double(), args[2].cast_to_size_t());
                        return true;
                    }
                }
                throw std::runtime_error{"invalid argument(s)"};
            });
            rt->add_method("array_buffer", "get_bool_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<bool>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_char_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<char>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_wchar_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<wchar_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_i8_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<int8_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_u8_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<uint8_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_i16_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<int16_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_u16_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<uint16_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_i32_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<int32_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_u32_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<uint32_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_i64_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<int64_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_u64_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<uint64_t>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_f32_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<float>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_f64_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<double>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "get_float_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->get_at<long double>(args[1].cast_to_size_t())};
                return res;
            });
            rt->add_method("array_buffer", "bool_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<bool>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "char_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<char>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "wchar_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<wchar_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "i8_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<int8_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "u8_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<uint8_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "i16_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<int16_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "u16_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<uint16_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "i32_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<int32_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "u32_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<uint32_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "i64_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<int64_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "u64_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<uint64_t>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "f32_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<float>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "f64_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<double>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
            rt->add_method("array_buffer", "float_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{SCFXTHIS(args, std::shared_ptr<array_buffer>)->ref_at<long double>(args[1].cast_to_size_t())};
                res.set_pointed(args[0]);
                return res;
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("array_buffer");
            rt_->remove_method("array_buffer", "size");
            rt_->remove_method("array_buffer", "resize");
            rt_->remove_method("array_buffer", "fill_zero");
            rt_->remove_method("array_buffer", "fill_with");
            rt_->remove_method("array_buffer", "assign");
            rt_->remove_method("array_buffer", "sub_buffer");
            rt_->remove_method("array_buffer", "to_string");
            rt_->remove_method("array_buffer", "append");
            rt_->remove_method("array_buffer", "put_at");

            rt_->remove_method("array_buffer", "get_bool_at");
            rt_->remove_method("array_buffer", "get_char_at");
            rt_->remove_method("array_buffer", "get_wchar_at");
            rt_->remove_method("array_buffer", "get_i8_at");
            rt_->remove_method("array_buffer", "get_u8_at");
            rt_->remove_method("array_buffer", "get_i16_at");
            rt_->remove_method("array_buffer", "get_u16_at");
            rt_->remove_method("array_buffer", "get_i32_at");
            rt_->remove_method("array_buffer", "get_u32_at");
            rt_->remove_method("array_buffer", "get_i64_at");
            rt_->remove_method("array_buffer", "get_u64_at");
            rt_->remove_method("array_buffer", "get_f32_at");
            rt_->remove_method("array_buffer", "get_f64_at");
            rt_->remove_method("array_buffer", "get_float_at");

            rt_->remove_method("array_buffer", "bool_at");
            rt_->remove_method("array_buffer", "char_at");
            rt_->remove_method("array_buffer", "wchar_at");
            rt_->remove_method("array_buffer", "i8_at");
            rt_->remove_method("array_buffer", "u8_at");
            rt_->remove_method("array_buffer", "i16_at");
            rt_->remove_method("array_buffer", "u16_at");
            rt_->remove_method("array_buffer", "i32_at");
            rt_->remove_method("array_buffer", "u32_at");
            rt_->remove_method("array_buffer", "i64_at");
            rt_->remove_method("array_buffer", "u64_at");
            rt_->remove_method("array_buffer", "f32_at");
            rt_->remove_method("array_buffer", "f64_at");
            rt_->remove_method("array_buffer", "float_at");
            rt_ = nullptr;
        }

    private:
        class array_buffer {
        public:
            array_buffer() = default;
            array_buffer(std::string const &strbuf) { assign(strbuf); }
            array_buffer(std::wstring const &strbuf) { assign(strbuf.data(), strbuf.size() * sizeof(std::wstring::value_type)); }
            array_buffer(std::vector<std::uint8_t>::size_type s) { buf_.resize(s); }
            void resize(std::vector<std::uint8_t>::size_type s) { buf_.resize(s); }
            std::size_t size() const noexcept { return buf_.size(); }

            template<typename T>
                requires(std::is_fundamental_v<T>)
            void assign_from_value(T v) {
                buf_.resize(sizeof(T));
                std::copy(
                    reinterpret_cast<std::uint8_t const *>(&v),
                    reinterpret_cast<std::uint8_t const *>(&v) + sizeof(T),
                    buf_.data()
                );
            }
            void fill_with(std::uint8_t c) { for(auto &&cc: buf_) { cc = c; } }

            void assign(array_buffer const &that) { if(&that != this) { buf_ = that.buf_; } }

            void assign(void const *buf, std::size_t bufsiz) {
                buf_.assign(reinterpret_cast<std::uint8_t const *>(buf), reinterpret_cast<std::uint8_t const *>(buf) + bufsiz);
            }

            void assign(std::string const &strbuf) {
                buf_.assign(strbuf.begin(), strbuf.end());
            }

            template<typename T>
                requires(std::is_fundamental_v<T>)
            void assign(T val) {
                std::uint8_t const *val_ptr{reinterpret_cast<std::uint8_t const *>(&val)};
                buf_.assign(val_ptr, val_ptr + sizeof(val));
            }

            void append(array_buffer const &that) {
                buf_.insert(buf_.end(), that.buf_.cbegin(), that.buf_.cend());
            }

            void append(void const *buf, std::size_t bufsiz) {
                for(std::size_t i{}; i < bufsiz; ++i) {
                    buf_.push_back(reinterpret_cast<std::uint8_t const *>(buf)[i]);
                }
            }

            void append(std::string const &strbuf) {
                for(std::size_t i{}; i < strbuf.size(); ++i) {
                    buf_.push_back(static_cast<std::uint8_t>(strbuf[i]));
                }
            }

            void append(std::wstring const &strbuf) {
                for(std::size_t i{}; i < strbuf.size(); ++i) {
                    buf_.push_back(static_cast<std::uint8_t>(strbuf[i]));
                }
            }

            template<typename T>
                requires(std::is_fundamental_v<T>)
            void append(T val) {
                std::uint8_t const *val_ptr{reinterpret_cast<std::uint8_t const *>(&val)};
                for(std::size_t i{}; i < sizeof(T); ++i) {
                    buf_.push_back(val_ptr[i]);
                }
            }

            void put_at(array_buffer const &that, std::size_t at) {
                while(buf_.size() < at) { buf_.push_back(0); }
                for(std::size_t i{}; i < that.buf_.size(); ++i) {
                    if(at + i < buf_.size()) {
                        buf_[at + i] = that.buf_[i];
                    } else {
                        buf_.push_back(that.buf_[i]);
                    }
                }
            }

            void put_at(std::string const &val, std::size_t at) {
                while(buf_.size() < at) { buf_.push_back(0); }
                for(std::size_t i{}; i < val.size(); ++i) {
                    if(at + i < buf_.size()) {
                        buf_[at + i] = val[i];
                    } else {
                        buf_.push_back(val[i]);
                    }
                }
            }

            template<typename T>
                requires(std::is_fundamental_v<T>)
            void put_at(T val, std::size_t at) {
                std::uint8_t const *valptr{reinterpret_cast<std::uint8_t const *>(&val)};
                while(buf_.size() < at) { buf_.push_back(0); }
                for(std::size_t i{}; i < sizeof(val); ++i) {
                    if(at + i < buf_.size()) {
                        buf_[at + i] = valptr[i];
                    } else {
                        buf_.push_back(valptr[i]);
                    }
                }
            }

            template<typename T>
                requires(std::is_fundamental_v<std::remove_cvref_t<T>>)
            std::remove_cvref_t<T> get_at(std::size_t at) const {
                using res_t = std::remove_cvref_t<T>;
                if(buf_.size() < at + sizeof(res_t)) {
                    throw std::runtime_error{"requested data is out of buffer bounds"};
                }
                res_t res{};
                std::uint8_t const *dptr{buf_.data() + at};
                std::copy(dptr, dptr + sizeof(res_t), reinterpret_cast<std::uint8_t *>(&res));
                return res;
            }

            template<typename T>
                requires(std::is_fundamental_v<std::remove_cvref_t<T>>)
            valbox ref_at(std::size_t at) {
                if(buf_.size() < at + sizeof(std::remove_cvref_t<T>)) {
                    throw std::runtime_error{"requested data is out of buffer bounds"};
                }
                return valbox{reinterpret_cast<std::remove_cvref_t<T> *>(&buf_[at])};
            }

            std::string to_string() const {
                return std::string{buf_.begin(), buf_.end()};
            }

            std::shared_ptr<array_buffer> sub_buffer(size_t start = 0, size_t count = static_cast<size_t>(-1)) const {
                std::shared_ptr<array_buffer> res{std::make_shared<array_buffer>()};
                if(start >= buf_.size()) {
                    return res;
                }
                res->buf_.assign(
                    buf_.begin() + start,
                    buf_.begin() + start + (start + count <= buf_.size() ? count : buf_.size() - start)
                );
                return res;
            }

            std::vector<std::uint8_t> const &bytes() const noexcept {
                return buf_;
            }

        private:
            std::vector<std::uint8_t> buf_{};
        };

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
