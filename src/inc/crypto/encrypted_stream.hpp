#pragma once

#include "../commondefs.hpp"
#include "../bit_util.hpp"
#include "../str_util.hpp"
#include "../timespec_wrapper.hpp"
#include "../base16.hpp"
#include "sha3_512.hpp"
#include "sha3_256.hpp"
#include "whirlpool.hpp"

namespace teal::crypt {

    /*************************************************************************************************
     * FILE_INTERFACE_T should look like this:
     * class some_class_name {
     * public:
     *     using offset_type = std::int64_t;
     *     using size_type = std::uint64_t;
     *     using ssize_type = std::int64_t;
     *     using pos_type = std::uint64_t;
     *
     *     void open(...); // arguments are forwarded
     *     void close();
     *     bool is_open() const;
     *     bool seek(offset_type pos);
     *     ssize_type size() const;
     *     std::vector<std::uint8_t> read(size_type read_size);
     *     std::vector<std::uint8_t> read_ex(size_type read_size, size_type starting_from);
     *     ssize_type write(void const *data, size_type data_size);
     *     ssize_type write_ex(void const *data, size_type data_size, size_type starting_from);
     *     ssize_type write(std::vector<std::uint8_t> const &data);
     *     ssize_type write_ex(std::vector<std::uint8_t> const &data, size_type starting_from);
     *     size_type tell();
     *     bool truncate(size_type len);
     * };
     *************************************************************************************************/

    template<typename FILE_INTERFACE_T, typename CIPHER_T>
    class encrypted_stream {
    public:
        using size_type = typename FILE_INTERFACE_T::size_type;
        using ssize_type = typename FILE_INTERFACE_T::ssize_type;
        using offset_type = typename FILE_INTERFACE_T::offset_type;
        using pos_type = typename FILE_INTERFACE_T::pos_type;

    private:
        static constexpr size_type crypto_block_size_{CIPHER_T::block_size()};
        static constexpr size_type header_size_{crypto_block_size_};

    public:
        encrypted_stream() = default;
        encrypted_stream(std::shared_ptr<FILE_INTERFACE_T> const &file_ptr): file_ptr_{file_ptr} {}
        encrypted_stream(std::shared_ptr<FILE_INTERFACE_T> &&file_ptr): file_ptr_{std::move(file_ptr)} {}
        encrypted_stream(encrypted_stream const &) = delete;
        encrypted_stream &operator=(encrypted_stream const &) = delete;
        encrypted_stream(encrypted_stream &&) = default;
        encrypted_stream &operator=(encrypted_stream &&) = default;
        ~encrypted_stream() { close(); }

        void reset_file_object(std::shared_ptr<FILE_INTERFACE_T> const &s_ptr) {
            file_ptr_ = s_ptr;
            if(is_valid()) {
                content_pos_ = 0;
            } else {
                content_pos_ = -1;
            }
        }

        FILE_INTERFACE_T &file_object() {
            if(!file_ptr_) {
                throw std::runtime_error{"subsequent file is not assigned"};
            }
            return *file_ptr_;
        }

        template<typename...TT>
        bool open(TT&&...vv) {
            bool res{false};
            if(is_open()) {
                file_ptr_->close();
                content_pos_ = -1;
                file_ptr_->open(std::forward<TT>(vv)...);
                if(is_open()) {
                    if(is_valid()) {
                        content_pos_ = 0;
                    }
                    res = true;
                }
            } else {
                if(file_ptr_) {
                    file_ptr_->open(std::forward<TT>(vv)...);
                    if(is_open()) {
                        if(is_valid()) {
                            content_pos_ = 0;
                        }
                        res = true;
                    }
                }
            }
            return res;
        }

        bool is_valid() {
            bool res{false};
            if(is_open()) {
                auto whs{whole_size()};
                if((static_cast<ssize_type>(crypto_block_size_) <= whs) && (whs % crypto_block_size_ == 0)) {
                    ssize_type max_internal_size{whs - (ssize_type)crypto_block_size_};
                    ssize_type internal_size{content_size()};
                    if(max_internal_size == 0) {
                        res = internal_size == 0;
                    } else {
                        ssize_type min_internal_size{(ssize_type)(max_internal_size - crypto_block_size_ + 1)};
                        if(internal_size <= max_internal_size && internal_size >= min_internal_size) {
                            res = true;
                        }
                    }
                }
            }
            return res;
        }

        bool reset_all_data() {
            bool res{false};
            if(is_open()) {
                if(file_ptr_->truncate(0) && set_content_size(0)) {
                    content_pos_ = 0;
                    res = true;
                }
            }
            if(!res) {
                content_pos_ = -1;
            }
            return res;
        }

        void close() {
            if(file_ptr_) { file_ptr_->close(); }
        }

        bool is_open() const {
            return file_ptr_ && file_ptr_->is_open();
        }

        ssize_type size() {
            if(is_valid()) {
                return content_size();
            } else {
                return -1;
            }
        }

        ssize_type tell() const {
            return content_pos();
        }

        ssize_type seek(ssize_type pos) {
            ssize_type res{content_pos_};
            if(is_valid()) {
                if(pos >= 0) {
                    content_pos_ = pos;
                } else {
                    content_pos_ = 0;
                }
                res = content_pos_;
            }
            return res;
        }

        // binary
        void set_key(void const *k, std::size_t k_len) {
            if(k_len == 64) {
                for(std::size_t i{0}; i < 64; ++i) { key_[i] = reinterpret_cast<std::uint8_t const *>(k)[i]; }
            } else {
                key_ = teal::crypt::sha3_512_compute(k, k_len);
            }
            nonce_ = teal::crypt::sha3_512_compute(key_);;
        }

        void set_key(std::vector<std::uint8_t> const &k) {
            set_key(k.data(), k.size());
        }

        template<std::size_t N>
        void set_key(std::array<std::uint8_t, N> const &k) {
            set_key(k.data(), k.size());
        }

        // hex
        void set_key(std::string const &k) {
            set_key(hex_str_to_data(k));
        }

        ssize_type write(void const *buff_ptr, ssize_type write_count) {
            if(!is_valid()) {
                return -1;
            }
            if(content_pos_ < 0) {
                content_pos_ = 0;
            }
            if(write_count > 0) {
                ssize_type file_content_size{content_size()};
                if(file_content_size < 0) {
                    reset_all_data();
                    if(!is_valid()) {
                        content_pos_ = -1;
                        return -1;
                    }
                    file_content_size = 0;
                }

                ssize_type content_blocks_count{size_to_complete_segments_count(file_content_size)};
                ssize_type content_pos_block{offs_to_segment_no(content_pos())};
                ssize_type write_end_block{offs_to_segment_no(content_pos() + write_count - 1)};
                ssize_type write_blocks_count{write_end_block - content_pos_block + 1};

                std::vector<std::uint8_t> write_buff{};
                write_buff.resize(write_blocks_count * crypto_block_size_);

                if(content_blocks_count > 0) {
                    if(content_pos_block < content_blocks_count) {
                        std::vector<std::uint8_t> write_head{read_content_blocks(content_pos_block, 1)};
                        if(write_head.size() == crypto_block_size_) {
                            auto p{singleblock_decrypt(&write_head[0], content_pos_block)};
                            std::memcpy(&write_buff[0], &p[0], crypto_block_size_);
                        } else {
                            return -1;
                        }
                    }
                    if(write_end_block < content_blocks_count && write_end_block > content_pos_block) {
                        std::vector<std::uint8_t> write_tail{read_content_blocks(write_end_block, 1)};
                        if(write_tail.size() == crypto_block_size_) {
                            auto p{singleblock_decrypt(&write_tail[0], write_end_block)};
                            std::memcpy(&write_buff[write_buff.size() - crypto_block_size_], &p[0], crypto_block_size_);
                        } else {
                            return -1;
                        }
                    }
                }

                ssize_type new_data_offset_in_block{offs_to_offs_in_segment(content_pos())};
                std::memcpy(&write_buff[new_data_offset_in_block], buff_ptr, write_count);
                for(offset_type offs = 0; offs < write_blocks_count; ++offs) {
                    auto encrypted_write_data{singleblock_encrypt(&write_buff[offs * crypto_block_size_], content_pos_block + offs)};
                    std::memcpy(&write_buff[offs * crypto_block_size_], encrypted_write_data.data(), crypto_block_size_);
                }
                ssize_type written{raw_write(content_block_no_to_absolute_pos(content_pos_block), write_buff.data(), write_buff.size())};
                if(written == (ssize_type)write_buff.size()) {
                    ssize_type content_new_pos{content_pos() + write_count};
                    content_pos_ = content_new_pos;
                    if(content_new_pos > file_content_size) {
                        set_content_size(content_new_pos);
                    }
                    return write_count;
                } else {
                    return -1;
                }
            }
            return 0;
        }

        ssize_type write(std::string const &str) {
            return write(str.data(), str.size());
        }

        ssize_type write(std::vector<std::uint8_t> const &str) {
            return write(str.data(), str.size());
        }

        ssize_t read(void *buff, ssize_type read_count) {
            if(!is_valid()) {
                return -1;
            }
            if(content_pos_ < 0) {
                content_pos_ = 0;
            }
            auto file_content_size{content_size()};
            if(read_count > 0 && content_pos() < file_content_size) {
                ssize_type content_pos_block{offs_to_segment_no(content_pos())};
                ssize_type read_end_block{offs_to_segment_no(content_pos() + read_count - 1)};
                ssize_type read_blocks_count{read_end_block - content_pos_block + 1};

                std::vector<std::uint8_t> read_buff{read_content_blocks(content_pos_block, read_blocks_count)};
                if(read_buff.size() && read_buff.size() % crypto_block_size_ == 0) {
                    for(offset_type offs = 0; offs < (offset_type)(read_buff.size() / crypto_block_size_); ++offs) {
                        auto decrypted_read_data{singleblock_decrypt(&read_buff[offs * crypto_block_size_], content_pos_block + offs)};
                        std::memcpy(&read_buff[offs * crypto_block_size_], decrypted_read_data.data(), crypto_block_size_);
                    }
                    ssize_type offset_in_first_block{offs_to_offs_in_segment(content_pos())};

                    ssize_type new_calc_pos{content_pos() + read_count};
                    ssize_type new_max_pos{file_content_size};

                    auto real_rd_cnt{std::min(read_count, file_content_size - content_pos())};
                    std::memcpy(buff, read_buff.data() + offset_in_first_block, real_rd_cnt);

                    content_pos_ = std::min(new_calc_pos, new_max_pos);

                    return real_rd_cnt;
                } else {
                    return -1;
                }
            } else {
                return 0;
            }
        }

        std::vector<std::uint8_t> read(ssize_type read_count) {
            std::vector<std::uint8_t> res{};
            if(read_count > 0) {
                res.resize(read_count);
                auto actually_read{read(&res[0], read_count)};
                if(actually_read < 0) {
                    throw std::runtime_error{"encrypted_stream::read(): failed to read data"};
                }
                if(actually_read != read_count) {
                    res.resize(actually_read);
                }
            }
            return res;
        }

        bool truncate(size_type len = 0) {
            if(file_ptr_ && set_content_size(len)) {
                size_type trunc_size{(size_to_complete_segments_count(len) + 1) * crypto_block_size_};
                return file_ptr_->truncate(trunc_size);
            }
            return false;
        }

    private:
        ssize_type content_start() {
            return header_size_;
        }

        ssize_type content_pos_to_absolute_pos(pos_type pos) {
            return header_size_ + pos;
        }

        ssize_type content_block_no_to_absolute_pos(pos_type blk_no) {
            return header_size_ + blk_no * crypto_block_size_;
        }

        std::vector<std::uint8_t> raw_read(ssize_type from, ssize_type count) {
            if(is_open()) {
                return file_ptr_->read_ex(count, from);
            }
            return {};
        }

        std::vector<std::uint8_t> read_content_blocks(ssize_type starting_block_no, ssize_type blocks_count = 1) {
            if(is_open()) {
                return file_ptr_->read_ex(blocks_count * crypto_block_size_, content_block_no_to_absolute_pos(starting_block_no));
            }
            return {};
        }

        ssize_type raw_write(ssize_type from, void const *buf_ptr, ssize_type write_size) {
            if(is_open()) {
                return file_ptr_->write_ex(buf_ptr, write_size, from);
            }
            return -1;
        }

        ssize_type content_size() {
            std::vector<std::uint8_t> block_zero{raw_read(0, crypto_block_size_)};
            if(block_zero.size() != crypto_block_size_) {
                return -1;
            }
            auto decrypted_read_data{singleblock_decrypt(&block_zero[0], -1)};
            std::uint64_t sz{teal::bit_util::swap_on_be<std::uint64_t>{*(std::uint64_t *)decrypted_read_data.data()}.val};
            return sz;
        }

        bool set_content_size(ssize_type s) {
            if(s >= 0 && file_ptr_ && file_ptr_->is_open()) {
                std::array<std::uint8_t, crypto_block_size_> block_zero{};
                auto ft{timespec_wrapper::gmtnow().as_iso_8601_str(6)};
                std::memcpy(&block_zero[sizeof(std::uint64_t)], ft.data(), std::min(block_zero.size() - sizeof(std::uint64_t), ft.size()));
                std::uint64_t sz{teal::bit_util::swap_on_be<std::uint64_t>{(std::uint64_t)s}.val};
                std::memcpy(&block_zero[0], &sz, sizeof(std::uint64_t));
                auto encrypted_write_data{singleblock_encrypt(&block_zero[0], -1)};
                if(raw_write(0, &encrypted_write_data[0], crypto_block_size_) == crypto_block_size_) {
                    return true;
                }
            }
            return false;
        }

        ssize_type whole_size() const {
            return file_ptr_ ? file_ptr_->size() : -1;
        }

        ssize_type content_pos() const {
            return content_pos_;
        }

    private:
        std::array<std::uint8_t, CIPHER_T::iv_length()> iv(std::int64_t block_no) const {
            std::array<std::uint8_t, CIPHER_T::iv_length()> res{};
            std::uint64_t block_no_le{teal::bit_util::swap_on_be<std::uint64_t>{(std::uint64_t)block_no}.val};
            std::memcpy(&res[0], &block_no_le, sizeof(std::uint64_t));
            std::memcpy(&res[sizeof(std::uint64_t)], nonce_.data(), std::min(nonce_.size(), CIPHER_T::iv_length() - sizeof(std::uint64_t)));
            return res;
        }

        std::array<std::uint8_t, crypto_block_size_> singleblock_encrypt(void const *plain_ptr, std::int64_t block_no) {
            if(plain_ptr) {
                auto ivec{iv(block_no)};
                std::array<std::uint8_t, crypto_block_size_> ciphertext{};
                CIPHER_T cipher{key_.data(), ivec.data()};
                cipher.encrypt_block(plain_ptr, &ciphertext[0]);
                return ciphertext;
            }
            return {};
        }

        std::array<std::uint8_t, crypto_block_size_> singleblock_decrypt(void const *cipher_ptr, std::int64_t block_no) const {
            if(cipher_ptr) {
                auto ivec{iv(block_no)};
                std::array<std::uint8_t, crypto_block_size_> plaintext{};
                CIPHER_T cipher{key_.data(), ivec.data()};
                cipher.decrypt_block(cipher_ptr, &plaintext[0]);
                return plaintext;
            }
            return {};
        }

        template<typename T = size_type, T block_size = crypto_block_size_>
        static inline T offs_to_segment_no(T offs) {
            return offs / block_size;
        }
        template<typename T = size_type, T block_size = crypto_block_size_>
        static inline T size_to_complete_segments_count(T size) {
            T int_blks_cnt{size / block_size};
            T int_blks_rem{size % block_size};
            return int_blks_cnt  + (int_blks_rem ? 1 : 0);
        }
        template<typename T = ssize_type, T block_size = crypto_block_size_>
        static inline T offs_to_segmented_offs(T offs) {
            T int_blks_cnt{offs / block_size};
            return int_blks_cnt * block_size;
        }
        template<typename T = size_type, T block_size = crypto_block_size_>
        static inline T size_to_segmented_size(T size) {
            T int_blks_cnt{size / block_size};
            T int_blks_rem{size % block_size};
            return int_blks_cnt * block_size + (int_blks_rem ? block_size : 0);
        }
        template<typename T = ssize_type, T block_size = crypto_block_size_>
        static inline T offs_to_offs_in_segment(T offs) {
            return offs % block_size;
        }

    private:
        std::shared_ptr<FILE_INTERFACE_T> file_ptr_{std::make_shared<FILE_INTERFACE_T>()};
        ssize_type content_pos_{-1};
        std::array<std::uint8_t, 64> key_{};
        std::array<std::uint8_t, 64> nonce_{};
    };

}
