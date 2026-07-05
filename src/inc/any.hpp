#pragma once

#include "commondefs.hpp"

#if(__cplusplus < 202002L)
namespace std {

    template<class T>
    struct remove_cvref {
        using type = typename remove_cv<typename remove_reference<T>::type>::type;
    };

    template<class T>
    using remove_cvref_t = typename remove_cvref<T>::type;

}
#endif

namespace teal {

    class bad_any_cast: public std::bad_cast {
    public:
        char const *what() const noexcept override {
            return "bad any cast";
        }
    };

    inline void throw_bad_any_cast() {
#if __cpp_exceptions
        throw bad_any_cast{};
#else
        __builtin_abort();
#endif
    }

    class any {
        class holder_base {
        public:
            virtual std::unique_ptr<holder_base> clone() const = 0;
            virtual std::type_info const &type_info() = 0;
            virtual ~holder_base() = default;
        };

        template<class T>
        class any_holder: public holder_base {
        public:
            explicit any_holder(T const &value): value_{value} {
            }

            explicit any_holder(T &&value) noexcept: value_{std::move(value)} {
            }

            std::unique_ptr<holder_base> clone() const override {
                return std::make_unique<any_holder>(value_);
            }

            using type = T;

            std::type_info const &type_info() override {
                return typeid(value_);
            }

            T const &get_value() const {
                return value_;
            }

            T &get_value() {
                return value_;
            }

        private:
            T value_;
        };

    public:
        any() = default;

        any(any const &other):
            holder_{other.holder_ ? other.holder_->clone() : std::unique_ptr<holder_base>{}}
        {
        }

        any(any &&other) noexcept {
            swap(other);
        }

        template<class T>
        any(T const &value):
            holder_{std::make_unique<any_holder<std::remove_cvref_t<T>>>(value)} {
        }

        template<class T>
        any(T &&value):
            holder_{std::make_unique<any_holder<std::remove_cvref_t<T>>>(std::forward<T>(value))} {
        }

        any &operator=(any const &other) {
            if(this != &other) {
                holder_ = other.holder_ ? other.holder_->clone() : std::unique_ptr<holder_base>{};
            }
            return *this;
        }

        any &operator=(any &&other) noexcept {
            if(this != &other) {
                holder_ = std::move(other.holder_);
            }
            return *this;
        }

        template<class T>
        any &operator=(T const &value) {
            holder_ = std::make_unique<any_holder<std::remove_cvref_t<T>>>(value);
            return *this;
        }

        ~any() = default;

        void swap(any &other) noexcept {
            std::swap(holder_, other.holder_);
        }

        void reset() {
            holder_.reset();
        }

        void clear() {
            holder_.reset();
        }

        bool has_value() const {
            return holder_ != nullptr;
        }

        bool empty() const {
            return holder_ == nullptr;
        }

        template<typename T>
        bool is_of_type() const noexcept {
            return dynamic_cast<any_holder<std::remove_cvref_t<T>> *>(holder_.get()) != nullptr;
        }

        template<class T>
        T &as() {
            if(!holder_) {
                throw_bad_any_cast();
            }
            any_holder<std::remove_cvref_t<T>> *holder{
                dynamic_cast<any_holder<std::remove_cvref_t<T>> *>(holder_.get())
            };
            if(!holder) {
                throw_bad_any_cast();
            }
            return static_cast<std::remove_cvref_t<T> &>(holder->get_value());
        }

        template<class T>
        T const &as() const {
            if(!holder_) {
                throw_bad_any_cast();
            }
            any_holder<std::remove_cvref_t<T>> *holder{
                dynamic_cast<any_holder<std::remove_cvref_t<T>> *>(holder_.get())
            };
            if(!holder) {
                throw_bad_any_cast();
            }
            return static_cast<std::remove_cvref_t<T> const &>(holder->get_value());
        }

        template<typename T>
        T *as_ptr() noexcept {
            any_holder<std::remove_cvref_t<T>> *d{dynamic_cast<any_holder<std::remove_cvref_t<T>> *>(holder_.get())};
            return d ? &(d->get_value()) : 0;
        }

        template<typename T>
        T const *as_ptr() const noexcept {
            any_holder<std::remove_cvref_t<T>> *d{dynamic_cast<any_holder<std::remove_cvref_t<T>> *>(holder_.get())};
            return d ? &(d->get_value()) : 0;
        }

    private:
        std::unique_ptr<holder_base> holder_{};
    };

    template<class T>
    T const &any_cast(any const &value) {
        return value.as<T>();
    }

    template<class T>
    T &any_cast(any &value) {
        return value.as<T>();
    }

}
