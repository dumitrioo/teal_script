#pragma once

#include "../commondefs.hpp"

namespace teal {

    template<typename T, std::size_t N>
    class static_buff {
    public:
        using iterator = T *;
        using const_iterator = T const *;
        using size_type = std::uint64_t;
        using offset_type = std::int64_t;

        static_buff() = default;
        static_buff(std::initializer_list<T> vals) { for(auto &&v: vals) { push_back(v); } }
        static_buff(std::vector<T> const &vals) { for(auto &&v: vals) { push_back(v); } }
        template<typename U, std::size_t CNT>
        static_buff(std::array<U, CNT> const &vals) { for(auto &&v: vals) { push_back(v); } }

        static_buff(static_buff const &) = default;
        static_buff(static_buff &&) = default;
        static_buff &operator=(static_buff const &) = default;
        static_buff &operator=(static_buff &&) = default;
        ~static_buff() = default;

        static_buff &push_back(T const &v) { if(cnt_ >= N) { throw std::runtime_error{"buffer overflow"}; } buf_[cnt_++] = v; return *this; }
        T pop_back() { if(cnt_ == 0) { throw std::runtime_error{"buffer empty"}; } return std::move(buf_[cnt_--]); }
        T pop_front() {
            if(cnt_ == 0) { throw std::runtime_error{"buffer empty"}; }
            T res{std::move(buf_[0])};
            for(std::size_t i = 0; i + 1 < size(); ++i) { buf_[i] = std::move(buf_[i + 1]); }
            --cnt_;
            return res;
        }
        T &operator[](std::size_t indx) & { if(indx >= cnt_) { throw std::runtime_error{"index out of range"}; } return (T &)buf_[indx]; }
        T const &operator[](std::size_t indx) const & { if(indx >= cnt_) { throw std::runtime_error{"index out of range"}; } return (T const &)buf_[indx]; }
        T &operator[](int indx) & { if(indx >= (int)cnt_) { throw std::runtime_error{"index out of range"}; } return (T &)buf_[indx]; }
        T const &operator[](int indx) const & { if(indx >= cnt_) { throw std::runtime_error{"index out of range"}; } return (T const &)buf_[indx]; }
        T const *data() const & { return buf_; }
        T *data() & { return buf_; }
        std::size_t size() const { return cnt_; }
        bool empty() const { return cnt_ == 0; }
        std::size_t constexpr capacity() const { return cnt_; }
        T &back() & { if(cnt_ == 0) { throw std::runtime_error{"buffer empty"}; } return buf_[cnt_ - 1]; }
        T &front() & { if(cnt_ == 0) { throw std::runtime_error{"buffer empty"}; } return buf_[0]; }
        iterator begin() & { return buf_; }
        iterator end() & { return buf_ + cnt_; }
        operator T *() & { return buf_; }
        operator T const *() const & { return buf_; }
        template<typename FT>
        void traverse(FT fun) { for(std::size_t i = 0; i < size(); ++i) { fun(buf_[i]); } }

    private:
        T buf_[N]{};
        std::size_t cnt_{0};
    };

}

