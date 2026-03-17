#pragma once

#include "../include/commondefs.hpp"
#include "../include/str_util.hpp"
#include "../include/net/socket_wrapper.hpp"
#include "../include/net/socket_poller.hpp"

#include "../scaflux_value.hpp"
#include "../scaflux_util.hpp"
#include "../scaflux_interfaces.hpp"

namespace scfx {

    class socket_ext: public extension_interface {
    public:
        socket_ext() = default;
        ~socket_ext() {
            unregister_runtime();
        }
        socket_ext(socket_ext const &) = delete;
        socket_ext &operator=(socket_ext const &) = delete;
        socket_ext(socket_ext &&) = delete;
        socket_ext &operator=(socket_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }

            rt->add_var("address_family_inet4", static_cast<int>(scfx::net::address_family::inet4));
            rt->add_var("address_family_inet6", static_cast<int>(scfx::net::address_family::inet6));
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            rt->add_var("address_family_unix", static_cast<int>(scfx::net::address_family::unix_socket));
#endif
            rt->add_var("sock_stream", static_cast<int>(scfx::net::sock_type::stream));
            rt->add_var("sock_dgram", static_cast<int>(scfx::net::sock_type::dgram));
            rt->add_var("sock_raw", static_cast<int>(scfx::net::sock_type::raw));
            rt->add_var("sock_rdm", static_cast<int>(scfx::net::sock_type::rdm));
            rt->add_var("sock_seqpacket", static_cast<int>(scfx::net::sock_type::seqpacket));
            rt->add_var("sock_dccp", static_cast<int>(scfx::net::sock_type::dccp));
            rt->add_var("sock_packet", static_cast<int>(scfx::net::sock_type::packet));
            rt->add_var("sock_cloexec", static_cast<int>(scfx::net::sock_type::cloexec));
            rt->add_var("sock_nonblock", static_cast<int>(scfx::net::sock_type::nonblock));

            rt->add_function("socket", SCFXFUN() {
                return valbox{std::make_shared<scfx::net::socket>(), "socket"};
            });
            rt->add_method("socket", "create", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 4)
                if(args.size() == 1) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->create();
                } else if(args.size() == 2) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->create(
                        static_cast<scfx::net::address_family>(args[1].cast_to_int())
                    );
                } else if(args.size() == 3) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->create(
                        static_cast<scfx::net::address_family>(args[1].cast_to_int()),
                        static_cast<scfx::net::sock_type>(args[2].cast_to_int())
                    );
                } else if(args.size() == 4) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->create(
                        static_cast<scfx::net::address_family>(args[1].cast_to_int()),
                        static_cast<scfx::net::sock_type>(args[2].cast_to_int()),
                        args[3].cast_to_int()
                    );
                }
                return false;
            });
            rt->add_method("socket", "bind", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                if(args.size() == 3) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->bind(args[1].cast_to_string(), args[2].cast_to_u16());
                }
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->bind(args[1].cast_to_string(), 0);
            });
            rt->add_method("socket", "listen", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2)
                if(args.size() == 1) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->listen();
                }
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->listen(args[1].cast_to_s32());
            });
            rt->add_method("socket", "accept", SCFXFUN(args) {
                std::shared_ptr<scfx::net::socket> client_sock{std::make_shared<scfx::net::socket>()};
                SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->accept(*client_sock);
                return valbox{client_sock, "socket"};
            });
            rt->add_method("socket", "connect", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                if(args.size() == 3) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->connect(args[1].cast_to_string(), args[2].cast_to_u16());
                }
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->connect(args[1].cast_to_string(), 0);
            });
            rt->add_method("socket", "receive", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                auto vec{SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->receive(args[1].cast_to_s32())};
                return std::string{vec.begin(), vec.end()};
            });
            rt->add_method("socket", "send", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_GE(args, 2)
                auto data{args[1].cast_to_byte_array()};
                if(args.size() == 2) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->send(data.data(), data.size());
                } else if(args.size() >= 3) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->send(data.data(), data.size(), args[2].cast_to_s32());
                }
                return 0;
            });
            rt->add_method("socket", "write", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                auto data{args[1].cast_to_byte_array()};
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->write(data.data(), data.size());
            });
            rt->add_method("socket", "close", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->close();
            });
            rt->add_method("socket", "shutdown", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->shutdown();
            });
            rt->add_method("socket", "handle", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->handle();
            });
            rt->add_method("socket", "make_nonblocking", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->make_nonblocking();
            });
            rt->add_method("socket", "set_reuse_addr", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->set_reuse_addr();
            });
            rt->add_method("socket", "make_nodelay", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->make_nodelay();
            });
            rt->add_method("socket", "cork", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->cork(args[1].cast_to_bool());
            });
            rt->add_method("socket", "make_nosigpipe", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->make_nosigpipe();
            });
            rt->add_method("socket", "set_linger", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->set_linger(args[1].cast_to_bool(), args[1].cast_to_s32());
            });
            rt->add_method("socket", "set_keepalive", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->set_keepalive(args[1].cast_to_bool());
            });
            rt->add_method("socket", "get_keepalive", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->get_keepalive();
            });
            rt->add_method("socket", "set_tcp_keepidle", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->set_tcp_keepidle(args[1].cast_to_s32());
            });
            rt->add_method("socket", "get_tcp_keepidle", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->get_tcp_keepidle();
            });
            rt->add_method("socket", "set_tcp_keepitvl", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->set_tcp_keepitvl(args[1].cast_to_s32());
            });
            rt->add_method("socket", "get_tcp_keepitvl", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->get_tcp_keepitvl();
            });
            rt->add_method("socket", "set_tcp_keepcnt", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->set_tcp_keepcnt(args[1].cast_to_s32());
            });
            rt->add_method("socket", "get_tcp_keepcnt", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->get_tcp_keepcnt();
            });
            rt->add_method("socket", "set_rcv_timeout", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                scfx::timespec_wrapper orig_to;
                if(SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->set_rcv_timeout(scfx::timespec_wrapper{args[1].cast_to_long_double()}, orig_to)) {
                    return orig_to.fseconds();
                }
                return valbox{};
            });
            rt->add_method("socket", "peer_addr", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->peer_addr();
            });
            rt->add_method("socket", "ok", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->ok();
            });
            rt->add_method("socket", "error_status", SCFXFUN(args) {
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->error_status();
            });
            rt->add_method("socket", "send_message", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                auto data{args[1].cast_to_byte_array()};
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->send_message(data);
            });
            rt->add_method("socket", "receive_message", SCFXFUN(args) {
                auto vec{SCFXTHIS(args, std::shared_ptr<scfx::net::socket>)->receive_message()};
                return std::string{vec.begin(), vec.end()};
            });

#ifdef PLATFORM_APPLE
            rt->add_var("POLL_EVENT_IN", scfx::net::POLL_EVENT_IN);
            rt->add_var("POLL_EVENT_PRI", scfx::net::POLL_EVENT_PRI);
            rt->add_var("POLL_EVENT_OUT", scfx::net::POLL_EVENT_OUT);
            rt->add_var("POLL_EVENT_RDNORM", scfx::net::POLL_EVENT_RDNORM);
            rt->add_var("POLL_EVENT_RDBAND", scfx::net::POLL_EVENT_RDBAND);
            rt->add_var("POLL_EVENT_WRNORM", scfx::net::POLL_EVENT_WRNORM);
            rt->add_var("POLL_EVENT_WRBAND", scfx::net::POLL_EVENT_WRBAND);
#ifdef __USE_GNU
            rt->add_var("POLL_EVENT_MSG", scfx::net::POLL_EVENT_MSG);
            rt->add_var("POLL_EVENT_RDHUP", scfx::net::POLL_EVENT_RDHUP);
#endif
            rt->add_var("POLL_EVENT_ERR", scfx::net::POLL_EVENT_ERR);
            rt->add_var("POLL_EVENT_HUP", scfx::net::POLL_EVENT_HUP);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            rt->add_var("POLL_EVENT_IN", scfx::net::POLL_EVENT_IN);
            rt->add_var("POLL_EVENT_PRI", scfx::net::POLL_EVENT_PRI);
            rt->add_var("POLL_EVENT_OUT", scfx::net::POLL_EVENT_OUT);
            rt->add_var("POLL_EVENT_RDNORM", scfx::net::POLL_EVENT_RDNORM);
            rt->add_var("POLL_EVENT_RDBAND", scfx::net::POLL_EVENT_RDBAND);
            rt->add_var("POLL_EVENT_WRNORM", scfx::net::POLL_EVENT_WRNORM);
            rt->add_var("POLL_EVENT_WRBAND", scfx::net::POLL_EVENT_WRBAND);
            rt->add_var("POLL_EVENT_MSG", scfx::net::POLL_EVENT_MSG);
            rt->add_var("POLL_EVENT_ERR", scfx::net::POLL_EVENT_ERR);
            rt->add_var("POLL_EVENT_HUP", scfx::net::POLL_EVENT_HUP);
            rt->add_var("POLL_EVENT_RDHUP", scfx::net::POLL_EVENT_RDHUP);
            rt->add_var("POLL_EVENT_EXCLUSIVE", scfx::net::POLL_EVENT_EXCLUSIVE);
            rt->add_var("POLL_EVENT_WAKEUP", scfx::net::POLL_EVENT_WAKEUP);
            rt->add_var("POLL_EVENT_ONESHOT", scfx::net::POLL_EVENT_ONESHOT);
            rt->add_var("POLL_EVENT_ET", scfx::net::POLL_EVENT_ET);
#elif defined(PLATFORM_WINDOWS)
            rt->add_var("POLL_EVENT_ET", scfx::net::POLL_EVENT_ET);
            rt->add_var("POLL_EVENT_IN", scfx::net::POLL_EVENT_IN);
            rt->add_var("POLL_EVENT_PRI", scfx::net::POLL_EVENT_PRI);
            rt->add_var("POLL_EVENT_OUT", scfx::net::POLL_EVENT_OUT);
            rt->add_var("POLL_EVENT_RDNORM", scfx::net::POLL_EVENT_RDNORM);
            rt->add_var("POLL_EVENT_RDBAND", scfx::net::POLL_EVENT_RDBAND);
            rt->add_var("POLL_EVENT_WRNORM", scfx::net::POLL_EVENT_WRNORM);
            rt->add_var("POLL_EVENT_WRBAND", scfx::net::POLL_EVENT_WRBAND);
            rt->add_var("POLL_EVENT_ERR", scfx::net::POLL_EVENT_ERR);
            rt->add_var("POLL_EVENT_HUP", scfx::net::POLL_EVENT_HUP);
#endif

            rt->add_function("socket_poller", SCFXFUN() {
                return valbox{std::make_shared<scfx::net::socket_poller>(), "socket_poller"};
            });
            rt->add_method("socket_poller", "add_event", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                if(args.size() == 2) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket_poller>)->add_event(
                        SCFXCLASSARG(args, 1, std::shared_ptr<scfx::net::socket>)->handle()
                    );
                }
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket_poller>)->add_event(
                    SCFXCLASSARG(args, 1, std::shared_ptr<scfx::net::socket>)->handle(),
                    args[2].cast_to_s32()
                );
            });
            rt->add_method("socket_poller", "del_event", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                if(args[1].is_numeric()) {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket_poller>)->del_event(
                        args[1].cast_to_s32()
                    );
                } else {
                    return SCFXTHIS(args, std::shared_ptr<scfx::net::socket_poller>)->del_event(
                        SCFXCLASSARG(args, 1, std::shared_ptr<scfx::net::socket>)->handle()
                    );
                }
            });
            rt->add_method("socket_poller", "re_enable", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                return SCFXTHIS(args, std::shared_ptr<scfx::net::socket_poller>)->re_enable(
                    SCFXCLASSARG(args, 1, std::shared_ptr<scfx::net::socket>)->handle(),
                    args[1].cast_to_u32()
                );
            });
            rt->add_method("socket_poller", "close", SCFXFUN(args) {
                SCFXTHIS(args, std::shared_ptr<scfx::net::socket_poller>)->close();
                return 0;
            });
            rt->add_method("socket_poller", "wait", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                std::vector<net::poll_event> evts{};
                if(args.size() == 2) {
                    evts = SCFXTHIS(args, std::shared_ptr<scfx::net::socket_poller>)->wait(
                        args[1].cast_to_size_t()
                    );
                } else {
                    evts = SCFXTHIS(args, std::shared_ptr<scfx::net::socket_poller>)->wait(
                        args[1].cast_to_size_t(), scfx::timespec_wrapper{args[2].cast_to_long_double()}
                    );
                }
                valbox res{};
                res.become_array();
                for(auto &&e: evts) {
                    json j{};
                    j["data"]["ptr"] = reinterpret_cast<uintptr_t>(e.data.ptr);
                    j["data"]["fd"] = e.data.fd;
                    j["data"]["u32"] = e.data.u32;
                    j["data"]["u64"] = e.data.u64;
                    j["events"] = e.events;
                    res.as_array().push_back(j);
                }
                return res;
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("socket");
            rt_->remove_method("socket", "create");
            rt_->remove_method("socket", "bind");
            rt_->remove_method("socket", "listen");
            rt_->remove_method("socket", "accept");
            rt_->remove_method("socket", "connect");
            rt_->remove_method("socket", "receive");
            rt_->remove_method("socket", "send");
            rt_->remove_method("socket", "write");
            rt_->remove_method("socket", "close");
            rt_->remove_method("socket", "shutdown");
            rt_->remove_method("socket", "handle");
            rt_->remove_method("socket", "make_nonblocking");
            rt_->remove_method("socket", "set_reuse_addr");
            rt_->remove_method("socket", "make_nodelay");
            rt_->remove_method("socket", "cork");
            rt_->remove_method("socket", "make_nosigpipe");
            rt_->remove_method("socket", "set_linger");
            rt_->remove_method("socket", "set_keepalive");
            rt_->remove_method("socket", "get_keepalive");
            rt_->remove_method("socket", "set_tcp_keepidle");
            rt_->remove_method("socket", "get_tcp_keepidle");
            rt_->remove_method("socket", "set_tcp_keepitvl");
            rt_->remove_method("socket", "get_tcp_keepitvl");
            rt_->remove_method("socket", "set_tcp_keepcnt");
            rt_->remove_method("socket", "get_tcp_keepcnt");
            rt_->remove_method("socket", "set_rcv_timeout");
            rt_->remove_method("socket", "peer_addr");
            rt_->remove_method("socket", "ok");
            rt_->remove_method("socket", "error_status");
            rt_->remove_method("socket", "send_message");
            rt_->remove_method("socket", "receive_message");

#ifdef PLATFORM_APPLE
            rt_->remove_var("POLL_EVENT_IN");
            rt_->remove_var("POLL_EVENT_PRI");
            rt_->remove_var("POLL_EVENT_OUT");
            rt_->remove_var("POLL_EVENT_RDNORM");
            rt_->remove_var("POLL_EVENT_RDBAND");
            rt_->remove_var("POLL_EVENT_WRNORM");
            rt_->remove_var("POLL_EVENT_WRBAND");
#ifdef __USE_GNU
            rt_->remove_var("POLL_EVENT_MSG");
            rt_->remove_var("POLL_EVENT_RDHUP");
#endif
            rt_->remove_var("POLL_EVENT_ERR");
            rt_->remove_var("POLL_EVENT_HUP");
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            rt_->remove_var("POLL_EVENT_IN");
            rt_->remove_var("POLL_EVENT_PRI");
            rt_->remove_var("POLL_EVENT_OUT");
            rt_->remove_var("POLL_EVENT_RDNORM");
            rt_->remove_var("POLL_EVENT_RDBAND");
            rt_->remove_var("POLL_EVENT_WRNORM");
            rt_->remove_var("POLL_EVENT_WRBAND");
            rt_->remove_var("POLL_EVENT_MSG");
            rt_->remove_var("POLL_EVENT_ERR");
            rt_->remove_var("POLL_EVENT_HUP");
            rt_->remove_var("POLL_EVENT_RDHUP");
            rt_->remove_var("POLL_EVENT_EXCLUSIVE");
            rt_->remove_var("POLL_EVENT_WAKEUP");
            rt_->remove_var("POLL_EVENT_ONESHOT");
            rt_->remove_var("POLL_EVENT_ET");
#elif defined(PLATFORM_WINDOWS)
            rt_->remove_var("POLL_EVENT_ET");
            rt_->remove_var("POLL_EVENT_IN");
            rt_->remove_var("POLL_EVENT_PRI");
            rt_->remove_var("POLL_EVENT_OUT");
            rt_->remove_var("POLL_EVENT_RDNORM");
            rt_->remove_var("POLL_EVENT_RDBAND");
            rt_->remove_var("POLL_EVENT_WRNORM");
            rt_->remove_var("POLL_EVENT_WRBAND");
            rt_->remove_var("POLL_EVENT_ERR");
            rt_->remove_var("POLL_EVENT_HUP");
#endif
            rt_->remove_function("socket_poller");
            rt_->remove_method("socket_poller", "add_event");
            rt_->remove_method("socket_poller", "del_event");
            rt_->remove_method("socket_poller", "re_enable");
            rt_->remove_method("socket_poller", "close");
            rt_->remove_method("socket_poller", "wait");

            rt_ = nullptr;
        }

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
