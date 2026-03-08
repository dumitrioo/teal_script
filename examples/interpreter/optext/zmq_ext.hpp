#pragma once

#include <scaflux_value.hpp>
#include <scaflux_util.hpp>
#include <scaflux_interfaces.hpp>

#include <zmq.h>

class zmq_ext: public scfx::extension_interface {
public:
    zmq_ext() = default;
    zmq_ext(zmq_ext const &) = delete;
    zmq_ext(zmq_ext &&) = delete;
    zmq_ext&operator=(zmq_ext const &) = delete;
    zmq_ext&operator=(zmq_ext &&) = delete;
    ~zmq_ext() = default;

    void register_runtime(scfx::runtime_interface *rt) override {
        std::unique_lock l{rt_mtp_};
        if(rt_ != nullptr) {
            return;
        }
        rt_ = rt;
        if(rt_ == nullptr) {
            return;
        }
        /******************************************************************************/
        /*  0MQ errors.                                                               */
        /******************************************************************************/
        rt->add_var("ZMQ_ENOTSUP", ENOTSUP);
        rt->add_var("ZMQ_EPROTONOSUPPORT", EPROTONOSUPPORT);
        rt->add_var("ZMQ_ENOBUFS", ENOBUFS);
        rt->add_var("ZMQ_ENETDOWN", ENETDOWN);
        rt->add_var("ZMQ_EADDRINUSE", EADDRINUSE);
        rt->add_var("ZMQ_EADDRNOTAVAIL", EADDRNOTAVAIL);
        rt->add_var("ZMQ_ECONNREFUSED", ECONNREFUSED);
        rt->add_var("ZMQ_EINPROGRESS", EINPROGRESS);
        rt->add_var("ZMQ_ENOTSOCK", ENOTSOCK);
        rt->add_var("ZMQ_EMSGSIZE", EMSGSIZE);
        rt->add_var("ZMQ_EAFNOSUPPORT", EAFNOSUPPORT);
        rt->add_var("ZMQ_ENETUNREACH", ENETUNREACH);
        rt->add_var("ZMQ_ECONNABORTED", ECONNABORTED);
        rt->add_var("ZMQ_ECONNRESET", ECONNRESET);
        rt->add_var("ZMQ_ENOTCONN", ENOTCONN);
        rt->add_var("ZMQ_ETIMEDOUT", ETIMEDOUT);
        rt->add_var("ZMQ_EHOSTUNREACH", EHOSTUNREACH);
        rt->add_var("ZMQ_ENETRESET", ENETRESET);

        /*  Native 0MQ error codes.                                                   */
        rt->add_var("ZMQ_EFSM", EFSM);
        rt->add_var("ZMQ_ENOCOMPATPROTO", ENOCOMPATPROTO);
        rt->add_var("ZMQ_ETERM", ETERM);
        rt->add_var("ZMQ_EMTHREAD", EMTHREAD);

        /*  Context options                                                           */
        rt->add_var("ZMQ_IO_THREADS", ZMQ_IO_THREADS);
        rt->add_var("ZMQ_MAX_SOCKETS", ZMQ_MAX_SOCKETS);
        rt->add_var("ZMQ_SOCKET_LIMIT", ZMQ_SOCKET_LIMIT);
        rt->add_var("ZMQ_THREAD_PRIORITY", ZMQ_THREAD_PRIORITY);
        rt->add_var("ZMQ_THREAD_SCHED_POLICY", ZMQ_THREAD_SCHED_POLICY);
        rt->add_var("ZMQ_MAX_MSGSZ", ZMQ_MAX_MSGSZ);
        rt->add_var("ZMQ_MSG_T_SIZE", ZMQ_MSG_T_SIZE);
        rt->add_var("ZMQ_THREAD_AFFINITY_CPU_ADD", ZMQ_THREAD_AFFINITY_CPU_ADD);
        rt->add_var("ZMQ_THREAD_AFFINITY_CPU_REMOVE", ZMQ_THREAD_AFFINITY_CPU_REMOVE);
        rt->add_var("ZMQ_THREAD_NAME_PREFIX", ZMQ_THREAD_NAME_PREFIX);

        /*  Default for new contexts                                                  */
        rt->add_var("ZMQ_IO_THREADS_DFLT", ZMQ_IO_THREADS_DFLT);
        rt->add_var("ZMQ_MAX_SOCKETS_DFLT", ZMQ_MAX_SOCKETS_DFLT);
        rt->add_var("ZMQ_THREAD_PRIORITY_DFLT", ZMQ_THREAD_PRIORITY_DFLT);
        rt->add_var("ZMQ_THREAD_SCHED_POLICY_DFLT", ZMQ_THREAD_SCHED_POLICY_DFLT);

        /*  Socket types.                                                             */
        rt->add_var("ZMQ_PAIR", ZMQ_PAIR);
        rt->add_var("ZMQ_PUB", ZMQ_PUB);
        rt->add_var("ZMQ_SUB", ZMQ_SUB);
        rt->add_var("ZMQ_REQ", ZMQ_REQ);
        rt->add_var("ZMQ_REP", ZMQ_REP);
        rt->add_var("ZMQ_DEALER", ZMQ_DEALER);
        rt->add_var("ZMQ_ROUTER", ZMQ_ROUTER);
        rt->add_var("ZMQ_PULL", ZMQ_PULL);
        rt->add_var("ZMQ_PUSH", ZMQ_PUSH);
        rt->add_var("ZMQ_XPUB", ZMQ_XPUB);
        rt->add_var("ZMQ_XSUB", ZMQ_XSUB);
        rt->add_var("ZMQ_STREAM", ZMQ_STREAM);

        /*  Socket options.                                                           */
        rt->add_var("ZMQ_AFFINITY", ZMQ_AFFINITY);
        rt->add_var("ZMQ_ROUTING_ID", ZMQ_ROUTING_ID);
        rt->add_var("ZMQ_SUBSCRIBE", ZMQ_SUBSCRIBE);
        rt->add_var("ZMQ_UNSUBSCRIBE", ZMQ_UNSUBSCRIBE);
        rt->add_var("ZMQ_RATE", ZMQ_RATE);
        rt->add_var("ZMQ_RECOVERY_IVL", ZMQ_RECOVERY_IVL);
        rt->add_var("ZMQ_SNDBUF", ZMQ_SNDBUF);
        rt->add_var("ZMQ_RCVBUF", ZMQ_RCVBUF);
        rt->add_var("ZMQ_RCVMORE", ZMQ_RCVMORE);
        rt->add_var("ZMQ_FD", ZMQ_FD);
        rt->add_var("ZMQ_EVENTS", ZMQ_EVENTS);
        rt->add_var("ZMQ_TYPE", ZMQ_TYPE);
        rt->add_var("ZMQ_LINGER", ZMQ_LINGER);
        rt->add_var("ZMQ_RECONNECT_IVL", ZMQ_RECONNECT_IVL);
        rt->add_var("ZMQ_BACKLOG", ZMQ_BACKLOG);
        rt->add_var("ZMQ_RECONNECT_IVL_MAX", ZMQ_RECONNECT_IVL_MAX);
        rt->add_var("ZMQ_MAXMSGSIZE", ZMQ_MAXMSGSIZE);
        rt->add_var("ZMQ_SNDHWM", ZMQ_SNDHWM);
        rt->add_var("ZMQ_RCVHWM", ZMQ_RCVHWM);
        rt->add_var("ZMQ_MULTICAST_HOPS", ZMQ_MULTICAST_HOPS);
        rt->add_var("ZMQ_RCVTIMEO", ZMQ_RCVTIMEO);
        rt->add_var("ZMQ_SNDTIMEO", ZMQ_SNDTIMEO);
        rt->add_var("ZMQ_LAST_ENDPOINT", ZMQ_LAST_ENDPOINT);
        rt->add_var("ZMQ_ROUTER_MANDATORY", ZMQ_ROUTER_MANDATORY);
        rt->add_var("ZMQ_TCP_KEEPALIVE", ZMQ_TCP_KEEPALIVE);
        rt->add_var("ZMQ_TCP_KEEPALIVE_CNT", ZMQ_TCP_KEEPALIVE_CNT);
        rt->add_var("ZMQ_TCP_KEEPALIVE_IDLE", ZMQ_TCP_KEEPALIVE_IDLE);
        rt->add_var("ZMQ_TCP_KEEPALIVE_INTVL", ZMQ_TCP_KEEPALIVE_INTVL);
        rt->add_var("ZMQ_IMMEDIATE", ZMQ_IMMEDIATE);
        rt->add_var("ZMQ_XPUB_VERBOSE", ZMQ_XPUB_VERBOSE);
        rt->add_var("ZMQ_ROUTER_RAW", ZMQ_ROUTER_RAW);
        rt->add_var("ZMQ_IPV6", ZMQ_IPV6);
        rt->add_var("ZMQ_MECHANISM", ZMQ_MECHANISM);
        rt->add_var("ZMQ_PLAIN_SERVER", ZMQ_PLAIN_SERVER);
        rt->add_var("ZMQ_PLAIN_USERNAME", ZMQ_PLAIN_USERNAME);
        rt->add_var("ZMQ_PLAIN_PASSWORD", ZMQ_PLAIN_PASSWORD);
        rt->add_var("ZMQ_CURVE_SERVER", ZMQ_CURVE_SERVER);
        rt->add_var("ZMQ_CURVE_PUBLICKEY", ZMQ_CURVE_PUBLICKEY);
        rt->add_var("ZMQ_CURVE_SECRETKEY", ZMQ_CURVE_SECRETKEY);
        rt->add_var("ZMQ_CURVE_SERVERKEY", ZMQ_CURVE_SERVERKEY);
        rt->add_var("ZMQ_PROBE_ROUTER", ZMQ_PROBE_ROUTER);
        rt->add_var("ZMQ_REQ_CORRELATE", ZMQ_REQ_CORRELATE);
        rt->add_var("ZMQ_REQ_RELAXED", ZMQ_REQ_RELAXED);
        rt->add_var("ZMQ_CONFLATE", ZMQ_CONFLATE);
        rt->add_var("ZMQ_ZAP_DOMAIN", ZMQ_ZAP_DOMAIN);
        rt->add_var("ZMQ_ROUTER_HANDOVER", ZMQ_ROUTER_HANDOVER);
        rt->add_var("ZMQ_TOS", ZMQ_TOS);
        rt->add_var("ZMQ_CONNECT_ROUTING_ID", ZMQ_CONNECT_ROUTING_ID);
        rt->add_var("ZMQ_GSSAPI_SERVER", ZMQ_GSSAPI_SERVER);
        rt->add_var("ZMQ_GSSAPI_PRINCIPAL", ZMQ_GSSAPI_PRINCIPAL);
        rt->add_var("ZMQ_GSSAPI_SERVICE_PRINCIPAL", ZMQ_GSSAPI_SERVICE_PRINCIPAL);
        rt->add_var("ZMQ_GSSAPI_PLAINTEXT", ZMQ_GSSAPI_PLAINTEXT);
        rt->add_var("ZMQ_HANDSHAKE_IVL", ZMQ_HANDSHAKE_IVL);
        rt->add_var("ZMQ_SOCKS_PROXY", ZMQ_SOCKS_PROXY);
        rt->add_var("ZMQ_XPUB_NODROP", ZMQ_XPUB_NODROP);
        rt->add_var("ZMQ_BLOCKY", ZMQ_BLOCKY);
        rt->add_var("ZMQ_XPUB_MANUAL", ZMQ_XPUB_MANUAL);
        rt->add_var("ZMQ_XPUB_WELCOME_MSG", ZMQ_XPUB_WELCOME_MSG);
        rt->add_var("ZMQ_STREAM_NOTIFY", ZMQ_STREAM_NOTIFY);
        rt->add_var("ZMQ_INVERT_MATCHING", ZMQ_INVERT_MATCHING);
        rt->add_var("ZMQ_HEARTBEAT_IVL", ZMQ_HEARTBEAT_IVL);
        rt->add_var("ZMQ_HEARTBEAT_TTL", ZMQ_HEARTBEAT_TTL);
        rt->add_var("ZMQ_HEARTBEAT_TIMEOUT", ZMQ_HEARTBEAT_TIMEOUT);
        rt->add_var("ZMQ_XPUB_VERBOSER", ZMQ_XPUB_VERBOSER);
        rt->add_var("ZMQ_CONNECT_TIMEOUT", ZMQ_CONNECT_TIMEOUT);
        rt->add_var("ZMQ_TCP_MAXRT", ZMQ_TCP_MAXRT);
        rt->add_var("ZMQ_THREAD_SAFE", ZMQ_THREAD_SAFE);
        rt->add_var("ZMQ_MULTICAST_MAXTPDU", ZMQ_MULTICAST_MAXTPDU);
        rt->add_var("ZMQ_VMCI_BUFFER_SIZE", ZMQ_VMCI_BUFFER_SIZE);
        rt->add_var("ZMQ_VMCI_BUFFER_MIN_SIZE", ZMQ_VMCI_BUFFER_MIN_SIZE);
        rt->add_var("ZMQ_VMCI_BUFFER_MAX_SIZE", ZMQ_VMCI_BUFFER_MAX_SIZE);
        rt->add_var("ZMQ_VMCI_CONNECT_TIMEOUT", ZMQ_VMCI_CONNECT_TIMEOUT);
        rt->add_var("ZMQ_USE_FD", ZMQ_USE_FD);
        rt->add_var("ZMQ_GSSAPI_PRINCIPAL_NAMETYPE", ZMQ_GSSAPI_PRINCIPAL_NAMETYPE);
        rt->add_var("ZMQ_GSSAPI_SERVICE_PRINCIPAL_NAMETYPE", ZMQ_GSSAPI_SERVICE_PRINCIPAL_NAMETYPE);
        rt->add_var("ZMQ_BINDTODEVICE", ZMQ_BINDTODEVICE);

        /*  Message options                                                           */
        rt->add_var("ZMQ_MORE", ZMQ_MORE);
        rt->add_var("ZMQ_SHARED", ZMQ_SHARED);

        /*  Send/recv options.                                                        */
        rt->add_var("ZMQ_DONTWAIT", ZMQ_DONTWAIT);
        rt->add_var("ZMQ_SNDMORE", ZMQ_SNDMORE);

        /*  Security mechanisms                                                       */
        rt->add_var("ZMQ_NULL", ZMQ_NULL);
        rt->add_var("ZMQ_PLAIN", ZMQ_PLAIN);
        rt->add_var("ZMQ_CURVE", ZMQ_CURVE);
        rt->add_var("ZMQ_GSSAPI", ZMQ_GSSAPI);

        /*  RADIO-DISH protocol                                                       */
        rt->add_var("ZMQ_GROUP_MAX_LENGTH", ZMQ_GROUP_MAX_LENGTH);

        /*  Deprecated options and aliases                                            */
        rt->add_var("ZMQ_IDENTITY", ZMQ_IDENTITY);
        rt->add_var("ZMQ_CONNECT_RID", ZMQ_CONNECT_RID);
        rt->add_var("ZMQ_TCP_ACCEPT_FILTER", ZMQ_TCP_ACCEPT_FILTER);
        rt->add_var("ZMQ_IPC_FILTER_PID", ZMQ_IPC_FILTER_PID);
        rt->add_var("ZMQ_IPC_FILTER_UID", ZMQ_IPC_FILTER_UID);
        rt->add_var("ZMQ_IPC_FILTER_GID", ZMQ_IPC_FILTER_GID);
        rt->add_var("ZMQ_IPV4ONLY", ZMQ_IPV4ONLY);
        rt->add_var("ZMQ_DELAY_ATTACH_ON_CONNECT", ZMQ_DELAY_ATTACH_ON_CONNECT);
        rt->add_var("ZMQ_NOBLOCK", ZMQ_NOBLOCK);
        rt->add_var("ZMQ_FAIL_UNROUTABLE", ZMQ_FAIL_UNROUTABLE);
        rt->add_var("ZMQ_ROUTER_BEHAVIOR", ZMQ_ROUTER_BEHAVIOR);

        /*  Deprecated Message options                                                */
        rt->add_var("ZMQ_SRCFD", ZMQ_SRCFD);

        /******************************************************************************/
        /*  GSSAPI definitions                                                        */
        /******************************************************************************/

        /*  GSSAPI principal name types                                               */
        rt->add_var("ZMQ_GSSAPI_NT_HOSTBASED", ZMQ_GSSAPI_NT_HOSTBASED);
        rt->add_var("ZMQ_GSSAPI_NT_USER_NAME", ZMQ_GSSAPI_NT_USER_NAME);
        rt->add_var("ZMQ_GSSAPI_NT_KRB5_PRINCIPAL", ZMQ_GSSAPI_NT_KRB5_PRINCIPAL);

        /******************************************************************************/
        /*  0MQ socket events and monitoring                                          */
        /******************************************************************************/

        /*  Socket transport events (TCP, IPC and TIPC only)                          */

        rt->add_var("ZMQ_EVENT_CONNECTED", ZMQ_EVENT_CONNECTED);
        rt->add_var("ZMQ_EVENT_CONNECT_DELAYED", ZMQ_EVENT_CONNECT_DELAYED);
        rt->add_var("ZMQ_EVENT_CONNECT_RETRIED", ZMQ_EVENT_CONNECT_RETRIED);
        rt->add_var("ZMQ_EVENT_LISTENING", ZMQ_EVENT_LISTENING);
        rt->add_var("ZMQ_EVENT_BIND_FAILED", ZMQ_EVENT_BIND_FAILED);
        rt->add_var("ZMQ_EVENT_ACCEPTED", ZMQ_EVENT_ACCEPTED);
        rt->add_var("ZMQ_EVENT_ACCEPT_FAILED", ZMQ_EVENT_ACCEPT_FAILED);
        rt->add_var("ZMQ_EVENT_CLOSED", ZMQ_EVENT_CLOSED);
        rt->add_var("ZMQ_EVENT_CLOSE_FAILED", ZMQ_EVENT_CLOSE_FAILED);
        rt->add_var("ZMQ_EVENT_DISCONNECTED", ZMQ_EVENT_DISCONNECTED);
        rt->add_var("ZMQ_EVENT_MONITOR_STOPPED", ZMQ_EVENT_MONITOR_STOPPED);
        rt->add_var("ZMQ_EVENT_ALL", ZMQ_EVENT_ALL);
        /*  Unspecified system errors during handshake. Event value is an errno.      */
        rt->add_var("ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL", ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL);
        /*  Handshake complete successfully with successful authentication (if        *
         *  enabled). Event value is unused.                                          */
        rt->add_var("ZMQ_EVENT_HANDSHAKE_SUCCEEDED", ZMQ_EVENT_HANDSHAKE_SUCCEEDED);
        /*  Protocol errors between ZMTP peers or between server and ZAP handler.     *
         *  Event value is one of ZMQ_PROTOCOL_ERROR_*                                */
        rt->add_var("ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL", ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL);
        /*  Failed authentication requests. Event value is the numeric ZAP status     *
         *  code, i.e. 300, 400 or 500.                                               */
        rt->add_var("ZMQ_EVENT_HANDSHAKE_FAILED_AUTH", ZMQ_EVENT_HANDSHAKE_FAILED_AUTH);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_UNSPECIFIED", ZMQ_PROTOCOL_ERROR_ZMTP_UNSPECIFIED);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_UNEXPECTED_COMMAND", ZMQ_PROTOCOL_ERROR_ZMTP_UNEXPECTED_COMMAND);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_INVALID_SEQUENCE", ZMQ_PROTOCOL_ERROR_ZMTP_INVALID_SEQUENCE);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_KEY_EXCHANGE", ZMQ_PROTOCOL_ERROR_ZMTP_KEY_EXCHANGE);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_UNSPECIFIED", ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_UNSPECIFIED);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_MESSAGE", ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_MESSAGE);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_HELLO", ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_HELLO);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_INITIATE", ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_INITIATE);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_ERROR", ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_ERROR);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_READY", ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_READY);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_WELCOME", ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_WELCOME);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_INVALID_METADATA", ZMQ_PROTOCOL_ERROR_ZMTP_INVALID_METADATA);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_CRYPTOGRAPHIC", ZMQ_PROTOCOL_ERROR_ZMTP_CRYPTOGRAPHIC);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZMTP_MECHANISM_MISMATCH", ZMQ_PROTOCOL_ERROR_ZMTP_MECHANISM_MISMATCH);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZAP_UNSPECIFIED", ZMQ_PROTOCOL_ERROR_ZAP_UNSPECIFIED);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZAP_MALFORMED_REPLY", ZMQ_PROTOCOL_ERROR_ZAP_MALFORMED_REPLY);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZAP_BAD_REQUEST_ID", ZMQ_PROTOCOL_ERROR_ZAP_BAD_REQUEST_ID);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZAP_BAD_VERSION", ZMQ_PROTOCOL_ERROR_ZAP_BAD_VERSION);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZAP_INVALID_STATUS_CODE", ZMQ_PROTOCOL_ERROR_ZAP_INVALID_STATUS_CODE);
        rt->add_var("ZMQ_PROTOCOL_ERROR_ZAP_INVALID_METADATA", ZMQ_PROTOCOL_ERROR_ZAP_INVALID_METADATA);
        rt->add_var("ZMQ_PROTOCOL_ERROR_WS_UNSPECIFIED", ZMQ_PROTOCOL_ERROR_WS_UNSPECIFIED);


        rt->add_function("zmq_errno", SCFXFUN() { return zmq_errno(); });
        rt->add_function("zmq_errstr", SCFXFUN() { return zmq_strerror(zmq_errno()); });
        rt->add_function("zmq_err_to_str", SCFXFUN(args) { return zmq_strerror(args[0].cast_num_to_num<int>()); });
        rt->add_function("zmq_ctx_new", SCFXFUN() { return zmq_ctx_new(); });
        rt->add_function("zmq_ctx_term", SCFXFUN(args) { return zmq_ctx_term(args[0].as_ptr()); });
        rt->add_function("zmq_ctx_shutdown", SCFXFUN(args) { return zmq_ctx_shutdown(args[0].as_ptr()); });
        rt->add_function("zmq_ctx_set", SCFXFUN(args) { return zmq_ctx_set(args[0].as_ptr(), args[1].cast_num_to_num<int>(), args[2].cast_num_to_num<int>()); });
        rt->add_function("zmq_ctx_get", SCFXFUN(args) { return zmq_ctx_get(args[0].as_ptr(), args[1].cast_num_to_num<int>()); });
        rt->add_function("zmq_socket", SCFXFUN(args) { return zmq_socket(args[0].as_ptr(), args[1].cast_num_to_num<int>()); });
        rt->add_function("zmq_bind", SCFXFUN(args) { return zmq_bind(args[0].as_ptr(), args[1].cast_to_string().c_str()); });
        rt->add_function("zmq_connect", SCFXFUN(args) { return zmq_connect(args[0].as_ptr(), args[1].cast_to_string().c_str()); });

        rt->add_function("zmq_setsockopt", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 3, 3)
            int opt{args[1].cast_num_to_num<int>()};
            auto d{args[2].cast_to_byte_array()};
            return zmq_setsockopt(args[0].as_ptr(), opt, d.data(), d.size());
        });

        rt->add_function("zmq_z85_encode", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
            auto src{args[0].cast_to_byte_array()};
            if(src.empty()) {
                return std::string{};
            }
            std::vector<std::uint8_t> dst{};
            dst.resize(src.size() * 5 /4);
            zmq_z85_encode((char *)dst.data(), src.data(), src.size());
            return std::string{(char *)dst.data()};
        });

        rt->add_function("zmq_z85_decode", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
            auto src{args[0].cast_to_string()};
            if(src.empty()) {
                return std::vector<std::uint8_t>{};
            }
            std::vector<std::uint8_t> dst{};
            dst.resize(src.size() * 4 / 5);
            zmq_z85_decode(dst.data(), src.data());
            return dst;
        });

        rt->add_function("zmq_send", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
            std::vector<std::uint8_t> buf{args[1].cast_to_byte_array()};
            if(args.size() == 3) {
                return zmq_send(args[0].as_ptr(), buf.data(), buf.size(), args[2].cast_num_to_num<int>());
            } else {
                return zmq_send(args[0].as_ptr(), buf.data(), buf.size(), 0);
            }
        });
        rt->add_function("zmq_send_more", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 2)
            std::vector<std::uint8_t> buf{args[1].cast_to_byte_array()};
            return zmq_send(args[0].as_ptr(), buf.data(), buf.size(), ZMQ_MORE);
        });
        rt->add_function("zmq_recv", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
            int nrd{0};
            std::vector<std::uint8_t> buf{};
            if(args.size() > 1) {
                size_t nread{args[1].cast_num_to_num<std::size_t>()};
                if(nread == 0) { return std::string{}; }
                buf.resize(nread);
                if(args.size() == 3) {
                    nrd = zmq_recv(args[0].as_ptr(), buf.data(), nread, args[2].cast_num_to_num<int>());
                } else {
                    nrd = zmq_recv(args[0].as_ptr(), buf.data(), nread, 0);
                }
            } else {
                size_t nread{128 * 1024};
                buf.resize(nread);
                nrd = zmq_recv(args[0].as_ptr(), buf.data(), nread, 0);
            }
            if(nrd > 0) { return std::string{buf.begin(), buf.begin() + nrd}; }
            return std::string{};
        });
    }

    void unregister_runtime() override {
        std::unique_lock l{rt_mtp_};
        if(rt_ == nullptr) {
            return;
        }

        rt_->remove_var("ZMQ_ENOTSUP");
        rt_->remove_var("ZMQ_EPROTONOSUPPORT");
        rt_->remove_var("ZMQ_ENOBUFS");
        rt_->remove_var("ZMQ_ENETDOWN");
        rt_->remove_var("ZMQ_EADDRINUSE");
        rt_->remove_var("ZMQ_EADDRNOTAVAIL");
        rt_->remove_var("ZMQ_ECONNREFUSED");
        rt_->remove_var("ZMQ_EINPROGRESS");
        rt_->remove_var("ZMQ_ENOTSOCK");
        rt_->remove_var("ZMQ_EMSGSIZE");
        rt_->remove_var("ZMQ_EAFNOSUPPORT");
        rt_->remove_var("ZMQ_ENETUNREACH");
        rt_->remove_var("ZMQ_ECONNABORTED");
        rt_->remove_var("ZMQ_ECONNRESET");
        rt_->remove_var("ZMQ_ENOTCONN");
        rt_->remove_var("ZMQ_ETIMEDOUT");
        rt_->remove_var("ZMQ_EHOSTUNREACH");
        rt_->remove_var("ZMQ_ENETRESET");

        /*  Native 0MQ error codes.                                                   */
        rt_->remove_var("ZMQ_EFSM");
        rt_->remove_var("ZMQ_ENOCOMPATPROTO");
        rt_->remove_var("ZMQ_ETERM");
        rt_->remove_var("ZMQ_EMTHREAD");

        /*  Context options                                                           */
        rt_->remove_var("ZMQ_IO_THREADS");
        rt_->remove_var("ZMQ_MAX_SOCKETS");
        rt_->remove_var("ZMQ_SOCKET_LIMIT");
        rt_->remove_var("ZMQ_THREAD_PRIORITY");
        rt_->remove_var("ZMQ_THREAD_SCHED_POLICY");
        rt_->remove_var("ZMQ_MAX_MSGSZ");
        rt_->remove_var("ZMQ_MSG_T_SIZE");
        rt_->remove_var("ZMQ_THREAD_AFFINITY_CPU_ADD");
        rt_->remove_var("ZMQ_THREAD_AFFINITY_CPU_REMOVE");
        rt_->remove_var("ZMQ_THREAD_NAME_PREFIX");

        /*  Default for new contexts                                                  */
        rt_->remove_var("ZMQ_IO_THREADS_DFLT");
        rt_->remove_var("ZMQ_MAX_SOCKETS_DFLT");
        rt_->remove_var("ZMQ_THREAD_PRIORITY_DFLT");
        rt_->remove_var("ZMQ_THREAD_SCHED_POLICY_DFLT");

        /*  Socket types.                                                             */
        rt_->remove_var("ZMQ_PAIR");
        rt_->remove_var("ZMQ_PUB");
        rt_->remove_var("ZMQ_SUB");
        rt_->remove_var("ZMQ_REQ");
        rt_->remove_var("ZMQ_REP");
        rt_->remove_var("ZMQ_DEALER");
        rt_->remove_var("ZMQ_ROUTER");
        rt_->remove_var("ZMQ_PULL");
        rt_->remove_var("ZMQ_PUSH");
        rt_->remove_var("ZMQ_XPUB");
        rt_->remove_var("ZMQ_XSUB");
        rt_->remove_var("ZMQ_STREAM");

        /*  Socket options.                                                           */
        rt_->remove_var("ZMQ_AFFINITY");
        rt_->remove_var("ZMQ_ROUTING_ID");
        rt_->remove_var("ZMQ_SUBSCRIBE");
        rt_->remove_var("ZMQ_UNSUBSCRIBE");
        rt_->remove_var("ZMQ_RATE");
        rt_->remove_var("ZMQ_RECOVERY_IVL");
        rt_->remove_var("ZMQ_SNDBUF");
        rt_->remove_var("ZMQ_RCVBUF");
        rt_->remove_var("ZMQ_RCVMORE");
        rt_->remove_var("ZMQ_FD");
        rt_->remove_var("ZMQ_EVENTS");
        rt_->remove_var("ZMQ_TYPE");
        rt_->remove_var("ZMQ_LINGER");
        rt_->remove_var("ZMQ_RECONNECT_IVL");
        rt_->remove_var("ZMQ_BACKLOG");
        rt_->remove_var("ZMQ_RECONNECT_IVL_MAX");
        rt_->remove_var("ZMQ_MAXMSGSIZE");
        rt_->remove_var("ZMQ_SNDHWM");
        rt_->remove_var("ZMQ_RCVHWM");
        rt_->remove_var("ZMQ_MULTICAST_HOPS");
        rt_->remove_var("ZMQ_RCVTIMEO");
        rt_->remove_var("ZMQ_SNDTIMEO");
        rt_->remove_var("ZMQ_LAST_ENDPOINT");
        rt_->remove_var("ZMQ_ROUTER_MANDATORY");
        rt_->remove_var("ZMQ_TCP_KEEPALIVE");
        rt_->remove_var("ZMQ_TCP_KEEPALIVE_CNT");
        rt_->remove_var("ZMQ_TCP_KEEPALIVE_IDLE");
        rt_->remove_var("ZMQ_TCP_KEEPALIVE_INTVL");
        rt_->remove_var("ZMQ_IMMEDIATE");
        rt_->remove_var("ZMQ_XPUB_VERBOSE");
        rt_->remove_var("ZMQ_ROUTER_RAW");
        rt_->remove_var("ZMQ_IPV6");
        rt_->remove_var("ZMQ_MECHANISM");
        rt_->remove_var("ZMQ_PLAIN_SERVER");
        rt_->remove_var("ZMQ_PLAIN_USERNAME");
        rt_->remove_var("ZMQ_PLAIN_PASSWORD");
        rt_->remove_var("ZMQ_CURVE_SERVER");
        rt_->remove_var("ZMQ_CURVE_PUBLICKEY");
        rt_->remove_var("ZMQ_CURVE_SECRETKEY");
        rt_->remove_var("ZMQ_CURVE_SERVERKEY");
        rt_->remove_var("ZMQ_PROBE_ROUTER");
        rt_->remove_var("ZMQ_REQ_CORRELATE");
        rt_->remove_var("ZMQ_REQ_RELAXED");
        rt_->remove_var("ZMQ_CONFLATE");
        rt_->remove_var("ZMQ_ZAP_DOMAIN");
        rt_->remove_var("ZMQ_ROUTER_HANDOVER");
        rt_->remove_var("ZMQ_TOS");
        rt_->remove_var("ZMQ_CONNECT_ROUTING_ID");
        rt_->remove_var("ZMQ_GSSAPI_SERVER");
        rt_->remove_var("ZMQ_GSSAPI_PRINCIPAL");
        rt_->remove_var("ZMQ_GSSAPI_SERVICE_PRINCIPAL");
        rt_->remove_var("ZMQ_GSSAPI_PLAINTEXT");
        rt_->remove_var("ZMQ_HANDSHAKE_IVL");
        rt_->remove_var("ZMQ_SOCKS_PROXY");
        rt_->remove_var("ZMQ_XPUB_NODROP");
        rt_->remove_var("ZMQ_BLOCKY");
        rt_->remove_var("ZMQ_XPUB_MANUAL");
        rt_->remove_var("ZMQ_XPUB_WELCOME_MSG");
        rt_->remove_var("ZMQ_STREAM_NOTIFY");
        rt_->remove_var("ZMQ_INVERT_MATCHING");
        rt_->remove_var("ZMQ_HEARTBEAT_IVL");
        rt_->remove_var("ZMQ_HEARTBEAT_TTL");
        rt_->remove_var("ZMQ_HEARTBEAT_TIMEOUT");
        rt_->remove_var("ZMQ_XPUB_VERBOSER");
        rt_->remove_var("ZMQ_CONNECT_TIMEOUT");
        rt_->remove_var("ZMQ_TCP_MAXRT");
        rt_->remove_var("ZMQ_THREAD_SAFE");
        rt_->remove_var("ZMQ_MULTICAST_MAXTPDU");
        rt_->remove_var("ZMQ_VMCI_BUFFER_SIZE");
        rt_->remove_var("ZMQ_VMCI_BUFFER_MIN_SIZE");
        rt_->remove_var("ZMQ_VMCI_BUFFER_MAX_SIZE");
        rt_->remove_var("ZMQ_VMCI_CONNECT_TIMEOUT");
        rt_->remove_var("ZMQ_USE_FD");
        rt_->remove_var("ZMQ_GSSAPI_PRINCIPAL_NAMETYPE");
        rt_->remove_var("ZMQ_GSSAPI_SERVICE_PRINCIPAL_NAMETYPE");
        rt_->remove_var("ZMQ_BINDTODEVICE");

        /*  Message options                                                           */
        rt_->remove_var("ZMQ_MORE");
        rt_->remove_var("ZMQ_SHARED");

        /*  Send/recv options.                                                        */
        rt_->remove_var("ZMQ_DONTWAIT");
        rt_->remove_var("ZMQ_SNDMORE");

        /*  Security mechanisms                                                       */
        rt_->remove_var("ZMQ_NULL");
        rt_->remove_var("ZMQ_PLAIN");
        rt_->remove_var("ZMQ_CURVE");
        rt_->remove_var("ZMQ_GSSAPI");

        /*  RADIO-DISH protocol                                                       */
        rt_->remove_var("ZMQ_GROUP_MAX_LENGTH");

        /*  Deprecated options and aliases                                            */
        rt_->remove_var("ZMQ_IDENTITY");
        rt_->remove_var("ZMQ_CONNECT_RID");
        rt_->remove_var("ZMQ_TCP_ACCEPT_FILTER");
        rt_->remove_var("ZMQ_IPC_FILTER_PID");
        rt_->remove_var("ZMQ_IPC_FILTER_UID");
        rt_->remove_var("ZMQ_IPC_FILTER_GID");
        rt_->remove_var("ZMQ_IPV4ONLY");
        rt_->remove_var("ZMQ_DELAY_ATTACH_ON_CONNECT");
        rt_->remove_var("ZMQ_NOBLOCK");
        rt_->remove_var("ZMQ_FAIL_UNROUTABLE");
        rt_->remove_var("ZMQ_ROUTER_BEHAVIOR");

        /*  Deprecated Message options                                                */
        rt_->remove_var("ZMQ_SRCFD");

        /******************************************************************************/
        /*  GSSAPI definitions                                                        */
        /******************************************************************************/

        /*  GSSAPI principal name types                                               */
        rt_->remove_var("ZMQ_GSSAPI_NT_HOSTBASED");
        rt_->remove_var("ZMQ_GSSAPI_NT_USER_NAME");
        rt_->remove_var("ZMQ_GSSAPI_NT_KRB5_PRINCIPAL");

        /******************************************************************************/
        /*  0MQ socket events and monitoring                                          */
        /******************************************************************************/

        /*  Socket transport events (TCP, IPC and TIPC only)                          */

        rt_->remove_var("ZMQ_EVENT_CONNECTED");
        rt_->remove_var("ZMQ_EVENT_CONNECT_DELAYED");
        rt_->remove_var("ZMQ_EVENT_CONNECT_RETRIED");
        rt_->remove_var("ZMQ_EVENT_LISTENING");
        rt_->remove_var("ZMQ_EVENT_BIND_FAILED");
        rt_->remove_var("ZMQ_EVENT_ACCEPTED");
        rt_->remove_var("ZMQ_EVENT_ACCEPT_FAILED");
        rt_->remove_var("ZMQ_EVENT_CLOSED");
        rt_->remove_var("ZMQ_EVENT_CLOSE_FAILED");
        rt_->remove_var("ZMQ_EVENT_DISCONNECTED");
        rt_->remove_var("ZMQ_EVENT_MONITOR_STOPPED");
        rt_->remove_var("ZMQ_EVENT_ALL");
        rt_->remove_var("ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL");
        rt_->remove_var("ZMQ_EVENT_HANDSHAKE_SUCCEEDED");
        rt_->remove_var("ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL");

        rt_->remove_var("ZMQ_EVENT_HANDSHAKE_FAILED_AUTH");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_UNSPECIFIED");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_UNEXPECTED_COMMAND");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_INVALID_SEQUENCE");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_KEY_EXCHANGE");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_UNSPECIFIED");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_MESSAGE");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_HELLO");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_INITIATE");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_ERROR");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_READY");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_MALFORMED_COMMAND_WELCOME");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_INVALID_METADATA");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_CRYPTOGRAPHIC");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZMTP_MECHANISM_MISMATCH");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZAP_UNSPECIFIED");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZAP_MALFORMED_REPLY");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZAP_BAD_REQUEST_ID");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZAP_BAD_VERSION");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZAP_INVALID_STATUS_CODE");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_ZAP_INVALID_METADATA");
        rt_->remove_var("ZMQ_PROTOCOL_ERROR_WS_UNSPECIFIED");

        rt_->remove_function("zmq_errno");
        rt_->remove_function("zmq_errstr");
        rt_->remove_function("zmq_err_to_str");
        rt_->remove_function("zmq_ctx_new");
        rt_->remove_function("zmq_ctx_term");
        rt_->remove_function("zmq_ctx_shutdown");
        rt_->remove_function("zmq_ctx_set");
        rt_->remove_function("zmq_ctx_get");
        rt_->remove_function("zmq_socket");
        rt_->remove_function("zmq_bind");
        rt_->remove_function("zmq_connect");
        rt_->remove_function("zmq_setsockopt");
        rt_->remove_function("zmq_z85_encode");
        rt_->remove_function("zmq_z85_decode");
        rt_->remove_function("zmq_send");
        rt_->remove_function("zmq_send_more");
        rt_->remove_function("zmq_recv");

        rt_ = nullptr;
    }

private:
    std::shared_mutex rt_mtp_{};
    scfx::runtime_interface *rt_{nullptr};
};

