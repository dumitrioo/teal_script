#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/sequence_generator.hpp"
#include "../inc/str_util.hpp"
#include "../inc/sys_util.hpp"
#include "../inc/file_util.hpp"

#include "../tealscript_value.hpp"
#include "../tealscript_util.hpp"
#include "../tealscript_interfaces.hpp"

namespace teal {

    class persistence_ext: public extension_interface {
    public:
        persistence_ext() = default;
        ~persistence_ext() {
            unregister_runtime();
        }
        persistence_ext(persistence_ext const &) = delete;
        persistence_ext &operator=(persistence_ext const &) = delete;
        persistence_ext(persistence_ext &&) = delete;
        persistence_ext &operator=(persistence_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }

            std::shared_ptr<persistence> pers{std::make_shared<persistence>(rt_)};
            pers->open(rt_->persistence_file_path());
            rt->add_var("persistence", valbox{pers, "persistence"});

            rt->add_method("persistence", "open", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return TEALTHIS(args, std::shared_ptr<persistence>)->open(args[1].cast_to_string());
            });
            rt->add_method("persistence", "loaded", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return TEALTHIS(args, std::shared_ptr<persistence>)->loaded();
            });
            rt->add_method("persistence", "get", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return TEALTHIS(args, std::shared_ptr<persistence>)->get(args[1].cast_to_string());
            });
            rt->add_method("persistence", "put", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                return TEALTHIS(args, std::shared_ptr<persistence>)->put(args[1].cast_to_string(), args[2]);
            });
            rt->add_method("persistence", "close", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args.size() == 1) {
                    return TEALTHIS(args, std::shared_ptr<persistence>)->close();
                }
                return false;
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_var("persistence");
            rt_->remove_method("persistence", "open");
            rt_->remove_method("persistence", "loaded");
            rt_->remove_method("persistence", "get");
            rt_->remove_method("persistence", "put");
            rt_->remove_method("persistence", "close");
            rt_ = nullptr;
        }

    private:
        class persistence {
        public:
            persistence(runtime_interface *rt): rt_{rt} {}
            persistence(persistence const &) = delete;
            persistence(persistence &&) = delete;
            persistence &operator=(persistence const &) = delete;
            persistence &operator=(persistence &&) = delete;
            ~persistence() { close(); }

            bool open(std::string const &path) {
                std::unique_lock l{mtp_};
                path_ = path;
                return load();
            }

            bool close() {
                std::unique_lock l{mtp_};
                save();
                data_.clear();
                return true;
            }

            bool loaded() const {
                std::shared_lock l{mtp_};
                return loaded_;
            }

            valbox get(std::string const &k) const {
                std::shared_lock l{mtp_};
                auto i{data_.find(k)};
                if(i != data_.end()) {
                    return i->second;
                }
                return valbox{};
            }

            bool put(std::string const &k, valbox const &v) {
                if(!v.is_undefined()) {
                    update_waiters_++;
                    std::unique_lock l{mtp_};
                    update_waiters_--;
                    data_[k] = v;
                    need_sync_ = true;
                    if(update_waiters_ == 0) {
                        save();
                    }
                    return true;
                }
                return false;
            }

        private:
            bool save() {
                bool saved{false};
                if(!path_.empty() && need_sync_) {
                    try {
                        json j{};
                        j.become_object();
                        for(auto &&p: data_) {
                            j[p.first] = p.second.serialize(rt_);
                        }
                        file_util::save_to_file(path_, j.serialize5(1));
                        saved = true;
                        loaded_ = true;
                        need_sync_ = false;
                    } catch (...) {
                    }
                }
                return saved;
            }

            bool load() {
                loaded_ = false;
                try {
                    data_.clear();
                    json j{json::deserialize(file_util::load_from_file(path_))};
                    if(j.is_object()) {
                        j.traverse_object([this](std::string const &key, json const &val) {
                            data_[key].deserialize(val, rt_);
                        });
                    }
                    need_sync_ = false;
                    loaded_ = true;
                } catch (...) {
                    data_.clear();
                }
                return loaded_;
            }

        private:
            runtime_interface *rt_{nullptr};
            std::string path_{};
            mutable std::shared_mutex mtp_{};
            std::map<std::string, valbox> data_{};
            std::atomic<size_t> update_waiters_{0};
            std::atomic_bool need_sync_{false};
            bool loaded_{false};
        };

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
