#pragma once

#include "commondefs.hpp"
#include "threaded_queue_processing_base.hpp"
#include "terminable.hpp"
#include "timespec_wrapper.hpp"

namespace teal {

    class command_queue: public threaded_queue_processing_base<command_queue, std::function<void()>> {
    public:
        command_queue(std::size_t max_thrd_cnt, std::size_t min_thrd_cnt = 0):
            threaded_queue_processing_base{max_thrd_cnt, min_thrd_cnt}
        {
        }

        void serve_queue_item(std::function<void()> const &f) {
            try {
                f();
            } catch(...) {
            }
        }
    };

    class deferred_command_queue final: public terminable {
    public:
        deferred_command_queue(std::size_t thrd_cnt, std::shared_ptr<command_queue> worker_queue = {}):
            worker_queue_{worker_queue}
        {
            std::unique_lock pl{processors_mtp_};
            while(processors_.size() < thrd_cnt) {
                processors_.emplace_back(
                    [this]() {
                        while(!termination()) {
                            std::function<void()> j;
                            if(fetch_job(j)) {
                                try {
                                    std::shared_ptr<command_queue> wq_ptr{worker_queue_.lock()};
                                    if(wq_ptr) {
                                        wq_ptr->enqueue(std::move(j));
                                    } else {
                                        j();
                                    }
                                } catch(...) {}
                            }
                        }
                    }
                );
            }
        }
        deferred_command_queue(deferred_command_queue &&) = delete;
        deferred_command_queue(deferred_command_queue const &) = delete;
        deferred_command_queue &operator=(deferred_command_queue &&) = delete;
        deferred_command_queue &operator=(deferred_command_queue const &) = delete;
        ~deferred_command_queue() {
            terminate();
            {
                std::unique_lock el{evt_mtp_};
                evt_.notify_all();
            }
            {
                std::unique_lock pl{processors_mtp_};
                for(auto &&t: processors_) {
                    if(t.joinable()) {
                        t.join();
                    }
                }
                processors_.clear();
            }
        }

        void enqueue(std::function<void()> &&j, long double timeout, std::string const &group_id = {}) {
            std::unique_lock el{evt_mtp_};
            jobs_[steady_time_sec() + timeout].push_back({std::move(j), group_id});
            evt_.notify_one();
        }

        void enqueue(std::function<void()> const &j, long double timeout, std::string const &group_id = {}) {
            std::unique_lock el{evt_mtp_};
            jobs_[steady_time_sec() + timeout].push_back({j, group_id});
            evt_.notify_one();
        }

        void cancel_group(std::string const &group_id) {
            std::unique_lock el{evt_mtp_};
            for(std::map<long double, std::list<std::pair<std::function<void()>, std::string>>>::iterator mit{jobs_.begin()}; mit != jobs_.end();) {
                std::list<std::pair<std::function<void()>, std::string>> &lst{mit->second};
                for(std::list<std::pair<std::function<void()>, std::string>>::iterator lit{lst.begin()}; lit != lst.end();) {
                    if(lit->second == group_id) {
                        lit = lst.erase(lit);
                    } else {
                        ++lit;
                    }
                }
                if(lst.size() == 0) {
                    mit = jobs_.erase(mit);
                } else {
                    ++mit;
                }
            }
        }

    private:
        bool fetch_job(std::function<void()> &j) {
            std::unique_lock el{evt_mtp_};
            std::int64_t time_to_wait{0};
            if(!jobs_.empty()) {
                auto jobs_begin_iter{jobs_.begin()};
                if(0.1L + steady_time_sec() - jobs_begin_iter->first >= 0) {
                    std::list<std::pair<std::function<void()>, std::string>> &the_list{jobs_begin_iter->second};
                    j = std::move(the_list.front().first);
                    the_list.pop_front();
                    if(the_list.empty()) {
                        jobs_.erase(jobs_begin_iter);
                    }
                    return true;
                } else {
                    time_to_wait = (jobs_begin_iter->first - steady_time_sec()) * 1000;
                }
            } else {
                time_to_wait = 250L;
            }
            if(time_to_wait > 0) {
                evt_.wait_for(el, std::chrono::milliseconds{time_to_wait});
            }
            return false;
        }

    private:
        std::mutex processors_mtp_{};
        std::vector<std::thread> processors_{};
        mutable std::mutex evt_mtp_{};
        std::condition_variable evt_{};
        std::map<long double, std::list<std::pair<std::function<void()>, std::string>>> jobs_{};
        std::weak_ptr<command_queue> worker_queue_{};
    };

}
