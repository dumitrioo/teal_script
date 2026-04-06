#pragma once

#include "../commondefs.hpp"
#include "../containers/buffer_queue.hpp"
#include "../math/math_util.hpp"
#include "../str_util.hpp"
#include "../timespec_wrapper.hpp"
#include "../serialization.hpp"
#include "../sequence_generator.hpp"

namespace teal::net {

    class data_fragments_buffer final {
    public:
        data_fragments_buffer() {
        }

        data_fragments_buffer(std::vector<std::uint8_t> &&data) {
            total_size_ = data.size();
            data_[0] = std::move(data);
        }

        data_fragments_buffer(std::vector<std::uint8_t> const &data) {
            total_size_ = data.size();
            data_[0] = data;
        }

        data_fragments_buffer(data_fragments_buffer const &that) = default;

        data_fragments_buffer(data_fragments_buffer &&that) = default;

        data_fragments_buffer &operator=(data_fragments_buffer const &that) = default;

        data_fragments_buffer &operator=(data_fragments_buffer &&that) = default;

        ~data_fragments_buffer() = default;

        void clear() {
            total_size_ = 0;
            data_.clear();
        }

        void append(std::uint64_t index, void const *data, std::size_t data_size) {
            if(data && data_size) {
                auto it{data_.find(index)};
                std::size_t sub_size{0};
                if(it != data_.end()) { sub_size = it->first; }
                total_size_ = total_size_ + data_size - sub_size;
                data_[index] = std::vector<std::uint8_t>{reinterpret_cast<std::uint8_t const *>(data), reinterpret_cast<std::uint8_t const *>(data) + data_size};
            }
        }

        void append(void const *data, std::size_t data_size) {
            if(data && data_size) {
                if(data_.size()) {
                    auto it{data_.end()};
                    --it;
                    data_[it->first + 1] = std::vector<std::uint8_t>{reinterpret_cast<std::uint8_t const *>(data), reinterpret_cast<std::uint8_t const *>(data) + data_size};
                } else {
                    data_[0] = std::vector<std::uint8_t>{reinterpret_cast<std::uint8_t const *>(data), reinterpret_cast<std::uint8_t const *>(data) + data_size};
                }
                total_size_ += data_size;
            }
        }

        void append(std::uint64_t index, std::vector<std::uint8_t> const &data) {
            if(data.size()) {
                auto it{data_.find(index)};
                std::size_t sub_size{0};
                if(it != data_.end()) { sub_size = it->first; }
                total_size_ = total_size_ + data.size() - sub_size;
                data_[index] = data;
            }
        }

        void append(std::uint64_t index, std::vector<std::uint8_t> &&data) {
            if(data.size()) {
                auto it{data_.find(index)};
                std::size_t sub_size{0};
                if(it != data_.end()) { sub_size = it->first; }
                total_size_ = total_size_ + data.size() - sub_size;
                data_[index] = std::move(data);
            }
        }

        void append(std::vector<std::uint8_t> const &data) {
            if(data.size()) {
                total_size_ += data.size();
                if(data_.size()) {
                    auto it{data_.end()};
                    --it;
                    data_[it->first + 1] = data;
                } else {
                    data_[0] = data;
                }
            }
        }

        void append(std::vector<std::uint8_t> &&data) {
            if(data.size()) {
                total_size_ += data.size();
                if(data_.size()) {
                    auto it{data_.end()};
                    --it;
                    data_[it->first + 1] = std::move(data);
                } else {
                    data_[0] = std::move(data);
                }
            }
        }

        void set(const std::vector<std::uint8_t> &data) {
            data_.clear();
            if(data.size()) {
                total_size_ += data.size();
                data_[0] = data;
            }
        }

        void set(std::vector<std::uint8_t> &&data) {
            data_.clear();
            if(data.size()) {
                total_size_ += data.size();
                data_[0] = std::move(data);
            }
        }

        std::vector<std::uint8_t> compose() const {
            if(data_.size() == 1) {
                return data_.begin()->second;
            } else {
                std::vector<std::uint8_t> res{};
                res.reserve(total_size_);
                for(auto it{data_.begin()}; it != data_.end(); ++it) {
                    res.insert(res.end(), it->second.begin(), it->second.end());
                }
                return res;
            }
        }

        std::shared_ptr<std::vector<std::uint8_t>> compose_ptr() const {
            if(data_.size() == 1) {
                return std::make_shared<std::vector<std::uint8_t>>(data_.begin()->second);
            } else {
                std::shared_ptr<std::vector<std::uint8_t>> res{std::make_shared<std::vector<std::uint8_t>>()};
                res->reserve(total_size_);
                for(auto it{data_.begin()}; it != data_.end(); ++it) {
                    res->insert(res->end(), it->second.begin(), it->second.end());
                }
                return res;
            }
        }

        std::size_t size() const {
            return total_size_;
        }

        std::size_t chunks() const {
            return data_.size();
        }

    private:
        std::size_t total_size_{0};
        std::map<std::uint64_t, std::vector<std::uint8_t>> data_{};
    };


    class sized_packets_exchanger {
        static size_t size_size(std::uint8_t b) {
            for(int i{0}; i < 8; ++i) {
                if(((b << i) & 0x80) == 0) {
                    if(i == 1) {
                        break;
                    }
                    return i == 0 ? 1 : i;
                }
            }
            return 0;
        }

    public:
        sized_packets_exchanger() {}
        sized_packets_exchanger(sized_packets_exchanger const &) = default;
        sized_packets_exchanger(sized_packets_exchanger &&) = default;
        sized_packets_exchanger &operator=(sized_packets_exchanger const &) = default;
        sized_packets_exchanger &operator=(sized_packets_exchanger &&) = default;
        ~sized_packets_exchanger() = default;

        bytevec output_data_to_network_frame(void const *data, std::size_t data_size) {
            bytevec res{};
            if(data && data_size) {
                std::string ss{str_util::ucs_to_utf8(data_size)};
                res.insert(res.end(), ss.begin(), ss.end());
                res.insert(res.end(), (std::uint8_t const *)data, (std::uint8_t const *)data + data_size);
            }
            return res;
        }

        bytevec output_data_to_network_frame(bytevec const &data) {
            return output_data_to_network_frame(data.data(), data.size());
        }

        bytevec output_data_to_network_frame(char const *str) {
            return output_data_to_network_frame(str, strlen(str));
        }

        bytevec output_data_to_network_frame(std::string const &data) {
            return output_data_to_network_frame(data.data(), data.size());
        }

        bytevec output_data_to_network_frame(std::string_view data) {
            return output_data_to_network_frame(data.data(), data.size());
        }

        void set_network_input(void const *data, std::size_t data_size) {
            input_buf_.push(data, data_size);
        }

        void set_network_input(bytevec const &data) {
            set_network_input(data.data(), data.size());
        }

        void set_network_input(char const *str) {
            set_network_input(str, strlen(str));
        }

        void set_network_input(std::string const &data) {
            set_network_input(data.data(), data.size());
        }

        void set_network_input(std::string_view data) {
            set_network_input(data.data(), data.size());
        }

        std::size_t input_data_available() const {
            return input_buf_.size();
        }

        std::optional<bytevec> fetch_in_data() {
            if(!input_buf_.empty()) {
                size_t ss{size_size(input_buf_.peek_byte(0))};
                if(ss > 0 && input_buf_.size() >= ss) {
                    auto hdr{input_buf_.peek_bytevec(ss)};
                    std::int64_t s{str_util::utf8_to_ucs_maxlen(hdr.data(), hdr.size(), nullptr)};
                    if(input_buf_.size() >= ss + s) {
                        input_buf_.pop_bytevec(ss);
                        return input_buf_.pop_bytevec(s);
                    }
                }
            }
            return {};
        }

        void clear() {
            input_buf_.clear();
        }

    private:
        buffer_queue input_buf_{};
    };


    template<typename ID_T>
    class packets_muxer {
        static_assert(
            std::is_same<ID_T, std::uint8_t>::value ||
            std::is_same<ID_T, std::uint16_t>::value ||
            std::is_same<ID_T, std::uint32_t>::value ||
            std::is_same<ID_T, std::uint64_t>::value,
            "allowed packets identifiers types are: std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t"
        );

    public:
        packets_muxer(std::uint64_t max_packet_size = 1400):
            max_packet_size_{max_packet_size}
        {
        }
        packets_muxer(packets_muxer const &) = delete;
        packets_muxer(packets_muxer &&) = delete;
        packets_muxer &operator=(packets_muxer const &) = delete;
        packets_muxer &operator=(packets_muxer &&) = delete;
        ~packets_muxer() = default;

        void add_message(bytevec &&d) {
            if(!d.empty()) {
                std::uint64_t key{stream_id_gen_()};
                out_buffers_.insert_or_assign(key, out_message_wrapper{std::move(d), key, max_packet_size_});
            }
        }

        void add_message(bytevec const &d) {
            if(!d.empty()) {
                std::uint64_t key{stream_id_gen_()};
                out_buffers_.insert_or_assign(key, out_message_wrapper{d, key, max_packet_size_});
            }
        }

        void add_message(void const *d, std::uint64_t ds) {
            if(d != nullptr && ds > 0) {
                std::uint64_t key{stream_id_gen_()};
                out_buffers_.insert_or_assign(key, out_message_wrapper{bytevec{(std::uint8_t const *)d, (std::uint8_t const *)d + ds}, key, max_packet_size_});
            }
        }

        void add_message(std::string const &d) {
            add_message(d.data(), d.size());
        }

        std::optional<bytevec> fetch_out_chunk() {
            if(!out_buffers_.empty()) {
                auto it{out_buffers_.find(out_buffers_key_)};
                if(it == out_buffers_.end()) {
                    it = out_buffers_.begin();
                }
                if(it != out_buffers_.end()) {
                    bytevec res{};
                    bool fetched{it->second.fetch_out_chunk(res)};
                    if(it->second.rest() == 0) {
                        it = out_buffers_.erase(it);
                    } else {
                        ++it;
                    }
                    if(it != out_buffers_.end()) {
                        out_buffers_key_ = it->first;
                    } else {
                        out_buffers_key_ = 0;
                    }
                    if(fetched) {
                        return res;
                    }
                } else {
                    out_buffers_key_ = 0;
                }
            }
            return {};
        }

        void clear() {
            stream_id_gen_.reset(1, 0);
            max_packet_size_ = 1400;
            out_buffers_.clear();
            out_buffers_key_ = 0;
        }

    private:
        class out_message_wrapper {
        public:
            out_message_wrapper(out_message_wrapper const &) = default;
            out_message_wrapper(out_message_wrapper &&) = default;
            out_message_wrapper &operator=(out_message_wrapper const &) = default;
            out_message_wrapper &operator=(out_message_wrapper &&) = default;
            ~out_message_wrapper() = default;
            out_message_wrapper(bytevec const &v, std::uint64_t stream_id, std::uint64_t max_packet_size):
                data_to_send_{v},
                stream_id_{stream_id},
                max_packet_size_{max_packet_size},
                total_packets_{
                    ((std::uint64_t)data_to_send_.size()) / max_packet_size +
                    ((((std::uint64_t)data_to_send_.size()) % max_packet_size) > 0 ? 1 : 0)
                }
            {
            }

            out_message_wrapper(bytevec &&v, std::uint64_t stream_id, std::uint64_t max_packet_size = 1400):
                data_to_send_{std::move(v)},
                stream_id_{stream_id},
                max_packet_size_{max_packet_size},
                total_packets_{
                    ((std::uint64_t)data_to_send_.size()) / max_packet_size +
                    ((((std::uint64_t)data_to_send_.size()) % max_packet_size) > 0 ? 1 : 0)
                }
            {
            }

            std::int64_t rest() const {
                std::int64_t ret{static_cast<std::int64_t>(data_to_send_.size()) - curr_pos_};
                return ret < 0LL ? 0LL : ret;
            }

            bool fetch_out_chunk(bytevec &res) {
                std::int64_t send_rest{rest()};
                if(send_rest > 0) {
                    std::int64_t send_size{math::clamp<std::int64_t>(send_rest, 0, max_packet_size_)};
                    serializer serial_packet{};
                    serial_packet.reserve(send_size + (sizeof(std::int64_t) + 2) * 3 + 32);
                    serial_packet << str_util::utoa<std::string>(stream_id_, 36)
                                  << str_util::utoa<std::string>(packet_no_, 36)
                                  << str_util::utoa<std::string>(total_packets_, 36);
                    serial_packet.push_back(data_to_send_.data() + curr_pos_, send_size);
                    res = std::move(serial_packet).data_vec();
                    curr_pos_ += send_size;
                    ++packet_no_;
                    return true;
                }
                return false;
            }

            bool done() const noexcept {
                return curr_pos_ >= (std::int64_t)data_to_send_.size();
            }

            bool empty() const noexcept {
                return data_to_send_.empty();
            }

        private:
            bytevec data_to_send_{};
            std::uint64_t stream_id_{0};
            std::uint64_t max_packet_size_{0};
            std::uint64_t total_packets_{0};
            std::uint64_t packet_no_{0};
            std::int64_t curr_pos_{0};
        };

        atomic_sequence_generator<ID_T> stream_id_gen_{};
        std::uint64_t max_packet_size_{1400};
        std::map<std::uint64_t, out_message_wrapper> out_buffers_{};
        std::uint64_t out_buffers_key_{0};
    };


    class packets_demuxer {
    public:
        packets_demuxer() = default;
        packets_demuxer(packets_demuxer const &) = delete;
        packets_demuxer(packets_demuxer &&) = delete;
        packets_demuxer &operator=(packets_demuxer const &) = delete;
        packets_demuxer &operator=(packets_demuxer &&) = delete;
        ~packets_demuxer() = default;

        std::optional<bytevec> add_data(void const *d, std::uint64_t ds) {
            if(d && ds) {
                std::uint64_t channel_no{};
                std::uint64_t pckt_no{};
                std::uint64_t total_packets{};
                bytevec pckt_data{};
                bool some_err{false};
                serial_reader serial_packet{d, ds};
                serial_reader::const_iterator it{};
                try {
                    it = serial_packet.begin();
                    if(!it.valid()) { return {}; }
                    channel_no = str_util::atoui(it->as_string(), 36);
                    ++it;
                    if(!it.valid()) { return {}; }
                    pckt_no = str_util::atoui(it->as_string(), 36);
                    ++it;
                    if(!it.valid()) { return {}; }
                    total_packets = str_util::atoui(it->as_string(), 36);
                    ++it;
                    if(!it.valid()) { return {}; }
                    pckt_data = it->as_bytevec();
                } catch(...) {
                    some_err = true;
                }
                if(some_err) {
                    return {};
                }
                if(total_packets > 1) {
                    bytevec whole_data{};
                    if(in_buffers_[channel_no].add_data(pckt_no, total_packets, std::move(pckt_data), whole_data)) {
                        in_buffers_.erase(channel_no);
                        return whole_data;
                    }
                } else if(total_packets == 1) {
                    return pckt_data;
                }
            }
            return {};
        }

        std::optional<bytevec> add_data(bytevec const &d) {
            return add_data(d.data(), d.size());
        }

        std::optional<bytevec> add_data(std::string const &d) {
            return add_data(d.data(), d.size());
        }

        void clear() {
            in_buffers_.clear();
        }

        std::size_t size() const {
            std::size_t res{0};
            for(auto &&p: in_buffers_) {
                res += p.second.size();
            }
            return res;
        }

        std::size_t queued_items() const {
            return in_buffers_.size();
        }

        void remove_queued_items_older_than_seconds(long double seconds_old) {
            for(auto it{in_buffers_.begin()}; it != in_buffers_.end(); ) {
                if(it->second.seconds_alive() > seconds_old) {
                    it = in_buffers_.erase(it);
                } else {
                    ++it;
                }
            }
        }

    private:
        class in_data_wrapper {
        public:
            in_data_wrapper() = default;
            in_data_wrapper(in_data_wrapper const &) = default;
            in_data_wrapper(in_data_wrapper &&) = default;
            in_data_wrapper &operator=(in_data_wrapper const &) = default;
            in_data_wrapper &operator=(in_data_wrapper &&) = default;
            ~in_data_wrapper() = default;

            bool add_data(std::uint64_t pckt_no, std::uint64_t total_packets, bytevec &&in_packet, bytevec &whole_data) {
                data_.append(pckt_no, std::move(in_packet));
                if(data_.chunks() == total_packets) {
                    whole_data = data_.compose();
                    return true;
                }
                return false;
            }

            std::size_t size() const {
                return data_.size();
            }

            long double seconds_alive() const {
                return curr_timestamp_seconds() - created_;
            }

        private:
            net::data_fragments_buffer data_{};
            long double created_{curr_timestamp_seconds()};
        };

        std::map<std::uint64_t, in_data_wrapper> in_buffers_{};
    };

}
