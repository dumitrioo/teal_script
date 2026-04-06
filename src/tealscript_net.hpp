#pragma once

#include "inc/commondefs.hpp"
#include "inc/str_util.hpp"
#include "inc/terminable.hpp"
#include "inc/timespec_wrapper.hpp"
#include "inc/emhash/hash_table8.hpp"
#include "inc/hash/hash.hpp"
#include "inc/net/net_utils.hpp"
#include "inc/net/tcp_client.hpp"
#include "inc/net/tcp_server.hpp"
#include "inc/net/net_data_transfer.hpp"
#include "inc/net/url.hpp"

#include "tealscript_util.hpp"
#include "tealscript_token.hpp"
#include "tealscript_expr.hpp"
#include "tealscript_statement.hpp"
#include "tealscript_exec_ctx.hpp"

namespace teal {

    class pp_server {
        struct multiplexing;

    public:
        ~pp_server() {
            stop();
        }

        void set_on_data_arrived(std::function<void(conn_id_t, bytevec const &)> &&on_data_arrived) {
            std::unique_lock l{on_data_arrived_mtp_};
            on_data_arrived_ = std::move(on_data_arrived);
        }

        bool started() const {
            return tns_.started();
        }

        void start(const std::string &address, std::uint16_t port, std::size_t num_work_threads) {
            if(tns_.started()) { return; }
            tns_.set_on_new_connection([this](conn_id_t conn_id) {
            });
            tns_.set_on_data_arrived([this](conn_id_t conn_id) {
                auto d{tns_.get_conn_data(conn_id)};
                if(d) {
                    std::shared_ptr<multiplexing> mxr{get_or_create_muxerset(conn_id)};
                    std::unique_lock l{mxr->demux_mtp_};
                    mxr->demux_bottom_layer_.set_network_input(*d);
                    while(auto ofid{mxr->demux_bottom_layer_.fetch_in_data()}) {
                        if(std::optional<bytevec> message{mxr->demuxer_.add_data(*ofid)}) {
                            notify_on_data_arrived(conn_id, *message);
                        }
                    }
                }
            });
            tns_.set_on_connection_closed([this](conn_id_t conn_id) {
                remove_muxerset_by_conn_id(conn_id);
            });
            tns_.start(address, port, num_work_threads);
        }

        void stop() {
            tns_.stop();
            tns_.set_on_new_connection(nullptr);
            tns_.set_on_data_arrived(nullptr);
            tns_.set_on_connection_closed(nullptr);
            {
                std::unique_lock l{muxers_mtp_};
                muxers_.clear();
            }
        }

        int send(conn_id_t conn_id, void const *data, std::size_t dsize) {
            std::shared_ptr<multiplexing> mxr{get_or_create_muxerset(conn_id)};
            std::unique_lock l{mxr->mux_mtp_};
            mxr->muxer_.add_message(data, dsize);
            int res{};
            while(auto chnk{mxr->muxer_.fetch_out_chunk()}) {
                bytevec pckt{mxr->mux_bottom_layer_.output_data_to_network_frame(*chnk)};
                res += tns_.send(conn_id, pckt);
            }
            return res;
        }

        int send(conn_id_t conn_id, std::string const &data) {
            return send(conn_id, data.data(), data.size());
        }

        int send(conn_id_t conn_id, bytevec const &data) {
            return send(conn_id, data.data(), data.size());
        }

    private:
        void notify_on_data_arrived(conn_id_t c, bytevec const &d) {
            std::shared_lock l{on_data_arrived_mtp_};
            if(on_data_arrived_) {
                on_data_arrived_(c, d);
            }
        }

        std::shared_ptr<multiplexing> get_or_create_muxerset(conn_id_t conn_id) {
            std::shared_lock l{muxers_mtp_};
            std::shared_ptr<multiplexing> res{};
            auto it{muxers_.find(conn_id)};
            if(it != muxers_.end()) {
                res = it->second;
            } else {
                l.unlock();
                std::unique_lock l1{muxers_mtp_};
                res = muxers_[conn_id];
                if(!res) {
                    res = std::make_shared<multiplexing>();
                    muxers_[conn_id] = res;
                }
            }
            return res;
        }

        void remove_muxerset_by_conn_id(conn_id_t conn_id) {
            std::unique_lock l{muxers_mtp_};
            muxers_.erase(conn_id);
        }

    private:
        teal_net_server tns_{};

        mutable std::shared_mutex on_data_arrived_mtp_{};
        std::function<void(conn_id_t, bytevec const &)> on_data_arrived_{nullptr};

        struct multiplexing {
            mutable std::shared_mutex mux_mtp_{};
            net::sized_packets_exchanger mux_bottom_layer_{};
            net::packets_muxer<std::uint32_t> muxer_{1400};
            mutable std::shared_mutex demux_mtp_{};
            net::sized_packets_exchanger demux_bottom_layer_{};
            net::packets_demuxer demuxer_{};
        };
        mutable std::shared_mutex muxers_mtp_{};
        emhash8::HashMap<conn_id_t, std::shared_ptr<multiplexing>> muxers_{};
    };

    class pp_client {
    public:
        void start(std::string host, std::uint16_t port) {
            connect_ = true;
            host_ = host;
            port_ = port;
            // tnc_.set_on_conn_established([&]() {});
            // tnc_.set_on_data_arrived([&]() {
            //     /* auto d{tnc_.receive()}; */
            // });
            // tnc_.set_on_conn_closed([]() {});
            tnc_.connect(host, port);
        }

        void stop() {
            tnc_.disconnect();
            // tnc_.set_on_conn_established(nullptr);
            // tnc_.set_on_data_arrived(nullptr);
            // tnc_.set_on_conn_closed(nullptr);
        }

        bool connected() const {
            return tnc_.connected();
        }

        int send(const void *data, size_t data_size) {
            int res{};
            muxer_.add_message(data, data_size);
            while(auto ochnk{muxer_.fetch_out_chunk()}) {
                bytevec pkt{mux_bottom_layer_.output_data_to_network_frame(*ochnk)};
                res += tnc_.send(pkt);
            }
            return res;
        }

        int send(bytevec const &data) {
            return send(data.data(), data.size());
        }

        int send(std::string const &data) {
            return send(data.data(), data.size());
        }

        std::optional<bytevec> receive(long double timeout = 0) {
            std::optional<bytevec> in{tnc_.receive(timeout)};
            if(in) {
                demux_bottom_layer_.set_network_input(*in);
                if(auto ofid{demux_bottom_layer_.fetch_in_data()}) {
                    return demuxer_.add_data(*ofid);
                }
            }
            return {};
        }

        std::string const &host() const {
            return host_;
        }

        std::uint16_t port() const {
            return port_;
        }

    private:
        teal_net_client tnc_{};

        net::sized_packets_exchanger mux_bottom_layer_{};
        net::packets_muxer<std::uint32_t> muxer_{1400};
        net::sized_packets_exchanger demux_bottom_layer_{};
        net::packets_demuxer demuxer_{};

        std::string host_{};
        std::uint16_t port_{0};
        bool connect_{false};
    };

}
