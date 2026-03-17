#pragma once

#include "../commondefs.hpp"
#include "../sys_util.hpp"
// #include "../threading.hpp"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
extern "C" {
#include <sys/epoll.h>
}
#endif
#include "../timespec_wrapper.hpp"
#ifdef PLATFORM_APPLE
#include <poll.h>
#endif
#if defined(PLATFORM_WINDOWS)
#include <WinSock2.h>
#endif

namespace scfx::net {

    DEFINE_RUNTIME_ERROR_CLASS(poll_error)

#ifdef PLATFORM_APPLE
    enum POLL_EVENTS {
        POLL_EVENT_IN = POLLIN,
        POLL_EVENT_PRI = POLLPRI,
        POLL_EVENT_OUT = POLLOUT,
        POLL_EVENT_RDNORM = POLLRDNORM,
        POLL_EVENT_RDBAND = POLLRDBAND,
        POLL_EVENT_WRNORM = POLLWRNORM,
        POLL_EVENT_WRBAND = POLLWRBAND,
#ifdef __USE_GNU
        POLL_EVENT_MSG = POLLMSG,
        POLL_EVENT_RDHUP = POLLRDHUP,
#endif
        POLL_EVENT_ERR = POLLERR,
        POLL_EVENT_HUP = POLLHUP,
    };
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
    enum POLL_EVENTS {
        POLL_EVENT_IN = EPOLLIN,
        POLL_EVENT_PRI = EPOLLPRI,
        POLL_EVENT_OUT = EPOLLOUT,
        POLL_EVENT_RDNORM = EPOLLRDNORM,
        POLL_EVENT_RDBAND = EPOLLRDBAND,
        POLL_EVENT_WRNORM = EPOLLWRNORM,
        POLL_EVENT_WRBAND = EPOLLWRBAND,
        POLL_EVENT_MSG = EPOLLMSG,
        POLL_EVENT_ERR = EPOLLERR,
        POLL_EVENT_HUP = EPOLLHUP,
        POLL_EVENT_RDHUP = EPOLLRDHUP,
        POLL_EVENT_EXCLUSIVE = EPOLLEXCLUSIVE,
        POLL_EVENT_WAKEUP = EPOLLWAKEUP,
        POLL_EVENT_ONESHOT = EPOLLONESHOT,
        POLL_EVENT_ET = EPOLLET
    };
#elif defined(PLATFORM_WINDOWS)
    enum POLL_EVENTS {
        POLL_EVENT_ET = 0
        POLL_EVENT_IN = POLLIN,
        POLL_EVENT_PRI = POLLPRI,
        POLL_EVENT_OUT = POLLOUT,
        POLL_EVENT_RDNORM = POLLRDNORM,
        POLL_EVENT_RDBAND = POLLRDBAND,
        POLL_EVENT_WRNORM = POLLWRNORM,
        POLL_EVENT_WRBAND = POLLWRBAND,
        POLL_EVENT_ERR = POLLERR,
        POLL_EVENT_HUP = POLLHUP,
    };
#endif

    typedef union poll_data {
      void *ptr;
      int fd;
      uint32_t u32;
      uint64_t u64;
    } poll_data_t;

    #ifdef _WIN32
    #pragma pack(push, 1)
    #endif
    struct poll_event {
      uint32_t events;
      poll_data_t data;
    }
    #if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
    __attribute__ ((__packed__))
    #endif
    ;
    #ifdef _WIN32
    #pragma pack(pop)
    #endif

    namespace impl {

#if defined(PLATFORM_APPLE)

        class poll_wrapper_impl {
        public:
            poll_wrapper_impl() = default;
            ~poll_wrapper_impl() {
                close();
            }
            bool add_event(int fd, int ev_flags, void * /*sock_attached_data*/ = nullptr) {
                struct pollfd pfd{fd, 0, 0};
                if (ev_flags & POLL_EVENT_PRI) { pfd.events |= POLLPRI; }
                if (ev_flags & POLL_EVENT_RDBAND) { pfd.events |= POLLRDBAND; }
                if (ev_flags & POLL_EVENT_WRBAND) { pfd.events |= POLLWRBAND; }
                if (ev_flags & POLL_EVENT_RDNORM) { pfd.events |= POLLRDNORM; }
                if (ev_flags & POLL_EVENT_WRNORM) { pfd.events |= POLLWRNORM; }
                if (ev_flags & POLL_EVENT_IN) { pfd.events |= POLLIN; }
                if (ev_flags & POLL_EVENT_OUT) { pfd.events |= POLLOUT; }
                std::lock_guard lck{events_mtp_};
                events_map_[fd] = pfd;
                return true;
            }
            bool del_event(int fd) {
                std::lock_guard lck{events_mtp_};
                auto it{events_map_.find(fd)};
                if(it != events_map_.end()) {
                    events_map_.erase(fd);
                    return true;
                }
                return false;
            }
            bool re_enable(int, uint32_t) {
                return false;
            }
            void close() {
                std::lock_guard lck{events_mtp_};
                events_map_.clear();
                last_error_ = 0;
            }
            std::vector<poll_event> wait(std::size_t maxcount = 64, scfx::timespec_wrapper const &timeout = scfx::eternity) {
                std::vector<poll_event> result{};
                std::vector<struct pollfd> events_vec{};
                {
                    std::lock_guard lck{events_mtp_};
                    events_vec.resize(events_map_.size());
                    std::size_t i{0};
                    for(auto &&p: events_map_) {
                        events_vec[i++] = p.second;
                    }
                }
                if(!events_vec.empty()) {
                    int poll_res = ::poll(&events_vec[0], events_vec.size(), timeout == scfx::eternity ? std::numeric_limits<int>::max() : timeout.milliseconds());
                    if(poll_res > 0) {
                        result.reserve(poll_res);
                        for(std::size_t i = 0; i < events_vec.size(); ++i) {
                            if(events_vec[i].revents != 0) {
                                poll_event revt{};
                                revt.data.fd = events_vec[i].fd;
                                if(events_vec[i].revents & POLLERR) { revt.events |= POLL_EVENT_ERR; }
                                if(events_vec[i].revents & POLLHUP) { revt.events |= POLL_EVENT_HUP; }
                                if(events_vec[i].revents & POLLNVAL) { revt.events |= POLL_EVENT_ERR; }
                                if(events_vec[i].revents & POLLIN) { revt.events |= POLL_EVENT_IN; }
                                if(events_vec[i].revents & POLLPRI) { revt.events |= POLL_EVENT_PRI; }
                                if(events_vec[i].revents & POLLOUT) { revt.events |= POLL_EVENT_OUT; }
                                if(events_vec[i].revents & POLLRDNORM) { revt.events |= POLL_EVENT_RDNORM; }
                                if(events_vec[i].revents & POLLRDBAND) { revt.events |= POLL_EVENT_RDBAND; }
                                if(events_vec[i].revents & POLLWRNORM) { revt.events |= POLL_EVENT_WRNORM; }
                                if(events_vec[i].revents & POLLWRBAND) { revt.events |= POLL_EVENT_WRBAND; }
#ifdef __USE_GNU
                                if(events_vec[i].revents & POLLMSG) { revt.events |= POLL_EVENT_MSG; }
                                if(events_vec[i].revents & POLLRDHUP) { revt.events |= POLL_EVENT_RDHUP; }
#endif
                                result.push_back(revt);
                            }
                        }
                    } else if(poll_res < 0) {
                        last_error_ = errno;
                        throw_errno(errno);
                    }
                }
                return result;
            }
            int last_error() const {
                return last_error_;
            }

        private:
            std::map<int, struct pollfd> events_map_{};
            std::mutex events_mtp_{};
            int last_error_{0};
        };

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)

#ifdef PLATFORM_ANDROID
        int epoll_create1(int __flags) __INTRODUCED_IN(21);
#endif

        class poll_wrapper_impl {
        public:
            poll_wrapper_impl() {
                epfd_ = ::epoll_create1(0);
                if(epfd_ < 0) {
                    throw poll_error("epoll_create1()");
                }
            }
            ~poll_wrapper_impl() {
                this->close();
            }
            bool add_event(int fd, int ev_flags, void *sock_attached_data = nullptr) {
                epoll_event evt;
                std::memset(&evt, 0, sizeof(evt));
                if(sock_attached_data) {
                    evt.data.ptr = sock_attached_data;
                } else {
                    evt.data.fd = fd;
                }
                evt.events = ev_flags;
                int err_code = ::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &evt);
                last_error_ = errno;
                return err_code == 0;
            }
            bool del_event(int fd) {
                int err_code = ::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
                last_error_ = errno;
                return err_code == 0;
            }
            bool re_enable(int fd, uint32_t mod) {
                struct epoll_event evnt;
                evnt.data.fd = fd;
                evnt.events = mod;
                int err_code = ::epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &evnt);
                last_error_ = errno;
                return err_code == 0;
            }
            void close() {
                if(epfd_ != -1) {
                    ::close(epfd_);
                    last_error_ = errno;
                    epfd_ = -1;
                }
            }
            std::vector<poll_event> wait(std::size_t maxcount = 64, scfx::timespec_wrapper const &timeout = scfx::eternity) {
                if(maxcount <= 0) {
                    throw poll_error("bad arguments for epoll wait()");
                }

                if(events_buf_.size() < maxcount) {
                    events_buf_.resize(maxcount);
                }

                int wait_result = ::epoll_wait(epfd_, &events_buf_[0], maxcount, timeout == scfx::eternity ? -1 : timeout.milliseconds());
                if(wait_result > 0) {
                    std::vector<poll_event> events(wait_result);
                    for(int i = 0; i < wait_result; i++) {
                        events[i].data.fd = events_buf_[i].data.fd;
                        events[i].data.u32 = events_buf_[i].data.u32;
                        events[i].data.u64 = events_buf_[i].data.u64;
                        events[i].data.ptr = events_buf_[i].data.ptr;
                        events[i].events = events_buf_[i].events;
                    }
                    return events;
                } else if(wait_result < 0) {
                    last_error_ = errno;
                    throw poll_error{scfx::sys_util::error_str(scfx::sys_util::last_error())};
                }

                return {};
            }
            int last_error() const {
                return last_error_;
            }

        private:
            std::vector<epoll_event> events_buf_{};
            int epfd_{-1};
            int last_error_{0};
        };

#elif defined(PLATFORM_WINDOWS)
        class poll_wrapper_impl {
        public:
            poll_wrapper_impl() = default;
            ~poll_wrapper_impl() = default;
            bool add_event(int fd, int ev_flags, void * /*sock_attached_data*/ = nullptr) {
                WSAPOLLFD evt;
                memset(&evt, 0, sizeof(evt));
                evt.fd = fd;
                if (ev_flags & POLL_EVENT_PRI) {
                    evt.events |= POLLPRI;
                }
                if (ev_flags & POLL_EVENT_RDBAND) {
                    evt.events |= POLLRDBAND;
                }
                if (ev_flags & POLL_EVENT_WRBAND) {
                    evt.events |= POLLWRBAND;
                }
                if (ev_flags & POLL_EVENT_RDNORM) {
                    evt.events |= POLLRDNORM;
                }
                if (ev_flags & POLL_EVENT_WRNORM) {
                    evt.events |= POLLWRNORM;
                }
                if (ev_flags & POLL_EVENT_IN) {
                    evt.events |= POLLIN;
                }
                if (ev_flags & POLL_EVENT_OUT) {
                    evt.events |= POLLOUT;
                }
                std::unique_lock l_buf(events_buf_mtp_);
                events_buf_[fd] = evt;
                return true;
            }
            bool del_event(int e) {
                std::unique_lock l_buf(events_buf_mtp_);
                auto it{events_buf_.find(e)};
                if(it != events_buf_.end()) {
                    events_buf_.erase(it);
                    return true;
                }
                return false;
            }
            bool re_enable(int, uint32_t) {
                return true;
            }
            void close() {}
            std::vector<poll_event> wait(std::size_t maxcount = 64, scfx::timespec_wrapper const &timeout = scfx::eternity) {
                std::vector<poll_event> events;
                std::unique_lock l_buf(events_buf_mtp_);
                if (events_buf_.size()) {
                    std::vector<WSAPOLLFD> events_vec;
                    std::for_each(events_buf_.begin(), events_buf_.end(), [&](const std::pair<int, WSAPOLLFD> &poll_descr) {events_vec.push_back(poll_descr.second); });
                    l_buf.unlock();
                    int wait_result = WSAPoll(&events_vec[0], events_vec.size(), timeout == eternity ? INFINITE : timeout.milliseconds());
                    if (wait_result > 0) {
                        for (std::size_t i = 0; i < events_vec.size(); i++) {
                            if (events_vec[i].revents) {
                                poll_event evt = { 0 };
                                evt.data.fd = events_vec[i].fd;
                                if (events_vec[i].revents & POLLERR) {
                                    evt.events |= POLL_EVENT_ERR;
                                }
                                if (events_vec[i].revents & POLLHUP) {
                                    evt.events |= POLL_EVENT_HUP;
                                }
                                if (events_vec[i].revents & POLLPRI) {
                                    evt.events |= POLL_EVENT_IN | POLL_EVENT_PRI;
                                }
                                if (events_vec[i].revents & POLLRDBAND) {
                                    evt.events |= POLL_EVENT_IN | POLL_EVENT_RDBAND;
                                }
                                if (events_vec[i].revents & POLLRDNORM) {
                                    evt.events |= POLL_EVENT_IN | POLL_EVENT_RDNORM;
                                }
                                if (events_vec[i].revents & POLLWRNORM) {
                                    evt.events |= POLL_EVENT_OUT;
                                }
                                events.push_back(evt);
                            }
                        }
                    }
                }
                return events;
            }

            int last_error() const {
                return last_error_;
            }

        private:
            std::map<int, WSAPOLLFD> events_buf_{};
            std::shared_mutex events_buf_mtp_{};
            int last_error_{0};
        };
#endif

    }

    class socket_poller {
    public:
        socket_poller() {}
        ~socket_poller() {
            impl_.close();
        }
        bool add_event(int fd, int ev_flags = POLL_EVENT_IN
#ifndef PLATFORM_APPLE
            | POLL_EVENT_ET
#endif
            , void *sock_attached_data = 0
        ) {
            return impl_.add_event(fd, ev_flags, sock_attached_data);
        }
        bool del_event(int fd) {
            return impl_.del_event(fd);
        }
        bool re_enable(int fd, uint32_t mode) {
            return impl_.re_enable(fd, mode);
        }
        void close() {
            impl_.close();
        }
        std::vector<poll_event> wait(std::size_t maxcount = 64, scfx::timespec_wrapper const &timeout = scfx::eternity) {
            return impl_.wait(maxcount, timeout);
        }
        int last_error() const {
            return impl_.last_error();
        }

    private:
        impl::poll_wrapper_impl impl_{};
    };

}
