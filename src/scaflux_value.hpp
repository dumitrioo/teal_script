#pragma once

#include <commondefs.hpp>
#include <file_util.hpp>
#include <str_util.hpp>
#include <json.hpp>
#include <math/vector4.hpp>
#include <math/matrix4.hpp>
#include <math/math_util.hpp>
#include "scaflux_util.hpp"

using std::any_cast;
using std::any;

namespace scfx {

    enum class valbox_no_initialize {
        dont_do_it
    };

    class valbox {
    public:
        enum class type {
            BOOL,
            CHAR,
            S8,
            U8,
            S16,
            U16,
            WCHAR,
            S32,
            U32,
            S64,
            U64,
            FLOAT,
            DOUBLE,
            LONG_DOUBLE,
            VEC4,
            MAT4,
            POINTER,
            CLASS,
            FUNC,
            ARRAY,
            OBJECT,
            STRING,
            WSTRING,
            UNDEFINED,
            VALBOX,
        };

        struct valbox_type_hash {
            std::size_t operator()(const type &t) const noexcept {
                return static_cast<std::size_t>(t);
            }
        };

        static type str_to_type(std::string const &idr) {
            static map_t<std::string, type> const tmap{
                {"bool", type::BOOL},
                {"char", type::CHAR},
                {"wchar", type::WCHAR},
                {"string", type::STRING},
                {"wstring", type::WSTRING},
                {"s8", type::S8},
                {"u8", type::U8},
                {"s16", type::S16},
                {"u16", type::U16},
                {"s32", type::S32},
                {"u32", type::U32},
                {"s64", type::S64},
                {"u64", type::U64},
                {"vec4", type::VEC4},
                {"mat4", type::MAT4},
                {"pointer", type::POINTER},
                {"f32", type::FLOAT},
                {"f64", type::DOUBLE},
                {"float", type::LONG_DOUBLE},
                {"func", type::FUNC},
                {"array", type::ARRAY},
                {"object", type::OBJECT},
                {"undefined", type::UNDEFINED},
                {"class", type::CLASS},
                {"valbox", type::VALBOX},
            };
            auto it{tmap.find(idr)};
            if(it == tmap.end()) {
                throw std::runtime_error{"invalid type name"};
            }
            return it->second;
        }

        static std::string type_to_str(type t) {
            static std::array<std::string_view, 25> const names{
                "bool", "char", "s8", "u8", "s16", "u16", "wchar", "s32", "u32", "s64", "u64",
                "float", "double", "long_double", "vec4", "mat4", "pointer", "class", "func",
                "array",  "object", "string", "wstring", "undefined", "valbox",
            };
            auto i{static_cast<std::size_t>(t)};
            if(i > 24) {
                throw std::runtime_error{"invalid type"};
            }
            return std::string{names[i]};
        }

        static bool is_type(std::string const &idr) {
            static std::set<std::string> const tset{
                "object", "array", "bool", "char", "wchar", "string", "wstring", "u8", "s8", "u16"
                "s16", "u32", "s32", "u64", "s64", "f32", "f64", "float", "vec4", "mat4", "func",
                "pointer", "undefined", "valbox"
            };
            return tset.find(idr) != tset.end();
        }

    private:
        static size_t type_size(type t) {
            static std::array<size_t, 25> const sizes{
                sizeof(bool), sizeof(char), sizeof(int8_t), sizeof(uint8_t), sizeof(int16_t), sizeof(uint16_t),
                sizeof(wchar_t), sizeof(int32_t), sizeof(uint32_t), sizeof(int64_t), sizeof(uint64_t),
                sizeof(float), sizeof(double), sizeof(long double), sizeof(vec4_t), sizeof(mat4_t),
                sizeof(void *), sizeof(void *), sizeof(std::function<valbox(valbox const &, std::vector<valbox> &)>),
                sizeof(array_t), sizeof(object_t), sizeof(std::string), sizeof(std::wstring), 0, 0,
            };
            auto i{static_cast<std::size_t>(t)};
            if(i > 24) {
                throw std::runtime_error{"invalid type"};
            }
            return sizes[i];
        }

    public:
        using vec4_t = scfx::math::vector4<long double>;
        using mat4_t = scfx::math::matrix4<long double>;

    private:
        using array_t = std::deque<valbox>;
        // using array_t = std::vector<valbox>;
        // using array_t = map_array<valbox>;
        using object_t = map_t<std::string, valbox>;
        using value_t = std::variant<
            bool,
            char,
            std::int8_t,
            std::uint8_t,
            std::int16_t,
            std::uint16_t,
            wchar_t,
            std::int32_t,
            std::uint32_t,
            std::int64_t,
            std::uint64_t,
            float,
            double,
            long double,
            vec4_t,
            mat4_t,
            void *,
            std::function<valbox(valbox const &, std::vector<valbox> &)>,
            array_t,
            object_t,
            std::string,
            std::wstring,
            any
        >;

        struct box_data {
            box_data() = default;
            box_data(value_t const &v, type t, type pointed_type = type::UNDEFINED, std::string const &c = {}, std::string const &func_name = {}, bool user_func = false):
                value_{v}, type_{t}, pointed_type_{pointed_type}, class_{c}, func_name_{func_name}, user_func_{user_func}
            {
            }
            box_data(value_t &&v, type t, type pointed_type = type::UNDEFINED, std::string const &c = {}, std::string const &func_name = {}, bool user_func = false):
                value_{std::move(v)}, type_{t}, pointed_type_{pointed_type}, class_{c}, func_name_{func_name}, user_func_{user_func}
            {
            }
            value_t value_{nullptr};
            type type_{type::UNDEFINED};
            type pointed_type_{type::UNDEFINED};
            std::string class_{};
            std::string func_name_{};
            bool user_func_{false};
        };

    public:
        valbox(): box_{std::make_shared<box_data>(value_t{}, type::UNDEFINED)} {}
        valbox(valbox_no_initialize) {}
        valbox(bool v): box_{std::make_shared<box_data>(v, type::BOOL)} {}
        valbox(bool *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::BOOL)} {}
        valbox(float v): box_{std::make_shared<box_data>(v, type::FLOAT)} {}
        valbox(float *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::FLOAT)} {}
        valbox(double v): box_{std::make_shared<box_data>(v, type::DOUBLE)} {}
        valbox(double *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::DOUBLE)} {}
        valbox(long double v): box_{std::make_shared<box_data>(v, type::LONG_DOUBLE)} {}
        valbox(long double *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::LONG_DOUBLE)} {}
        valbox(char v): box_{std::make_shared<box_data>(v, type::CHAR)} {}
        valbox(char *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::CHAR)} {}
        valbox(wchar_t v): box_{std::make_shared<box_data>(v, type::WCHAR)} {}
        valbox(wchar_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::WCHAR)} {}
        valbox(std::string const &v): box_{std::make_shared<box_data>(v, type::STRING)} {}
        valbox(std::string *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::STRING)} {}
        valbox(char const *v): box_{std::make_shared<box_data>(std::string{v}, type::STRING)} {}
        valbox(std::wstring const &v): box_{std::make_shared<box_data>(v, type::WSTRING)} {}
        valbox(std::wstring *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::WSTRING)} {}
        valbox(wchar_t const *v): box_{std::make_shared<box_data>(std::wstring{v}, type::WSTRING)} {}
        valbox(std::string &&v): box_{std::make_shared<box_data>(std::move(v), type::STRING)} {}
        valbox(std::wstring &&v): box_{std::make_shared<box_data>(std::move(v), type::WSTRING)} {}
        valbox(std::int8_t v): box_{std::make_shared<box_data>(v, type::S8)} {}
        valbox(std::int8_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::S8)} {}
        valbox(std::uint8_t v): box_{std::make_shared<box_data>(v, type::U8)} {}
        valbox(std::uint8_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::U8)} {}
        valbox(std::int16_t v): box_{std::make_shared<box_data>(v, type::S16)} {}
        valbox(std::int16_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::S16)} {}
        valbox(std::uint16_t v): box_{std::make_shared<box_data>(v, type::U16)} {}
        valbox(std::uint16_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::U16)} {}
        valbox(std::int32_t v): box_{std::make_shared<box_data>(v, type::S32)} {}
        valbox(std::int32_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::S32)} {}
        valbox(std::uint32_t v): box_{std::make_shared<box_data>(v, type::U32)} {}
        valbox(std::uint32_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::U32)} {}
        valbox(std::int64_t v): box_{std::make_shared<box_data>(v, type::S64)} {}
        valbox(std::int64_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::S64)} {}
        valbox(std::uint64_t v): box_{std::make_shared<box_data>(v, type::U64)} {}
        valbox(std::uint64_t *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::U64)} {}
        valbox(vec4_t const &v): box_{std::make_shared<box_data>(v, type::VEC4)} {}
        valbox(mat4_t const &v): box_{std::make_shared<box_data>(v, type::MAT4)} {}
        valbox(long long v): box_{std::make_shared<box_data>((std::int64_t)v, type::S64)} {}
        valbox(long long *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::S64)} {}
        valbox(unsigned long long v): box_{std::make_shared<box_data>((std::uint64_t)v, type::U64)} {}
        valbox(unsigned long long *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::U64)} {}
        valbox(scfx::json const &v): box_{std::make_shared<box_data>(value_t{}, type::UNDEFINED)} { from_json(v); }
        valbox(object_t const &v): box_{std::make_shared<box_data>(v, type::OBJECT)} {}
        valbox(void *v, type pointed_type = type::POINTER): box_{std::make_shared<box_data>(v, type::POINTER, pointed_type)} {}
        valbox(valbox *v): box_{std::make_shared<box_data>((void *)v, type::POINTER, type::VALBOX)} {}
        template<typename T>
        valbox(T &&v, std::string const &classname):
            box_{std::make_shared<box_data>(any{std::forward<T>(v)}, type::CLASS, type::UNDEFINED, classname)}
        {
        }
        valbox(std::function<valbox(valbox const &, std::vector<valbox> &)> const &v, std::string const &func_name, bool user_func):
            box_{std::make_shared<box_data>(v, type::FUNC, type::UNDEFINED, std::string{}, func_name, user_func)}
        {
        }
        valbox(std::function<valbox(valbox const &, std::vector<valbox> &)> &&v, std::string const &func_name, bool user_func):
            box_{std::make_shared<box_data>(std::move(v), type::FUNC, type::UNDEFINED, std::string{}, func_name, user_func)}
        {
        }

        void construct(array_t const &arr) {
            box_ = std::make_shared<box_data>(arr, type::ARRAY);
            pointed_box_.reset();
            array_t &a{as_array()};
            for(auto &&v: arr) {
                //a[v.first] = v.second.clone();
                a.push_back(v.clone());
            }
            a.resize(arr.size());
        }

        void construct(array_t &&arr) {
            box_ = std::make_shared<box_data>(std::move(arr), type::ARRAY);
        }

        template<typename T
#if (__cplusplus < 202000L)
                 , std::enable_if_t<std::is_constructible_v<valbox, typename T::value_type> && !std::is_same_v<valbox, typename T::value_type>, bool> = true
#endif
                 >
#if (__cplusplus >= 202000L)
            requires(
                std::is_constructible_v<valbox, typename T::value_type> &&
                !std::is_same_v<valbox, typename T::value_type>
            )
#endif
        valbox(T &&container): box_{std::make_shared<box_data>(array_t{}, type::ARRAY)} {
            auto &a{as_array()};
            for(auto &&v: container) {
                a.push_back(v);
            }
        }

        template<typename T
#if (__cplusplus < 202000L)
                 , std::enable_if_t<std::is_constructible_v<valbox, typename T::value_type> && !std::is_same_v<valbox, typename T::value_type>, bool> = true
#endif
                 >
#if (__cplusplus >= 202000L)
            requires(
                std::is_constructible_v<valbox, typename T::value_type> &&
                !std::is_same_v<valbox, typename T::value_type>
                )
#endif
        valbox(T const &container): box_{std::make_shared<box_data>(array_t{}, type::ARRAY)} {
            auto &a{as_array()};
            for(auto &&v: container) {
                a.push_back(v);
            }
        }

#if 0
        valbox(std::initializer_list<valbox> initlist):
            box_{std::make_shared<box_data>(array_t{}, type::ARRAY)}
        {
            for(auto &&v: initlist) {
                as_array().push_back(v);
            }
        }
#endif
        ~valbox() = default;
        valbox(valbox const &that):
            box_{that.box_},
            pointed_box_{that.pointed_box_}
        {
        }
        valbox(valbox &&that) noexcept:
            box_{std::move(that.box_)},
            pointed_box_{std::move(that.pointed_box_)}
        {
        }
        valbox &operator=(valbox const &that) {
            if(&that != this) {
                pointed_box_ = that.pointed_box_;
                box_ = that.box_;
            }
            return *this;
        }
        valbox &operator=(valbox &&that) noexcept {
            if(&that != this) {
                std::swap(box_, that.box_);
                std::swap(pointed_box_, that.pointed_box_);
            }
            return *this;
        }

        bool is_another_ref(valbox const &other) const {
            return box_.get() == other.box_.get();
        }

        valbox clone() const {
            valbox const &dr{deref()};
            type drt{dr.val_or_pointed_type()};
            if(drt == type::OBJECT) {
                object_t o{};
                for(auto &&p: dr.as_object()) {
                    o[p.first] = p.second.clone();
                }
                valbox res{std::make_shared<box_data>(
                        std::move(o),
                        dr.box_->type_,
                        dr.box_->pointed_type_,
                        dr.box_->class_
                    )
                };
                res.pointed_box_ = dr.pointed_box_;
                return res;
            } else if(drt == type::ARRAY) {
                array_t a{};
                // for(auto &&i: dr.as_array().m()) {
                //     a[i.first] = i.second.clone();
                // }
                for(auto &&i: dr.as_array()) {
                    // a[i.first] = i.second.clone();
                    a.push_back(i.clone());
                }
                a.resize(dr.as_array().size());
                valbox res{std::make_shared<box_data>(
                        std::move(a),
                        dr.box_->type_,
                        dr.box_->pointed_type_,
                        dr.box_->class_
                    )
                };
                res.pointed_box_ = dr.pointed_box_;
                return res;
            } else if(dr.box_) {
                valbox res{std::make_shared<box_data>(
                        dr.box_->value_,
                        dr.box_->type_,
                        dr.box_->pointed_type_,
                        dr.box_->class_
                    )
                };
                res.pointed_box_ = dr.pointed_box_;
                return res;
            } else {
                return valbox{valbox_no_initialize::dont_do_it};
            }
        }

        type val_type() const { return box_ ? box_->type_ : type::UNDEFINED; }
        type pointed_type() const {
            return box_ ?
                (box_->type_ == type::POINTER ? box_->pointed_type_ : type::UNDEFINED) :
                type::UNDEFINED;
        }
        type val_or_pointed_type() const {
            if(!box_) { return type::UNDEFINED; }
            return box_->type_ == type::POINTER ?
               (
                    box_->pointed_type_ == type::VALBOX ?
                    reinterpret_cast<valbox const *>(std::get<void *>(box_->value_))->val_or_pointed_type() :
                    box_->pointed_type_
               ) :
               box_->type_;
        }

        std::string val_class_name() const {
            return box_ ? box_->class_ : std::string{};
        }
        std::string ref_class_name() const {
            valbox const &dr{deref()};
            return dr.box_ ? dr.box_->class_ : std::string{};
        }

        template<typename T>
        std::remove_cv_t<T> &deref_ptr() { return *reinterpret_cast<std::remove_cv_t<T> *>(as_ptr()); }
        template<typename T>
        std::remove_cv_t<T> const &deref_ptr() const { return *reinterpret_cast<std::remove_cv_t<T> const *>(as_ptr()); }

        void *as_ptr() {
            if(!box_) { return nullptr; }
            return std::get<void *>(box_->value_);
        }
        void const *as_ptr() const {
            if(!box_) { return nullptr; }
            return std::get<void *>(box_->value_);
        }
        template<typename T>
        std::remove_cv_t<T> *as_typed_ptr() {
            if(!box_) { return nullptr; }
            return reinterpret_cast<std::remove_cv_t<T> *>(std::get<void *>(box_->value_));
        }
        template<typename T>
        std::remove_cv_t<T> const *as_typed_ptr() const {
            if(!box_) { return nullptr; }
            return reinterpret_cast<std::remove_cv_t<T> *>(std::get<void *>(box_->value_));
        }
        bool is_ptr() const { return box_ && box_->type_ == type::POINTER; }

        box_data *as_valbox_ptr() const {
            box_data const *box{box_.get()};
            return reinterpret_cast<box_data *>(
                static_cast<uintptr_t>(
                    box != nullptr &&
                    box->type_ == type::POINTER &&
                    box->pointed_type_ == type::VALBOX
                )
                *
                reinterpret_cast<uintptr_t>(box)
            );
        }
        valbox &deref() {
            box_data *bptr{as_valbox_ptr()};
            if(bptr != nullptr) {
                void *p{std::get<void *>(bptr->value_)};
                if(p == reinterpret_cast<void *>(this)) {
                    throw std::runtime_error{"reference loop"};
                }
                return reinterpret_cast<valbox *>(p)->deref();
            }
            return *this;
        }
        valbox const &deref() const {
            box_data *bptr{as_valbox_ptr()};
            if(bptr != nullptr) {
                void *p{std::get<void *>(bptr->value_)};
                if(p == reinterpret_cast<void const *>(this)) {
                    throw std::runtime_error{"reference loop"};
                }
                return reinterpret_cast<valbox const *>(p)->deref();
            }
            return *this;
        }

        template<typename T>
        T &as_class() {
            if(!box_) {
                throw std::runtime_error{"not an object"};
            }
            return any_cast<T &>(std::get<any>(deref().box_->value_));
        }
        template<typename T>
        T const &as_class() const {
            if(!box_) {
                throw std::runtime_error{"not an object"};
            }
            return any_cast<T &>(std::get<any>(deref().box_->value_));
        }
        bool is_class_ref() const { return val_or_pointed_type() == type::CLASS; }
        std::string class_name() const { return box_ ? deref().box_->class_ : std::string{}; }

        bool is_string_ref() const { return val_or_pointed_type() == type::STRING; }
        std::string &as_string() {
            if(!box_) { throw std::runtime_error{"not a string"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::STRING ? dr.deref_ptr<std::string>() :
                   std::get<std::string>(dr.box_->value_);
        }
        std::string const &as_string() const {
            if(!box_) { throw std::runtime_error{"not a string"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::STRING ? dr.deref_ptr<std::string>() :
                   std::get<std::string>(dr.box_->value_);
        }

        bool is_wstring_ref() const { return val_or_pointed_type() == type::WSTRING; }
        std::wstring &as_wstring() {
            if(!box_) { throw std::runtime_error{"not a wstring"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::WSTRING ? dr.deref_ptr<std::wstring>() :
                   std::get<std::wstring>(dr.box_->value_);
        }
        std::wstring const &as_wstring() const {
            if(!box_) { throw std::runtime_error{"not a wstring"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::WSTRING ? dr.deref_ptr<std::wstring>() :
                   std::get<std::wstring>(dr.box_->value_);
        }

        bool is_long_double_ref() const { return val_or_pointed_type() == type::LONG_DOUBLE; }
        long double &as_long_double() {
            if(!box_) { throw std::runtime_error{"not a floating point value"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::LONG_DOUBLE ? dr.deref_ptr<long double>() :
                    std::get<long double>(dr.box_->value_);
        }
        long double as_long_double() const {
            if(!box_) { throw std::runtime_error{"not a floating point value"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::LONG_DOUBLE ? deref_ptr<long double>() :
                    std::get<long double>(dr.box_->value_);
        }

        bool is_double_ref() const { return val_or_pointed_type() == type::DOUBLE; }
        double &as_double() {
            if(!box_) { throw std::runtime_error{"not a floating point value"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::DOUBLE ? dr.deref_ptr<double>() :
                       std::get<double>(dr.box_->value_);
        }
        double as_double() const {
            if(!box_) { throw std::runtime_error{"not a floating point value"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::DOUBLE ? dr.deref_ptr<double>() :
                       std::get<double>(dr.box_->value_);
        }

        bool is_float_ref() const { return val_or_pointed_type() == type::FLOAT; }
        float &as_float() {
            if(!box_) { throw std::runtime_error{"not a floating point value"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::FLOAT ? dr.deref_ptr<float>() :
                       std::get<float>(dr.box_->value_);
        }
        float as_float() const {
            if(!box_) { throw std::runtime_error{"not a floating point value"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::FLOAT ? dr.deref_ptr<float>() :
                       std::get<float>(dr.box_->value_);
        }

        bool is_bool_ref() const { return val_or_pointed_type() == type::BOOL; }
        bool &as_bool() {
            if(!box_) { throw std::runtime_error{"not a boolean value"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::BOOL ? dr.deref_ptr<bool>() :
                   std::get<bool>(dr.box_->value_);
        }
        bool as_bool() const {
            if(!box_) { throw std::runtime_error{"not a boolean value"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::BOOL ? dr.deref_ptr<bool>() :
                   std::get<bool>(dr.box_->value_);
        }

        bool is_char_ref() const { return val_or_pointed_type() == type::CHAR; }
        char &as_char() {
            if(!box_) { throw std::runtime_error{"not a character"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::CHAR ? dr.deref_ptr<char>() :
                   std::get<char>(dr.box_->value_);
        }
        char as_char() const {
            if(!box_) { throw std::runtime_error{"not a character"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::CHAR ? dr.deref_ptr<char>() :
                   std::get<char>(dr.box_->value_);
        }

        bool is_wchar_ref() const { return val_or_pointed_type() == type::WCHAR; }
        wchar_t &as_wchar() {
            if(!box_) { throw std::runtime_error{"not a character"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::WCHAR ? dr.deref_ptr<wchar_t>() :
                    std::get<wchar_t>(dr.box_->value_);
        }
        wchar_t as_wchar() const {
            if(!box_) { throw std::runtime_error{"not a character"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::WCHAR ? dr.deref_ptr<wchar_t>() :
                       std::get<wchar_t>(dr.box_->value_);
        }

        bool is_u8_ref() const { return val_or_pointed_type() == type::U8; }
        std::uint8_t &as_u8() {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::U8 ? dr.deref_ptr<std::uint8_t>() :
                       std::get<std::uint8_t>(dr.box_->value_);
        }
        std::uint8_t as_u8() const {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::U8 ? dr.deref_ptr<std::uint8_t>() :
                       std::get<std::uint8_t>(dr.box_->value_);
        }

        bool is_s8_ref() const { return val_or_pointed_type() == type::S8; }
        std::int8_t &as_s8() {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::S8 ? dr.deref_ptr<std::int8_t>() :
                       std::get<std::int8_t>(dr.box_->value_);
        }
        std::int8_t as_s8() const {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::S8 ? dr.deref_ptr<std::int8_t>() :
                   std::get<std::int8_t>(dr.box_->value_);
        }

        bool is_u16_ref() const { return val_or_pointed_type() == type::U16; }
        std::uint16_t &as_u16() {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::U16 ? dr.deref_ptr<std::uint16_t>() :
                   std::get<std::uint16_t>(dr.box_->value_);
        }
        std::uint16_t as_u16() const {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::U16 ? dr.deref_ptr<std::uint16_t>() :
                   std::get<std::uint16_t>(dr.box_->value_);
        }

        bool is_s16_ref() const { return val_or_pointed_type() == type::S16; }
        std::int16_t &as_s16() {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::S16 ? dr.deref_ptr<std::int16_t>() :
                   std::get<std::int16_t>(dr.box_->value_);
        }
        std::int16_t as_s16() const {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::S16 ? dr.deref_ptr<std::int16_t>() :
                   std::get<std::int16_t>(dr.box_->value_);
        }

        bool is_u32_ref() const { return val_or_pointed_type() == type::U32; }
        std::uint32_t &as_u32() {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::U32 ? dr.deref_ptr<std::uint32_t>() :
                   std::get<std::uint32_t>(dr.box_->value_);
        }
        std::uint32_t as_u32() const {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::U32 ? dr.deref_ptr<std::uint32_t>() :
                   std::get<std::uint32_t>(dr.box_->value_);
        }

        bool is_s32_ref() const { return val_or_pointed_type() == type::S32; }
        std::int32_t &as_s32() {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::S32 ? dr.deref_ptr<std::int32_t>() :
                   std::get<std::int32_t>(dr.box_->value_);
        }
        std::int32_t as_s32() const {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::S32 ? dr.deref_ptr<std::int32_t>() :
                   std::get<std::int32_t>(dr.box_->value_);
        }

        bool is_u64_ref() const { return val_or_pointed_type() == type::U64; }
        std::uint64_t &as_u64() {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::U64 ? dr.deref_ptr<std::uint64_t>() :
                   std::get<std::uint64_t>(dr.box_->value_);
        }
        std::uint64_t as_u64() const {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::U64 ? dr.deref_ptr<std::uint64_t>() :
                   std::get<std::uint64_t>(dr.box_->value_);
        }

        bool is_s64_ref() const { return val_or_pointed_type() == type::S64; }
        std::int64_t &as_s64() {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::S64 ? dr.deref_ptr<std::int64_t>() :
                   std::get<std::int64_t>(dr.box_->value_);
        }
        std::int64_t as_s64() const {
            if(!box_) { throw std::runtime_error{"not an integer"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::S64 ? dr.deref_ptr<std::int64_t>() :
                   std::get<std::int64_t>(dr.box_->value_);
        }

        std::string func_name() const {
            if(!box_) { throw std::runtime_error{"not a function"}; }
            valbox const &dr{deref()};
            if(dr.is_func_ref()) {
                return dr.box_->func_name_;
            } else {
                throw std::runtime_error{"not a function"};
            }
            return std::string{};
        }
        bool is_user_func() const {
            if(!box_) { return false; }
            valbox const &dr{deref()};
            if(dr.is_func_ref()) {
                return dr.box_->user_func_;
            }
            return false;
        }
        bool is_func_ref() const { return val_or_pointed_type() == type::FUNC; }
        std::function<valbox(valbox const &, std::vector<valbox> &)> const &as_func() {
            if(!box_) { throw std::runtime_error{"not a function"}; }
            valbox &dr{deref()};
            return std::get<std::function<valbox(valbox const &, std::vector<valbox> &)>>(dr.box_->value_);
        }
        std::function<valbox(valbox const &, std::vector<valbox> &)> const &as_func() const {
            if(!box_) { throw std::runtime_error{"not a function"}; }
            valbox const &dr{deref()};
            return std::get<std::function<valbox(valbox const &, std::vector<valbox> &)>>(dr.box_->value_);
        }

        bool is_vec4_ref() const { return val_or_pointed_type() == type::VEC4; }
        vec4_t &as_vec4() {
            if(!box_) { throw std::runtime_error{"not a vec4"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::VEC4 ? dr.deref_ptr<vec4_t>() :
                std::get<vec4_t>(dr.box_->value_);
        }
        vec4_t const &as_vec4() const {
            if(!box_) { throw std::runtime_error{"not a vec4"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::VEC4 ? dr.deref_ptr<vec4_t>() :
                std::get<vec4_t>(dr.box_->value_);
        }

        bool is_mat4_ref() const { return val_or_pointed_type() == type::MAT4; }
        mat4_t &as_mat4() {
            if(!box_) { throw std::runtime_error{"not a mat4"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::MAT4 ? dr.deref_ptr<mat4_t>() :
                std::get<mat4_t>(dr.box_->value_);
        }
        mat4_t const &as_mat4() const {
            if(!box_) { throw std::runtime_error{"not a mat4"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::MAT4 ? dr.deref_ptr<mat4_t>() :
                std::get<mat4_t>(dr.box_->value_);
        }

        bool is_array_ref() const { return val_or_pointed_type() == type::ARRAY; }
        array_t &as_array() {
            if(!box_) { throw std::runtime_error{"not array"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::ARRAY ? dr.deref_ptr<array_t>() :
                std::get<array_t>(dr.box_->value_);
        }
        array_t const &as_array() const {
            if(!box_) { throw std::runtime_error{"not array"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::ARRAY ? dr.deref_ptr<array_t>() :
                std::get<array_t>(dr.box_->value_);
        }

        valbox subarray(std::size_t start = 0, std::size_t n = std::numeric_limits<std::size_t>::max()) const {
            if(!is_array_ref()) {
                throw std::runtime_error{"not array"};
            }
            array_t const &a{as_array()};
            valbox res{};
            res.become_array();
            array_t &resarr{res.as_array()};
            if(n > 0 && start < a.size()) {
                //resarr.resize(std::min(n, a.size() - start));
                // for(auto it{a.m().lower_bound(8)}; it != a.m().end() && it->first - start < n; ++it) {
                //     resarr[it->first] = it->second.clone();
                // }
                std::size_t cnt{0};
                for(auto it{a.begin() + start}; it != a.end() && cnt < n; ++it) {
                    resarr.push_back(it->clone());
                    ++cnt;
                }
            }
            return res;
        }

        bool is_object_ref() const { return val_or_pointed_type() == type::OBJECT; }
        object_t &as_object() & {
            if(!box_) { throw std::runtime_error{"not an object"}; }
            valbox &dr{deref()};
            return dr.pointed_type() == type::OBJECT ? dr.deref_ptr<object_t>() :
                std::get<object_t>(dr.box_->value_);
        }
        object_t const &as_object() const & {
            if(!box_) { throw std::runtime_error{"not an object"}; }
            valbox const &dr{deref()};
            return dr.pointed_type() == type::OBJECT ? dr.deref_ptr<object_t>() :
                std::get<object_t>(dr.box_->value_);
        }

        bool is_undefined() const { return !box_ || box_->type_ == type::UNDEFINED; }
        bool is_undefined_ref() const { return val_or_pointed_type() == type::UNDEFINED; }

        valbox &become_array() {
            valbox &dr{deref()};
            if(!dr.box_) {
                dr.box_ = std::make_shared<box_data>(array_t{}, type::ARRAY);
                dr.pointed_box_.reset();
            } else if(dr.val_or_pointed_type() != type::ARRAY) {
                dr.box_->value_ = array_t{};
                dr.box_->type_ = type::ARRAY;
                dr.box_->class_.clear();
                dr.box_->pointed_type_ = type::UNDEFINED;
                dr.pointed_box_.reset();
            }
            return *this;
        }

        valbox &become_object() {
            valbox &dr{deref()};
            if(!dr.box_) {
                dr.box_ = std::make_shared<box_data>(object_t{}, type::OBJECT);
            } else if(dr.val_or_pointed_type() != type::OBJECT) {
                dr.box_->value_ = object_t{};
                dr.box_->type_ = type::OBJECT;
                dr.box_->class_.clear();
                dr.box_->pointed_type_ = type::UNDEFINED;
                dr.pointed_box_.reset();
            }
            return *this;
        }

        bool is_numeric() const {
            int t{(int)val_or_pointed_type()};
            return t >= (int)type::BOOL && t <= (int)type::LONG_DOUBLE;
        }

        bool is_any_fp_number() const {
            int t{(int)val_or_pointed_type()};
            return t >= (int)type::FLOAT && t <= (int)type::LONG_DOUBLE;
        }

        bool is_any_int_number() const {
            int t{(int)val_or_pointed_type()};
            return t >= (int)type::CHAR && t <= (int)type::U64;
        }

        bool is_any_signed_int_number() const {
            auto t{val_or_pointed_type()};
            return t == type::CHAR || t == type::S8 || t == type::S16 || t == type::S32 || t == type::S64;
        }

        bool is_any_unsigned_int_number() const {
            auto t{val_or_pointed_type()};
            return t == type::WCHAR || t == type::U8 || t == type::U16 || t == type::U32 || t == type::U64;
        }


        static bool is_numeric_type(type t) {
            int i{static_cast<int>(t)};
            return i >= static_cast<int>(type::BOOL) && i <= static_cast<int>(type::LONG_DOUBLE);
        }

        static bool is_any_fp_number_type(type t) {
            int i{static_cast<int>(t)};
            return i >= static_cast<int>(type::FLOAT) && i <= static_cast<int>(type::LONG_DOUBLE);
        }

        static bool is_any_int_number_type(type t) {
            int i{static_cast<int>(t)};
            return i >= static_cast<int>(type::CHAR) && i <= static_cast<int>(type::U64);
        }

        static bool is_any_signed_int_number_type(type t) {
            return t == type::CHAR || t == type::S8 || t == type::S16 || t == type::S32 || t == type::S64;
        }

        static bool is_any_unsigned_int_number_type(type t) {
            return t == type::WCHAR || t == type::U8 || t == type::U16 || t == type::U32 || t == type::U64;
        }

        static bool is_any_string_type(type t) {
            return t == type::WSTRING || t == type::STRING;
        }

        valbox operator[](valbox const &indx) {
            valbox const &indr{indx.deref()};
            valbox &der{deref()};
            auto t_of_indx{indr.val_or_pointed_type()};
            auto t{val_or_pointed_type()};
            if(t_of_indx == type::STRING) {
                if(is_undefined_ref()) {
                    become_object();
                }
                if(is_object_ref()) {
                    std::string s{indx.as_string()};
                    object_t &o{as_object()};
                    return &o[s];
                }
            } else if(t_of_indx == type::WSTRING) {
                if(t == type::UNDEFINED) {
                    become_object();
                }
                if(t == type::OBJECT) {
                    object_t &o{as_object()};
                    return o[scfx::str_util::to_utf8(indx.as_wstring())];
                }
            } else if(is_numeric_type(t_of_indx)) {
                std::uint64_t i{indx.cast_to_u64()};
                if(t == type::OBJECT) {
                    object_t &o{as_object()};
                    if(o.size() > i) {
                        std::size_t curr_indx{0};
                        for(auto &&kv: o) {
                            if(curr_indx == i) {
                                return &kv.second;
                            }
                            ++curr_indx;
                        }
                    } else {
                        throw std::range_error{"index out of range"};
                    }
                } else if(t == type::STRING) {
                    std::string &s{as_string()};
                    if(s.size() > i) {
                        return &s[i];
                    } else {
                        throw std::range_error{"index out of range"};
                    }
                } else if(t == type::WSTRING) {
                    std::wstring &s{as_wstring()};
                    if(s.size() > i) {
                        return &s[i];
                    } else {
                        throw std::range_error{"index out of range"};
                    }
                } else {
                    if(t == type::UNDEFINED) {
                        der.become_array();
                        t = type::ARRAY;
                    }
                    if(t == type::ARRAY) {
                        auto &a{as_array()};
                        if(a.size() <= i) { a.resize(i + 1); }
                        return as_array()[i];
                    }
                    if(t == type::MAT4) {
                        mat4_t &a{as_mat4()};
                        if(i < 4) {
                            return a[i].ptr();
                        } else {
                            throw std::range_error{"index out of range"};
                        }
                    }
                    if(t == type::VEC4) {
                        vec4_t &a{as_vec4()};
                        valbox res{&a[i]};
                        return res;
                    }
                    if(der.is_ptr()) {
                        if(der.pointed_type() == type::LONG_DOUBLE) {
                            valbox res{der.as_typed_ptr<long double>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::BOOL) {
                            valbox res{der.as_typed_ptr<bool>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::CHAR) {
                            valbox res{der.as_typed_ptr<char>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::S8) {
                            valbox res{der.as_typed_ptr<std::int8_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::U8) {
                            valbox res{der.as_typed_ptr<std::uint8_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::S16) {
                            valbox res{der.as_typed_ptr<std::int16_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::U16) {
                            valbox res{der.as_typed_ptr<std::uint16_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::WCHAR) {
                            valbox res{der.as_typed_ptr<wchar_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::S32) {
                            valbox res{der.as_typed_ptr<std::int32_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::U32) {
                            valbox res{der.as_typed_ptr<std::uint32_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::S64) {
                            valbox res{der.as_typed_ptr<std::int64_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::U64) {
                            valbox res{der.as_typed_ptr<std::uint64_t>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::FLOAT) {
                            valbox res{der.as_typed_ptr<float>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::DOUBLE) {
                            valbox res{der.as_typed_ptr<double>() + i};
                            return res;
                        }
                        if(der.pointed_type() == type::LONG_DOUBLE) {
                            valbox res{der.as_typed_ptr<long double>() + i};
                            return res;
                        }
                    }
                }
            }
            throw std::runtime_error{"operation not applicable"};
        }

        void become_undefined() {
            valbox &thisref{deref()};
            if(!thisref.is_undefined()) {
                thisref.box_->type_ = type::UNDEFINED;
                thisref.box_->pointed_type_ = type::UNDEFINED;
                thisref.box_->class_.clear();
                thisref.box_->value_ = value_t{};
                thisref.pointed_box_.reset();
            }
        }

        void become_same_type_as(valbox const &that) {
            valbox const &thatref{that.deref()};
            valbox &thisref{deref()};
            if(thatref.val_type() == thisref.val_type()) { return; }
            thisref.pointed_box_.reset();
            if(thatref.val_type() == type::UNDEFINED) {
                if(!thisref.is_undefined()) {
                    thisref.box_->type_ = type::UNDEFINED;
                    thisref.box_->pointed_type_ = type::UNDEFINED;
                    thisref.box_->class_.clear();
                    thisref.box_->value_ = value_t{};
                }
                return;
            }
            if(thisref.val_type() == type::UNDEFINED) { thisref.become_type(thatref.val_type()); return; }
            if(thisref.box_ && thatref.box_ && thisref.box_->type_ == thatref.box_->type_) { return; }
            if(!thatref.box_) { thisref.box_.reset(); return; }
            switch(thatref.box_->type_) {
                case type::U64:         thisref.box_->value_ = thisref.cast_to_u64(); break;
                case type::S64:         thisref.box_->value_ = thisref.cast_to_s64(); break;
                case type::CHAR:        thisref.box_->value_ = thisref.cast_to_char(); break;
                case type::U8:          thisref.box_->value_ = thisref.cast_to_u8(); break;
                case type::S8:          thisref.box_->value_ = thisref.cast_to_s8(); break;
                case type::U16:         thisref.box_->value_ = thisref.cast_to_u16(); break;
                case type::S16:         thisref.box_->value_ = thisref.cast_to_s16(); break;
                case type::U32:         thisref.box_->value_ = thisref.cast_to_u32(); break;
                case type::S32:         thisref.box_->value_ = thisref.cast_to_s32(); break;
                case type::FLOAT:       thisref.box_->value_ = thisref.cast_to_float(); break;
                case type::DOUBLE:      thisref.box_->value_ = thisref.cast_to_double(); break;
                case type::LONG_DOUBLE: thisref.box_->value_ = thisref.cast_to_long_double(); break;
                case type::BOOL:        thisref.box_->value_ = thisref.cast_to_bool(); break;
                case type::WCHAR:       thisref.box_->value_ = thisref.cast_to_wchar(); break;
                case type::STRING:      thisref.box_->value_ = thisref.cast_to_string(); break;
                case type::WSTRING:     thisref.box_->value_ = thisref.cast_to_wstring(); break;
                case type::UNDEFINED:
                    if(thisref.box_) {
                        thisref.box_->type_ = type::UNDEFINED;
                        thisref.box_->pointed_type_ = type::UNDEFINED;
                        thisref.box_->class_.clear();
                        thisref.box_->value_ = value_t{};
                    }
                    break;
                case type::ARRAY:       thisref.box_->value_ = thisref.cast_to_array(); break;
                case type::OBJECT:      thisref.box_->value_ = thisref.cast_to_object(); break;
                default: return;
            }
            thisref.box_->type_ = thatref.box_->type_;
        }

        void become_type(type t) {
            valbox &vref{deref()};
            vref.pointed_box_.reset();
            if(!vref.box_) {
                switch(t) {
                    case type::U64:         vref.box_ = std::make_shared<box_data>((std::uint64_t)0, type::U64); break;
                    case type::S64:         vref.box_ = std::make_shared<box_data>((std::int64_t)0, type::S64); break;
                    case type::CHAR:        vref.box_ = std::make_shared<box_data>((char)0, type::CHAR); break;
                    case type::U8:          vref.box_ = std::make_shared<box_data>((std::uint8_t)0, type::U8); break;
                    case type::S8:          vref.box_ = std::make_shared<box_data>((std::int8_t)0, type::S8); break;
                    case type::U16:         vref.box_ = std::make_shared<box_data>((std::uint16_t)0, type::U16); break;
                    case type::S16:         vref.box_ = std::make_shared<box_data>((std::int16_t)0, type::S16); break;
                    case type::U32:         vref.box_ = std::make_shared<box_data>((std::uint32_t)0, type::U32); break;
                    case type::S32:         vref.box_ = std::make_shared<box_data>((std::int32_t)0, type::S32); break;
                    case type::FLOAT:       vref.box_ = std::make_shared<box_data>((float)0, type::FLOAT); break;
                    case type::DOUBLE:      vref.box_ = std::make_shared<box_data>((double)0, type::DOUBLE); break;
                    case type::LONG_DOUBLE: vref.box_ = std::make_shared<box_data>((long double)0, type::LONG_DOUBLE); break;
                    case type::BOOL:        vref.box_ = std::make_shared<box_data>(false, type::BOOL); break;
                    case type::WCHAR:       vref.box_ = std::make_shared<box_data>((wchar_t)0, type::WCHAR); break;
                    case type::STRING:      vref.box_ = std::make_shared<box_data>(std::string{}, type::STRING); break;
                    case type::WSTRING:     vref.box_ = std::make_shared<box_data>(std::wstring{}, type::WSTRING); break;
                    case type::ARRAY:       vref.box_ = std::make_shared<box_data>(array_t{}, type::ARRAY); break;
                    case type::OBJECT:      vref.box_ = std::make_shared<box_data>(object_t{}, type::OBJECT); break;
                    default: break;
                }
                return;
            }
            if(vref.box_->type_ == t) {
                return;
            }
            switch(t) {
                case type::U64:         vref.box_->value_ = cast_to_u64(); break;
                case type::S64:         vref.box_->value_ = cast_to_s64(); break;
                case type::CHAR:        vref.box_->value_ = cast_to_char(); break;
                case type::U8:          vref.box_->value_ = cast_to_u8(); break;
                case type::S8:          vref.box_->value_ = cast_to_s8(); break;
                case type::U16:         vref.box_->value_ = cast_to_u16(); break;
                case type::S16:         vref.box_->value_ = cast_to_s16(); break;
                case type::U32:         vref.box_->value_ = cast_to_u32(); break;
                case type::S32:         vref.box_->value_ = cast_to_s32(); break;
                case type::FLOAT:       vref.box_->value_ = cast_to_float(); break;
                case type::DOUBLE:      vref.box_->value_ = cast_to_double(); break;
                case type::LONG_DOUBLE: vref.box_->value_ = cast_to_long_double(); break;
                case type::BOOL:        vref.box_->value_ = cast_to_bool(); break;
                case type::WCHAR:       vref.box_->value_ = cast_to_wchar(); break;
                case type::STRING:      vref.box_->value_ = cast_to_string(); break;
                case type::WSTRING:     vref.box_->value_ = cast_to_wstring(); break;
                case type::UNDEFINED:
                    if(vref.box_) {
                        vref.box_->type_ = type::UNDEFINED;
                        vref.box_->pointed_type_ = type::UNDEFINED;
                        vref.box_->class_.clear();
                        vref.box_->value_ = value_t{};
                    }
                    break;
                case type::ARRAY:       vref.box_->value_ = cast_to_array(); break;
                case type::OBJECT:      vref.box_->value_ = cast_to_object(); break;
                default: return;
            }
            vref.box_->pointed_type_ = type::UNDEFINED;
            vref.box_->class_.clear();
            vref.box_->type_ = t;
        }

        template<typename T>
        T cast_num_to_num() const {
            switch(val_or_pointed_type()) {
                case type::BOOL: return as_bool();
                case type::CHAR: return as_char();
                case type::S8: return as_s8();
                case type::U8: return as_u8();
                case type::S16: return as_s16();
                case type::U16: return as_u16();
                case type::WCHAR: return as_wchar();
                case type::S32: return as_s32();
                case type::U32: return as_u32();
                case type::S64: return as_s64();
                case type::U64: return as_u64();
                case type::FLOAT: return as_float();
                case type::DOUBLE: return as_double();
                case type::LONG_DOUBLE: return as_long_double();
                case type::VEC4: return as_vec4()[0];
                case type::MAT4: return as_mat4()[0][0];
                case type::POINTER: return (uintptr_t)as_ptr();
                case type::UNDEFINED: return 0;
                default: break;
            }
            throw std::runtime_error{"not a numeric type"};
        }

        valbox operator-() const {
            type t{val_or_pointed_type()};
            switch(t) {
                case type::LONG_DOUBLE: return -as_long_double();
                case type::DOUBLE: return -as_double();
                case type::FLOAT: return -as_float();
                case type::CHAR: return -as_char();
                case type::WCHAR: return -as_wchar();
                case type::S8: return -as_s8();
                case type::U8: return -as_u8();
                case type::S16: return -as_s16();
                case type::U16: return -as_u16();
                case type::S32: return -as_s32();
                case type::U32: return -as_u32();
                case type::S64: return -as_s64();
                case type::U64: return -as_u64();
                case type::VEC4: return -as_vec4();
                case type::UNDEFINED: return 0LL;
                default: throw std::runtime_error{"operation not applicable"};
            }
        }

        valbox &operator++() {
            type t{val_or_pointed_type()};
            switch(t) {
                case type::S64: ++as_s64(); break;
                case type::U64: ++as_u64(); break;
                case type::S32: ++as_s32(); break;
                case type::U32: ++as_u32(); break;
                case type::S8: ++as_s8(); break;
                case type::U8: ++as_u8(); break;
                case type::S16: ++as_s16(); break;
                case type::U16: ++as_u16(); break;
                case type::CHAR: ++as_char(); break;
                case type::WCHAR: ++as_wchar(); break;
                case type::BOOL: as_bool() = true; break;
                case type::UNDEFINED: {
                    valbox &dr{deref()};
                    if(!dr.box_) {
                        dr.box_ = std::make_shared<box_data>(static_cast<std::int64_t>(1), type::S64);
                    } else {
                        dr.box_->value_ = static_cast<std::int64_t>(1);
                        dr.box_->type_ = type::S64;
                    }
                    break;
                }
                default:
                    throw std::runtime_error{"operation not applicable"};
            }
            return *this;
        }

        valbox operator++(int) {
            type t{val_or_pointed_type()};
            switch(t) {
                case type::S64: { valbox res{as_s64()}; ++as_s64(); return res; }
                case type::U64: { valbox res{as_u64()}; ++as_u64(); return res; }
                case type::S32: { valbox res{as_s32()}; ++as_s32(); return res; }
                case type::U32: { valbox res{as_u32()}; ++as_u32(); return res; }
                case type::S8: { valbox res{as_s8()}; ++as_s8(); return res; }
                case type::U8: { valbox res{as_u8()}; ++as_u8(); return res; }
                case type::S16: { valbox res{as_s16()}; ++as_s16(); return res; }
                case type::U16: { valbox res{as_u16()}; ++as_u16(); return res; }
                case type::CHAR: { valbox res{as_char()}; ++as_char(); return res; }
                case type::WCHAR: { valbox res{as_wchar()}; ++as_wchar(); return res; }
                case type::BOOL: { valbox res{as_bool()}; as_bool() = true; return res; }
                case type::UNDEFINED: {
                    valbox res{static_cast<std::int64_t>(0)};
                    valbox &dr{deref()};
                    dr.pointed_box_.reset();
                    if(!dr.box_) {
                        dr.box_ = std::make_shared<box_data>(static_cast<std::int64_t>(1), type::S64);
                    } else {
                        dr.box_->value_ = static_cast<std::int64_t>(1);
                        dr.box_->type_ = type::S64;
                    }
                    return res;
                }
                default:
                    throw std::runtime_error{"operation not applicable"};
            }
        }

        valbox &operator--() {
            type t{val_or_pointed_type()};
            switch(t) {
            case type::S64: --as_s64(); break;
            case type::U64: --as_u64(); break;
            case type::S32: --as_s32(); break;
            case type::U32: --as_u32(); break;
            case type::S8: --as_s8(); break;
            case type::U8: --as_u8(); break;
            case type::S16: --as_s16(); break;
            case type::U16: --as_u16(); break;
            case type::CHAR: --as_char(); break;
            case type::WCHAR: --as_wchar(); break;
            case type::BOOL: as_bool() = false; break;
            case type::UNDEFINED: {
                auto dr{deref()};
                dr.pointed_box_.reset();
                if(!dr.box_) {
                    dr.box_ = std::make_shared<box_data>(static_cast<std::int64_t>(-1), type::S64);
                } else {
                    dr.box_->value_ = static_cast<std::int64_t>(-1);
                    dr.box_->type_ = type::S64;
                }
                break;
            }
            default:
                throw std::runtime_error{"operation not applicable"};
            }
            return *this;
        }

        valbox operator--(int) {
            type t{val_or_pointed_type()};
            switch(t) {
                case type::S64: { valbox res{as_s64()}; --as_s64(); return res; }
                case type::U64: { valbox res{as_u64()}; --as_u64(); return res; }
                case type::S32: { valbox res{as_s32()}; --as_s32(); return res; }
                case type::U32: { valbox res{as_u32()}; --as_u32(); return res; }
                case type::S8: { valbox res{as_s8()}; --as_s8(); return res; }
                case type::U8: { valbox res{as_u8()}; --as_u8(); return res; }
                case type::S16: { valbox res{as_s16()}; --as_s16(); return res; }
                case type::U16: { valbox res{as_u16()}; --as_u16(); return res; }
                case type::CHAR: { valbox res{as_char()}; --as_char(); return res; }
                case type::WCHAR: { valbox res{as_wchar()}; --as_wchar(); return res; }
                case type::BOOL: { valbox res{as_bool()}; as_bool() = false; return res; }
                case type::UNDEFINED: {
                    valbox res{static_cast<std::int64_t>(0)};
                    valbox &dr{deref()};
                    dr.pointed_box_.reset();
                    if(!dr.box_) {
                        dr.box_ = std::make_shared<box_data>(static_cast<std::int64_t>(-1), type::S64);
                    } else {
                        dr.box_->value_ = static_cast<std::int64_t>(-1);
                        dr.box_->type_ = type::S64;
                    }
                    return res;
                }
                default:
                    throw std::runtime_error{"operation not applicable"};
            }
        }

        valbox operator~() const {
            auto t{val_or_pointed_type()};
            switch(t) {
                case type::CHAR: return (char)~as_char();
                case type::WCHAR: return (wchar_t)~as_wchar();
                case type::BOOL: return !as_bool();
                case type::S8: return (std::int8_t)~as_s8();
                case type::U8: return (std::uint8_t)~as_u8();
                case type::S16: return (std::int16_t)~as_s16();
                case type::U16: return (std::uint16_t)~as_u16();
                case type::S32: return (std::int32_t)~as_s32();
                case type::U32: return (std::uint32_t)~as_u32();
                case type::S64: return (std::int64_t)~as_s64();
                case type::U64: return (std::uint64_t)~as_u64();
                default: throw std::runtime_error{"operation not applicable"};
            }
        }

        valbox &operator+() {
            return *this;
        }

        static bool stronger_numeric_type(type t1, type t2, type &res) {
            int it1{static_cast<int>(t1)};
            int it2{static_cast<int>(t2)};
            if(
                it1 >= static_cast<int>(type::BOOL) && it2 >= static_cast<int>(type::BOOL)
                &&
                it1 <= static_cast<int>(type::LONG_DOUBLE) && it2 <= static_cast<int>(type::LONG_DOUBLE)
            ) {
                res = it1 >= it2 ? t1 : t2;
                return true;
            }
            return false;
        }

        static type stronger_type(type t1, type t2) {
            return static_cast<int>(t1) > static_cast<int>(t2) ? t1 : t2;
        }

        valbox &operator+=(valbox const &other) {
            valbox &l{deref()};
            valbox const &r{other.deref()};
            auto ltype{l.val_or_pointed_type()};
            auto rtype{r.val_or_pointed_type()};
            if(ltype == rtype) {
                switch(ltype) {
                    case type::BOOL: l.as_bool() = l.as_bool() || r.as_bool(); break;
                    case type::CHAR: l.as_char() += r.as_char(); break;
                    case type::S8: l.as_s8() += r.as_s8(); break;
                    case type::U8: l.as_u8() += r.as_u8(); break;
                    case type::S16: l.as_s16() += r.as_s16(); break;
                    case type::U16: l.as_u16() += r.as_u16(); break;
                    case type::WCHAR: l.as_wchar() += r.as_wchar(); break;
                    case type::S32: l.as_s32() += r.as_s32(); break;
                    case type::U32: l.as_u32() += r.as_u32(); break;
                    case type::S64: l.as_s64() += r.as_s64(); break;
                    case type::U64: l.as_u64() += r.as_u64(); break;
                    case type::FLOAT: l.as_float() += r.as_float(); break;
                    case type::DOUBLE: l.as_double() += r.as_double(); break;
                    case type::LONG_DOUBLE: l.as_long_double() += r.as_long_double(); break;
                    case type::VEC4: l.as_vec4() += r.as_vec4(); break;
                    case type::ARRAY: {
                            array_t &la{l.as_array()};
                            // size_t lasize{la.size()};
                            // r.as_array().traverse_real([&](size_t i, valbox const &v) {
                            //     la[lasize + i] = v.deref().clone();
                            // });
                            for(auto &&el: la) {
                                la.push_back(el.deref().clone());
                            }
                        }
                        break;
                    case type::OBJECT:
                        for(auto &&p: r.as_object()) {
                            if(l.as_object().find(p.first) == l.as_object().end()) {
                                l[p.first] = p.second;
                            }
                        }
                        break;
                    case type::STRING: l.as_string() += r.as_string(); break;
                    case type::WSTRING: l.as_wstring() += r.as_wstring(); break;
                    case type::UNDEFINED: break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
            } else {
                switch(ltype) {
                    case type::BOOL: l.as_bool() = l.as_bool() || r.cast_to_bool(); break;
                    case type::CHAR: if(r.is_numeric()) { l.as_char() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::S8: if(r.is_numeric()) { l.as_s8() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::U8: if(r.is_numeric()) { l.as_u8() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::S16: if(r.is_numeric()) { l.as_s16() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::U16: if(r.is_numeric()) { l.as_u16() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::WCHAR: if(r.is_numeric()) { l.as_wchar() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::S32: if(r.is_numeric()) { l.as_s32() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::U32: if(r.is_numeric()) { l.as_u32() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::S64: if(r.is_numeric()) { l.as_s64() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::U64: if(r.is_numeric()) { l.as_u64() += r.cast_to_s64(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::FLOAT: if(r.is_numeric()) { l.as_float() += r.cast_to_long_double(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::DOUBLE: if(r.is_numeric()) { l.as_double() += r.cast_to_long_double(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::LONG_DOUBLE: if(r.is_numeric()) { l.as_long_double() += r.cast_to_long_double(); } else { throw std::runtime_error{"operation not applicable"}; } break;
                    case type::VEC4:
                        if(r.is_numeric()) {
                            l.as_vec4() += r.cast_to_long_double();
                        } else {
                            switch(rtype) {
                                case type::VEC4:
                                    l.as_vec4() += r.as_vec4();
                                    break;
                                case type::STRING:
                                    l.as_vec4() = vec4_from_str<long double>(r.as_string());
                                    break;
                                case type::WSTRING:
                                    l.as_vec4() = vec4_from_str<long double>(r.as_wstring());
                                    break;
                                case type::ARRAY:
                                case type::OBJECT:
                                    l.as_vec4() = l.as_vec4() + vec4_from_json<long double>(r.to_json());
                                    break;
                                default:
                                    throw std::runtime_error{"operation not applicable"};
                            }
                        }
                        break;
                    case type::ARRAY:
                        switch(rtype) {
                            case type::ARRAY: {
                                    // auto thissize{l.as_array().size()};
                                    // for(auto &&v: r.as_array().m()) {
                                    //     l.as_array()[thissize + v.first] = v.second.deref().clone();
                                    // }
                                    for(auto &&el: r.as_array()) {
                                        l.as_array().push_back(el.deref().clone());
                                    }
                                }
                                break;
                            default: l.as_array().push_back(r);
                        }
                        break;
                    case type::OBJECT:
                        switch(rtype) {
                            case type::OBJECT:
                                for(auto &&p: r.as_object()) {
                                    if(l.as_object().find(p.first) == l.as_object().end()) {
                                        l[p.first] = p.second;
                                    }
                                }
                                break;
                            default: throw std::runtime_error{"operation not applicable"};
                        }
                        break;
                    case type::STRING:
                        switch(rtype) {
                            case type::CHAR: l.as_string() += r.as_char(); break;
                            case type::STRING: l.as_string() += r.as_string(); break;
                            case type::WSTRING: l.as_string() += r.cast_to_string(); break;
                            default: l.as_string() += r.cast_to_char(); break;
                        }
                        break;
                    case type::WSTRING:
                        switch(rtype) {
                            case type::CHAR: l.as_wstring() += r.cast_to_wchar(); break;
                            case type::WCHAR: l.as_wstring() += r.as_wchar(); break;
                            case type::STRING: l.as_wstring() += r.cast_to_wstring(); break;
                            case type::WSTRING: l.as_wstring() += r.as_wstring(); break;
                            default: l.as_wstring() += r.cast_to_wchar(); break;
                        }
                        break;
                    case type::UNDEFINED:
                        l.assign(r);
                        break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
            }
            return *this;
        }

        friend valbox operator+(valbox const &l, valbox const &r) {
            valbox res{};
            type st{};
            if(l.val_or_pointed_type() == r.val_or_pointed_type()) {
                switch(l.val_or_pointed_type()) {
                    case type::BOOL: res = l.as_bool() || r.as_bool(); break;
                    case type::CHAR: res = l.as_char() + r.as_char(); break;
                    case type::S8: res = l.as_s8() + r.as_s8(); break;
                    case type::U8: res = l.as_u8() + r.as_u8(); break;
                    case type::S16: res = l.as_s16() + r.as_s16(); break;
                    case type::U16: res = l.as_u16() + r.as_u16(); break;
                    case type::WCHAR: res = l.as_wchar() + r.as_wchar(); break;
                    case type::S32: res = l.as_s32() + r.as_s32(); break;
                    case type::U32: res = l.as_u32() + r.as_u32(); break;
                    case type::S64: res = l.as_s64() + r.as_s64(); break;
                    case type::U64: res = l.as_u64() + r.as_u64(); break;
                    case type::FLOAT: res = l.as_float() + r.as_float(); break;
                    case type::DOUBLE: res = l.as_double() + r.as_double(); break;
                    case type::LONG_DOUBLE: res = l.as_long_double() + r.as_long_double(); break;
                    case type::VEC4: res = l.as_vec4() + r.as_vec4(); break;
                    case type::POINTER: res = (void *)((std::uintptr_t)l.as_ptr() + (std::uintptr_t)r.as_ptr()); break;
                    case type::ARRAY:
                        res.become_array();
                        res.as_array().resize(l.as_array().size() + r.as_array().size());
                        // l.as_array().traverse_real([&](size_t i, valbox const &v) { res.as_array()[i] = v.clone(); });
                        for(auto &&el: l.as_array()) { res.as_array().push_back(el.clone()); }
                        // r.as_array().traverse_real([&](size_t i, valbox const &v) { res.as_array()[i + l.as_array().size()] = v.clone(); });
                        for(auto &&el: r.as_array()) { res.as_array().push_back(el.clone()); }
                        break;
                    case type::OBJECT:
                        res = l.as_object();
                        for(auto &&p: r.as_object()) {
                            if(res.as_object().find(p.first) == res.as_object().end()) {
                                res[p.first] = p.second;
                            }
                        }
                        break;
                    case type::STRING: res = l.as_string() + r.as_string(); break;
                    case type::WSTRING: res = l.as_wstring() + r.as_wstring(); break;
                    case type::UNDEFINED: break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            } else if(stronger_numeric_type(l.val_or_pointed_type(), r.val_or_pointed_type(), st)) {
                if(st == l.val_or_pointed_type()) {
                    res.become_same_type_as(l);
                } else {
                    res.become_same_type_as(r);
                }
                switch(st) {
                    case type::CHAR: res = l.cast_num_to_num<char>() + r.cast_num_to_num<char>(); break;
                    case type::WCHAR: res = l.cast_num_to_num<wchar_t>() + r.cast_num_to_num<wchar_t>(); break;
                    case type::U8: res = l.cast_num_to_num<std::uint8_t>() + r.cast_num_to_num<std::uint8_t>(); break;
                    case type::S8: res = l.cast_num_to_num<std::int8_t>() + r.cast_num_to_num<std::int8_t>(); break;
                    case type::U16: res = l.cast_num_to_num<std::uint16_t>() + r.cast_num_to_num<std::uint16_t>(); break;
                    case type::S16: res = l.cast_num_to_num<std::int16_t>() + r.cast_num_to_num<std::int16_t>(); break;
                    case type::U32: res = l.cast_num_to_num<std::uint32_t>() + r.cast_num_to_num<std::uint32_t>(); break;
                    case type::S32: res = l.cast_num_to_num<std::int32_t>() + r.cast_num_to_num<std::int32_t>(); break;
                    case type::U64: res = l.cast_num_to_num<std::uint64_t>() + r.cast_num_to_num<std::uint64_t>(); break;
                    case type::S64: res = l.cast_num_to_num<std::int64_t>() + r.cast_num_to_num<std::int64_t>(); break;
                    case type::FLOAT: res = l.cast_num_to_num<float>() + r.cast_num_to_num<float>(); break;
                    case type::DOUBLE: res = l.cast_num_to_num<double>() + r.cast_num_to_num<double>(); break;
                    case type::LONG_DOUBLE: res = l.cast_num_to_num<long double>() + r.cast_num_to_num<long double>(); break;
                    case type::POINTER: res = (void *)(l.cast_num_to_num<std::uintptr_t>() + r.cast_num_to_num<std::uintptr_t>()); break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            } else {
                if(l.is_vec4_ref() || r.is_vec4_ref()) {
                    if(l.is_vec4_ref()) {
                        vec4_t rv{};
                        if(r.is_string_ref() || r.is_wstring_ref()) {
                            if(r.is_string_ref()) {
                                rv = vec4_from_str<long double>(r.as_string());
                            } else {
                                rv = vec4_from_str<long double>(r.as_wstring());
                            }
                            res = l.as_vec4() + rv;
                            return res;
                        } else if(r.is_array_ref() || r.is_object_ref()) {
                            rv = vec4_from_json<long double>(r.to_json());
                            res = l.as_vec4() + rv;
                            return res;
                        } else if(r.is_any_fp_number()) {
                            rv = vec4_t(r.cast_to_long_double());
                            res = l.as_vec4() + rv;
                            return res;
                        } else if(r.is_any_int_number()) {
                            rv = vec4_t(r.cast_num_to_num<long double>());
                            res = l.as_vec4() + rv;
                            return res;
                        }
                    } else {
                        vec4_t lv{};
                        if(l.is_string_ref() || l.is_wstring_ref()) {
                            if(l.is_string_ref()) {
                                lv = vec4_from_str<long double>(l.as_string());
                            } else {
                                lv = vec4_from_str<long double>(l.as_wstring());
                            }
                            res = r.as_vec4() + lv;
                            return res;
                        } else if(l.is_array_ref() || l.is_object_ref()) {
                            lv = vec4_from_json<long double>(l.to_json());
                            res = r.as_vec4() + lv;
                            return res;
                        } else if(r.is_any_fp_number()) {
                            lv = vec4_t(r.cast_to_long_double());
                            res = r.as_vec4() + lv;
                            return res;
                        } else if(r.is_any_int_number()) {
                            lv = vec4_t(r.cast_num_to_num<long double>());
                            res = r.as_vec4() + lv;
                            return res;
                        }
                    }
                } else if(l.is_array_ref()) {
                    res.become_array();
                    res.box_->value_ = l.as_array();
                    if(r.is_string_ref() || r.is_wstring_ref()) {
                        auto rarr{r.cast_to_array()};
                        // for(auto &&v: rarr.m()) {
                        //     res.as_array().push_back(v.second.deref().clone());
                        // }
                        for(auto &&v: rarr) { res.as_array().push_back(v.clone()); }
                    } else if(r.is_array_ref()) {
                        std::string &resstr{res.as_string()};
                        for(auto &&itm: r.as_array()) {
                            resstr.push_back(itm.deref().cast_to_char());
                        }
                    } else {
                        res.as_array().push_back(r);
                    }
                    return res;
                } else if(l.is_string_ref()) {
                    if(r.is_string_ref()) {
                        res = l.as_string() + r.as_string();
                    } else if(r.is_any_int_number()) {
                        res = l.as_string() + r.cast_to_char();
                    } else if(r.is_array_ref()) {
                        res = l;
                        std::string &resstr{res.as_string()};
                        for(auto &&itm: r.as_array()) {
                            resstr.push_back(itm.deref().cast_to_char());
                        }
                    }
                    return res;
                } else if(r.is_string_ref()) {
                    if(l.is_any_int_number()) {
                        std::string s{};
                        s += l.cast_to_char();
                        s += r.as_string();
                        res = std::move(s);
                    }
                    return res;
                } else if(l.is_wstring_ref()) {
                    if(r.is_wstring_ref()) {
                        res = l.as_wstring() + r.as_wstring();
                    } else if(r.is_any_int_number()) {
                        res = l.as_wstring() + r.cast_to_wchar();
                    } else if(r.is_array_ref()) {
                        res = l;
                        std::wstring &resstr{res.as_wstring()};
                        for(auto &&itm: r.as_array()) {
                            resstr.push_back(itm.cast_to_wchar());
                        }
                    }
                    return res;
                } else if(r.is_wstring_ref()) {
                    if(l.is_any_int_number()) {
                        std::wstring ws{};
                        ws += l.cast_to_wchar();
                        ws += r.as_wstring();
                        res = std::move(ws);
                    }
                    return res;
                }
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend bool operator==(valbox const &l, valbox const &r) {
            type lt{l.val_or_pointed_type()};
            type rt{r.val_or_pointed_type()};
            if(lt == rt) {
                switch(lt) {
                    case type::CHAR: return l.as_char() == r.as_char();
                    case type::WCHAR: return l.as_wchar() == r.as_wchar();
                    case type::U8: return l.as_u8() == r.as_u8();
                    case type::S8: return l.as_s8() == r.as_s8();
                    case type::U16: return l.as_u16() == r.as_u16();
                    case type::S16: return l.as_s16() == r.as_s16();
                    case type::U32: return l.as_u32() == r.as_u32();
                    case type::S32: return l.as_s32() == r.as_s32();
                    case type::U64: return l.as_u64() == r.as_u64();
                    case type::S64: return l.as_s64() == r.as_s64();
                    case type::FLOAT: return l.as_float() == r.as_float();
                    case type::DOUBLE: return l.as_double() == r.as_double();
                    case type::LONG_DOUBLE: return l.as_long_double() == r.as_long_double();
                    case type::BOOL: return l.as_bool() == r.as_bool();
                    case type::STRING: return l.as_string() == r.as_string();
                    case type::WSTRING: return l.as_wstring() == r.as_wstring();
                    case type::POINTER: return l.as_ptr() == r.as_ptr();
                    case type::UNDEFINED: return true;
                    // case type::FUNC: {
                    //     std::function<valbox(valbox const &, std::vector<valbox> &)> const &f1{l.as_func()};
                    //     std::function<valbox(valbox const &, std::vector<valbox> &)> const &f2{r.as_func()};
                    //     auto s1{sizeof(f1)};
                    //     auto s2{sizeof(f2)};
                    //     bool res{false};
                    //     if(s1 == s2) {
                    //         res = memcmp(&f1, &f2, s1) == 0;
                    //     }
                    //     return res;
                    // }
                    default: break;
                }
            } else {
                type st;
                bool applicable{stronger_numeric_type(lt, rt, st)};
                if(applicable) {
                    switch(st) {
                        case type::CHAR: return l.cast_num_to_num<char>() == r.cast_num_to_num<char>();
                        case type::WCHAR: return l.cast_num_to_num<wchar_t>() == r.cast_num_to_num<wchar_t>();
                        case type::U8: return l.cast_num_to_num<std::uint8_t>() == r.cast_num_to_num<std::uint8_t>();
                        case type::S8: return l.cast_num_to_num<std::int8_t>() == r.cast_num_to_num<std::int8_t>();
                        case type::U16: return l.cast_num_to_num<std::uint16_t>() == r.cast_num_to_num<std::uint16_t>();
                        case type::S16: return l.cast_num_to_num<std::int16_t>() == r.cast_num_to_num<std::int16_t>();
                        case type::U32: return l.cast_num_to_num<std::uint32_t>() == r.cast_num_to_num<std::uint32_t>();
                        case type::S32: return l.cast_num_to_num<std::int32_t>() == r.cast_num_to_num<std::int32_t>();
                        case type::U64: return l.cast_num_to_num<std::uint64_t>() == r.cast_num_to_num<std::uint64_t>();
                        case type::S64: return l.cast_num_to_num<std::int64_t>() == r.cast_num_to_num<std::int64_t>();
                        case type::FLOAT: return l.cast_num_to_num<float>() == r.cast_num_to_num<float>();
                        case type::DOUBLE: return l.cast_num_to_num<double>() == r.cast_num_to_num<double>();
                        case type::LONG_DOUBLE: return l.cast_num_to_num<long double>() == r.cast_num_to_num<long double>();
                        case type::POINTER: return l.cast_num_to_num<std::uintptr_t>() == r.cast_num_to_num<std::uintptr_t>();
                        default: break;
                    }
                } else {
                    type st{stronger_type(lt, rt)};
                    switch(st) {
                        case type::BOOL: return l.cast_to_bool() == r.cast_to_bool();
                        case type::STRING: return l.cast_to_string() == r.cast_to_string();
                        case type::WCHAR: return l.cast_to_wchar() == r.cast_to_wchar();
                        case type::WSTRING: return l.cast_to_wstring() == r.cast_to_wstring();
                        case type::UNDEFINED: return false;
                        default: break;
                    }
                }
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend bool operator<=(valbox const &l, valbox const &r) {
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == rt) {
                switch(lt) {
                    case type::CHAR: return lr.as_char() <= rr.as_char();
                    case type::U8: return lr.as_u8() <= rr.as_u8();
                    case type::S8: return lr.as_s8() <= rr.as_s8();
                    case type::U16: return lr.as_u16() <= rr.as_u16();
                    case type::S16: return lr.as_s16() <= rr.as_s16();
                    case type::U32: return lr.as_u32() <= rr.as_u32();
                    case type::S32: return lr.as_s32() <= rr.as_s32();
                    case type::U64: return lr.as_u64() <= rr.as_u64();
                    case type::S64: return lr.as_s64() <= rr.as_s64();
                    case type::FLOAT: return lr.as_float() <= rr.as_float();
                    case type::DOUBLE: return lr.as_double() <= rr.as_double();
                    case type::LONG_DOUBLE: return lr.as_long_double() <= rr.as_long_double();
                    case type::BOOL: return lr.as_bool() <= rr.as_bool();
                    case type::WCHAR: return lr.as_wchar() <= rr.as_wchar();
                    case type::STRING: return lr.as_string() <= rr.as_string();
                    case type::WSTRING: return lr.as_wstring() <= rr.as_wstring();
                    case type::POINTER: return (std::uintptr_t)lr.as_ptr() <= (std::uintptr_t)rr.as_ptr();
                    case type::UNDEFINED: return true;
                    default: break;
                }
            } else {
                type st;
                bool applicable{stronger_numeric_type(lt, rt, st)};
                if(applicable) {
                    valbox res{l};
                    switch(st) {
                        case type::CHAR: return lr.cast_num_to_num<char>() <= rr.cast_num_to_num<char>();
                        case type::U8: return lr.cast_num_to_num<std::uint8_t>() <= rr.cast_num_to_num<std::uint8_t>();
                        case type::S8: return lr.cast_num_to_num<std::int8_t>() <= rr.cast_num_to_num<std::int8_t>();
                        case type::U16: return lr.cast_num_to_num<std::uint16_t>() <= rr.cast_num_to_num<std::uint16_t>();
                        case type::S16: return lr.cast_num_to_num<std::int16_t>() <= rr.cast_num_to_num<std::int16_t>();
                        case type::U32: return lr.cast_num_to_num<std::uint32_t>() <= rr.cast_num_to_num<std::uint32_t>();
                        case type::S32: return lr.cast_num_to_num<std::int32_t>() <= rr.cast_num_to_num<std::int32_t>();
                        case type::U64: return lr.cast_num_to_num<std::uint64_t>() <= rr.cast_num_to_num<std::uint64_t>();
                        case type::S64: return lr.cast_num_to_num<std::int64_t>() <= rr.cast_num_to_num<std::int64_t>();
                        case type::FLOAT: return lr.cast_num_to_num<float>() <= rr.cast_num_to_num<float>();
                        case type::DOUBLE: return lr.cast_num_to_num<double>() <= rr.cast_num_to_num<double>();
                        case type::LONG_DOUBLE: return lr.cast_num_to_num<long double>() <= rr.cast_num_to_num<long double>();
                        case type::POINTER: return lr.cast_num_to_num<std::uintptr_t>() <= rr.cast_num_to_num<std::uintptr_t>();
                        default: break;
                    }
                } else {
                    type st{stronger_type(lt, rt)};
                    switch(st) {
                        case type::BOOL: return lr.cast_to_bool() <= rr.cast_to_bool(); break;
                        case type::STRING: return lr.cast_to_string() <= rr.cast_to_string();
                        case type::WCHAR: return lr.cast_to_wchar() <= rr.cast_to_wchar();
                        case type::WSTRING: return lr.cast_to_wstring() <= rr.cast_to_wstring();
                        case type::UNDEFINED: return false;
                        default: break;
                    }
                }
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend bool operator<(valbox const &l, valbox const &r) {
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == rt) {
                switch(lt) {
                    case type::S64: return lr.as_s64() < rr.as_s64();
                    case type::U64: return lr.as_u64() < rr.as_u64();
                    case type::CHAR: return lr.as_char() < rr.as_char();
                    case type::U8: return lr.as_u8() < rr.as_u8();
                    case type::S8: return lr.as_s8() < rr.as_s8();
                    case type::U16: return lr.as_u16() < rr.as_u16();
                    case type::S16: return lr.as_s16() < rr.as_s16();
                    case type::U32: return lr.as_u32() < rr.as_u32();
                    case type::S32: return lr.as_s32() < rr.as_s32();
                    case type::FLOAT: return lr.as_float() < rr.as_float();
                    case type::DOUBLE: return lr.as_double() < rr.as_double();
                    case type::LONG_DOUBLE: return lr.as_long_double() < rr.as_long_double();
                    case type::BOOL: return lr.as_bool() < rr.as_bool();
                    case type::WCHAR: return lr.as_wchar() < rr.as_wchar();
                    case type::STRING: return lr.as_string() < rr.as_string();
                    case type::WSTRING: return lr.as_wstring() < rr.as_wstring();
                    case type::POINTER: return lr.cast_num_to_num<std::uintptr_t>() < rr.cast_num_to_num<std::uintptr_t>();
                    case type::UNDEFINED: return false;
                    default: break;
                }
            } else {
                type st;
                bool applicable{stronger_numeric_type(lt, rt, st)};
                if(applicable) {
                    valbox res{lr};
                    switch(st) {
                        case type::CHAR: return lr.cast_num_to_num<char>() < rr.cast_num_to_num<char>();
                        case type::WCHAR: return lr.cast_num_to_num<wchar_t>() < rr.cast_num_to_num<wchar_t>();
                        case type::U8: return lr.cast_num_to_num<std::uint8_t>() < rr.cast_num_to_num<std::uint8_t>(); break;
                        case type::S8: return lr.cast_num_to_num<std::int8_t>() < rr.cast_num_to_num<std::int8_t>(); break;
                        case type::U16: return lr.cast_num_to_num<std::uint16_t>() < rr.cast_num_to_num<std::uint16_t>(); break;
                        case type::S16: return lr.cast_num_to_num<std::int16_t>() < rr.cast_num_to_num<std::int16_t>(); break;
                        case type::U32: return lr.cast_num_to_num<std::uint32_t>() < rr.cast_num_to_num<std::uint32_t>(); break;
                        case type::S32: return lr.cast_num_to_num<std::int32_t>() < rr.cast_num_to_num<std::int32_t>(); break;
                        case type::U64: return lr.cast_num_to_num<std::uint64_t>() < rr.cast_num_to_num<std::uint64_t>(); break;
                        case type::S64: return lr.cast_num_to_num<std::int64_t>() < rr.cast_num_to_num<std::int64_t>(); break;
                        case type::FLOAT: return lr.cast_num_to_num<float>() < rr.cast_num_to_num<float>(); break;
                        case type::DOUBLE: return lr.cast_num_to_num<double>() < rr.cast_num_to_num<double>(); break;
                        case type::LONG_DOUBLE: return lr.cast_num_to_num<long double>() < rr.cast_num_to_num<long double>(); break;
                        case type::POINTER: return lr.cast_num_to_num<std::uintptr_t>() <= rr.cast_num_to_num<std::uintptr_t>();
                        default: break;
                    }
                } else {
                    type st{stronger_type(lt, rt)};
                    switch(st) {
                        case type::BOOL: return lr.cast_to_bool() < rr.cast_to_bool(); break;
                        case type::STRING: return lr.cast_to_string() < rr.cast_to_string();
                        case type::WCHAR: return lr.cast_to_wchar() < rr.cast_to_wchar();
                        case type::WSTRING: return lr.cast_to_wstring() < rr.cast_to_wstring();
                        case type::UNDEFINED: return false;
                        default: break;
                    }
                }
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend bool operator>(valbox const &l, valbox const &r) {
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED && rt == type::UNDEFINED) {
                return false;
            }
            return !(lr <= rr);
        }

        friend bool operator>=(valbox const &l, valbox const &r) {
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED && rt == type::UNDEFINED) {
                return true;
            }
            return !(lr < rr);
        }

        friend bool operator!=(valbox const &l, valbox const &r) {
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED && rt == type::UNDEFINED) {
                return false;
            }
            return !(lr == rr);
        }

        friend valbox operator-(valbox const &l, valbox const &r) {
            valbox res{};
            type st;
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lt, rt, st)};
            if(applicable) {
                if(st == lt) {
                    res.become_same_type_as(lr);
                } else {
                    res.become_same_type_as(rr);
                }
                switch(st) {
                    case type::CHAR: res.assign_preserving_type(lr.cast_num_to_num<char>() - rr.cast_num_to_num<char>()); break;
                    case type::U8: res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() - rr.cast_num_to_num<std::uint8_t>()); break;
                    case type::S8: res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() - rr.cast_num_to_num<std::int8_t>()); break;
                    case type::U16: res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() - rr.cast_num_to_num<std::uint16_t>()); break;
                    case type::S16: res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() - rr.cast_num_to_num<std::int16_t>()); break;
                    case type::U32: res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() - rr.cast_num_to_num<std::uint32_t>()); break;
                    case type::S32: res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() - rr.cast_num_to_num<std::int32_t>()); break;
                    case type::U64: res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() - rr.cast_num_to_num<std::uint64_t>()); break;
                    case type::S64: res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() - rr.cast_num_to_num<std::int64_t>()); break;
                    case type::FLOAT: res.assign_preserving_type(lr.cast_num_to_num<float>() - rr.cast_num_to_num<float>()); break;
                    case type::DOUBLE: res.assign_preserving_type(lr.cast_num_to_num<double>() - rr.cast_num_to_num<double>()); break;
                    case type::LONG_DOUBLE: res.assign_preserving_type(lr.cast_num_to_num<long double>() - rr.cast_num_to_num<long double>()); break;
                    case type::POINTER: res.assign_preserving_type(lr.cast_num_to_num<std::uintptr_t>() - rr.cast_num_to_num<std::uintptr_t>()); break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend valbox operator*(valbox const &l, valbox const &r) {
            valbox res;
            type st;
            auto lr{l.deref()};
            auto rr{r.deref()};
            auto lt{lr.val_type()};
            auto rt{rr.val_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lr.val_or_pointed_type(), rr.val_or_pointed_type(), st)};
            if(applicable) {
                if(st == lr.val_or_pointed_type()) {
                    res.become_type(lr.val_or_pointed_type());
                } else {
                    res.become_type(rr.val_or_pointed_type());
                }
                switch(st) {
                    case type::CHAR: res.assign_preserving_type(lr.cast_num_to_num<char>() * rr.cast_num_to_num<char>()); break;
                    case type::U8: res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() * rr.cast_num_to_num<std::uint8_t>()); break;
                    case type::S8: res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() * rr.cast_num_to_num<std::int8_t>()); break;
                    case type::U16: res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() * rr.cast_num_to_num<std::uint16_t>()); break;
                    case type::S16: res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() * rr.cast_num_to_num<std::int16_t>()); break;
                    case type::U32: res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() * rr.cast_num_to_num<std::uint32_t>()); break;
                    case type::S32: res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() * rr.cast_num_to_num<std::int32_t>()); break;
                    case type::U64: res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() * rr.cast_num_to_num<std::uint64_t>()); break;
                    case type::S64: res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() * rr.cast_num_to_num<std::int64_t>()); break;
                    case type::FLOAT: res.assign_preserving_type(lr.cast_num_to_num<float>() * rr.cast_num_to_num<float>()); break;
                    case type::DOUBLE: res.assign_preserving_type(lr.cast_num_to_num<double>() * rr.cast_num_to_num<double>()); break;
                    case type::LONG_DOUBLE: res.assign_preserving_type(lr.cast_num_to_num<long double>() * rr.cast_num_to_num<long double>()); break;
                    case type::POINTER: res.assign_preserving_type(lr.cast_num_to_num<std::uintptr_t>() * rr.cast_num_to_num<std::uintptr_t>()); break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            } else {
                if(lr.val_or_pointed_type() == type::VEC4) {
                    if(rr.is_any_fp_number() || rr.is_any_int_number()) {
                        return lr.as_vec4() * rr.cast_num_to_num<long double>();
                    } else if(rr.val_or_pointed_type() == type::MAT4) {
                        return lr.as_vec4() * rr.as_mat4();
                    }
                } else if(lr.val_or_pointed_type() == type::MAT4) {
                    if(rr.is_any_fp_number() || rr.is_any_int_number()) {
                        return lr.as_mat4() * rr.cast_num_to_num<long double>();
                    } else if(rr.val_or_pointed_type() == type::MAT4) {
                        return lr.as_mat4() * rr.as_mat4();
                    } else if(rr.val_or_pointed_type() == type::VEC4) {
                        return lr.as_mat4() * rr.as_vec4();
                    }
                } else if(rr.val_or_pointed_type() == type::VEC4) {
                    if(lr.is_any_fp_number() || lr.is_any_int_number()) {
                        return lr.cast_num_to_num<long double>() * rr.as_vec4();
                    }
                } else if(rr.val_or_pointed_type() == type::MAT4) {
                    if(lr.is_any_fp_number() || lr.is_any_int_number()) {
                        return lr.cast_num_to_num<long double>() * rr.as_mat4();
                    }
                } else if(lr.val_or_pointed_type() == type::STRING && rr.is_numeric()) {
                    if(rr.is_any_int_number()) {
                        std::string res{};
                        for(uint64_t i{0}; i < rr.cast_to_u64(); ++i) {
                            res += lr.as_string();
                        }
                        return res;
                    } else if(rr.is_any_fp_number()) {
                        std::string res{};
                        long double rest{rr.cast_to_long_double() - rr.cast_to_u64()};
                        std::string tail{lr.as_string().substr(0, lr.as_string().size() * rest)};
                        for(uint64_t i{0}; i < rr.cast_to_u64(); ++i) {
                            res += lr.as_string();
                        }
                        res += tail;
                        return res;
                    }
                } else if(lr.val_or_pointed_type() == type::WSTRING && rr.is_numeric()) {
                    if(rr.is_any_int_number()) {
                        std::wstring res{};
                        for(uint64_t i{0}; i < rr.cast_to_u64(); ++i) {
                            res += lr.as_wstring();
                        }
                        return res;
                    } else if(rr.is_any_fp_number()) {
                        std::wstring res{};
                        long double rest{rr.cast_to_long_double() - rr.cast_to_u64()};
                        std::wstring tail{lr.as_wstring().substr(0, lr.as_wstring().size() * rest)};
                        for(uint64_t i{0}; i < rr.cast_to_u64(); ++i) {
                            res += lr.as_wstring();
                        }
                        res += tail;
                        return res;
                    }
                } else if(rr.val_or_pointed_type() == type::STRING && lr.is_numeric()) {
                    if(lr.is_any_int_number()) {
                        std::string res{};
                        for(uint64_t i{0}; i < lr.cast_to_u64(); ++i) {
                            res += rr.as_string();
                        }
                        return res;
                    } else if(lr.is_any_fp_number()) {
                        std::string res{};
                        long double rest{lr.cast_to_long_double() - lr.cast_to_u64()};
                        std::string tail{rr.as_string().substr(0, rr.as_string().size() * rest)};
                        for(uint64_t i{0}; i < lr.cast_to_u64(); ++i) {
                            res += rr.as_string();
                        }
                        res += tail;
                        return res;
                    }
                } else if(rr.val_or_pointed_type() == type::WSTRING && lr.is_numeric()) {
                    if(lr.is_any_int_number()) {
                        std::wstring res{};
                        for(uint64_t i{0}; i < lr.cast_to_u64(); ++i) {
                            res += rr.as_wstring();
                        }
                        return res;
                    } else if(lr.is_any_fp_number()) {
                        std::wstring res{};
                        long double rest{lr.cast_to_long_double() - lr.cast_to_u64()};
                        std::wstring tail{rr.as_wstring().substr(0, rr.as_wstring().size() * rest)};
                        for(uint64_t i{0}; i < lr.cast_to_u64(); ++i) {
                            res += rr.as_wstring();
                        }
                        res += tail;
                        return res;
                    }
                }
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend valbox operator/(valbox const &l, valbox const &r) {
            valbox res;
            type st;
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_type()};
            type rt{rr.val_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lt, rt, st)};
            if(applicable) {
                if(st == lt) {
                    res.become_same_type_as(lr);
                } else {
                    res.become_same_type_as(rr);
                }
                switch(st) {
                    case type::CHAR: {
                            auto dvzr{rr.cast_num_to_num<char>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<char>() / dvzr);
                        }
                        break;
                    case type::U8: {
                            auto dvzr{rr.cast_num_to_num<std::uint8_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() / dvzr);
                        }
                        break;
                    case type::S8: {
                            auto dvzr{rr.cast_num_to_num<std::int8_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() / dvzr);
                        }
                        break;
                    case type::U16: {
                            auto dvzr{rr.cast_num_to_num<std::uint16_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() / dvzr);
                        }
                        break;
                    case type::S16: {
                            auto dvzr{rr.cast_num_to_num<std::int16_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() / dvzr);
                        }
                        break;
                    case type::U32: {
                            auto dvzr{rr.cast_num_to_num<std::uint32_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() / dvzr);
                        }
                        break;
                    case type::S32: {
                            auto dvzr{rr.cast_num_to_num<std::int32_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() / dvzr);
                        }
                        break;
                    case type::U64: {
                            auto dvzr{rr.cast_num_to_num<std::uint64_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() / dvzr);
                        }
                        break;
                    case type::S64: {
                            auto dvzr{rr.cast_num_to_num<std::int64_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() / dvzr);
                        }
                        break;
                    case type::FLOAT:
                        res.assign_preserving_type(lr.cast_num_to_num<float>() / rr.cast_num_to_num<float>());
                        break;
                    case type::DOUBLE:
                        res.assign_preserving_type(lr.cast_num_to_num<double>() / rr.cast_num_to_num<double>());
                        break;
                    case type::LONG_DOUBLE:
                        res.assign_preserving_type(lr.cast_num_to_num<long double>() / rr.cast_num_to_num<long double>());
                        break;
                    case type::POINTER: {
                            auto dvzr{rr.cast_num_to_num<std::uintptr_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uintptr_t>() / dvzr);
                        }
                        break;
                    default:
                        throw std::runtime_error{"operation not applicable"};
                }
                return res;
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend valbox operator%(valbox const &l, valbox const &r) {
            valbox res{};
            type st{type::UNDEFINED};
            auto lr{l.deref()};
            auto rr{r.deref()};
            auto lt{lr.val_type()};
            auto rt{rr.val_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lr.val_or_pointed_type(), rr.val_or_pointed_type(), st)};
            if(applicable) {
                if(lr.is_any_fp_number() || rr.is_any_fp_number()) {
                    res.become_type(st);
                    switch(st) {
                        case type::FLOAT: res.assign_preserving_type(std::fmod(lr.cast_num_to_num<float>(), rr.cast_num_to_num<float>())); break;
                        case type::DOUBLE: res.assign_preserving_type(std::fmod(lr.cast_num_to_num<double>(), rr.cast_num_to_num<double>())); break;
                        case type::LONG_DOUBLE: res.assign_preserving_type(std::fmod(lr.cast_num_to_num<long double>(), rr.cast_num_to_num<long double>())); break;
                        default: break;
                    }
                } else {
                    res.become_same_type_as(lr);
                    switch(lr.val_or_pointed_type()) {
                    case type::CHAR: {
                            auto dvzr{rr.cast_num_to_num<char>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<char>() % dvzr);
                        }
                        break;
                    case type::U8: {
                            auto dvzr{rr.cast_num_to_num<std::uint8_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() % dvzr);
                        }
                        break;
                    case type::S8: {
                            auto dvzr{rr.cast_num_to_num<std::int8_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() % dvzr);
                        }
                        break;
                    case type::U16: {
                            auto dvzr{rr.cast_num_to_num<std::uint16_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() % dvzr);
                        }
                        break;
                    case type::S16: {
                            auto dvzr{rr.cast_num_to_num<std::int16_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() % dvzr);
                        }
                        break;
                    case type::U32: {
                            auto dvzr{rr.cast_num_to_num<std::uint32_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() % dvzr);
                        }
                        break;
                    case type::S32: {
                            auto dvzr{rr.cast_num_to_num<std::int32_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() % dvzr);
                        }
                        break;
                    case type::U64: {
                            auto dvzr{rr.cast_num_to_num<std::uint64_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() % dvzr);
                        }
                        break;
                    case type::S64: {
                            auto dvzr{rr.cast_num_to_num<std::int64_t>()};
                            if(dvzr == 0) { throw std::runtime_error{"integer divizion by zero"}; }
                            res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() % dvzr);
                        }
                        break;
                    default: throw std::runtime_error{"operation not applicable"};
                    }
                }
                return res;
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend valbox operator<<(valbox const &l, valbox const &r) {
            valbox res{valbox_no_initialize::dont_do_it};
            type st;
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lt, rt, st)};
            if(applicable) {
                if(st == lt) {
                    res.become_same_type_as(lr);
                } else {
                    res.become_same_type_as(rr);
                }
                switch(st) {
                    case type::CHAR: res.assign_preserving_type(lr.cast_num_to_num<char>() << rr.cast_num_to_num<int>()); break;
                    case type::WCHAR: res.assign_preserving_type(lr.cast_num_to_num<wchar_t>() << rr.cast_num_to_num<int>()); break;
                    case type::U8: res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() << rr.cast_num_to_num<int>()); break;
                    case type::S8: res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() << rr.cast_num_to_num<int>()); break;
                    case type::U16: res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() << rr.cast_num_to_num<int>()); break;
                    case type::S16: res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() << rr.cast_num_to_num<int>()); break;
                    case type::U32: res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() << rr.cast_num_to_num<int>()); break;
                    case type::S32: res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() << rr.cast_num_to_num<int>()); break;
                    case type::U64: res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() << rr.cast_num_to_num<int>()); break;
                    case type::S64: res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() << rr.cast_num_to_num<int>()); break;
                    case type::POINTER: res.assign_preserving_type(lr.cast_num_to_num<std::uintptr_t>() << rr.cast_num_to_num<std::uintptr_t>()); break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend valbox operator>>(valbox const &l, valbox const &r) {
            valbox res;
            type st;
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lt, rt, st)};
            if(applicable) {
                if(st == lt) {
                    res.become_same_type_as(lr);
                } else {
                    res.become_same_type_as(rr);
                }
                switch(st) {
                    case type::CHAR: res.assign_preserving_type(lr.cast_num_to_num<char>() >> rr.cast_num_to_num<int>()); break;
                    case type::WCHAR: res.assign_preserving_type(lr.cast_num_to_num<wchar_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::U8:   res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::S8:   res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::U16:  res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::S16:  res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::U32:  res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::S32:  res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::U64:  res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::S64:  res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() >> rr.cast_num_to_num<int>()); break;
                    case type::POINTER: res.assign_preserving_type(lr.cast_num_to_num<std::uintptr_t>() >> rr.cast_num_to_num<std::uintptr_t>()); break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend valbox operator&(valbox const &l, valbox const &r) {
            valbox res;
            type st;
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lt, rt, st)};
            if(applicable) {
                if(st == lt) {
                    res.become_same_type_as(lr);
                } else {
                    res.become_same_type_as(rr);
                }
                switch(st) {
                    case type::CHAR: res.assign_preserving_type(lr.cast_num_to_num<char>() & rr.cast_num_to_num<char>()); break;
                    case type::U8:   res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() & rr.cast_num_to_num<std::uint8_t>()); break;
                    case type::S8:   res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() & rr.cast_num_to_num<std::int8_t>()); break;
                    case type::U16:  res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() & rr.cast_num_to_num<std::uint16_t>()); break;
                    case type::S16:  res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() & rr.cast_num_to_num<std::int16_t>()); break;
                    case type::U32:  res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() & rr.cast_num_to_num<std::uint32_t>()); break;
                    case type::S32:  res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() & rr.cast_num_to_num<std::int32_t>()); break;
                    case type::U64:  res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() & rr.cast_num_to_num<std::uint64_t>()); break;
                    case type::S64:  res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() & rr.cast_num_to_num<std::int64_t>()); break;
                    case type::BOOL:  res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() & rr.cast_num_to_num<std::int64_t>()); break;
                    case type::POINTER: res.assign_preserving_type(lr.cast_num_to_num<std::uintptr_t>() & rr.cast_num_to_num<std::uintptr_t>()); break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend valbox operator|(valbox const &l, valbox const &r) {
            valbox res{valbox_no_initialize::dont_do_it};
            type st;
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lt, rt, st)};
            if(applicable) {
                if(st == lt) {
                    res.become_same_type_as(lr);
                } else {
                    res.become_same_type_as(rr);
                }
                switch(st) {
                    case type::CHAR: res.assign_preserving_type(lr.cast_num_to_num<char>() | rr.cast_num_to_num<char>()); break;
                    case type::U8:   res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() | rr.cast_num_to_num<std::uint8_t>()); break;
                    case type::S8:   res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() | rr.cast_num_to_num<std::int8_t>()); break;
                    case type::U16:  res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() | rr.cast_num_to_num<std::uint16_t>()); break;
                    case type::S16:  res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() | rr.cast_num_to_num<std::int16_t>()); break;
                    case type::U32:  res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() | rr.cast_num_to_num<std::uint32_t>()); break;
                    case type::S32:  res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() | rr.cast_num_to_num<std::int32_t>()); break;
                    case type::U64:  res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() | rr.cast_num_to_num<std::uint64_t>()); break;
                    case type::S64:  res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() | rr.cast_num_to_num<std::int64_t>()); break;
                    case type::BOOL:  res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() | rr.cast_num_to_num<std::int64_t>()); break;
                    case type::POINTER: res.assign_preserving_type(lr.cast_num_to_num<std::uintptr_t>() | rr.cast_num_to_num<std::uintptr_t>()); break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            }
            throw std::runtime_error{"operation not applicable"};
        }

        friend valbox operator^(valbox const &l, valbox const &r) {
            valbox res;
            type st;
            auto lr{l.deref()};
            auto rr{r.deref()};
            type lt{lr.val_or_pointed_type()};
            type rt{rr.val_or_pointed_type()};
            if(lt == type::UNDEFINED || rt == type::UNDEFINED) {
                return res;
            }
            bool applicable{stronger_numeric_type(lt, rt, st)};
            if(applicable) {
                if(st == lt) {
                    res.become_same_type_as(lr);
                } else {
                    res.become_same_type_as(rr);
                }
                switch(st) {
                    case type::CHAR: res.assign_preserving_type(lr.cast_num_to_num<char>() ^ rr.cast_num_to_num<char>()); break;
                    case type::U8:   res.assign_preserving_type(lr.cast_num_to_num<std::uint8_t>() ^ rr.cast_num_to_num<std::uint8_t>()); break;
                    case type::S8:   res.assign_preserving_type(lr.cast_num_to_num<std::int8_t>() ^ rr.cast_num_to_num<std::int8_t>()); break;
                    case type::U16:  res.assign_preserving_type(lr.cast_num_to_num<std::uint16_t>() ^ rr.cast_num_to_num<std::uint16_t>()); break;
                    case type::S16:  res.assign_preserving_type(lr.cast_num_to_num<std::int16_t>() ^ rr.cast_num_to_num<std::int16_t>()); break;
                    case type::U32:  res.assign_preserving_type(lr.cast_num_to_num<std::uint32_t>() ^ rr.cast_num_to_num<std::uint32_t>()); break;
                    case type::S32:  res.assign_preserving_type(lr.cast_num_to_num<std::int32_t>() ^ rr.cast_num_to_num<std::int32_t>()); break;
                    case type::U64:  res.assign_preserving_type(lr.cast_num_to_num<std::uint64_t>() ^ rr.cast_num_to_num<std::uint64_t>()); break;
                    case type::S64:  res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() ^ rr.cast_num_to_num<std::int64_t>()); break;
                    case type::BOOL:  res.assign_preserving_type(lr.cast_num_to_num<std::int64_t>() ^ rr.cast_num_to_num<std::int64_t>()); break;
                    case type::POINTER: res.assign_preserving_type(lr.cast_num_to_num<std::uintptr_t>() ^ rr.cast_num_to_num<std::uintptr_t>()); break;
                    default: throw std::runtime_error{"operation not applicable"};
                }
                return res;
            }
            throw std::runtime_error{"operation not applicable"};
        }

        valbox &assign_preserving_type(valbox const &that) {
            valbox const &that_ref{that.deref()};
            valbox &this_ref{deref()};
            this_ref.pointed_box_.reset();
            if(this_ref.is_ptr()) {
                switch(this_ref.pointed_type()) {
                    case type::CHAR: this_ref.deref_ptr<char>() = that_ref.cast_to_char(); break;
                    case type::U8: this_ref.deref_ptr<std::uint8_t>() = that_ref.cast_to_u8(); break;
                    case type::S8: this_ref.deref_ptr<std::int8_t>() = that_ref.cast_to_s8(); break;
                    case type::U16: this_ref.deref_ptr<std::uint16_t>() = that_ref.cast_to_u16(); break;
                    case type::S16: this_ref.deref_ptr<std::int16_t>() = that_ref.cast_to_s16(); break;
                    case type::U32: this_ref.deref_ptr<std::uint32_t>() = that_ref.cast_to_u32(); break;
                    case type::S32: this_ref.deref_ptr<std::int32_t>() = that_ref.cast_to_s32(); break;
                    case type::U64: this_ref.deref_ptr<std::uint64_t>() = that_ref.cast_to_u64(); break;
                    case type::S64: this_ref.deref_ptr<std::int64_t>() = that_ref.cast_to_s64(); break;
                    case type::FLOAT: this_ref.deref_ptr<float>() = that_ref.cast_to_float(); break;
                    case type::DOUBLE: this_ref.deref_ptr<double>() = that_ref.cast_to_double(); break;
                    case type::LONG_DOUBLE: this_ref.deref_ptr<long double>() = that_ref.cast_to_long_double(); break;
                    case type::BOOL: this_ref.deref_ptr<bool>() = that_ref.cast_to_bool(); break;
                    case type::WCHAR: this_ref.deref_ptr<wchar_t>() = that_ref.cast_to_wchar(); break;
                    case type::STRING:
                        if(that_ref.is_array_ref()) {
                            std::string &thisstr{this_ref.deref_ptr<std::string>()};
                            thisstr.clear();
                            for(auto &&itm: that_ref.as_array()) {
                                thisstr.push_back(itm.cast_to_char());
                            }
                        } else if(that_ref.is_string_ref()) {
                            this_ref.deref_ptr<std::string>() = that_ref.as_string();
                        } else if(that_ref.is_wstring_ref()) {
                            this_ref.deref_ptr<std::string>() = scfx::str_util::to_utf8(that_ref.as_wstring());
                        } else if(that_ref.is_char_ref()) {
                            std::string &thisstr{this_ref.deref_ptr<std::string>()};
                            thisstr.resize(1);
                            thisstr[0] = that_ref.as_char();
                        } else if(that_ref.is_wchar_ref()) {
                            std::string &thisstr{this_ref.deref_ptr<std::string>()};
                            std::string that_wstr{};
                            that_wstr.resize(1);
                            that_wstr[0] = that_ref.as_wchar();
                            thisstr = scfx::str_util::to_utf8(that_wstr);
                        } else if(that_ref.is_any_signed_int_number()) {
                            std::string &thisstr{this_ref.deref_ptr<std::string>()};
                            thisstr = scfx::str_util::itoa(that_ref.cast_to_s64());
                        } else if(that_ref.is_any_unsigned_int_number()) {
                            std::string &thisstr{this_ref.deref_ptr<std::string>()};
                            thisstr = scfx::str_util::utoa(that_ref.cast_to_u64());
                        } else if(that_ref.is_any_fp_number()) {
                            std::string &thisstr{this_ref.deref_ptr<std::string>()};
                            thisstr = scfx::str_util::ftoa(that_ref.cast_to_long_double());
                        }
                        break;
                    case type::WSTRING:
                        if(that_ref.is_array_ref()) {
                            std::wstring &thiswstr{this_ref.deref_ptr<std::wstring>()};
                            thiswstr.clear();
                            for(auto &&itm: that_ref.as_array()) {
                                thiswstr.push_back(itm.cast_to_wchar());
                            }
                        } else if(that_ref.is_string_ref()) {
                            std::wstring &thiswstr{this_ref.deref_ptr<std::wstring>()};
                            thiswstr = scfx::str_util::from_utf8(that_ref.as_string());
                        } else if(that_ref.is_wstring_ref()) {
                            this_ref.deref_ptr<std::wstring>() = that_ref.as_wstring();
                        } else if(that_ref.is_char_ref()) {
                            std::wstring &thisstr{this_ref.deref_ptr<std::wstring>()};
                            thisstr.resize(1);
                            thisstr[0] = that_ref.as_char();
                        } else if(that_ref.is_wchar_ref()) {
                            std::wstring &thisstr{this_ref.deref_ptr<std::wstring>()};
                            thisstr.resize(1);
                            thisstr[0] = that_ref.as_wchar();
                        } else if(that_ref.is_any_signed_int_number()) {
                            std::wstring &thisstr{this_ref.deref_ptr<std::wstring>()};
                            thisstr = scfx::str_util::from_utf8(scfx::str_util::itoa(that_ref.cast_to_s64()));
                        } else if(that_ref.is_any_unsigned_int_number()) {
                            std::wstring &thisstr{this_ref.deref_ptr<std::wstring>()};
                            thisstr = scfx::str_util::from_utf8(scfx::str_util::utoa(that_ref.cast_to_u64()));
                        } else if(that_ref.is_any_fp_number()) {
                            std::wstring &thisstr{this_ref.deref_ptr<std::wstring>()};
                            thisstr = scfx::str_util::from_utf8(scfx::str_util::ftoa(that_ref.cast_to_long_double()));
                        }
                        break;
                    case type::ARRAY:
                        if(that_ref.is_array_ref()) {
                            this_ref.deref_ptr<array_t>() = that_ref.as_array();
                        } else if(that_ref.is_string_ref()) {
                            array_t &thisarr{this_ref.deref_ptr<array_t>()};
                            thisarr.clear();
                            for(auto &&c: that_ref.as_string()) {
                                thisarr.push_back(c);
                            }
                        } else if(that_ref.is_wstring_ref()) {
                            array_t &thisarr{this_ref.deref_ptr<array_t>()};
                            thisarr.clear();
                            for(auto &&c: that_ref.as_wstring()) {
                                thisarr.push_back(c);
                            }
                        }
                        break;
                    case type::OBJECT:
                        if(that_ref.is_object_ref()) {
                            this_ref.deref_ptr<object_t>() = that_ref.as_object();
                        }
                        break;
                    default:
                        throw std::runtime_error{"operation not applicable"};
                }
            } else {
                switch(val_or_pointed_type()) {
                    case type::CHAR: this_ref.as_char() = that_ref.cast_to_char(); break;
                    case type::U8: this_ref.as_u8() = that_ref.cast_to_u8(); break;
                    case type::S8: this_ref.as_s8() = that_ref.cast_to_s8(); break;
                    case type::U16: this_ref.as_u16() = that_ref.cast_to_u16(); break;
                    case type::S16: this_ref.as_s16() = that_ref.cast_to_s16(); break;
                    case type::U32: this_ref.as_u32() = that_ref.cast_to_u32(); break;
                    case type::S32: this_ref.as_s32() = that_ref.cast_to_s32(); break;
                    case type::U64: this_ref.as_u64() = that_ref.cast_to_u64(); break;
                    case type::S64: this_ref.as_s64() = that_ref.cast_to_s64(); break;
                    case type::FLOAT: this_ref.as_float() = that_ref.cast_to_float(); break;
                    case type::DOUBLE: this_ref.as_double() = that_ref.cast_to_double(); break;
                    case type::LONG_DOUBLE: this_ref.as_long_double() = that_ref.cast_to_long_double(); break;
                    case type::BOOL: this_ref.as_bool() = that_ref.cast_to_bool(); break;
                    case type::WCHAR: this_ref.as_wchar() = that_ref.cast_to_wchar(); break;
                    case type::STRING:
                        if(that_ref.is_array_ref()) {
                            std::string &thisstr{this_ref.as_string()};
                            thisstr.clear();
                            for(auto &&itm: that_ref.as_array()) {
                                thisstr.push_back(itm.cast_to_char());
                            }
                        } else if(that_ref.is_string_ref()) {
                            this_ref.as_string() = that_ref.as_string();
                        } else if(that_ref.is_wstring_ref()) {
                            this_ref.as_string() = scfx::str_util::to_utf8(that_ref.as_wstring());
                        } else if(that_ref.is_char_ref()) {
                            std::string &thisstr{this_ref.as_string()};
                            thisstr.resize(1);
                            thisstr[0] = that_ref.as_char();
                        } else if(that_ref.is_wchar_ref()) {
                            std::string &thisstr{as_string()};
                            std::string that_wstr{};
                            that_wstr.resize(1);
                            that_wstr[0] = that_ref.as_wchar();
                            thisstr = scfx::str_util::to_utf8(that_wstr);
                        } else if(that_ref.is_any_signed_int_number()) {
                            std::string &thisstr{this_ref.as_string()};
                            thisstr = scfx::str_util::itoa(that_ref.cast_to_s64());
                        } else if(that_ref.is_any_unsigned_int_number()) {
                            std::string &thisstr{this_ref.as_string()};
                            thisstr = scfx::str_util::utoa(that_ref.cast_to_u64());
                        } else if(that_ref.is_any_fp_number()) {
                            std::string &thisstr{this_ref.as_string()};
                            thisstr = scfx::str_util::ftoa(that_ref.cast_to_long_double());
                        }
                        break;
                    case type::WSTRING:
                        if(that_ref.is_array_ref()) {
                            std::wstring &thiswstr{this_ref.as_wstring()};
                            thiswstr.clear();
                            for(auto &&itm: that_ref.as_array()) {
                                thiswstr.push_back(itm.cast_to_wchar());
                            }
                        } else if(that_ref.is_string_ref()) {
                            std::wstring &thiswstr{this_ref.as_wstring()};
                            thiswstr = scfx::str_util::from_utf8(that_ref.as_string());
                        } else if(that_ref.is_wstring_ref()) {
                            this_ref.box_->value_ = that_ref.box_->value_;
                        } else if(that_ref.is_char_ref()) {
                            std::wstring &thisstr{this_ref.as_wstring()};
                            thisstr.resize(1);
                            thisstr[0] = that_ref.as_char();
                        } else if(that_ref.is_wchar_ref()) {
                            std::wstring &thisstr{this_ref.as_wstring()};
                            thisstr.resize(1);
                            thisstr[0] = that_ref.as_wchar();
                        } else if(that_ref.is_any_signed_int_number()) {
                            std::wstring &thisstr{this_ref.as_wstring()};
                            thisstr = scfx::str_util::from_utf8(scfx::str_util::itoa(that_ref.cast_to_s64()));
                        } else if(that_ref.is_any_unsigned_int_number()) {
                            std::wstring &thisstr{this_ref.as_wstring()};
                            thisstr = scfx::str_util::from_utf8(scfx::str_util::utoa(that_ref.cast_to_u64()));
                        } else if(that_ref.is_any_fp_number()) {
                            std::wstring &thisstr{this_ref.as_wstring()};
                            thisstr = scfx::str_util::from_utf8(scfx::str_util::ftoa(that_ref.cast_to_long_double()));
                        }
                        break;
                    case type::CLASS: if(that_ref.is_class_ref()) {
                            this_ref.box_->value_ = that_ref.box_->value_;
                            this_ref.box_->class_ = that_ref.box_->class_;
                        }
                        break;
                    case type::FUNC: if(that_ref.is_func_ref()) {
                            this_ref.box_->value_ = that_ref.box_->value_;
                        } break;
                    case type::ARRAY:
                        if(that_ref.is_array_ref()) {
                            this_ref.box_->value_ = that_ref.box_->value_;
                        } else if(that_ref.is_string_ref()) {
                            array_t &thisarr{this_ref.as_array()};
                            thisarr.clear();
                            for(auto &&c: that_ref.as_string()) {
                                thisarr.push_back(c);
                            }
                        } else if(that_ref.is_wstring_ref()) {
                            array_t &thisarr{this_ref.as_array()};
                            thisarr.clear();
                            for(auto &&c: that_ref.as_wstring()) {
                                thisarr.push_back(c);
                            }
                        }
                        break;
                    case type::OBJECT:
                        if(that_ref.is_object_ref()) {
                            this_ref.as_object() = that_ref.as_object();
                        }
                        break;
                    case type::UNDEFINED:
                        this_ref.assign(that_ref);
                        break;
                    default:
                        break;
                }
            }
            return *this;
        }

        valbox &assign(valbox const &that) {
            valbox const &that_ref{that.deref()};
            valbox &ref{deref()};
            if(ref.box_.get() == that_ref.box_.get()) {
                return *this;
            }
            ref.pointed_box_.reset();
            if(that_ref.is_undefined()) {
                if(ref.box_) {
                    ref.box_->value_ = value_t{};
                    ref.box_->type_ = type::UNDEFINED;
                    ref.box_->pointed_type_ = type::UNDEFINED;
                    ref.box_->class_.clear();
                }
                return *this;
            }
            if(!ref.box_) {
                ref.box_ = std::make_shared<box_data>(that_ref.box_->value_, that_ref.box_->type_, that_ref.box_->pointed_type_, that_ref.box_->class_);
                ref.pointed_box_ = that_ref.pointed_box_;
                return *this;
            }
            ref.box_->value_ = that_ref.box_->value_;
            ref.box_->type_ = that_ref.box_->type_;
            ref.box_->pointed_type_ = that_ref.box_->pointed_type_;
            ref.box_->class_ = that_ref.box_->class_;
            ref.pointed_box_ = that_ref.pointed_box_;
            return *this;
        }

        valbox &assign(valbox &&that) {
            valbox &that_ref{that.deref()};
            valbox &ref{deref()};
            if(ref.box_.get() == that_ref.box_.get()) {
                return *this;
            }
            if(that_ref.is_undefined()) {
                ref.box_->value_ = value_t{};
                ref.box_->type_ = type::UNDEFINED;
                ref.box_->pointed_type_ = type::UNDEFINED;
                ref.box_->class_.clear();
                ref.pointed_box_.reset();
                return *this;
            }
            if(!ref.box_) {
                ref.box_ = std::make_shared<box_data>(std::move(that_ref.box_->value_), that_ref.box_->type_, that_ref.box_->pointed_type_, that_ref.box_->class_);
                that_ref.box_->type_ = type::UNDEFINED;
                that_ref.box_->pointed_type_ = type::UNDEFINED;
                that_ref.box_->class_.clear();
                ref.pointed_box_ = std::move(that_ref.pointed_box_);
                return *this;
            }
            ref.box_->value_ = std::move(that_ref.box_->value_);
            ref.box_->type_ = that_ref.box_->type_; that_ref.box_->type_ = type::UNDEFINED;
            ref.box_->pointed_type_ = that_ref.box_->pointed_type_; that_ref.box_->pointed_type_ = type::UNDEFINED;
            ref.box_->class_ = that_ref.box_->class_; that_ref.box_->class_.clear();
            ref.pointed_box_ = std::move(that_ref.pointed_box_);
            return *this;
        }

        char cast_to_char() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                default: break;
            }
            return 0;
        }

        std::uint8_t cast_to_u8() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::CLASS:         return 0;
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                case type::UNDEFINED:     return 0;
                default: break;
            }
            return 0;
        }

        std::int8_t cast_to_s8() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::CLASS:         return 0;
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                case type::UNDEFINED:     return 0;
                default: break;
            }
            return 0;
        }

        std::uint16_t cast_to_u16() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::CLASS:         return 0;
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                case type::UNDEFINED:     return 0;
                default: break;
            }
            return 0;
        }

        std::int16_t cast_to_s16() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::CLASS:         return 0;
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                case type::UNDEFINED:     return 0;
                default: break;
            }
            return 0;
        }

        std::uint32_t cast_to_u32() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::CLASS:         return 0;
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                case type::UNDEFINED:     return 0;
                default: break;
            }
            return 0;
        }

        std::int32_t cast_to_s32() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::CLASS:         return 0;
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                case type::UNDEFINED:     return 0;
                default: break;
            }
            return 0;
        }

        std::uint64_t cast_to_u64() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                default: break;
            }
            return 0;
        }

        std::int64_t cast_to_s64() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                default: break;
            }
            return 0;
        }

        float cast_to_float() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                default: break;
            }
            return 0;
        }

        double cast_to_double() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                default: break;
            }
            return 0;
        }

        long double cast_to_long_double() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                default: break;
            }
            return 0;
        }

        bool cast_to_bool() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return !as_string().empty();
                case type::WSTRING:       return !as_wstring().empty();
                case type::OBJECT:        return !as_object().empty();
                case type::ARRAY:         return !as_array().empty();
                case type::CLASS:         return true;
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                default: break;
            }
            return false;
        }

        wchar_t cast_to_wchar() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          return as_char();
                case type::U8:            return as_u8();
                case type::S8:            return as_s8();
                case type::U16:           return as_u16();
                case type::S16:           return as_s16();
                case type::U32:           return as_u32();
                case type::S32:           return as_s32();
                case type::U64:           return as_u64();
                case type::S64:           return as_s64();
                case type::FLOAT:         return as_float();
                case type::DOUBLE:        return as_double();
                case type::LONG_DOUBLE:   return as_long_double();
                case type::BOOL:          return as_bool();
                case type::WCHAR:         return as_wchar();
                case type::STRING:        return as_string().empty() ? 0 : as_string()[0];
                case type::WSTRING:       return as_wstring().empty() ? 0 : as_wstring()[0];
                case type::POINTER:       return (uintptr_t)deref().as_ptr();
                default: break;
            }
            return 0;
        }

        std::string cast_to_string() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          { std::stringstream ss{}; ss << as_char(); return ss.str(); }
                case type::U8:            { std::stringstream ss{}; ss << as_u8(); return ss.str(); }
                case type::S8:            { std::stringstream ss{}; ss << as_s8(); return ss.str(); }
                case type::U16:           { std::stringstream ss{}; ss << as_u16(); return ss.str(); }
                case type::S16:           { std::stringstream ss{}; ss << as_s16(); return ss.str(); }
                case type::U32:           { std::stringstream ss{}; ss << as_u32(); return ss.str(); }
                case type::S32:           { std::stringstream ss{}; ss << as_s32(); return ss.str(); }
                case type::U64:           { std::stringstream ss{}; ss << as_u64(); return ss.str(); }
                case type::S64:           { std::stringstream ss{}; ss << as_s64(); return ss.str(); }
                case type::FLOAT:         { std::stringstream ss{}; ss << as_float(); return ss.str(); }
                case type::DOUBLE:        { std::stringstream ss{}; ss << as_double(); return ss.str(); }
                case type::LONG_DOUBLE:   { std::stringstream ss{}; ss << as_long_double(); return ss.str(); }
                case type::BOOL:          { return as_bool() ? "true" : "false"; }
                case type::WCHAR:         { std::wstring s{}; s += as_wchar(); return scfx::str_util::to_utf8(s); }
                case type::STRING:        return as_string();
                case type::WSTRING:       return scfx::str_util::to_utf8(as_wstring());
                case type::OBJECT:        return to_json().serialize5(4);
                case type::ARRAY: {
                    std::string res{};
                    // as_array().traverse_full([&](size_t /*idx*/, valbox const &v) {
                    //     res += v.cast_to_char();
                    // });
                    for(auto &&v: as_array()) {
                        res += v.cast_to_char();
                    }
                    return res;
                }
                case type::CLASS:         { std::stringstream ss{}; ss << deref().box_->class_; return ss.str(); }
                case type::POINTER:       { return scfx::str_util::utoa((uintptr_t)deref().as_ptr(), 16, sizeof(void *) * 8); }
                case type::UNDEFINED:     return "<undefined>";
                default: break;
            }
            return {};
        }

        std::wstring cast_to_wstring() const {
            switch(val_or_pointed_type()) {
                case type::CHAR:          { std::wstringstream ss{}; ss << as_char(); return ss.str(); }
                case type::U8:            { std::wstringstream ss{}; ss << as_u8(); return ss.str(); }
                case type::S8:            { std::wstringstream ss{}; ss << as_s8(); return ss.str(); }
                case type::U16:           { std::wstringstream ss{}; ss << as_u16(); return ss.str(); }
                case type::S16:           { std::wstringstream ss{}; ss << as_s16(); return ss.str(); }
                case type::U32:           { std::wstringstream ss{}; ss << as_u32(); return ss.str(); }
                case type::S32:           { std::wstringstream ss{}; ss << as_s32(); return ss.str(); }
                case type::U64:           { std::wstringstream ss{}; ss << as_u64(); return ss.str(); }
                case type::S64:           { std::wstringstream ss{}; ss << as_s64(); return ss.str(); }
                case type::FLOAT:         { std::wstringstream ss{}; ss << as_float(); return ss.str(); }
                case type::DOUBLE:        { std::wstringstream ss{}; ss << as_double(); return ss.str(); }
                case type::LONG_DOUBLE:   { std::wstringstream ss{}; ss << as_long_double(); return ss.str(); }
                case type::BOOL:          { return as_bool() ? L"true" : L"false"; }
                case type::WCHAR:         { std::wstring s{}; s += as_wchar(); return s; }
                case type::STRING:        return scfx::str_util::from_utf8(as_string());
                case type::WSTRING:       return as_wstring();
                case type::OBJECT:        return scfx::str_util::from_utf8(to_json().serialize5(4));
                case type::ARRAY: {
                    std::wstring res{};
                    // as_array().traverse_full([&](size_t /*idx*/, valbox const &v) {
                    //     res += v.cast_to_wchar();
                    // });
                    for(auto &&v: as_array()) {
                        res += v.cast_to_wchar();
                    }
                    return res;
                }
                case type::CLASS:         { std::wstringstream ss{}; ss <<  scfx::str_util::from_utf8(deref().box_->class_); return ss.str(); }
                case type::POINTER:       return scfx::str_util::from_utf8(scfx::str_util::utoa((uintptr_t)deref().as_ptr(), 16, sizeof(void *) * 8)); // deref().cast_to_wstring();
                case type::UNDEFINED:     return L"<undefined>";
                default: break;
            }
            return {};
        }

        array_t cast_to_array() const {
            array_t res{};
            switch(val_or_pointed_type()) {
                case type::STRING:      for(auto &&c: as_string()) { res.push_back(c); } break;
                case type::WSTRING:     for(auto &&c: as_wstring()) { res.push_back(c); } break;
                case type::ARRAY:       res = as_array(); break;
                default:                res.push_back(*this); break;
            }
            return res;
        }

        std::vector<std::uint8_t> cast_to_byte_array() const {
            std::vector<std::uint8_t> res{};
            switch(val_or_pointed_type()) {
                case type::BOOL:         res.push_back(cast_to_u8());                 break;
                case type::CHAR:         res.push_back(cast_to_u8());                 break;
                case type::S8:           res.push_back(cast_to_u8());                 break;
                case type::U8:           res.push_back(as_u8());                      break;
                case type::S16: {
                        auto v{as_s16()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::U16: {
                        auto v{as_u16()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::WCHAR: {
                        auto v{as_wchar()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::S32: {
                        auto v{as_s32()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::U32: {
                        auto v{as_u32()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::S64: {
                        auto v{as_s64()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::U64: {
                        auto v{as_u64()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::FLOAT: {
                        auto v{as_float()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::DOUBLE: {
                        auto v{as_double()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::LONG_DOUBLE: {
                        auto v{as_long_double()};
                        std::uint8_t *vptr{(std::uint8_t *)&v};
                        for(std::size_t i = 0; i < sizeof(v); ++i) {
                            res.push_back(vptr[i]);
                        }
                    }
                    break;
                case type::STRING:
                    return std::vector<std::uint8_t>{as_string().begin(), as_string().end()};
                case type::WSTRING: {
                        res.assign((uint8_t *)as_wstring().data(), (uint8_t *)as_wstring().data() + as_wstring().size() * sizeof(std::wstring::value_type));
                    }
                    break;
                case type::ARRAY:
                    res.reserve(as_array().size());
                    // as_array().traverse_full([&](size_t, valbox const &v) {
                    //     auto itm_bytes{v.cast_to_byte_array()};
                    //     res.insert(res.end(), itm_bytes.begin(), itm_bytes.end());
                    // });
                    for(auto &&v: as_array()) {
                        auto itm_bytes{v.cast_to_byte_array()};
                        res.insert(res.end(), itm_bytes.begin(), itm_bytes.end());
                    }
                    break;
                case type::OBJECT: {
                        auto js{to_json().serialize5()};
                        res = std::vector<std::uint8_t>{js.begin(), js.end()};
                    }
                    break;
                default:
                    break;
            }
            return res;
        }

        object_t cast_to_object() const {
            object_t res{};
            switch(val_or_pointed_type()) {
                case type::OBJECT:
                    res = as_object();
                    break;
                case type::STRING: {
                    try {
                            res = valbox{scfx::json::deserialize(as_string())}.as_object();
                        } catch(...) {
                        }
                    }
                    break;
                case type::WSTRING: {
                        try {
                            res = valbox{scfx::json::deserialize(cast_to_string())}.as_object();
                        } catch(...) {
                        }
                    }
                    break;
                default:
                    break;
            }
            return res;
        }

        scfx::json to_json() const {
            if(is_undefined_ref()) {
                return scfx::json{};
            } else if(is_string_ref()) {
                return scfx::json{as_string()};
            } else if(is_bool_ref()) {
                return scfx::json{as_bool()};
            } else if(is_char_ref()) {
                std::string s{}; s += as_char();
                return scfx::json{s};
            } else if(is_wchar_ref()) {
                std::wstring s{}; s += as_wchar();
                return scfx::json{s};
            } else if(is_wstring_ref()) {
                return scfx::json{as_wstring()};
            } else if(is_any_int_number()) {
                return scfx::json{cast_num_to_num<std::int64_t>()};
            } else if(is_any_fp_number()) {
                return scfx::json{cast_num_to_num<long double>()};
            } else if(is_array_ref()) {
                scfx::json res{};
                res.become_array();
                for(auto &&v: as_array()) {
                    res.push_back(v.to_json());
                }
                return res;
            } else if(is_object_ref()) {
                scfx::json res{};
                res.become_object();
                object_t const &o{as_object()};
                for(auto &&p: o) {
                    res[p.first] = p.second.to_json();
                }
                return res;
            } else if(as_valbox_ptr() != nullptr) {
                return deref().to_json();
            }
            return {};
        }

        valbox &from_json(scfx::json const &v) {
            valbox &vr{deref()};
            vr.pointed_box_.reset();
            if(v.is_number()) {
                vr.box_ = std::make_shared<box_data>(v.as_number(), type::S64);
            } else if(v.is_float()) {
                vr.box_ = std::make_shared<box_data>(v.as_longdouble(), type::LONG_DOUBLE);
            } else if(v.is_string()) {
                vr.box_ = std::make_shared<box_data>(v.as_string(), type::STRING);
            } else if(v.is_bool()) {
                vr.box_ = std::make_shared<box_data>(v.as_boolean(), type::BOOL);
            } else if(v.is_array()) {
                if(!vr.box_) {
                    vr.box_ = std::make_shared<box_data>(array_t{}, type::ARRAY);
                } else {
                    vr.box_->value_ = array_t{};
                    vr.box_->type_ = type::ARRAY;
                    vr.box_->pointed_type_ = type::UNDEFINED;
                    vr.box_->class_.clear();
                }
                for(std::size_t i{0}; i < v.size(); ++i) {
                    valbox item{};
                    item.from_json(v[i]);
                    vr.as_array().push_back(std::move(item));
                }
            } else if(v.is_object()) {
                if(!vr.box_) {
                    vr.box_ = std::make_shared<box_data>(object_t{}, type::OBJECT);
                } else {
                    vr.box_->value_ = object_t{};
                    vr.box_->type_ = type::OBJECT;
                    vr.box_->pointed_type_ = type::UNDEFINED;
                    vr.box_->class_.clear();
                }
                v.traverse_object([&](std::string const &key, scfx::json const &val) {
                    vr[key].from_json(val);
                });
            } else if(v.is_null()) {
                vr.box_ = std::make_shared<box_data>();
            }
            return *this;
        }

        void set_pointed(valbox &vbp) {
            pointed_box_ = vbp.box_;
        }

    private:
        valbox(std::shared_ptr<box_data> &&b): box_{std::move(b)} {}

        std::shared_ptr<box_data> box_{};
        std::shared_ptr<box_data> pointed_box_{};
    };

    static std::ostream &operator<<(std::ostream &os, valbox const &v) {
        switch(v.val_or_pointed_type()) {
            case valbox::type::CHAR: os << v.as_char(); break;
            case valbox::type::U8: os << v.as_u8(); break;
            case valbox::type::S8: os << v.as_s8(); break;
            case valbox::type::U16: os << v.as_u16(); break;
            case valbox::type::S16: os << v.as_s16(); break;
            case valbox::type::U32: os << v.as_u32(); break;
            case valbox::type::S32: os << v.as_s32(); break;
            case valbox::type::U64: os << v.as_u64(); break;
            case valbox::type::S64: os << v.as_s64(); break;
            case valbox::type::FLOAT: os << v.as_float(); break;
            case valbox::type::DOUBLE: os << v.as_double(); break;
            case valbox::type::LONG_DOUBLE: os << v.as_long_double(); break;
            case valbox::type::BOOL: os << (v.as_bool() ? "true" : "false"); break;
            case valbox::type::WCHAR: { std::wstring ws{}; ws += v.as_wchar(); os << scfx::str_util::to_utf8(ws); } break;
            case valbox::type::STRING: os << v.as_string(); break;
            case valbox::type::WSTRING: os << scfx::str_util::to_utf8(v.as_wstring()); break;
            case valbox::type::CLASS: os << "<" << v.deref().class_name() << ">"; break;
            case valbox::type::POINTER: os << v.deref(); break;
            case valbox::type::UNDEFINED: os << "<undefined>"; break;
            case valbox::type::FUNC: os << "<function>"; break;
            case valbox::type::ARRAY: {
                    auto const &a{v.as_array()};
                    os << "[";
                    std::string sep{};
                    for(auto &&v: a) {
                        os << sep << v; sep = ",";
                    }
                    os << "]";
                }
                break;
            case valbox::type::VEC4: {
                    std::stringstream ss{};
                    ss << "vec4{";
                    std::string sep{};
                    for(std::size_t i{0}; i < 4; ++i) {
                        ss << sep << std::fixed << v.as_vec4()[i];
                        if(sep.empty()) {
                            sep = ",";
                        }
                    }
                    ss << "}";
                    os << ss.str();
                }
                break;
            case valbox::type::MAT4: {
                    std::stringstream ss{};
                    ss << "mat4{";
                    std::string sep1{};
                    for(std::size_t r{0}; r < 4; ++r) {
                        ss << sep1 << "{";
                        std::string sep2{};
                        for(std::size_t c{0}; c < 4; ++c) {
                            ss << sep2 << std::fixed << v.as_mat4().at(r, c);
                            sep2 = ", ";
                        }
                        ss << "}";
                        if(sep1.empty()) {
                            sep1 = ",";
                        }
                    }
                    ss << "}";
                    os << ss.str();
                }
                break;
            case valbox::type::OBJECT: os << v.to_json().serialize5(); break;
            default: os << "<corrupted>"; break;
        }
        return os;
    }

    static std::wostream &operator<<(std::wostream &os, valbox const &v) {
        switch(v.val_or_pointed_type()) {
            case valbox::type::CHAR: os << v.cast_to_wchar(); break;
            case valbox::type::U8: os << v.as_u8(); break;
            case valbox::type::S8: os << v.as_s8(); break;
            case valbox::type::U16: os << v.as_u16(); break;
            case valbox::type::S16: os << v.as_s16(); break;
            case valbox::type::U32: os << v.as_u32(); break;
            case valbox::type::S32: os << v.as_s32(); break;
            case valbox::type::U64: os << v.as_u64(); break;
            case valbox::type::S64: os << v.as_s64(); break;
            case valbox::type::FLOAT: os << v.as_float(); break;
            case valbox::type::DOUBLE: os << v.as_double(); break;
            case valbox::type::LONG_DOUBLE: os << v.as_long_double(); break;
            case valbox::type::BOOL: os << (v.as_bool() ? L"true" : L"false"); break;
            case valbox::type::WCHAR: os << v.as_wchar(); break;
            case valbox::type::STRING: os << v.cast_to_wstring(); break;
            case valbox::type::WSTRING: os << v.as_wstring(); break;
            case valbox::type::CLASS: os << L"<" << v.deref().class_name() << L">"; break;
            case valbox::type::POINTER: os << v.deref(); break;
            case valbox::type::UNDEFINED: os << L"<undefined>"; break;
            case valbox::type::FUNC: os << L"<function>"; break;
            case valbox::type::ARRAY: {
                auto const &a{v.as_array()};
                os << L"[";
                std::wstring sep{};
                for(auto &&v: a) {
                    os << sep << v; sep = L",";
                }
                os << L"]";
            }
            break;
            case valbox::type::VEC4: {
                std::wstringstream ss{};
                ss << L"vec4{";
                std::wstring sep{};
                for(std::size_t i{0}; i < 4; ++i) {
                    ss << sep << std::fixed << v.as_vec4()[i];
                    if(sep.empty()) {
                        sep = L",";
                    }
                }
                ss << L"}";
                os << ss.str();
            }
            break;
            case valbox::type::MAT4: {
                std::wstringstream ss{};
                ss << L"mat4{";
                std::wstring sep1{};
                for(std::size_t r{0}; r < 4; ++r) {
                    ss << sep1 << L"{";
                    std::wstring sep2{};
                    for(std::size_t c{0}; c < 4; ++c) {
                        ss << sep2 << std::fixed << v.as_mat4().at(r, c);
                        sep2 = L", ";
                    }
                    ss << L"}";
                    if(sep1.empty()) {
                        sep1 = L",";
                    }
                }
                ss << L"}";
                os << ss.str();
            }
            break;
            case valbox::type::OBJECT: os << v.to_json().serialize5(); break;
            default: os << L"<corrupted>"; break;
        }
        return os;
    }

}
