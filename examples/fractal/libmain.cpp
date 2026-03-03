// #include <commondefs.hpp>
#include "../../src/scaflux_interfaces.hpp"

class fractal_ext: public scfx::extension_interface {
    class fractal {
    public:
        fractal() {
        }

        void set_at(int x, int y, char c) {
            std::unique_lock l{field_mtp_};
            field_[y][x] = c;
        }

        std::string dump() const {
            std::shared_lock l{field_mtp_};
            std::pair<int, int> yr{yrange_unlocked()};
            std::pair<int, int> xr{xrange_unlocked()};
            std::stringstream ss{};
            for(int y{yr.first}; y < yr.second; ++y) {
                for(int x{xr.first}; x < xr.second; ++x) {
                    ss << at_unlocked(x, y);
                }
                ss << std::endl;
            }
            return ss.str();
        }

        void reset() {
            std::unique_lock l2{field_mtp_};
            field_.clear();
        }

        char at(int x, int y) const {
            std::unique_lock l{field_mtp_};
            return at_unlocked(x, y);
        }

    private:
        char at_unlocked(int x, int y) const {
            auto yit{field_.find(y)};
            if(yit != field_.end()) {
                auto xit{yit->second.find(x)};
                if(xit != yit->second.end()) {
                    return xit->second;
                }
            }
            return ' ';
        }

        std::pair<int, int> xrange_unlocked() const {
            std::pair<int, int> res{-1, 0};
            if(field_.empty()) {
                return res;
            }
            bool initial{true};
            for(auto &&yp: field_) {
                std::map<int, char> const &xslice{yp.second};
                if(xslice.empty()) {
                    continue;
                }
                auto itlast{xslice.end()};
                --itlast;
                if(initial) {
                    res = std::pair<int, int>{xslice.begin()->first, itlast->first};
                    initial = false;
                } else {
                    if(xslice.begin()->first < res.first) {
                        res.first = field_.begin()->first;
                    }
                    if(itlast->first > res.second) {
                        res.second = itlast->first;
                    }
                }
            }
            return res;
        }

        std::pair<int, int> yrange_unlocked() const {
            if(field_.empty()) {
                return std::pair<int, int>{-1, 0};
            }
            auto itlast{field_.end()};
            --itlast;
            return std::pair<int, int>{field_.begin()->first, itlast->first};
        }

        mutable std::shared_mutex field_mtp_{};
        std::map<int, std::map<int, char>> field_{};
    };

public:
    fractal_ext() = default;
    ~fractal_ext() {
        unregister_runtime();
    }
    fractal_ext(fractal_ext const &) = delete;
    fractal_ext &operator=(fractal_ext const &) = delete;
    fractal_ext(fractal_ext &&) = delete;
    fractal_ext &operator=(fractal_ext &&) = delete;

    void register_runtime(scfx::runtime_interface *rt) override {
        std::unique_lock l{rt_mtp_};
        if(rt_ != nullptr) {
            return;
        }
        rt_ = rt;
        if(rt_ == nullptr) {
            return;
        }
        try {
            rt->add_var("fractal_field", scfx::valbox{&frc, "fractal"});
        } catch (...) {
        }
        try {
            rt->add_method("fractal", "set_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(4)
                args[0].as_class<fractal *>()->set_at(args[1].cast_to_s32(), args[2].cast_to_s32(), args[3].cast_to_char());
                return true;
            });
        } catch (...) {
        }
        try {
            rt->add_method("fractal", "get_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(3)
                return args[0].as_class<fractal *>()->at(args[1].cast_to_s32(), args[2].cast_to_s32());
            });
        } catch (...) {
        }
        try {
            rt->add_method("fractal", "dump", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(1)
                return args[0].as_class<fractal *>()->dump();
            });
        } catch (...) {
        }
    }

    void unregister_runtime() override {
        std::unique_lock l{rt_mtp_};
        if(rt_ == nullptr) {
            return;
        }
        try { rt_->remove_var("fractal_field"); } catch(...) {}
        try { rt_->remove_method("fractal", "get_at"); } catch(...) {}
        try { rt_->remove_method("fractal", "set_at"); } catch(...) {}
        try { rt_->remove_method("fractal", "dump"); } catch(...) {}
        rt_ = nullptr;
    }

private:
    std::shared_mutex rt_mtp_{};
    scfx::runtime_interface *rt_{nullptr};

    fractal frc{};
};

scfx::extension_interface *create_scfx_extension() {
    static fractal_ext extinst{};
    return &extinst;
}

void remove_scfx_extension(scfx::extension_interface *) {
}
