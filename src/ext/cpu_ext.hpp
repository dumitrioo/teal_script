#pragma once

#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
#include <sys/times.h>
#endif

#include "../include/commondefs.hpp"
#include "../include/timespec_wrapper.hpp"
#include "../include/containers/circular_buffer.hpp"

#include "../scaflux_value.hpp"
#include "../scaflux_util.hpp"
#include "../scaflux_interfaces.hpp"

namespace scfx {

    class cpu_ext: public extension_interface {
    public:
        cpu_ext() = default;
        ~cpu_ext() {
            unregister_runtime();
        }
        cpu_ext(cpu_ext const &) = delete;
        cpu_ext &operator=(cpu_ext const &) = delete;
        cpu_ext(cpu_ext &&) = delete;
        cpu_ext &operator=(cpu_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt->add_function("perf_stat", SCFXFUN(, /*args*/) {
                return scfx::valbox{std::make_shared<cpu_pc<double>>(), "perf_stat"};
            });
            rt->add_method("perf_stat", "start", SCFXFUN(, args) {
                return SCFXTHIS(std::shared_ptr<cpu_pc<double>>)->start();
            });
            rt->add_method("perf_stat", "stop", SCFXFUN(, args) {
                SCFXTHIS(std::shared_ptr<cpu_pc<double>>)->stop();
                return 0;
            });
            rt->add_method("perf_stat", "cpu_load", SCFXFUN(, args) {
                return SCFXTHIS(std::shared_ptr<cpu_pc<double>>)->cpu_load();
            });
            rt->add_method("perf_stat", "self_cpu_consumption", SCFXFUN(, args) {
                return SCFXTHIS(std::shared_ptr<cpu_pc<double>>)->self_cpu_consumption();
            });
        }
        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("perf_stat");
            rt_->remove_method("perf_stat", "start");
            rt_->remove_method("perf_stat", "stop");
            rt_->remove_method("perf_stat", "cpu_load");
            rt_->remove_method("perf_stat", "self_cpu_consumption");
            rt_ = nullptr;
        }

    private:
        template<typename FLT_T>
        class cpu_pc {
        public:
            cpu_pc(/*FLT_T scan_period = 0.1*/):
                cpu_usage_{0},
                self_cpu_consumption_{0}/*,
            scan_period_{std::max<FLT_T>(std::min<FLT_T>(scan_period, 5), 0)}*/
            {
            }

            ~cpu_pc() {
                stop();
            }

            static void thr_proc(cpu_pc *this_, std::promise<bool> prom) {
                prom.set_value(true);
                while(!this_->termination_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    this_->actualize_all_cpu_value();
                    this_->actualize_curr_cpu_value();
                }
                this_->running_ = false;
            }

            bool start() {
                cpu_usage_ = 0;
                self_cpu_consumption_ = 0;
                std::lock_guard<std::mutex> l{cpu_usage_thread_mtp_};
                if(!running_) {
                    if(cpu_state_init()) {
                        termination_ = false;
                        std::promise<bool> prom{};
                        std::future<bool> fut{prom.get_future()};
                        cpu_usage_thread_ = std::thread{[&]() {
                            prom.set_value(true);
                            while(!termination_) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                actualize_all_cpu_value();
                                actualize_curr_cpu_value();
                            }
                            running_ = false;
                        }};
                        running_ = fut.get();
                        return running_;
                    } else {
                        return false;
                    }
                }
                return true;
            }

            void stop() {
                std::lock_guard<std::mutex> l{cpu_usage_thread_mtp_};
                termination_ = true;
                if(cpu_usage_thread_.joinable()) {
                    cpu_usage_thread_.join();
                }
                running_ = false;
            }

            FLT_T cpu_load() const {
                return cpu_usage_;
            }

            FLT_T self_cpu_consumption() const {
                FLT_T res{ self_cpu_consumption_.load() };
                if(res > 0) {
                    return res;
                } else {
                    return 0;
                }
            }

        private:
            bool cpu_state_init() {
                n_of_processors_ = std::thread::hardware_concurrency();
#if defined __linux
                last_clk_ = ::times(&tsmpl_last_);
                if(last_clk_ == static_cast<clock_t>(-1)) {
                    return false;
                }
                if(n_of_processors_ == 0ULL) {
                    return false;
                }
                self_cpu_consumption_buff_.clear();
                return true;
#elif defined(WINDOWS)
                if(n_of_processors_ == 0ULL) {
                    return false;
                }
                HANDLE hProcess{ GetCurrentProcess() };
                FILETIME CreationTime;
                FILETIME ExitTime;
                FILETIME KernelTime;
                FILETIME UserTime;
                if(GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime)) {
                    scfx::timespec_wrapper now{ scfx::timespec_wrapper::now() };
                    curr_cpu_kernel_last_scan_time_ = KernelTime;
                    curr_cpu_user_last_scan_time_ = UserTime;
                    curr_cpu_milestone_last_scan_time_ = now;
                } else {
                    return false;
                }
                return true;
#endif
            }

#if defined(WINDOWS)
            unsigned long long _previousTotalTicks{ 0 };
            unsigned long long _previousIdleTicks{ 0 };

            float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks) {
                unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
                unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;
                float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);
                _previousTotalTicks = totalTicks;
                _previousIdleTicks = idleTicks;
                return ret;
            }

            static unsigned long long FileTimeToInt64(const FILETIME& ft) {
                return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
            }

            float GetCPULoad() {
                FILETIME idleTime, kernelTime, userTime;
                return
                    GetSystemTimes(&idleTime, &kernelTime, &userTime)
                        ?
                        CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime))
                        :
                        -1.0f;
            }
#endif

            void actualize_curr_cpu_value() {
#if defined __linux
                struct tms tsmpl;
                clock_t now = ::times(&tsmpl);
                FLT_T res = 0;
                if(now != static_cast<clock_t>(-1)) {
                    res = (tsmpl.tms_stime - tsmpl_last_.tms_stime) + (tsmpl.tms_utime - tsmpl_last_.tms_utime);
                    res /= static_cast<FLT_T>(now - last_clk_) * n_of_processors_;
                }
                last_clk_ = now;
                tsmpl_last_ = tsmpl;
                self_cpu_consumption_buff_.push_back(res > 1 ? 1 : res);
                FLT_T sum{};
                for(std::size_t k{}; k < self_cpu_consumption_buff_.size(); ++k) {
                    sum += self_cpu_consumption_buff_[k];
                }
                self_cpu_consumption_ = sum / static_cast<FLT_T>(self_cpu_consumption_buff_.size());
#elif defined(WINDOWS)
                FILETIME CreationTime;
                FILETIME ExitTime;
                FILETIME KernelTime;
                FILETIME UserTime;
                DWORD pid{ GetCurrentProcessId() };
                if(GetProcessTimes(GetCurrentProcess(), &CreationTime, &ExitTime, &KernelTime, &UserTime)) {
                    scfx::timespec_wrapper now{ scfx::timespec_wrapper::now() };
                    scfx::timespec_wrapper TSWKernelTime{ KernelTime };
                    scfx::timespec_wrapper TSWUserTime{ UserTime };
                    scfx::timespec_wrapper dt{ now - curr_cpu_milestone_last_scan_time_ };
                    if(/*TSWKernelTime != curr_cpu_kernel_last_scan_time_ ||
                    TSWUserTime != curr_cpu_user_last_scan_time_ ||*/
                        dt.fseconds() > 1
                        ) {
                        dt *= n_of_processors_;
                        scfx::timespec_wrapper dt_k{ TSWKernelTime - curr_cpu_kernel_last_scan_time_ };
                        scfx::timespec_wrapper dt_u{ TSWUserTime - curr_cpu_user_last_scan_time_ };
                        FLT_T self_cpu_consumption{ (FLT_T)((/*dt_k + */dt_u).fseconds() / dt.fseconds()) };
                        self_cpu_consumption_buff_.push_back(self_cpu_consumption);
                        curr_cpu_kernel_last_scan_time_ = TSWKernelTime;
                        curr_cpu_user_last_scan_time_ = TSWUserTime;
                        curr_cpu_milestone_last_scan_time_ = now;
                    }
                    if(self_cpu_consumption_buff_.size()) {
                        FLT_T s = 0;
                        for(int i = 0; i < self_cpu_consumption_buff_.size(); ++i) { s += self_cpu_consumption_buff_[i]; }
                        self_cpu_consumption_ = std::clamp<FLT_T>(s / self_cpu_consumption_buff_.size(), 0, 1);
                    }

                }

#endif
            }

#if defined __linux
            std::ifstream proc_stat_ifs_{/*"/proc/stat"*/};
#endif

            void actualize_all_cpu_value() {
#if defined __linux
                if(!proc_stat_ifs_) {
                    proc_stat_ifs_.open("/proc/stat");
                }
                if(proc_stat_ifs_) {
                    proc_stat_ifs_.seekg(0);
                    std::string cpu;
                    std::uint64_t user, nice, system;
                    std::uint64_t other1, other2, other3,
                        other4, other5, other6, other7;
                    proc_stat_ifs_ >> cpu >> user >> nice
                        >> system >> other1
                        >> other2 >> other3 >> other4
                        >> other5 >> other6 >> other7;
                    if(cpu == "cpu") {
                        std::uint64_t work_j = user + nice + system;
                        std::uint64_t total_j =
                            work_j + other1 + other2 + other3 +
                            other4 + other5 + other6 + other7;
                        std::uint64_t d_work_j = work_j - work_j_;
                        std::uint64_t d_total_j = total_j - total_j_;
                        work_j_ = work_j;
                        total_j_ = total_j;
                        cpu_usage_.store(static_cast<FLT_T>(d_work_j) / static_cast<FLT_T>(d_total_j));
                    }
                }
#elif defined(WINDOWS)
                cpu_usage_ = static_cast<FLT_T>(GetCPULoad());
#endif
            }

        private:
            std::uint64_t n_of_processors_{ 0ULL };
            std::uint64_t work_j_{ 0ULL };
            std::uint64_t total_j_{ 0ULL };
            std::mutex cpu_usage_thread_mtp_{};
            std::thread cpu_usage_thread_;
            clock_t last_clk_{0};
            clock_t clock_last{0};
            scfx::scalar_circular_buffer<FLT_T, 5> self_cpu_consumption_buff_{};
#if defined(__linux)
            struct tms tsmpl_last_{};
#elif defined(WINDOWS)
            scfx::timespec_wrapper curr_cpu_milestone_last_scan_time_{};
            scfx::timespec_wrapper curr_cpu_kernel_last_scan_time_{};
            scfx::timespec_wrapper curr_cpu_user_last_scan_time_{};
#endif
            std::atomic<FLT_T> cpu_usage_{ 0 };
            std::atomic<FLT_T> self_cpu_consumption_{ 0 };
            bool termination_{ false };
            bool running_{ false };
        };
        std::shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
