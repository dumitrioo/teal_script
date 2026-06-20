#pragma once

#include "commondefs.hpp"

namespace teal {

    class memory_file {
    public:
        using offset_type = std::int64_t;
        using size_type = std::uint64_t;
        using ssize_type = std::int64_t;
        using pos_type = std::uint64_t;

        void open(std::string const & = {}, bool for_write = true, bool /*create_if_ne*/ = true, bool truncate_on_open = false) {
            writeable_ = for_write;
            if(writeable_ && truncate_on_open) {
                mem_buff_.resize(0);
            }
            pos_ = 0;
            opened_ = true;
        }

        void close() {
            opened_ = false;
        }

        bool is_open() const { return opened_; }

        ssize_type size() const { return is_open() ? mem_buff_.size() : -1; }

        std::vector<std::uint8_t> read(size_type bytes_to_read) {
            std::vector<std::uint8_t> res{};
            if(is_open() && pos_ >= 0) {
                if(bytes_to_read > 0 && pos_ < static_cast<ssize_type>(mem_buff_.size())) {
                    res.assign(mem_buff_.begin() + pos_, mem_buff_.begin() + pos_ + std::min<ssize_type>(bytes_to_read, mem_buff_.size() - pos_));
                    pos_ = std::min<ssize_type>(pos_ + bytes_to_read, mem_buff_.size());
                }
            } else {
                throw std::runtime_error{"memory file is not opened or in invalid state"};
            }
            return res;
        }

        std::vector<std::uint8_t> read_ex(size_type read_size, size_type starting_from) {
            std::vector<std::uint8_t> res{};
            if(is_open() && pos_ >= 0) {
                if(seek_from_begin(starting_from)) {
                    return read(read_size);
                } else {
                    throw std::runtime_error{"error seeking appropriate position"};
                }
            } else {
                throw std::runtime_error{"memory file is not opened or in invalid state"};
            }
            return res;
        }

        ssize_type write(void const *data_ptr, size_type data_size) {
            if(is_open() && pos_ >= 0) {
                if(pos_ > (ssize_type)mem_buff_.size()) {
                    mem_buff_.resize(pos_);
                }
                size_type src_index{0};
                std::uint8_t const *ucdata_ptr{(std::uint8_t const *)data_ptr};
                while(pos_ < (ssize_type)mem_buff_.size() && src_index < data_size) {
                    mem_buff_[pos_++] = ucdata_ptr[src_index++];
                }
                if(src_index < data_size) {
                    mem_buff_.insert(mem_buff_.end(), ucdata_ptr + src_index, ucdata_ptr + data_size);
                    pos_ = mem_buff_.size();
                }
                return data_size;
            }
            return -1;
        }

        ssize_type write(std::vector<std::uint8_t> const &data) {
            return write(data.data(), data.size());
        }

        ssize_type write_ex(void const *data_ptr, size_type data_size, size_type starting_from) {
            if(seek(starting_from)) {
                return write(data_ptr, data_size);
            }
            return -1;
        }

        ssize_type write_ex(std::vector<std::uint8_t> const &data, size_type starting_from) {
            return write_ex(data.data(), data.size(), starting_from);
        }

        bool seek(offset_type pos) {
            return seek_from_begin(pos);
        }

        bool seek_from_curr(offset_type offs) {
            if(is_open()) {
                ssize_type res{pos_ + offs};
                if(res >= 0) {
                    pos_ = res;
                    return true;
                }
            }
            return false;
        }

        bool seek_from_begin(offset_type offs) {
            if(offs >= 0 && is_open()) {
                pos_ = offs;
                return true;
            }
            return false;
        }

        bool seek_to_begin() {
            return seek_from_begin(0);
        }

        bool seek_from_end(offset_type offs) {
            if(offs >= 0 && is_open()) {
                pos_ = mem_buff_.size() + offs;
                return true;
            }
            return false;
        }

        bool seek_to_end() {
            return seek_from_end(0);
        }

        ssize_type tell() {
            if(is_open()) {
                return pos_;
            }
            return -1;
        }

        bool truncate(size_type len) {
            if(is_open()) {
                mem_buff_.resize(len);
                return true;
            }
            return false;
        }


    //////////////////////////////////////////////////////////////////////////////////////////
    //
        // extra methods (bypasses file-like interface)
        void set_immediate_data(std::vector<std::uint8_t> const &d) {
            mem_buff_.assign(d.begin(), d.end());
        }

        void set_immediate_data(std::vector<std::uint8_t> &&d) {
            mem_buff_ = std::move(d);
        }

#ifdef PLATFORM_WINDOWS
#ifdef max
#undef max
#endif
#endif

        std::vector<std::uint8_t> get_immediate_data(size_type start_pos = 0, size_type data_size = std::numeric_limits<size_type>::max()) const {
            if(mem_buff_.empty() || start_pos >= mem_buff_.size() || data_size == 0) {
                return {};
            }
            size_type end_pos{start_pos + data_size};
            if(end_pos > mem_buff_.size()) {
                end_pos = mem_buff_.size();
            }
            return {mem_buff_.begin() + start_pos, mem_buff_.begin() + end_pos};
        }

        void reset_data() {
            mem_buff_.clear();
            pos_ = -1;
        }
    //
    //////////////////////////////////////////////////////////////////////////////////////////

    private:
        void adjust_pos() {
            if(opened_ && pos_ < 0) {
                pos_ = 0;
            }
        }

    private:
        bool writeable_{true};
        ssize_type pos_{-1};
        bool opened_{false};
        std::vector<std::uint8_t> mem_buff_{};
    };

}
