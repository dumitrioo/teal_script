#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/sequence_generator.hpp"
#include "../inc/str_util.hpp"

#include "../scaflux_value.hpp"
#include "../scaflux_util.hpp"
#include "../scaflux_interfaces.hpp"

namespace scfx {

    class containers_ext: public extension_interface {
    public:
        containers_ext() = default;
        ~containers_ext() {
            unregister_runtime();
        }
        containers_ext(containers_ext const &) = delete;
        containers_ext &operator=(containers_ext const &) = delete;
        containers_ext(containers_ext &&) = delete;
        containers_ext &operator=(containers_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt_->add_function("dictionary", SCFXFUN() {
                return valbox{std::make_shared<dictionary>(), "dictionary"};
            });
            rt_->add_method("dictionary", "put", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                return SCFXTHIS(args, std::shared_ptr<dictionary>)->put(args[1], args[2]);
            });
            rt_->add_method("dictionary", "key_exists", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<dictionary>)->key_exists(args[1]);
            });
            rt_->add_method("dictionary", "list_keys", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return SCFXTHIS(args, std::shared_ptr<dictionary>)->list_keys();
            });
            rt_->add_method("dictionary", "at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<dictionary>)->at(args[1]);
            });
            rt_->add_method("dictionary", "erase", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<dictionary>)->erase(args[1]);
            });
            rt_->add_method("dictionary", "clear", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                SCFXTHIS(args, std::shared_ptr<dictionary>)->clear();
                return 0;
            });
            rt_->add_method("dictionary", "size", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return SCFXTHIS(args, std::shared_ptr<dictionary>)->size();
            });






            rt_->add_function("queue", SCFXFUN() {
                return valbox{std::make_shared<queue>(), "queue"};
            });
            rt_->add_method("queue", "push_front", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                SCFXTHIS(args, std::shared_ptr<queue>)->push_front(args[1]);
                return args[1];
            });
            rt_->add_method("queue", "push_back", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                SCFXTHIS(args, std::shared_ptr<queue>)->push_back(args[1]);
                return args[1];
            });
            rt_->add_method("queue", "pop_front", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return SCFXTHIS(args, std::shared_ptr<queue>)->pop_front();
            });
            rt_->add_method("queue", "pop_back", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return SCFXTHIS(args, std::shared_ptr<queue>)->pop_back();
            });
            rt_->add_method("queue", "at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<queue>)->at(args[1].cast_to_size_t());
            });
            rt_->add_method("queue", "erase", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return SCFXTHIS(args, std::shared_ptr<queue>)->erase(args[1].cast_to_size_t());
            });
            rt_->add_method("queue", "clear", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return SCFXTHIS(args, std::shared_ptr<queue>)->clear();
            });


            rt_->add_function("sequence_generator", SCFXFUN() {
                return valbox{std::make_shared<atomic_sequence_generator<uint64_t>>(), "sequence_generator"};
            });
            rt_->add_method("sequence_generator", "reset", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2)
                if(args.size() == 2) {
                    SCFXTHIS(args, std::shared_ptr<atomic_sequence_generator<uint64_t>>)->reset(args[1].cast_to_u64());
                }
                return args[1];
            });
            rt_->add_method("sequence_generator", "next", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return SCFXTHIS(args, std::shared_ptr<atomic_sequence_generator<uint64_t>>)->next();
            });

        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }

            rt_->remove_function("dictionary");
            rt_->remove_method("dictionary", "put");
            rt_->remove_method("dictionary", "key_exists");
            rt_->remove_method("dictionary", "list_keys");
            rt_->remove_method("dictionary", "at");
            rt_->remove_method("dictionary", "erase");
            rt_->remove_method("dictionary", "clear");
            rt_->remove_method("dictionary", "size");

            rt_->remove_function("queue");
            rt_->remove_method("queue", "push_front");
            rt_->remove_method("queue", "push_back");
            rt_->remove_method("queue", "pop_front");
            rt_->remove_method("queue", "pop_back");
            rt_->remove_method("queue", "at");
            rt_->remove_method("queue", "erase");
            rt_->remove_method("queue", "clear");

            rt_->remove_function("sequence_generator");
            rt_->remove_method("sequence_generator", "reset");
            rt_->remove_method("sequence_generator", "next");

            rt_ = nullptr;
        }

    private:
        class dictionary {
        public:
            dictionary() = default;
            ~dictionary() = default;
            dictionary(dictionary const &) = delete;
            dictionary &operator=(dictionary const &) = delete;
            dictionary(dictionary &&that) = delete;
            dictionary &operator=(dictionary &&) = delete;

            valbox put(valbox const &k, valbox const &v) {
                std::unique_lock l{mtp_};
                return m_[k] = v;
            }
            bool key_exists(valbox const &k) const {
                std::shared_lock l{mtp_};
                return m_.find(k) != m_.end();
            }
            valbox list_keys() const {
                valbox res{valbox_no_initialize::dont_do_it};
                res.become_array();
                std::shared_lock l{mtp_};
                for(auto &&p: m_) {
                    res.as_array().push_back(p.first);
                }
                return res;
            }
            valbox &at(valbox const &k) {
                std::unique_lock l{mtp_};
                return m_[k];
            }
            std::size_t erase(valbox const &k) {
                std::unique_lock l{mtp_};
                return m_.erase(k);
            }
            void clear() {
                std::unique_lock l{mtp_};
                m_.clear();
            }
            std::size_t size() const {
                std::shared_lock l{mtp_};
                return m_.size();
            }

        private:
            mutable shared_mutex mtp_{};
            std::map<valbox, valbox> m_{};
        };

        class queue {
        public:
            queue() = default;
            ~queue() = default;
            queue(queue const &) = delete;
            queue &operator=(queue const &) = delete;
            queue(queue &&that) = delete;
            queue &operator=(queue &&) = delete;

            valbox push_front(valbox const &v) {
                std::unique_lock l{mtp_};
                q_.push_front(v);
                return q_.front();
            }
            valbox push_back(valbox const &v) {
                std::unique_lock l{mtp_};
                q_.push_back(v);
                return q_.back();
            }
            valbox pop_front() {
                valbox res{valbox_no_initialize::dont_do_it};
                std::unique_lock l{mtp_};
                if(!q_.empty()) {
                    res = std::move(q_.front());
                    q_.pop_front();
                }
                return res;
            }
            valbox pop_back() {
                valbox res{valbox_no_initialize::dont_do_it};
                std::unique_lock l{mtp_};
                if(!q_.empty()) {
                    res = std::move(q_.front());
                    q_.pop_front();
                }
                return res;
            }
            valbox at(std::size_t indx) const {
                valbox res{valbox_no_initialize::dont_do_it};
                std::shared_lock l{mtp_};
                if(!q_.empty()) {
                    res = q_.at(indx);
                }
                return res;
            }
            bool erase(std::size_t indx) {
                std::unique_lock l{mtp_};
                if(indx < q_.size()) {
                    q_.erase(q_.begin() + indx);
                    return true;
                }
                return false;
            }
            bool clear() {
                std::unique_lock l{mtp_};
                bool res{!q_.empty()};
                q_.clear();
                return res;
            }
            std::size_t size() const {
                std::shared_lock l{mtp_};
                return q_.size();
            }

        private:
            mutable shared_mutex mtp_{};
            std::deque<valbox> q_{};
        };

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
