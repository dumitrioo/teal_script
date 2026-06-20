#pragma once

#include "../../commondefs.hpp"
#include "../socket_wrapper.hpp"
#include "ssl_utils.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace teal {

    namespace ssl {

        class connection {
        public:
            enum action {
                NONE,
                CONNECT,
                ACCEPT
            };

            connection() = default;
            connection(SSL *ssl_conn, std::shared_ptr<net::socket> skt, std::string const &ca_cert, action act = NONE) {
                ok_ = false;
                if(act == CONNECT || act == ACCEPT) {
                    try {
                        if(ssl_conn && skt) {
                            sk_ = skt;
                            ssl_conn_ptr_.reset(ssl_conn);
                            SSL_set_fd(ssl_conn_ptr_.get(), sk_->handle());
                            SSL_set_verify(ssl_conn_ptr_.get(), SSL_VERIFY_NONE, nullptr);
                            if(act == CONNECT) {
                                connect_unlocked(ca_cert);
                            } else if(act == ACCEPT) {
                                accept_unlocked();
                            }
                            ok_ = true;
                        }
                    } catch(...) {
                    }
                }
            }

            connection(connection const &) = delete;
            connection &operator=(connection const &) = delete;
            connection(connection &&) = delete;
            connection &operator=(connection &&) = delete;
            ~connection() {
                close();
            }

            connection &init(teal::ssl::unique_ptr<SSL> &&ssl_conn, std::shared_ptr<net::socket> skt, std::string const &ca_cert, action act = NONE) {
                ok_ = false;
                if(!ssl_conn || !skt || !skt->ok()) {
                    throw ssl_error("ssl connection: wrong connection state variable passed");
                }
                sk_ = skt;
                ssl_conn_ptr_ = std::move(ssl_conn);

                if(!SSL_set_fd(ssl_conn_ptr_.get(), sk_->handle())) {
                    throw ssl_error("ssl connection: failed to set socket descriptor");
                }

                if(act == CONNECT) {
                    connect_unlocked(ca_cert);
                } else if(act == ACCEPT) {
                    accept_unlocked();
                }
                ok_ = true;
                return *this;
            }

            void accept() {
                accept_unlocked();
            }
            void connect(std::string const &ca_cert = std::string{}) {
                connect_unlocked(ca_cert);
            }

            int send_message(const void *data, std::uint32_t data_size) {
                int res{};
                if(!sk_ || !sk_->ok() || !ssl_conn_ptr_) {
                    throw ssl_error("ssl connection: connection uninitialized");
                }
                if(data && data_size > 0) {
                    std::uint32_t net_size = teal::bit_util::hnswap<std::uint32_t>{data_size}.val;
                    const char *curr_buff = reinterpret_cast<const char *>(&net_size);
                    res = send_data_unlocked(curr_buff, sizeof(std::uint32_t));
                    if(res == sizeof(std::uint32_t)) {
                        curr_buff = reinterpret_cast<const char *>(data);
                        res = send_data_unlocked(curr_buff, data_size);
                    }
                }
                return res;
            }

            int send_data(const void *data, std::uint32_t data_size) {
                return send_data_unlocked(data, data_size);
            }

            std::vector<std::uint8_t> receive(size_t len) {
                if(!sk_ || !sk_->ok() || !ssl_conn_ptr_) {
                    throw ssl_error("ssl connection: uninitialized");
                }
                return receive_unlocked(len);
            }

            std::vector<std::uint8_t> receive_nb(size_t len) {
                if(!sk_ || !sk_->ok() || !ssl_conn_ptr_) {
                    throw ssl_error("ssl connection: uninitialized");
                }
                std::vector<std::uint8_t> result{};
                if(len > 0) {
                    result.resize(len);
                    int rd_cnt{SSL_read(ssl_conn_ptr_.get(), &result[0], len)};
                    if(rd_cnt >= 0) {
                        if(rd_cnt < static_cast<int>(len)) {
                            result.resize(rd_cnt);
                        }
                    } else {
                        int err = SSL_get_error(ssl_conn_ptr_.get(), rd_cnt);
                        if(!(err == SSL_ERROR_NONE || err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)) {
                            throw_last_error();
                        }
                    }
                }
                return result;
            }

#if 0
        std::vector<std::uint8_t> receive_message() {
            if(!sk_ || !sk_->ok() || !ssl_conn_ptr_) {
                throw ssl_error("ssl connection: uninitialized");
            }
            std::vector<std::uint8_t> size_buff{receive_unlocked(4)};
            std::uint32_t size_to_read{};
            std::memcpy(&size_to_read, size_buff.data(), 4);
            size_to_read = bit_util::hnswap<std::uint32_t>{size_to_read}.val;
            std::vector<std::uint8_t> res{receive_unlocked(size_to_read)};
            return res;
        }
#endif
            size_t send(const void *buff, size_t len) {
                if(!sk_ || !sk_->ok() || !ssl_conn_ptr_) { throw ssl_error("ssl connection: uninitialized"); }
                size_t total_size{len};
                size_t total_sent{0};
                if(ssl_conn_ptr_) {
                    const char *curr_buff = reinterpret_cast<const char *>(buff);
                    do {
                        int wrote_count{SSL_write(ssl_conn_ptr_.get(), curr_buff + total_sent, total_size - total_sent)};
                        if(wrote_count <= 0) {
                            throw_last_error_if_nonzero();
                        } else {
                            total_sent += wrote_count;
                        }
                    } while(total_sent != total_size);
                } else {
                    throw ssl_error("ssl connection: connection uninitialized");
                }
                return total_sent;
            }
            bool close() {
                if(ssl_conn_ptr_ && sk_ && sk_->ok()) {
                    int sres{1};
                    if(sk_->still_engaged()) {
                        sres = SSL_shutdown(ssl_conn_ptr_.get());
                        for(int k = 0; sres == 2 && k < 1000; ++k) {
                            sres = SSL_shutdown(ssl_conn_ptr_.get());
                        }
                    }
                    sres = sk_->close() ? 1 : 0;
                    return sres == 1;
                }
                return false;
            }

            bool shutdown() {
                if(ssl_conn_ptr_ && sk_ && sk_->ok()) {
                    if(sk_->still_engaged()) {
                        int sres{SSL_shutdown(ssl_conn_ptr_.get())};
                        for(int k = 0; sres == 2 && k < 1000; ++k) {
                            sres = SSL_shutdown(ssl_conn_ptr_.get());
                        }
                        return sres == 1;
                    }
                }
                return false;
            }

            bool ok() const { return ok_; }

#if 0
        std::int64_t interruptible_send(std::vector<std::uint8_t> &&data_for_out) {
            if(!ssl_conn_ptr_) { throw ssl_error("ssl connection: connection uninitialized"); }
            output_buffer_ = std::move(data_for_out);
            output_buffer_sent_ = 0;
            return interruptible_send_continue();
        }

        std::int64_t interruptible_send(const void *data, std::uint32_t data_size) {
            if(!ssl_conn_ptr_) { throw ssl_error("ssl connection: connection uninitialized"); }
            if(data && data_size > 0) {
                return interruptible_send(std::vector<std::uint8_t>{(std::uint8_t const *)data, (std::uint8_t const *)data + data_size});
            }
            return 0;
        }

        std::int64_t interruptible_send_continue() {
            if(!ssl_conn_ptr_) { throw ssl_error("ssl connection: connection uninitialized"); }
            while(output_buffer_sent_ < output_buffer_.size()) {
                std::uint32_t write_portion{(std::uint32_t)output_buffer_.size() - (std::uint32_t)output_buffer_sent_};
                int wrote_count{SSL_write(ssl_conn_ptr_.get(), &output_buffer_[output_buffer_sent_], write_portion)};
                if(wrote_count <= 0) {
                    int err_code{SSL_get_error(ssl_conn_ptr_.get(), wrote_count)};
                    if(err_code == SSL_ERROR_WANT_READ) {
                        break;
                    } else if(err_code != SSL_ERROR_WANT_WRITE) {
                        throw_last_error();
                    }
                } else {
                    output_buffer_sent_ += wrote_count;
                }
            }
            if(output_buffer_sent_ == output_buffer_.size()) {
                output_buffer_ = {};
            }
            return output_buffer_sent_;
        }

        std::int64_t interruptible_send_message(std::vector<std::uint8_t> const &data_for_out) {
            if(!ssl_conn_ptr_) { throw ssl_error("ssl connection: connection uninitialized"); }
            if(data_for_out.size() == 0) { return 0; }
            size_buffer_ = teal::bit_util::hnswap<std::uint32_t>{data_for_out.size()}.val;
            size_buffer_sent_ = 0;
            output_buffer_ = {};
            output_buffer_sent_ = 0;
            output_buffer_.insert(output_buffer_.end(), data_for_out.begin(), data_for_out.end());
            return interruptible_send_message_continue();
        }

        std::int64_t interruptible_send_message(const void *data, std::uint32_t data_size) {
            return interruptible_send_message(std::vector<std::uint8_t>{(std::uint8_t const *)data, (std::uint8_t const *)data + data_size});
        }

        std::int64_t interruptible_send_message_continue() {
            if(!ssl_conn_ptr_) { throw ssl_error("ssl connection: connection uninitialized"); }

            while(size_buffer_sent_ < sizeof(size_buffer_)) {
                std::uint32_t write_portion{(std::uint32_t)sizeof(size_buffer_) - (std::uint32_t)size_buffer_sent_};
                int wrote_count{SSL_write(ssl_conn_ptr_.get(), ((std::uint8_t const *)&size_buffer_) + size_buffer_sent_, write_portion)};
                if(wrote_count <= 0) {
                    int err_code{SSL_get_error(ssl_conn_ptr_.get(), wrote_count)};
                    if(err_code == SSL_ERROR_WANT_READ) {
                        return 0;
                    } else if(err_code != SSL_ERROR_WANT_WRITE) {
                        throw_last_error();
                    }
                } else {
                    size_buffer_sent_ += wrote_count;
                }
            }

            while(output_buffer_sent_ < output_buffer_.size()) {
                std::uint32_t write_portion{(std::uint32_t)output_buffer_.size() - (std::uint32_t)output_buffer_sent_};
                int wrote_count{SSL_write(ssl_conn_ptr_.get(), &output_buffer_[output_buffer_sent_], write_portion)};
                if(wrote_count <= 0) {
                    int err_code{SSL_get_error(ssl_conn_ptr_.get(), wrote_count)};
                    if(err_code == SSL_ERROR_WANT_READ) {
                        break;
                    } else if(err_code != SSL_ERROR_WANT_WRITE) {
                        throw_last_error();
                    }
                } else {
                    output_buffer_sent_ += wrote_count;
                }
            }
            if(output_buffer_sent_ == output_buffer_.size()) {
                output_buffer_ = {};
            }

            return output_buffer_sent_;
        }
#endif
        private:
            int send_data_unlocked(const void *data, std::uint32_t data_size) {
                if(!sk_ || !sk_->ok() || !ssl_conn_ptr_) {
                    throw ssl_error("ssl connection: connection uninitialized");
                }
                if(!data || data_size == 0) { return 0; }
                std::uint32_t total_size = data_size;
                std::uint32_t total_sent = 0;
                const char *curr_buff = reinterpret_cast<const char *>(data);
                do {
                    std::uint32_t write_portion{total_size - total_sent};
                    int wrote_count{SSL_write(ssl_conn_ptr_.get(), curr_buff + total_sent, write_portion)};
                    if(wrote_count <= 0) {
                        int err_code{SSL_get_error(ssl_conn_ptr_.get(), wrote_count)};
                        if(err_code != SSL_ERROR_WANT_WRITE) {
                            throw_last_error();
                        }
                    } else {
                        total_sent += wrote_count;
                    }
                } while(total_sent < total_size);
                return total_sent;
            }

            std::vector<std::uint8_t> receive_unlocked(size_t len = 1024) {
                std::vector<std::uint8_t> result{};
                if(len > 0) {
                    result.resize(len);
                    int rd_cnt{SSL_read(ssl_conn_ptr_.get(), &result[0], len)};
                    if(rd_cnt >= 0) {
                        if(rd_cnt < static_cast<int>(len)) {
                            result.resize(rd_cnt);
                        }
                    } else {
                        throw_last_error();
                    }
                }
                return result;
            }

            void accept_unlocked() {
                if(!sk_ || !sk_->ok() || !ssl_conn_ptr_) {
                    throw ssl_error("ssl connection: object not initialized");
                }
                if(SSL_accept(ssl_conn_ptr_.get()) != 1) {
                    throw_last_error();
                }
            }

            void connect_unlocked(std::string const &ca_cert) {
                if(!sk_ || !sk_->ok() || !ssl_conn_ptr_) {
                    throw ssl_error("ssl connection: ssl not initialized for connecting");
                }

                if(SSL_connect(ssl_conn_ptr_.get()) <= 0) {
                    throw_last_error();
                }

                if(!ca_cert.empty()) {
                    teal::ssl::unique_ptr<X509> server_cert{SSL_get_peer_certificate(ssl_conn_ptr_.get())};
                    if(!verify_peer_cert(ca_cert, server_cert.get())) {
                        throw ssl_error("ssl connection: untrusted server certificate!");
                    }
                }
            }

            bool verify_peer_cert(std::string const &ca_cert_data, X509 *cert_to_check) {
              teal::ssl::unique_ptr<X509_STORE> store{teal::ssl::NEW<X509_STORE>([](){ return X509_STORE_new(); })};
              if (!store) {
                  return false;
              }
              teal::ssl::unique_ptr<X509_STORE_CTX> vrfy_ctx{teal::ssl::NEW<X509_STORE_CTX>([](){ return X509_STORE_CTX_new(); })};
              teal::ssl::unique_ptr<BIO> cbio{teal::ssl::NEW<BIO>([&]() { return BIO_new_mem_buf(ca_cert_data.data(), ca_cert_data.size()); })};
              teal::ssl::unique_ptr<X509> ca_crt{teal::ssl::NEW<X509>([&]() { return PEM_read_bio_X509(cbio.get(), NULL, 0, NULL); })};
              if (X509_STORE_add_cert(store.get(), ca_crt.get()) != 1) {
                  return false;
              }
              X509_STORE_CTX_init(vrfy_ctx.get(), store.get(), cert_to_check, NULL);
              return X509_verify_cert(vrfy_ctx.get()) == 1;
            }

            std::shared_ptr<net::socket> sk_{nullptr};
            teal::ssl::unique_ptr<SSL> ssl_conn_ptr_{nullptr};
            bool ok_{false};

#if 0
            std::vector<std::uint8_t> output_buffer_{};
            std::size_t output_buffer_sent_{0};
            std::uint32_t size_buffer_{0};
            std::size_t size_buffer_sent_{0};
#endif
        };

        enum context_kind {
            SERVER_CTX,
            CLIENT_CTX
        };

        class context {
        public:
            enum ssl_method {
                TLS,
                DTLS
            };

            context(bool is_server = false):
                server_{is_server}
            {
            }

            ~context() {
                free();
            }

            void init(ssl_method m = TLS) {
                static char const *cipher_list{
                    "TLS_AES_256_GCM_SHA384"
                    ":TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256"
                    ":TLS_CHACHA20_POLY1305_SHA256"
                    ":TLS_AES_128_GCM_SHA256"
                    ":TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384"
                    ":TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384"
                    ":TLS_DHE_RSA_WITH_AES_256_GCM_SHA384"
                    ":TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256"
                    ":TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256"
                    ":TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256"
                    ":TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256"
                    ":TLS_DHE_RSA_WITH_AES_128_GCM_SHA256"
                    ":TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384"
                    ":TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384"
                    ":TLS_DHE_RSA_WITH_AES_256_CBC_SHA256"
                    ":TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256"
                    ":TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256"
                    ":TLS_DHE_RSA_WITH_AES_128_CBC_SHA256"
                    ":TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA"
                    ":TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA"
                    ":TLS_DHE_RSA_WITH_AES_128_CBC_SHA"
                    ":TLS_RSA_WITH_AES_256_GCM_SHA384"
                    ":TLS_RSA_WITH_AES_128_GCM_SHA256"
                    ":TLS_RSA_WITH_AES_256_CBC_SHA256"
                    ":TLS_RSA_WITH_AES_128_CBC_SHA256"
                    ":TLS_RSA_WITH_AES_256_CBC_SHA"
                    ":TLS_RSA_WITH_AES_128_CBC_SHA"
                    ":TLS_EMPTY_RENEGOTIATION_INFO_SCSV"
                    ":TLS_CHACHA_POLY1305_SHA256"
                    ":DHE-RSA-AES256-GCM-SHA384"
                    ":DHE-RSA-AES256-CCM8"
                    ":DHE-PSK-CHACHA20-POLY1305"
                    ":DHE-DSS-AES256-GCM-SHA384"
                    ":DHE-PSK-AES256-CCM8"
                    ":DHE-PSK-AES256-CCM"
                    ":ECDHE-RSA-AES256-GCM-SHA384"
                    ":ECDHE-RSA-CHACHA20-POLY1305"
                    ":ECDHE-ECDSA-CHACHA20-POLY1305"
                    ":ECDHE-PSK-CHACHA20-POLY1305"
                    ":ECDHE-ECDSA-AES256-GCM-SHA384"
                    ":ECDHE-ECDSA-AES256-CCM8"
                    ":ECDHE-ECDSA-AES256-CCM"
                };

                if(ctx_) {
                    return;
                }

                SSL_METHOD const *mthd{nullptr};
                if(server_) {
                    if(m == TLS) {
                        mthd = TLS_server_method();
                    } else if(m == DTLS) {
                        mthd = DTLS_server_method();
                    } else {
                        throw ssl_error("ssl context: invalid transport security method passed");
                    }

                    if(!mthd) {
                        throw ssl_error("ssl context: error setting server transport security method");
                    }

                    ctx_ = teal::ssl::NEW<SSL_CTX>([&]() { return SSL_CTX_new(mthd); });

                    if(!ctx_) {
                        throw_last_error();
                    }

                    SSL_CTX_set_min_proto_version(ctx_.get(), TLS1_3_VERSION);
                    SSL_CTX_set_max_proto_version(ctx_.get(), TLS_MAX_VERSION);

                    SSL_CTX_set_options(ctx_.get(), SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
#if 1
                    const char PREFERRED_CIPHERS[] = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
                    if(!(1 == SSL_CTX_set_cipher_list(ctx_.get(), PREFERRED_CIPHERS))) {
                        throw_last_error();
                    }
#else
                    if(!SSL_CTX_set_cipher_list(ctx_.get(), cipher_list)) {
                        throw_last_error();
                    }
#endif
                } else {
                    if(m == TLS) {
                        mthd = TLS_client_method();
                    } else if(m == DTLS) {
                        mthd = DTLS_client_method();
                    } else {
                        throw ssl_error("ssl context: invalid transport security method passed");
                    }

                    if(!mthd) {
                        throw ssl_error("ssl context: error setting client transport security method");
                    }

                    ctx_ = teal::ssl::NEW<SSL_CTX>([&]() { return SSL_CTX_new(mthd); });

                    if(!ctx_) {
                        throw_last_error();
                    }

                    SSL_CTX_set_verify(ctx_.get(), SSL_VERIFY_NONE, nullptr);

                    SSL_CTX_set_options(ctx_.get(), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);

                    SSL_CTX_set_min_proto_version(ctx_.get(), TLS1_3_VERSION);
                    SSL_CTX_set_max_proto_version(ctx_.get(), TLS_MAX_VERSION);
#if 0
                    if(!SSL_CTX_set_cipher_list(ctx_.get(), cipher_list)) {
                        throw_last_error();
                    }
#else
                    const char PREFERRED_CIPHERS[] = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
                    if(!(1 == SSL_CTX_set_cipher_list(ctx_.get(), PREFERRED_CIPHERS))) {
                        throw_last_error();
                    }
#endif
                    SSL_CTX_set_post_handshake_auth(ctx_.get(), 1);
                }
            }

            void free() {
                if(ctx_) {
                    ctx_.reset();
                }
            }
            bool initialized() const {
                return ctx_.get() != nullptr;
            }
            void add_cert(const std::vector<std::uint8_t> &cr) {
                if(!ctx_) {
                    throw ssl_error("ssl context: null");
                }

                teal::ssl::unique_ptr<BIO> cbio{teal::ssl::NEW<BIO>([&]() { return BIO_new_mem_buf(cr.data(), -1); })};
                teal::ssl::unique_ptr<X509> cert{teal::ssl::NEW<X509>([&]() { return PEM_read_bio_X509(cbio.get(), nullptr, 0, nullptr); })};
                if(!cert) {
                    throw_last_error();
                }
                if(SSL_CTX_add_extra_chain_cert(ctx_.get(), cert.get()) != 1) {
                    throw_last_error();
                }
            }

            void set_server_keys(const std::vector<std::uint8_t> &cr, const std::vector<std::uint8_t> &pk) {
                if(!ctx_) {
                    throw ssl_error("ssl context: null");
                }

                if(cr.empty() || pk.empty()) {
                    throw ssl_error("ssl context: empty key or certificate");
                }

                teal::ssl::unique_ptr<BIO> cbio{teal::ssl::NEW<BIO>([&]() { return BIO_new_mem_buf(cr.data(), -1); })};
                teal::ssl::unique_ptr<X509> cert{teal::ssl::NEW<X509>([&]() { return PEM_read_bio_X509(cbio.get(), nullptr, 0, nullptr); })};
                if(!cert) {
                    throw_last_error();
                }

                if(SSL_CTX_use_certificate(ctx_.get(), cert.get()) != 1) {
                    throw_last_error();
                }

                teal::ssl::unique_ptr<BIO> kbio{teal::ssl::NEW<BIO>([&]() { return BIO_new_mem_buf(pk.data(), -1); })};
                teal::ssl::unique_ptr<RSA> rsa{teal::ssl::NEW<RSA>([&]() { return PEM_read_bio_RSAPrivateKey(kbio.get(), nullptr, 0, nullptr); })};
                if(!rsa) {
                    throw_last_error();
                }
                if(SSL_CTX_use_RSAPrivateKey(ctx_.get(), rsa.get()) != 1) {
                    throw_last_error();
                }

                SSL_CTX_set_verify(ctx_.get(), SSL_VERIFY_NONE, nullptr);
            }

            void set_server_keys(const std::string &cert_file_name, const std::string &pk_file_name) {
                if(!ctx_) {
                    throw ssl_error("ssl context: null");
                }

                if(
                    SSL_CTX_use_certificate_file(ctx_.get(), cert_file_name.c_str(), SSL_FILETYPE_PEM) <= 0
                    ||
                    SSL_CTX_use_PrivateKey_file(ctx_.get(), pk_file_name.c_str(), SSL_FILETYPE_PEM) <= 0
                    // ||
                    // !SSL_CTX_check_private_key(ctx_.get())
                    ) {
                    throw_last_error();
                }
            }

            std::shared_ptr<connection> get_new_connection(std::shared_ptr<net::socket> sk, std::string const &ca_cert) {
                if(!ctx_) {
                    throw ssl_error("ssl context: null");
                }
                std::shared_ptr<connection> res{std::make_shared<connection>()};

                teal::ssl::unique_ptr<SSL> ssl_cnn{teal::ssl::NEW<SSL>([&]() { return SSL_new(ctx_.get()); })};

                if(ssl_cnn) {
                    res->init(std::move(ssl_cnn), sk, ca_cert, server_ ? connection::ACCEPT : connection::CONNECT);
                } else {
                    throw ssl_error{"ssl context: failed to create new ssl connection"};
                }
                return res;
            }

        private:
            teal::ssl::unique_ptr<SSL_CTX> ctx_{};
            bool server_{false};
        };

    }

}
