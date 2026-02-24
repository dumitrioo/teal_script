#pragma once

#include "../commondefs.hpp"

namespace scfx {

    template<typename T, std::size_t CAPACITY
#if (__cplusplus < 202000L)
         , std::enable_if_t<
             std::is_copy_constructible_v<T> || std::is_move_constructible_v<T> ||
             std::is_move_constructible_v<T> || std::is_move_assignable_v<T>,
             bool> = true
#endif
         >
#if (__cplusplus >= 202000L)
    requires(
        std::is_copy_constructible_v<T> || std::is_move_constructible_v<T> ||
        std::is_move_constructible_v<T> || std::is_move_assignable_v<T>
    )
#endif
    class circular_buffer {
        static_assert(CAPACITY > 0, "capacity must be a number greater than zero");

        class iter {
        public:
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T *;
            using reference = T &;

            iter(circular_buffer *owner_ptr = nullptr, std::size_t curr_pos = static_cast<std::size_t>(-1)):
                owner_ptr_{owner_ptr}, curr_pos_{owner_ptr_ && curr_pos < owner_ptr_->size_ ? curr_pos : static_cast<std::size_t>(-1)} {
            }

            iter &operator++() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1) && curr_pos_ < owner_ptr_->size_) {
                    ++curr_pos_;
                    if(curr_pos_ >= owner_ptr_->size_) {
                        curr_pos_ = static_cast<std::size_t>(-1);
                    }
                }
                return *this;
            }

            iter operator++(int) {
                iter retval{*this};
                ++(*this);
                return retval;
            }

            bool operator==(iter const &other) const {
                return owner_ptr_ == other.owner_ptr_ && curr_pos_ == other.curr_pos_;
            }

            bool operator!=(iter const &other) const {
                return owner_ptr_ != other.owner_ptr_ || curr_pos_ != other.curr_pos_;
            }

            T &operator*() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

            T const &operator*() const {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

        private:
            circular_buffer *owner_ptr_;
            std::size_t curr_pos_{static_cast<std::size_t>(-1)};
        };

        class const_iter {
        public:
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T *;
            using reference = T &;

            const_iter(circular_buffer const *owner_ptr = nullptr, std::size_t curr_pos = static_cast<std::size_t>(-1)):
                owner_ptr_{owner_ptr}, curr_pos_{owner_ptr_ && curr_pos < owner_ptr_->size_ ? curr_pos : static_cast<std::size_t>(-1)} {
            }

            const_iter &operator++() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1) && curr_pos_ < owner_ptr_->size_) {
                    ++curr_pos_;
                    if(curr_pos_ >= owner_ptr_->size_) {
                        curr_pos_ = static_cast<std::size_t>(-1);
                    }
                }
                return *this;
            }

            const_iter operator++(int) {
                const_iter retval{*this};
                ++(*this);
                return retval;
            }

            bool operator==(const_iter other) const {
                return owner_ptr_ == other.owner_ptr_ && curr_pos_ == other.curr_pos_;
            }

            bool operator!=(const_iter other) const {
                return owner_ptr_ != other.owner_ptr_ || curr_pos_ != other.curr_pos_;
            }

            T const &operator*() const {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

        private:
            circular_buffer const *owner_ptr_;
            std::size_t curr_pos_{static_cast<std::size_t>(-1)};
        };

    public:
        using iterator = iter;
        using const_iterator = const_iter;
        using ssize_type = ssize_t;
        using size_type = std::size_t;

        circular_buffer() = default;

        ~circular_buffer() noexcept {
            clear();
        }

        circular_buffer(circular_buffer const &that) {
            for(size_type i{0}; i < that.size_; ++i) {
                push_back(that[i]);
            }
        }

        circular_buffer(circular_buffer &&that) {
            for(size_type i{0}; i < that.size_; ++i) {
                new(buff() + wrp_idx(i)) T(std::move(that[i]));
                ++size_;
            }
            that.clear();
        }

        circular_buffer &operator=(circular_buffer const &that) {
            if(&that != this) {
                clear();
                for(size_type i{0}; i < that.size_; ++i) {
                    new(buff() + wrp_idx(i)) T(that[i]);
                    ++size_;
                }
            }
            return *this;
        }

        circular_buffer &operator=(circular_buffer &&that) {
            if(&that != this) {
                clear();
                for(size_type i{0}; i < that.size_; ++i) {
                    new(buff() + wrp_idx(i)) T(std::move(that[i]));
                    ++size_;
                }
                that.clear();
            }
            return *this;
        }

        T &operator[](size_type indx) {
            if(indx < size()) {
                return buff()[wrp_idx(indx)];
            } else {
                throw std::range_error{"index out of bounds"};
            }
        }

        T const &operator[](size_type indx) const {
            if(indx < size()) {
                return buff()[wrp_idx(indx)];
            } else {
                throw std::range_error{"index out of bounds"};
            }
        }

        size_type constexpr capacity() const noexcept {
            return CAPACITY;
        }

        size_type size() const noexcept {
            return size_;
        }

        void push_front(T const &val) {
            pos_ = pos_ == 0 ? CAPACITY - 1 : pos_ - 1;
            if(size_ == CAPACITY) {
                if constexpr (std::is_copy_assignable_v<T>) {
                    buff()[pos_] = val;
                    return;
                }
                if constexpr (std::is_copy_constructible_v<T>) {
                    buff()[pos_].~T();
                    new(buff() + pos_) T{val};
                    return;
                }
            } else {
                if constexpr (std::is_copy_constructible_v<T>) {
                    new(buff() + wrp_idx(0)) T{val};
                    ++size_;
                    return;
                }
                if constexpr (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>) {
                    new(buff() + wrp_idx(0)) T;
                    buff()[wrp_idx(size_)] = val;
                    ++size_;
                    return;
                }
            }
            throw std::runtime_error{"value insert error: cannot copy"};
        }

        void push_back(T const &val) {
            if(size_ == CAPACITY) {
                if constexpr (std::is_copy_assignable_v<T>) {
                    buff()[pos_] = val;
                    pos_ = wrp_idx(1);
                    return;
                }
                if constexpr (std::is_copy_constructible_v<T>) {
                    buff()[pos_].~T();
                    new(buff() + pos_) T{val};
                    pos_ = wrp_idx(1);
                    return;
                }
            } else {
                if constexpr (std::is_copy_constructible_v<T>) {
                    new(buff() + wrp_idx(size_)) T{val};
                    ++size_;
                    return;
                }
                if constexpr (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>) {
                    new(buff() + wrp_idx(size_)) T;
                    buff()[wrp_idx(size_)] = val;
                    ++size_;
                    return;
                }
            }
            throw std::runtime_error{"value insert error: cannot copy"};
        }

        void push_front(T &&val) {
            pos_ = pos_ == 0 ? CAPACITY - 1 : pos_ - 1;
            if(size_ == CAPACITY) {
                if constexpr (std::is_move_assignable_v<T>) {
                    buff()[pos_] = std::move(val);
                    return;
                }
                if constexpr (std::is_move_constructible_v<T>) {
                    buff()[pos_].~T();
                    new(buff() + pos_) T{std::move(val)};
                    return;
                }
                if constexpr (std::is_copy_assignable_v<T>) {
                    buff()[pos_] = val;
                    return;
                }
                if constexpr (std::is_copy_constructible_v<T>) {
                    buff()[pos_].~T();
                    new(buff() + pos_) T{val};
                    return;
                }
            } else {
                if constexpr (std::is_move_constructible_v<T>) {
                    new(buff() + wrp_idx(0)) T{std::move(val)};
                    ++size_;
                    return;
                }
                if constexpr (std::is_default_constructible_v<T> && std::is_move_assignable_v<T>) {
                    new(buff() + wrp_idx(0)) T;
                    buff()[wrp_idx(size_)] = std::move(val);
                    ++size_;
                    return;
                }
                if constexpr (std::is_copy_constructible_v<T>) {
                    new(buff() + wrp_idx(0)) T{val};
                    ++size_;
                    return;
                }
                if constexpr (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>) {
                    new(buff() + wrp_idx(0)) T;
                    buff()[wrp_idx(size_)] = val;
                    ++size_;
                    return;
                }
            }
            throw std::runtime_error{"value insert error: cannot copy"};
        }

        void push_back(T &&val) {
            if(size_ == CAPACITY) {
                if constexpr (std::is_move_assignable_v<T>) {
                    buff()[pos_] = std::move(val);
                    pos_ = wrp_idx(1);
                    return;
                }
                if constexpr (std::is_move_constructible_v<T>) {
                    buff()[pos_].~T();
                    new(buff() + pos_) T{std::move(val)};
                    pos_ = wrp_idx(1);
                    return;
                }
                if constexpr (std::is_copy_assignable_v<T>) {
                    buff()[pos_] = val;
                    pos_ = wrp_idx(1);
                    return;
                }
                if constexpr (std::is_copy_constructible_v<T>) {
                    buff()[pos_].~T();
                    new(buff() + pos_) T{val};
                    pos_ = wrp_idx(1);
                    return;
                }
            } else {
                if constexpr (std::is_move_constructible_v<T>) {
                    new(buff() + wrp_idx(size_)) T{std::move(val)};
                    ++size_;
                    return;
                }
                if constexpr (std::is_default_constructible_v<T> && std::is_move_assignable_v<T>) {
                    new(buff() + wrp_idx(size_)) T;
                    buff()[wrp_idx(size_)] = std::move(val);
                    ++size_;
                    return;
                }
                if constexpr (std::is_copy_constructible_v<T>) {
                    new(buff() + wrp_idx(size_)) T{val};
                    ++size_;
                    return;
                }
                if constexpr (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>) {
                    new(buff() + wrp_idx(size_)) T;
                    buff()[wrp_idx(size_)] = val;
                    ++size_;
                    return;
                }
            }
            throw std::runtime_error{"value insert error: cannot move"};
        }

        template<typename ...ARGS>
        void emplace_front(ARGS &&...args) {
            pos_ = pos_ == 0 ? CAPACITY - 1 : pos_ - 1;
            if(size_ == CAPACITY) {
                if constexpr (std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>) {
                    buff()[pos_] = T{std::forward<ARGS>(args)...};
                    return;
                }
                if constexpr (std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>) {
                    buff()[pos_].~T();
                    new(buff() + pos_) T{std::forward<ARGS>(args)...};
                    return;
                }
            } else {
                if constexpr (std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>) {
                    new(buff() + wrp_idx(0)) T{std::forward<ARGS>(args)...};
                    ++size_;
                    return;
                }
                if constexpr (std::is_default_constructible_v<T> && (std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>)) {
                    new(buff() + wrp_idx(0)) T;
                    buff()[wrp_idx(size_)] = T{std::forward<ARGS>(args)...};
                    ++size_;
                    return;
                }
            }
            throw std::runtime_error{"value insert error: cannot copy"};
        }

        template<typename ...ARGS>
        void emplace_back(ARGS &&...args) {
            if(size_ == CAPACITY) {
                if constexpr (std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>) {
                    buff()[pos_] = T{std::forward<ARGS>(args)...};
                    pos_ = wrp_idx(1);
                    return;
                }
                if constexpr (std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>) {
                    buff()[pos_].~T();
                    new(buff() + pos_) T(std::forward<ARGS>(args)...);
                    pos_ = wrp_idx(1);
                    return;
                }
            } else {
                if constexpr (std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>) {
                    new(buff() + wrp_idx(size_++)) T(std::forward<ARGS>(args)...);
                    return;
                }
                if constexpr (std::is_default_constructible_v<T> && (std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>)) {
                    new(buff() + wrp_idx(size_)) T;
                    buff()[wrp_idx(size_)] = T{std::forward<ARGS>(args)...};
                    ++size_;
                    return;
                }
            }
            throw std::runtime_error{"value emplacing error: cannot construct"};
        }

        void clear() noexcept {
            for(size_type i{0}; i < size_; ++i) {
                buff()[wrp_idx(i)].~T();
            }
            pos_ = 0;
            size_ = 0;
        }

        iterator begin() {
            return iterator{this, 0};
        }

        iterator end() {
            return iterator{this, static_cast<size_type>(-1)};
        }

        const_iterator begin() const {
            return const_iterator{this, 0};
        }

        const_iterator end() const {
            return const_iterator{this, static_cast<size_type>(-1)};
        }

        const_iterator cbegin() const {
            return const_iterator{this, 0};
        }

        const_iterator cend() const {
            return const_iterator{this, static_cast<size_type>(-1)};
        }

        bool empty() const noexcept {
            return size() == 0;
        }

        T &front() {
            if(!empty()) {
                return buff()[pos_];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T &back() {
            if(!empty()) {
                return buff()[wrp_idx(size_ - 1)];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T const &front() const {
            if(!empty()) {
                return buff()[pos_];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T const &back() const {
            if(!empty()) {
                return buff()[wrp_idx(size_ - 1)];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T pop_front() {
            if(!empty()) {
                T res{std::move(buff()[pos_])};
                buff()[pos_].~T();
                pos_ = wrp_idx(1);
                --size_;
                return res;
            } else {
                throw std::runtime_error{"the buffer is empty"};
            }
        }

        T pop_back() {
            if(!empty()) {
                T res{std::move(buff()[wrp_idx(size_ - 1)])};
                buff()[wrp_idx(--size_)].~T();
                --size_;
                return res;
            } else {
                throw std::runtime_error{"the buffer is empty"};
            }
        }

    private:
        T *buff() {
            return reinterpret_cast<T *>(buf_.data());
        }

        T const *buff() const {
            return reinterpret_cast<T const *>(buf_.data());
        }

        size_type wrp_idx(ssize_type indx) const {
            return (pos_ + indx) % CAPACITY;
        }

    private:
        std::array<std::uint8_t, sizeof(T) * CAPACITY> buf_{};
        size_type pos_{0};
        size_type size_{0};
    };


    template<typename T, std::size_t CAPACITY>
    class scalar_circular_buffer {
        static_assert(std::is_scalar_v<T>, "element type must be a scalar type");
        static_assert(CAPACITY > 0, "capacity must be a number greater than zero");

        class iter {
        public:
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T *;
            using reference = T &;

            iter(scalar_circular_buffer *owner_ptr = nullptr, std::size_t curr_pos = static_cast<std::size_t>(-1)):
                owner_ptr_{owner_ptr}, curr_pos_{owner_ptr_ && curr_pos < owner_ptr_->size_ ? curr_pos : static_cast<std::size_t>(-1)} {
            }

            iter &operator++() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1) && curr_pos_ < owner_ptr_->size_) {
                    ++curr_pos_;
                    if(curr_pos_ >= owner_ptr_->size_) {
                        curr_pos_ = static_cast<std::size_t>(-1);
                    }
                }
                return *this;
            }

            iter operator++(int) {
                iter retval{*this};
                ++(*this);
                return retval;
            }

            bool operator==(iter const &other) const {
                return owner_ptr_ == other.owner_ptr_ && curr_pos_ == other.curr_pos_;
            }

            bool operator!=(iter const &other) const {
                return owner_ptr_ != other.owner_ptr_ || curr_pos_ != other.curr_pos_;
            }

            T &operator*() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

            T const &operator*() const {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

        private:
            scalar_circular_buffer *owner_ptr_;
            std::size_t curr_pos_{static_cast<std::size_t>(-1)};
        };

        class const_iter {
        public:
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T *;
            using reference = T &;

            const_iter(scalar_circular_buffer const *owner_ptr = nullptr, std::size_t curr_pos = static_cast<std::size_t>(-1)):
                owner_ptr_{owner_ptr}, curr_pos_{owner_ptr_ && curr_pos < owner_ptr_->size_ ? curr_pos : static_cast<std::size_t>(-1)} {
            }

            const_iter &operator++() {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1) && curr_pos_ < owner_ptr_->size_) {
                    ++curr_pos_;
                    if(curr_pos_ >= owner_ptr_->size_) {
                        curr_pos_ = static_cast<std::size_t>(-1);
                    }
                }
                return *this;
            }

            const_iter operator++(int) {
                const_iter retval{*this};
                ++(*this);
                return retval;
            }

            bool operator==(const_iter other) const {
                return owner_ptr_ == other.owner_ptr_ && curr_pos_ == other.curr_pos_;
            }

            bool operator!=(const_iter other) const {
                return owner_ptr_ != other.owner_ptr_ || curr_pos_ != other.curr_pos_;
            }

            T const &operator*() const {
                if(owner_ptr_ && curr_pos_ != static_cast<std::size_t>(-1)) {
                    if(curr_pos_ < owner_ptr_->size_) {
                        return (*owner_ptr_)[curr_pos_];
                    } else {
                        throw std::range_error{"iterator out of range"};
                    }
                } else {
                    throw std::runtime_error{"invalid iterator"};
                }
            }

        private:
            scalar_circular_buffer const *owner_ptr_;
            std::size_t curr_pos_{static_cast<std::size_t>(-1)};
        };

    public:
        using ssize_type = ssize_t;
        using size_type = std::size_t;
        using iterator = iter;
        using const_iterator = const_iter;

        T &operator[](size_type indx) {
            if(indx >= size_) {
                throw std::runtime_error{"index out of range"};
            }
            return buff()[wrp_idx(indx)];
        }

        T const &operator[](size_type indx) const {
            if(indx >= size_) {
                throw std::runtime_error{"index out of range"};
            }
            return buff()[wrp_idx(indx)];
        }

        size_type constexpr capacity() const noexcept {
            return CAPACITY;
        }

        size_type size() const noexcept {
            return size_;
        }

        void push_front(T val) noexcept {
            pos_ = pos_ == 0 ? CAPACITY - 1 : pos_ - 1;
            buff()[pos_] = val;
            ++size_;
            if(size_ > capacity()) {
                size_ = capacity();
            }
        }

        void push_back(T val) noexcept {
            if(size_ < capacity()) {
                buff()[wrp_idx(size_++)] = val;
            } else {
                buff()[pos_++] = val;
                pos_ %= CAPACITY;
            }
        }

        void clear() noexcept {
            pos_ = 0;
            size_ = 0;
        }

        bool empty() const noexcept {
            return size() == 0;
        }

        T &front() {
            if(!empty()) {
                return buff()[pos_];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T &back() {
            if(!empty()) {
                return buff()[wrp_idx(size_ - 1)];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T const &front() const {
            if(!empty()) {
                return buff()[pos_];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T const &back() const {
            if(!empty()) {
                return buff()[wrp_idx(size_ - 1)];
            }
            throw std::runtime_error{"buffer is empty"};
        }

        T pop_front() {
            if(!empty()) {
                T res{buff()[pos_]};
                pos_ = wrp_idx(1);
                --size_;
                return res;
            } else {
                throw std::runtime_error{"the buffer is empty"};
            }
        }

        T pop_back() {
            if(!empty()) {
                T res{buff()[wrp_idx(size_ - 1)]};
                --size_;
                return res;
            } else {
                throw std::runtime_error{"the buffer is empty"};
            }
        }

        iterator begin() {
            return iterator{this, 0};
        }

        iterator end() {
            return iterator{this, static_cast<size_type>(-1)};
        }

        const_iterator begin() const {
            return const_iterator{this, 0};
        }

        const_iterator end() const {
            return const_iterator{this, static_cast<size_type>(-1)};
        }

        const_iterator cbegin() const {
            return const_iterator{this, 0};
        }

        const_iterator cend() const {
            return const_iterator{this, static_cast<size_type>(-1)};
        }

    private:
        T *buff() noexcept {
            return reinterpret_cast<T *>(buf_.data());
        }

        T const *buff() const noexcept {
            return reinterpret_cast<T const *>(buf_.data());
        }

        size_type wrp_idx(size_type indx) const noexcept {
            return (pos_ + indx) % CAPACITY;
        }

    private:
        std::array<std::uint8_t, sizeof(T) * CAPACITY> buf_{};
        size_type pos_{0};
        size_type size_{0};
    };

}
