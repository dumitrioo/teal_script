#pragma once

#include "../include/commondefs.hpp"
#include "../include/math/math_util.hpp"

#include "../scaflux_value.hpp"
#include "../scaflux_util.hpp"
#include "../scaflux_interfaces.hpp"

namespace scfx {

    class math_ext: public extension_interface {
    public:
        math_ext() = default;
        ~math_ext() {
            unregister_runtime();
        }
        math_ext(math_ext const &) = delete;
        math_ext &operator=(math_ext const &) = delete;
        math_ext(math_ext &&) = delete;
        math_ext &operator=(math_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) { return; }
            rt_ = rt;
            if(rt_ == nullptr) { return; }

            rt->add_function("abs", SCFXFUN(args) {
                static std::array<std::function<valbox(valbox const &)>, 25> const abses{
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return std::abs(v.as_char()); },
                    [](valbox const &v) -> valbox { return std::abs(v.as_s8()); },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return std::abs(v.as_s16()); },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return std::abs(v.as_s32()); },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return std::abs(v.as_s64()); },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return std::abs(v.as_float()); },
                    [](valbox const &v) -> valbox { return std::abs(v.as_double()); },
                    [](valbox const &v) -> valbox { return std::abs(v.as_long_double()); },
                    [](valbox const &v) -> valbox { return v.as_vec4().all_positive(); },
                    [](valbox const &v) -> valbox { return v.as_mat4().all_positive(); },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                    [](valbox const &v) -> valbox { return v; },
                };
                return abses[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });

            rt->add_function("mod", SCFXFUN(args) {
                static std::unordered_map<valbox::type, std::function<valbox(valbox const &, valbox const &)>> const abses{
                    { valbox::type::CHAR,        [](valbox const &x, valbox const &y) -> valbox { return x.as_char() % y.cast_num_to_num<char>() + y.cast_num_to_num<char>(); } },
                    { valbox::type::U8,          [](valbox const &x, valbox const &y) -> valbox { return x.as_u8() % y.cast_num_to_num<uint8_t>() + y.cast_num_to_num<uint8_t>(); } },
                    { valbox::type::S8,          [](valbox const &x, valbox const &y) -> valbox { return x.as_s8() % y.cast_num_to_num<int8_t>() + y.cast_num_to_num<int8_t>(); } },
                    { valbox::type::S16,         [](valbox const &x, valbox const &y) -> valbox { return x.as_s16() % y.cast_num_to_num<int16_t>() + y.cast_num_to_num<int16_t>(); } },
                    { valbox::type::U16,         [](valbox const &x, valbox const &y) -> valbox { return x.as_u16() % y.cast_num_to_num<uint16_t>() + y.cast_num_to_num<uint16_t>(); } },
                    { valbox::type::WCHAR,       [](valbox const &x, valbox const &y) -> valbox { return x.as_wchar() % y.cast_num_to_num<wchar_t>() + y.cast_num_to_num<wchar_t>(); } },
                    { valbox::type::S32,         [](valbox const &x, valbox const &y) -> valbox { return x.as_s32() % y.cast_num_to_num<int32_t>() + y.cast_num_to_num<int32_t>(); } },
                    { valbox::type::U32,         [](valbox const &x, valbox const &y) -> valbox { return x.as_u32() % y.cast_num_to_num<uint32_t>() + y.cast_num_to_num<uint32_t>(); } },
                    { valbox::type::S64,         [](valbox const &x, valbox const &y) -> valbox { return x.as_s64() % y.cast_num_to_num<int64_t>() + y.cast_num_to_num<int64_t>(); } },
                    { valbox::type::U64,         [](valbox const &x, valbox const &y) -> valbox { return x.as_u64() % y.cast_num_to_num<uint64_t>() + y.cast_num_to_num<uint64_t>(); } },
                    { valbox::type::FLOAT,       [](valbox const &x, valbox const &y) -> valbox { return std::fmod(x.as_float(), y.cast_num_to_num<float>()); } },
                    { valbox::type::DOUBLE,      [](valbox const &x, valbox const &y) -> valbox { return std::fmod(x.as_double(), y.cast_num_to_num<double>()); } },
                    { valbox::type::LONG_DOUBLE, [](valbox const &x, valbox const &y) -> valbox { return std::fmod(x.as_long_double(), y.cast_num_to_num<long double>()); } },
                    { valbox::type::CLASS,       [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::POINTER,     [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::FUNC,        [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::STRING,      [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::WSTRING,     [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::BOOL,        [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::UNDEFINED,        [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                };
                auto vt{args[0].val_or_pointed_type()};
                return abses.at(vt)(args[0], args[1]);
            });

            rt->add_function("remainder", SCFXFUN(args) {
                static std::unordered_map<valbox::type, std::function<valbox(valbox const &, valbox const &)>, valbox::valbox_type_hash> const abses{
                    { valbox::type::CHAR,        [](valbox const &x, valbox const &y) -> valbox { return x.as_char() % y.cast_num_to_num<char>() + y.cast_num_to_num<char>(); } },
                    { valbox::type::U8,          [](valbox const &x, valbox const &y) -> valbox { return x.as_u8() % y.cast_num_to_num<uint8_t>() + y.cast_num_to_num<uint8_t>(); } },
                    { valbox::type::S8,          [](valbox const &x, valbox const &y) -> valbox { return x.as_s8() % y.cast_num_to_num<int8_t>() + y.cast_num_to_num<int8_t>(); } },
                    { valbox::type::S16,         [](valbox const &x, valbox const &y) -> valbox { return x.as_s16() % y.cast_num_to_num<int16_t>() + y.cast_num_to_num<int16_t>(); } },
                    { valbox::type::U16,         [](valbox const &x, valbox const &y) -> valbox { return x.as_u16() % y.cast_num_to_num<uint16_t>() + y.cast_num_to_num<uint16_t>(); } },
                    { valbox::type::WCHAR,       [](valbox const &x, valbox const &y) -> valbox { return x.as_wchar() % y.cast_num_to_num<wchar_t>() + y.cast_num_to_num<wchar_t>(); } },
                    { valbox::type::S32,         [](valbox const &x, valbox const &y) -> valbox { return x.as_s32() % y.cast_num_to_num<int32_t>() + y.cast_num_to_num<int32_t>(); } },
                    { valbox::type::U32,         [](valbox const &x, valbox const &y) -> valbox { return x.as_u32() % y.cast_num_to_num<uint32_t>() + y.cast_num_to_num<uint32_t>(); } },
                    { valbox::type::S64,         [](valbox const &x, valbox const &y) -> valbox { return x.as_s64() % y.cast_num_to_num<int64_t>() + y.cast_num_to_num<int64_t>(); } },
                    { valbox::type::U64,         [](valbox const &x, valbox const &y) -> valbox { return x.as_u64() % y.cast_num_to_num<uint64_t>() + y.cast_num_to_num<uint64_t>(); } },
                    { valbox::type::FLOAT,       [](valbox const &x, valbox const &y) -> valbox { return std::fmod(x.as_float(), y.cast_num_to_num<float>()); } },
                    { valbox::type::DOUBLE,      [](valbox const &x, valbox const &y) -> valbox { return std::fmod(x.as_double(), y.cast_num_to_num<double>()); } },
                    { valbox::type::LONG_DOUBLE, [](valbox const &x, valbox const &y) -> valbox { return std::fmod(x.as_long_double(), y.cast_num_to_num<long double>()); } },
                    { valbox::type::CLASS,       [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::POINTER,     [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::FUNC,        [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::STRING,      [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::WSTRING,     [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::BOOL,        [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                    { valbox::type::UNDEFINED,        [](valbox const &x, valbox const &/*y*/) -> valbox { return x; } },
                };
                return abses.at(args[0].val_or_pointed_type())(args[0], args[1]);
            });

            rt->add_function("fma", SCFXFUN(args) {
                static std::unordered_map<valbox::type, std::function<valbox(valbox const &, valbox const &, valbox const &)>, valbox::valbox_type_hash> const abses{
                    { valbox::type::CHAR,        [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_char() * y.cast_num_to_num<char>() + z.cast_num_to_num<char>(); } },
                    { valbox::type::U8,          [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_u8() * y.cast_num_to_num<uint8_t>() + z.cast_num_to_num<uint8_t>(); } },
                    { valbox::type::S8,          [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_s8() * y.cast_num_to_num<int8_t>() + z.cast_num_to_num<int8_t>(); } },
                    { valbox::type::S16,         [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_s16() * y.cast_num_to_num<int16_t>() + z.cast_num_to_num<int16_t>(); } },
                    { valbox::type::U16,         [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_u16() * y.cast_num_to_num<uint16_t>() + z.cast_num_to_num<uint16_t>(); } },
                    { valbox::type::WCHAR,       [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_wchar() * y.cast_num_to_num<wchar_t>() + z.cast_num_to_num<wchar_t>(); } },
                    { valbox::type::S32,         [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_s32() * y.cast_num_to_num<int32_t>() + z.cast_num_to_num<int32_t>(); } },
                    { valbox::type::U32,         [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_u32() * y.cast_num_to_num<uint32_t>() + z.cast_num_to_num<uint32_t>(); } },
                    { valbox::type::S64,         [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_s64() * y.cast_num_to_num<int64_t>() + z.cast_num_to_num<int64_t>(); } },
                    { valbox::type::U64,         [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return x.as_u64() * y.cast_num_to_num<uint64_t>() + z.cast_num_to_num<uint64_t>(); } },
                    { valbox::type::FLOAT,       [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return std::fma(x.as_float(), y.cast_num_to_num<float>(), z.cast_num_to_num<float>()); } },
                    { valbox::type::DOUBLE,      [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return std::fma(x.as_double(), y.cast_num_to_num<double>(), z.cast_num_to_num<double>()); } },
                    { valbox::type::LONG_DOUBLE, [](valbox const &x, valbox const &y, valbox const &z) -> valbox { return std::fma(x.as_long_double(), y.cast_num_to_num<long double>(), z.cast_num_to_num<long double>()); } },
                    { valbox::type::CLASS,       [](valbox const &x, valbox const &/*y*/, valbox const &/*z*/) -> valbox { return x; } },
                    { valbox::type::POINTER,     [](valbox const &x, valbox const &/*y*/, valbox const &/*z*/) -> valbox { return x; } },
                    { valbox::type::FUNC,        [](valbox const &x, valbox const &/*y*/, valbox const &/*z*/) -> valbox { return x; } },
                    { valbox::type::STRING,      [](valbox const &x, valbox const &/*y*/, valbox const &/*z*/) -> valbox { return x; } },
                    { valbox::type::WSTRING,     [](valbox const &x, valbox const &/*y*/, valbox const &/*z*/) -> valbox { return x; } },
                    { valbox::type::BOOL,        [](valbox const &x, valbox const &/*y*/, valbox const &/*z*/) -> valbox { return x; } },
                    { valbox::type::UNDEFINED,        [](valbox const &x, valbox const &/*y*/, valbox const &/*z*/) -> valbox { return x; } },
                };
                return abses.at(args[0].val_or_pointed_type())(args[0], args[1], args[2]);
            });

            rt->add_function("deg2rad", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return scfx::math::deg2rad(args[0].cast_num_to_num<long double>()); });
            rt->add_function("rad2deg", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return scfx::math::rad2deg(args[0].cast_num_to_num<long double>()); });

            rt->add_function("gaussian", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                switch(args.size()) {
                    case 3: return scfx::math::gaussian<long double>(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>(), args[2].cast_num_to_num<long double>());
                    case 2: return scfx::math::gaussian<long double>(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>());
                    case 1: return scfx::math::gaussian<long double>(args[0].cast_num_to_num<long double>());
                    default: break;
                }
                return 0.0L;
            });

            rt->add_function("cos", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::cos(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::cos(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::cos(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::cos(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("sin", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::sin(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::sin(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::sin(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::sin(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("tan", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::tan(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::tan(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::tan(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::tan(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("acos", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::acos(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::acos(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::acos(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::acos(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("asin", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::asin(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::asin(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::asin(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::asin(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("atan", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::atan(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::atan(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::atan(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::atan(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("atan2", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return std::atan2(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>());
            });

            rt->add_function("cosh", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::cosh(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::cosh(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::cosh(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::cosh(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("sinh", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::sinh(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::sinh(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::sinh(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::sinh(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("tanh", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::tanh(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::tanh(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::tanh(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::tanh(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("acosh", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::acosh(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::acosh(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::acosh(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::acosh(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("asinh", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::asinh(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::asinh(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::asinh(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::asinh(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("atanh", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::atanh(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::atanh(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::atanh(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::atanh(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });

            rt->add_function("exp", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::exp(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::exp(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::exp(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::exp(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("frexp", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                int exp{};
                long double res{std::frexp(args[0].cast_num_to_num<long double>(), &exp)};
                args[1].assign(exp);
                return res;
            });
            rt->add_function("ldexp", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return std::ldexp(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>()); });
            rt->add_function("log", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::log(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::log(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::log(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::log(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("log10", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::log10(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::log10(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::log10(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::log10(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("modf", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                static std::array<std::function<valbox(valbox const &, long double &)>, 25> const funcs{
  /* BOOL */        [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_char()), &i); },
  /* S8 */          [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_s8()), &i); },
  /* U8 */          [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_u8()), &i); },
  /* S16 */         [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_s16()), &i); },
  /* U16 */         [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_u16()), &i); },
  /* WCHAR */       [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_wchar()), &i); },
  /* S32 */         [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_s32()), &i); },
  /* U32 */         [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_u32()), &i); },
  /* S64 */         [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_s64()), &i); },
  /* U64 */         [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_u64()), &i); },
  /* FLOAT */       [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_float()), &i); },
  /* DOUBLE */      [](valbox const &v, long double &i) -> valbox { return std::modf(static_cast<long double>(v.as_double()), &i); },
  /* LONG_DOUBLE */ [](valbox const &v, long double &i) -> valbox { return std::modf(v.as_long_double(), &i); },
  /* VEC4 */        [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & , long double & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & , long double & ) -> valbox { return valbox{}; },
                };
                long double i{};
                valbox res{funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0], i)};
                args[1].assign(i);
                return res;
            });
            rt->add_function("exp2", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::exp2(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::exp2(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::exp2(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::exp2(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("expm1", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::expm1(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::expm1(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::expm1(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::expm1(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("ilogb", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::ilogb(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::ilogb(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::ilogb(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::ilogb(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("log1p", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::log1p(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::log1p(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::log1p(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::log1p(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("log2", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::log2(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::log2(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::log2(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::log2(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("logb", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::logb(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::logb(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::logb(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::logb(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("scalbn", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return std::scalbn(args[0].cast_num_to_num<long double>(), args[1].cast_to_s64()); });
            rt->add_function("scalbln", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return std::scalbln(args[0].cast_num_to_num<long double>(), args[1].cast_to_s64()); });

            rt->add_function("pow", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return std::pow(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>()); });
            rt->add_function("square", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return scfx::math::square(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return scfx::math::square(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return scfx::math::square(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return scfx::math::square(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("qube", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return scfx::math::cube(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return scfx::math::cube(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return scfx::math::cube(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return scfx::math::cube(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("sqrt", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::sqrt(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::sqrt(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::sqrt(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::sqrt(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("cbrt", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::cbrt(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::cbrt(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::cbrt(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::cbrt(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("hypot", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return std::hypot(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>());
            });

            rt->add_function("erf", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::erf(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::erf(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::erf(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::erf(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("erfc", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::erfc(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::erfc(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::erfc(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::erfc(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("tgamma", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::tgamma(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::tgamma(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::tgamma(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::tgamma(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("lgamma", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::lgamma(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::lgamma(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::lgamma(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::lgamma(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });

            rt->add_function("ceil", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return static_cast<long double>(v.as_char()); },
  /* S8 */          [](valbox const &v) -> valbox { return static_cast<long double>(v.as_s8()); },
  /* U8 */          [](valbox const &v) -> valbox { return static_cast<long double>(v.as_u8()); },
  /* S16 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_s16()); },
  /* U16 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_u16()); },
  /* WCHAR */       [](valbox const &v) -> valbox { return static_cast<long double>(v.as_wchar()); },
  /* S32 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_s32()); },
  /* U32 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_u32()); },
  /* S64 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_s64()); },
  /* U64 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_u64()); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::ceil(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::ceil(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::ceil(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("floor", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return static_cast<long double>(v.as_char()); },
  /* S8 */          [](valbox const &v) -> valbox { return static_cast<long double>(v.as_s8()); },
  /* U8 */          [](valbox const &v) -> valbox { return static_cast<long double>(v.as_u8()); },
  /* S16 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_s16()); },
  /* U16 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_u16()); },
  /* WCHAR */       [](valbox const &v) -> valbox { return static_cast<long double>(v.as_wchar()); },
  /* S32 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_s32()); },
  /* U32 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_u32()); },
  /* S64 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_s64()); },
  /* U64 */         [](valbox const &v) -> valbox { return static_cast<long double>(v.as_u64()); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::floor(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::floor(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::floor(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("fmod", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return std::fmod(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>()); });
            rt->add_function("trunc", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::trunc(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::trunc(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::trunc(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::trunc(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("round", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::round(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::round(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::round(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::round(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("lround", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::lround(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::lround(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::lround(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::lround(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("llround", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::llround(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::llround(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::llround(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::llround(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("rint", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::rint(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::rint(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::rint(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::rint(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("lrint", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::lrint(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::lrint(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::lrint(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::lrint(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("llrint", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::llrint(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::llrint(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::llrint(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::llrint(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });
            rt->add_function("nearbyint", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                static std::array<std::function<valbox(valbox const &)>, 25> const funcs{
  /* BOOL */        [](valbox const & ) -> valbox { return valbox{}; },
  /* CHAR */        [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_char())); },
  /* S8 */          [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_s8())); },
  /* U8 */          [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_u8())); },
  /* S16 */         [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_s16())); },
  /* U16 */         [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_u16())); },
  /* WCHAR */       [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_wchar())); },
  /* S32 */         [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_s32())); },
  /* U32 */         [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_u32())); },
  /* S64 */         [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_s64())); },
  /* U64 */         [](valbox const &v) -> valbox { return std::nearbyint(static_cast<long double>(v.as_u64())); },
  /* FLOAT */       [](valbox const &v) -> valbox { return std::nearbyint(v.as_float()); },
  /* DOUBLE */      [](valbox const &v) -> valbox { return std::nearbyint(v.as_double()); },
  /* LONG_DOUBLE */ [](valbox const &v) -> valbox { return std::nearbyint(v.as_long_double()); },
  /* VEC4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* MAT4 */        [](valbox const & ) -> valbox { return valbox{}; },
  /* POINTER */     [](valbox const & ) -> valbox { return valbox{}; },
  /* CLASS */       [](valbox const & ) -> valbox { return valbox{}; },
  /* FUNC */        [](valbox const & ) -> valbox { return valbox{}; },
  /* ARRAY */       [](valbox const & ) -> valbox { return valbox{}; },
  /* OBJECT */      [](valbox const & ) -> valbox { return valbox{}; },
  /* STRING */      [](valbox const & ) -> valbox { return valbox{}; },
  /* WSTRING */     [](valbox const & ) -> valbox { return valbox{}; },
  /* UNDEFINED */   [](valbox const & ) -> valbox { return valbox{}; },
  /* VALBOX */      [](valbox const & ) -> valbox { return valbox{}; },
                };
                return funcs[static_cast<int>(args[0].val_or_pointed_type())](args[0]);
            });

            rt->add_function("sign", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                auto dr{args[0].deref()};
                switch(dr.val_or_pointed_type()) {
                    case valbox::type::BOOL: return (int64_t)dr.as_bool();
                    case valbox::type::CHAR: return (char)(dr.as_char() < 0 ? -1 : (dr.as_char() == 0 ? 0 : 1));
                    case valbox::type::S8: return (int8_t)(dr.as_s8() < 0 ? -1 : (dr.as_s8() == 0 ? 0 : 1));
                    case valbox::type::U8: return (uint8_t)(dr.as_u8() == 0 ? 0 : 1);
                    case valbox::type::S16: return (int16_t)(dr.as_s16() < 0 ? -1 : (dr.as_s16() == 0 ? 0 : 1));
                    case valbox::type::U16: return (uint16_t)(dr.as_u16() == 0 ? 0 : 1);
                    case valbox::type::WCHAR: return (wchar_t)(dr.as_wchar() < 0 ? -1 : (dr.as_wchar() == 0 ? 0 : 1));
                    case valbox::type::S32: return (int32_t)(dr.as_s32() < 0 ? -1 : (dr.as_s32() == 0 ? 0 : 1));
                    case valbox::type::U32: return (uint32_t)(dr.as_u32() == 0 ? 0 : 1);
                    case valbox::type::S64: return (int64_t)(dr.as_s64() < 0 ? -1 : (dr.as_s64() == 0 ? 0 : 1));
                    case valbox::type::U64: return (uint64_t)(dr.as_u64() == 0 ? 0 : 1);
                    case valbox::type::FLOAT: return (float)(dr.as_float() < 0 ? -1 : (dr.as_float() == 0 ? 0 : 1));
                    case valbox::type::DOUBLE: return (double)(dr.as_double() < 0 ? -1 : (dr.as_double() == 0 ? 0 : 1));
                    case valbox::type::LONG_DOUBLE: return (long double)(dr.as_long_double() < 0 ? -1 : (dr.as_long_double() == 0 ? 0 : 1));
                    case valbox::type::VEC4: return (int64_t)0;
                    case valbox::type::MAT4: return (int64_t)0;
                    case valbox::type::POINTER: return (int64_t)0;
                    case valbox::type::CLASS: return (int64_t)0;
                    case valbox::type::FUNC: return (int64_t)0;
                    case valbox::type::ARRAY: return (int64_t)0;
                    case valbox::type::OBJECT: return (int64_t)0;
                    case valbox::type::STRING: return (int64_t)0;
                    case valbox::type::WSTRING: return (int64_t)0;
                    case valbox::type::UNDEFINED: return (int64_t)0;
                    default: break;
                }
                return (int64_t)0;
            });

            rt->add_function("remquo", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                int i{};
                long double res{std::remquo(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>(), &i)};
                args[2].assign(i);
                return res;
            });

            rt->add_function("copysign", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return std::copysign(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>()); });
            rt->add_function("nan", SCFXFUN(args) { if(args.empty()) return std::nan(""); else return std::nan(args[0].cast_to_string().c_str()); });
            rt->add_function("nextafter", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return std::nextafter(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>()); });
            rt->add_function("nexttoward", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return std::nexttoward(args[0].cast_num_to_num<long double>(), args[1].cast_num_to_num<long double>()); });

            rt->add_function("isfp", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                auto t{args[0].val_or_pointed_type()};
                return t == valbox::type::FLOAT || t == valbox::type::DOUBLE || t == valbox::type::LONG_DOUBLE;
            });

            rt_->add_var("FP_INFINITE", static_cast<int>(FP_INFINITE));
            rt_->add_var("FP_NAN", static_cast<int>(FP_NAN));
            rt_->add_var("FP_ZERO", static_cast<int>(FP_ZERO));
            rt_->add_var("FP_SUBNORMAL", static_cast<int>(FP_SUBNORMAL));
            rt_->add_var("FP_NORMAL", static_cast<int>(FP_NORMAL));

            rt_->add_function("fpclassify", SCFXFUN(args) {
                switch(args[0].val_or_pointed_type()) {
                    case valbox::type::FLOAT: return std::fpclassify(args[0].as_float());
                    case valbox::type::DOUBLE: return std::fpclassify(args[0].as_double());
                    case valbox::type::LONG_DOUBLE: return std::fpclassify(args[0].as_long_double());
                    default: break;
                }
                throw std::runtime_error{"operation not applicable - floating point type required"};
            });
            rt->add_function("isfinite", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                switch(args[0].val_or_pointed_type()) {
                    case valbox::type::FLOAT: return std::isfinite(args[0].as_float());
                    case valbox::type::DOUBLE: return std::isfinite(args[0].as_double());
                    case valbox::type::LONG_DOUBLE: return std::isfinite(args[0].as_long_double());
                    default: break;
                }
                throw std::runtime_error{"operation not applicable - floating point type required"};
            });
            rt->add_function("isinf", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                switch(args[0].val_or_pointed_type()) {
                    case valbox::type::FLOAT: return std::isinf(args[0].as_float());
                    case valbox::type::DOUBLE: return std::isinf(args[0].as_double());
                    case valbox::type::LONG_DOUBLE: return std::isinf(args[0].as_long_double());
                    default: break;
                }
                throw std::runtime_error{"operation not applicable - floating point type required"};
            });
            rt->add_function("isnan", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                switch(args[0].val_or_pointed_type()) {
                    case valbox::type::FLOAT: return std::isnan(args[0].as_float());
                    case valbox::type::DOUBLE: return std::isnan(args[0].as_double());
                    case valbox::type::LONG_DOUBLE: return std::isnan(args[0].as_long_double());
                    default: break;
                }
                throw std::runtime_error{"operation not applicable - floating point type required"};
            });
            rt->add_function("isnormal", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                switch(args[0].val_or_pointed_type()) {
                    case valbox::type::FLOAT: return std::isnormal(args[0].as_float());
                    case valbox::type::DOUBLE: return std::isnormal(args[0].as_double());
                    case valbox::type::LONG_DOUBLE: return std::isnormal(args[0].as_long_double());
                    default: break;
                }
                throw std::runtime_error{"operation not applicable - floating point type required"};
            });
            rt->add_function("signbit", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                switch(args[0].val_or_pointed_type()) {
                    case valbox::type::FLOAT: return std::signbit(args[0].as_float());
                    case valbox::type::DOUBLE: return std::signbit(args[0].as_double());
                    case valbox::type::LONG_DOUBLE: return std::signbit(args[0].as_long_double());
                    default: break;
                }
                throw std::runtime_error{"operation not applicable - floating point type required"};
            });

            rt->add_function("rotation_z", SCFXFUN(args) {
                long double ang{0.0L};
                if(args.size() == 1 && (args[0].is_any_fp_number() || args[0].is_any_int_number())) {
                    ang = args[0].cast_to_long_double();
                }
                return valbox::mat4_t::rotate_z(ang);
            });
            rt->add_function("rotation_x", SCFXFUN(args) {
                long double ang{0.0L};
                if(args.size() == 1 && (args[0].is_any_fp_number() || args[0].is_any_int_number())) {
                    ang = args[0].cast_to_long_double();
                }
                return valbox::mat4_t::rotate_x(ang);
            });
            rt->add_function("rotation_y", SCFXFUN(args) {
                long double ang{0.0L};
                if(args.size() == 1 && (args[0].is_any_fp_number() || args[0].is_any_int_number())) {
                    ang = args[0].cast_to_long_double();
                }
                return valbox::mat4_t::rotate_y(ang);
            });
            rt->add_function("rotation", SCFXFUN(args) {
                long double ang{0.0L};
                long double vx{0.0L};
                long double vy{0.0L};
                long double vz{0.0L};
                if(
                    args.size() == 2 &&
                    (args[0].is_any_fp_number() || args[0].is_any_int_number()) &&
                    (args[1].is_array_ref() || args[1].is_vec4_ref())
                ) {
                    ang = args[0].cast_to_long_double();
                    if(args[1].is_array_ref()) {
                        auto const &a{args[1].as_array()};
                        if(a.size() > 0) { vx = a.at(0).cast_to_long_double(); }
                        if(a.size() > 1) { vy = a.at(1).cast_to_long_double(); }
                        if(a.size() > 2) { vz = a.at(2).cast_to_long_double(); }
                    } else if(args[1].is_vec4_ref()) {
                        auto const &a{args[1].as_vec4()};
                        vx = a[0];
                        vy = a[1];
                        vz = a[2];
                    }
                } else {
                    if(args.size() > 0) { ang = args[0].cast_to_long_double(); }
                    if(args.size() > 1) { vx = args[1].cast_to_long_double(); }
                    if(args.size() > 2) { vy = args[2].cast_to_long_double(); }
                    if(args.size() > 3) { vz = args[3].cast_to_long_double(); }
                }
                return valbox::mat4_t::rotation(ang, vx, vy, vz);
            });
            rt->add_function("identity", SCFXFUN() { return valbox::mat4_t::identity(); });
            rt->add_function("mirror_x", SCFXFUN() { return valbox::mat4_t::mirror_x(); });
            rt->add_function("mirror_y", SCFXFUN() { return valbox::mat4_t::mirror_y(); });
            rt->add_function("mirror_z", SCFXFUN() { return valbox::mat4_t::mirror_z(); });
            rt->add_function("length", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return args[0].as_vec4().length();
            });
            rt->add_function("at", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3) return args[0].as_mat4().at(args[1].cast_to_u64(), args[2].cast_to_u64()); });
            rt->add_function("x", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_vec4().x(); });
            rt->add_function("y", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_vec4().y(); });
            rt->add_function("z", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_vec4().z(); });
            rt->add_function("w", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_vec4().w(); });

            rt->add_function("spheric_to_cartesian", SCFXFUN(args) {
                if(args.size() == 1) {
                    if(args[0].is_array_ref()) {
                        long double vx{};
                        long double vy{};
                        long double vz{};
                        auto const &a{args[0].as_array()};
                        if(a.size() > 0) { vx = a.at(0).cast_to_long_double(); }
                        if(a.size() > 1) { vy = a.at(1).cast_to_long_double(); }
                        if(a.size() > 2) { vz = a.at(2).cast_to_long_double(); }
                        return scfx::math::spheric_to_cartesian<long double>(vx, vy, vz);
                    } else if(args[0].is_vec4_ref()) {
                        auto const &a{args[0].as_vec4()};
                        return scfx::math::spheric_to_cartesian<long double>(a.x(), a.y(), a.z());
                    }
                }
                long double vx{};
                long double vy{};
                long double vz{};
                if(args.size() > 0) { vx = args[0].cast_to_long_double(); }
                if(args.size() > 1) { vy = args[1].cast_to_long_double(); }
                if(args.size() > 2) { vz = args[2].cast_to_long_double(); }
                return scfx::math::spheric_to_cartesian<long double>(vx, vy, vz);
            });
            rt->add_function("cartesian_to_spheric", SCFXFUN(args) {
                if(args.size() == 1) {
                    if(args[0].is_array_ref()) {
                        long double vx{};
                        long double vy{};
                        long double vz{};
                        auto const &a{args[0].as_array()};
                        if(a.size() > 0) { vx = a.at(0).cast_to_long_double(); }
                        if(a.size() > 1) { vy = a.at(1).cast_to_long_double(); }
                        if(a.size() > 2) { vz = a.at(2).cast_to_long_double(); }
                        return scfx::math::cartesian_to_spheric<long double>(vx, vy, vz);
                    } else if(args[0].is_vec4_ref()) {
                        auto const &a{args[0].as_vec4()};
                        return scfx::math::cartesian_to_spheric<long double>(a.x(), a.y(), a.z());
                    }
                }
                long double vx{};
                long double vy{};
                long double vz{};
                if(args.size() > 0) { vx = args[0].cast_to_long_double(); }
                if(args.size() > 1) { vy = args[1].cast_to_long_double(); }
                if(args.size() > 2) { vz = args[2].cast_to_long_double(); }
                return scfx::math::cartesian_to_spheric<long double>(vx, vy, vz);
            });
            rt->add_function("vec4_r", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_vec4().r(); });
            rt->add_function("vec4_theta", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_vec4().theta(); });
            rt->add_function("vec4_phi", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_vec4().phi(); });

            rt->add_function("angle3d", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return scfx::math::angle3d(args[0].as_vec4(), args[1].as_vec4());
            });

            rt->add_function("normalized", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_vec4().normalized(); });
            rt->add_function("inverted", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].as_mat4().inverted(); });
            rt->add_function("translation", SCFXFUN(args) {
                long double dx{0.0L};
                long double dy{0.0L};
                long double dz{0.0L};
                if(args.size() == 1 && (args[0].is_array_ref() || args[0].is_vec4_ref())) {
                    if(args[0].is_array_ref()) {
                        auto const &a{args[0].as_array()};
                        if(a.size() > 0) { dx = a.at(0).cast_to_long_double(); }
                        if(a.size() > 1) { dy = a.at(1).cast_to_long_double(); }
                        if(a.size() > 2) { dz = a.at(2).cast_to_long_double(); }
                    } else if(args[0].is_vec4_ref()) {
                        auto const &a{args[0].as_vec4()};
                        dx = a[0];
                        dy = a[1];
                        dz = a[2];
                    }
                } else {
                    if(args.size() > 0) { dx = args[0].cast_to_long_double(); }
                    if(args.size() > 1) { dy = args[1].cast_to_long_double(); }
                    if(args.size() > 2) { dz = args[2].cast_to_long_double(); }
                }
                return valbox::mat4_t::translate(dx, dy, dz);
            });
            rt->add_function("perspective", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 4)
                return valbox::mat4_t::perspective(
                    args[0].cast_to_long_double(), // fov_y_degrees
                    args[1].cast_to_long_double(), // aspect_ratio
                    args[2].cast_to_long_double(), // znear
                    args[3].cast_to_long_double()  // zfar
                );
            });
            rt->add_function("look_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 9)
                return scfx::math::look_at_matrix<long double>(
                    args[0].cast_to_long_double(), // eyeX
                    args[1].cast_to_long_double(), // eyeY
                    args[2].cast_to_long_double(), // eyeZ
                    args[3].cast_to_long_double(), // centerX
                    args[4].cast_to_long_double(), // centerY
                    args[5].cast_to_long_double(), // centerZ
                    args[6].cast_to_long_double(), // upX
                    args[7].cast_to_long_double(), // upY
                    args[8].cast_to_long_double()  // upZ
                );
            });

        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }





            rt_ = nullptr;
        }

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
