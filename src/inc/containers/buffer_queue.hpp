#pragma once

#include "../commondefs.hpp"

namespace teal {

    class buffer_queue final {
    public:
        std::size_t size() const {
            return buf_.size();
        }

        bool empty() const {
            return buf_.empty();
        }

        void push(std::uint8_t b) {
            buf_.push_back(b);
        }

        void push(void const *data, std::size_t data_size) {
            buf_.insert(buf_.end(), (std::uint8_t const *)data, (std::uint8_t const *)data + data_size);
        }

        void push(std::vector<std::uint8_t> const &v) {
            buf_.insert(buf_.end(), v.begin(), v.end());
        }

        void push(std::string const &s) {
            buf_.insert(buf_.end(), s.begin(), s.end());
        }

        std::uint8_t pop_byte() {
            if(buf_.empty()) {
                throw std::runtime_error{"the buffer is empty"};
            }
            std::uint8_t res{buf_[0]};
            buf_.pop_front();
            return res;
        }

        std::vector<std::uint8_t> pop_bytevec(std::size_t n) {
            std::vector<std::uint8_t> res{};
            if(buf_.empty()) {
                return res;
            }
            res.assign(buf_.begin(), buf_.begin() + std::min<std::size_t>(n, buf_.size()));
            buf_.erase(buf_.begin(), buf_.begin() + std::min<std::size_t>(n, buf_.size()));
            return res;
        }

        std::string pop_str(std::size_t n) {
            std::string res{};
            if(buf_.empty()) {
                return res;
            }
            res.assign(buf_.begin(), buf_.begin() + std::min<std::size_t>(n, buf_.size()));
            buf_.erase(buf_.begin(), buf_.begin() + std::min<std::size_t>(n, buf_.size()));
            return res;
        }

        std::uint8_t peek_byte(size_t pos) const {
            if(buf_.empty()) {
                throw std::runtime_error{"the buffer is empty"};
            }
            return buf_.at(pos);
        }

        std::vector<std::uint8_t> peek_bytevec(std::size_t n) const {
            std::vector<std::uint8_t> res{};
            if(buf_.empty()) { return res; }
            res.assign(buf_.cbegin(), buf_.cbegin() + std::min<std::size_t>(n, buf_.size()));
            return res;
        }

        std::string as_string() const {
            if(buf_.empty()) { return {}; }
            return std::string{buf_.cbegin(), buf_.cbegin() + buf_.size()};
        }

        std::vector<std::uint8_t> as_bytevec() const {
            if(buf_.empty()) { return {}; }
            return std::vector<std::uint8_t>{buf_.cbegin(), buf_.cbegin() + buf_.size()};
        }

        void clear() {
            buf_.clear();
        }

    private:
        std::deque<std::uint8_t> buf_{};
    };

}
