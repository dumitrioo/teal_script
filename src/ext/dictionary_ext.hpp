#pragma once

#include "../include/commondefs.hpp"
#include "../include/sequence_generator.hpp"
#include "../include/str_util.hpp"

#include "../scaflux_value.hpp"
#include "../scaflux_util.hpp"
#include "../scaflux_interfaces.hpp"

namespace scfx {

    class dictionary_ext: public extension_interface {
    public:
        dictionary_ext() = default;
        ~dictionary_ext() {
            unregister_runtime();
        }
        dictionary_ext(dictionary_ext const &) = delete;
        dictionary_ext &operator=(dictionary_ext const &) = delete;
        dictionary_ext(dictionary_ext &&) = delete;
        dictionary_ext &operator=(dictionary_ext &&) = delete;

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
                using map_type = std::shared_ptr<dictionary>;
                return SCFXTHIS(args, map_type)->put(args[1], args[2]);
            });
            rt_->add_method("dictionary", "key_exists", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                using map_type = std::shared_ptr<dictionary>;
                return SCFXTHIS(args, map_type)->key_exists(args[1]);
            });
            rt_->add_method("dictionary", "at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                using map_type = std::shared_ptr<dictionary>;
                return SCFXTHIS(args, map_type)->at(args[1]);
            });
            rt_->add_method("dictionary", "erase", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                using map_type = std::shared_ptr<dictionary>;
                return SCFXTHIS(args, map_type)->erase(args[1]);
            });
            rt_->add_method("dictionary", "clear", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                using map_type = std::shared_ptr<dictionary>;
                SCFXTHIS(args, map_type)->clear();
                return 0;
            });
            rt_->add_method("dictionary", "size", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                using map_type = std::shared_ptr<dictionary>;
                return SCFXTHIS(args, map_type)->size();
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("file");
            rt_->remove_method("file", "open");
            rt_->remove_method("file", "to_string");
            rt_->remove_method("file", "ok");
            rt_->remove_method("file", "read");
            rt_->remove_method("file", "write");
            rt_->remove_method("file", "close");
            rt_->remove_method("file", "seek");
            rt_->remove_method("file", "seek_end");
            rt_->remove_method("file", "seek_begin");
            rt_->remove_method("file", "seek_cur");
            rt_->remove_method("file", "tell");
            rt_->remove_method("file", "size");
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
            valbox at(valbox const &k) const {
                std::shared_lock l{mtp_};
                auto it{m_.find(k)};
                if(it != m_.end()) {
                    return m_.at(k);
                }
                return valbox{valbox_no_initialize::dont_do_it};
            }
            valbox at(valbox const &k) {
                std::shared_lock l{mtp_};
                auto it{m_.find(k)};
                if(it != m_.end()) {
                    return m_.at(k);
                }
                return valbox{valbox_no_initialize::dont_do_it};
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

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
