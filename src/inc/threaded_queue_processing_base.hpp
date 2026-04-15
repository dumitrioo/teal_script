#pragma once

#include "commondefs.hpp"
#include "terminable.hpp"
#include "containers/circular_buffer.hpp"
#include "math/math_util.hpp"
#include "timespec_wrapper.hpp"
#include "containers/concurrentqueue.h"

namespace teal {

    template<typename DERIVED_T, typename QITEM_T>
    class threaded_queue_processing_base: public terminable {
    public:
        threaded_queue_processing_base(std::size_t max_thrd_cnt, std::size_t min_thrd_cnt):
            max_workers_{max_thrd_cnt},
            min_workers_{min_thrd_cnt}
        {
            if(max_workers_ == 0 || max_workers_ < min_workers_) {
                throw std::runtime_error{"invalid arguments"};
            }
            ctl_thr_ = std::thread{
                [&]() {
                    while(!termination()) {
                        update_stats();
                        if(need_to_kill_one()) {
                            kill_someone();
                        } else if(need_to_spawn_one()) {
                            create_worker();
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds{100});
                    }
                }
            };
        }
        threaded_queue_processing_base(threaded_queue_processing_base const &) = delete;
        threaded_queue_processing_base(threaded_queue_processing_base &&) = delete;
        threaded_queue_processing_base &operator=(threaded_queue_processing_base const &) = delete;
        threaded_queue_processing_base &operator=(threaded_queue_processing_base &&) = delete;
        ~threaded_queue_processing_base() {
            terminate();
            QITEM_T itm;
            for(;hp_q_.try_dequeue(itm);) {}
            for(;lp_q_.try_dequeue(itm);) {}
            {
                std::unique_lock l{q_mtp_};
                q_cvar_.notify_all();
            }
            wait();
            {
                std::unique_lock l{workers_mtp_};
                while(!workers_.empty()) {
                    workers_.pop_front();
                    num_workers_.store(workers_.size(), std::memory_order_release);
                }
            }
        }

        void wait() noexcept {
            if(ctl_thr_.joinable()) {
                ctl_thr_.join();
            }
        }

        void enqueue(QITEM_T &&f) {
            if(termination()) {
                return;
            }
            {
                lp_q_.enqueue(std::move(f));
                std::unique_lock lq{q_mtp_};
                q_cvar_.notify_one();
            }
            if(num_workers_.load(std::memory_order_acquire) == 0) {
                create_worker();
            }
        }

        void enqueue(QITEM_T const &f) {
            if(termination()) {
                return;
            }
            {
                lp_q_.enqueue(f);
                std::unique_lock lq{q_mtp_};
                q_cvar_.notify_one();
            }
            if(num_workers_.load(std::memory_order_acquire) == 0) {
                create_worker();
            }
        }

        void enqueue_urgent(QITEM_T &&f) {
            if(termination()) {
                return;
            }
            {
                hp_q_.enqueue(std::move(f));
                std::unique_lock lq{q_mtp_};
                q_cvar_.notify_one();
            }
            if(num_workers_.load(std::memory_order_acquire) == 0) {
                create_worker();
            }
        }

        void enqueue_urgent(QITEM_T const &f) {
            if(termination()) {
                return;
            }
            {
                hp_q_.enqueue(f);
                std::unique_lock lq{q_mtp_};
                q_cvar_.notify_one();
            }
            if(num_workers_.load(std::memory_order_acquire) == 0) {
                create_worker();
            }
        }

        void set_workload_to_start_spawn_threads(long double val) noexcept {
            long double clamped{math::clamp<long double>(val, 0, 1)};
            if(clamped < load_to_start_kill_) {
                load_to_start_spawn_ = load_to_start_kill_;
            } else {
                load_to_start_spawn_ = clamped;
            }
        }

        void set_workload_to_start_kill_threads(double val) noexcept {
            long double clamped{math::clamp<long double>(val, 0, 1)};
            if(val > load_to_start_spawn_) {
                load_to_start_kill_ = load_to_start_spawn_;
            } else {
                load_to_start_kill_ = clamped;
            }
        }

        void set_min_seconds_between_killings(long double val) {
            min_seconds_between_killings_ = val;
        }

        std::size_t num_enqueued_items() const {
            return hp_q_.size_approx() + lp_q_.size_approx();
        }

        std::uint64_t current_workload_percents() const noexcept {
            return avg_lt_ld_.load(std::memory_order_acquire) * 100;
        }

        void set_sleep_interval(double) {
        }

        double sleep_interval() const {
            return 0;
        }

        std::size_t num_worker_threads() const {
            return num_workers_.load(std::memory_order_acquire);
        }

        double curr_load() const {
            return avg_lt_ld_.load(std::memory_order_acquire);
        }

    private:
        class worker_thread: public terminable {
        public:
            worker_thread(threaded_queue_processing_base *owner):
                owner_{owner},
                thr_{
                    [&]() {
                        while(!termination() && !owner_->termination()) {
                            QITEM_T i;
                            if(owner_->dequeue(i)) {
                                busy_.store(1, std::memory_order_release);
                                static_cast<DERIVED_T *>(owner_)->serve_queue_item(i);
                                busy_.store(0, std::memory_order_release);
                            }
                        }
                    }
                }
            {
            }
            worker_thread(worker_thread const &) = delete;
            worker_thread(worker_thread &&) = delete;
            worker_thread &operator=(worker_thread const &) = delete;
            worker_thread &operator=(worker_thread &&) = delete;
            ~worker_thread() { terminate(); wait(); }
            void wait() { if(thr_.joinable()) { thr_.join(); } }
            std::size_t busy() const { return busy_.load(std::memory_order_acquire); }

        private:
            threaded_queue_processing_base *owner_{nullptr};
            std::thread thr_{};
            std::atomic<std::size_t>busy_{0};
        };

        bool dequeue(QITEM_T &j) {
            if(!hp_q_.try_dequeue(j) && !lp_q_.try_dequeue(j)) {
                std::unique_lock lq{q_mtp_};
                q_cvar_.wait_for(lq, std::chrono::milliseconds{250});
                lq.unlock();
                if(termination() || (!hp_q_.try_dequeue(j) && !lp_q_.try_dequeue(j))) {
                    return false;
                }
            }
            return true;
        }

        void update_stats() {
            double num_busy{0};
            double num_workers{static_cast<double>(num_worker_threads())};
            if(num_workers > 0) {
                {
                    std::shared_lock l{workers_mtp_};
                    for(auto &&w: workers_) {
                        num_busy += w->busy();
                    }
                }
                st_loads_.push_back(num_busy / num_workers);
                lt_loads_.push_back(num_busy / num_workers);
            } else {
                st_loads_.push_back(0);
                lt_loads_.push_back(0);
            }
            recalc_avg_load_lt();
            recalc_avg_load_st();
        }

        void recalc_avg_load_lt() {
            if(lt_loads_.size() == 0) { return; }
            double res{};
            for(std::size_t i{0}; i < lt_loads_.size(); ++i) {
                res += lt_loads_[i];
            }
            res /= static_cast<double>(lt_loads_.size());
            avg_lt_ld_.store(res, std::memory_order_release);
        }

        void recalc_avg_load_st() {
            if(st_loads_.size() == 0) { return; }
            double res{};
            for(std::size_t i{0}; i < st_loads_.size(); ++i) {
                res += st_loads_[i];
            }
            res /= static_cast<double>(st_loads_.size());
            avg_st_ld_.store(res, std::memory_order_release);
        }

        bool need_to_kill_one() const {
            std::size_t wtn{num_worker_threads()};
            return
                wtn > 0
                &&
                (
                    wtn > max_workers_
                    ||
                    termination()
                    ||
                    (
                        wtn > min_workers_
                        &&
                        !(wtn == 1 && num_enqueued_items() > 0)
                        &&
                        steady_time_sec() - last_kill_time_ > min_seconds_between_killings_
                        &&
                        avg_lt_ld_.load(std::memory_order_acquire) <= load_to_start_kill_
                    )
                )
            ;
        }

        bool need_to_spawn_one() const {
            if(termination()) {
                return false;
            }
            std::size_t wtn{num_worker_threads()};
            std::size_t enqueued_items{num_enqueued_items()};
            return
                wtn < min_workers_
                ||
                (
                    wtn < max_workers_
                    &&
                    (
                        (wtn == 0 && enqueued_items > 0)
                        ||
                        wtn < static_cast<std::size_t>(workers_for_works<double>(enqueued_items))
                        ||
                        avg_st_ld_.load(std::memory_order_acquire) >= load_to_start_spawn_
                    )
                )
            ;
        }

        void kill_someone() {
            std::unique_lock lw{workers_mtp_};
            if(workers_.size() > 0) {
                auto wptr{std::move(workers_.front())};
                workers_.pop_front();
                num_workers_.store(workers_.size(), std::memory_order_release);
                lw.unlock();
                wptr->terminate();
                {
                    std::unique_lock lq{q_mtp_};
                    q_cvar_.notify_all();
                }
                wptr.reset();
                last_kill_time_ = steady_time_sec();
            }
        }

        template<typename T> static T workers_for_works(T num_works) {
            return std::min(std::sqrt(num_works) * 3, num_works);
        }

        void maybe_create_worker() {
            if(need_to_spawn_one()) {
                std::unique_lock lw{workers_mtp_};
                workers_.emplace_back(std::make_unique<worker_thread>(this));
                num_workers_.store(workers_.size(), std::memory_order_release);
            }
        }

        void create_worker() {
            std::unique_lock lw{workers_mtp_};
            workers_.emplace_back(std::make_unique<worker_thread>(this));
            num_workers_.store(workers_.size(), std::memory_order_release);
        }

    private:
        std::size_t max_workers_{0};
        std::size_t min_workers_{0};
        std::atomic<std::size_t> num_busy_workers_{0};
        mutable std::mutex q_mtp_{};
        std::condition_variable q_cvar_{};
        moodycamel::ConcurrentQueue<QITEM_T> hp_q_{};
        moodycamel::ConcurrentQueue<QITEM_T> lp_q_{};
        long double load_to_start_spawn_{0.9L};
        long double load_to_start_kill_{0.1L};
        long double min_seconds_between_killings_{0.0L};
        long double last_kill_time_{0.0L};
        mutable std::shared_mutex workers_mtp_{};
        std::list<std::unique_ptr<worker_thread>> workers_{};
        std::atomic<std::size_t> num_workers_{0};
        std::thread ctl_thr_{};
        scalar_circular_buffer<double, 10> st_loads_{};
        scalar_circular_buffer<double, 30> lt_loads_{};
        mutable std::atomic<double> avg_st_ld_{0};
        mutable std::atomic<double> avg_lt_ld_{0};
    };

}
