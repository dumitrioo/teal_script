// #include <commondefs.hpp>
#include "../../src/scaflux_interfaces.hpp"

class fractal_ext: public scfx::extension_interface {
    class fractal {
    public:
        fractal(int xstart, int xend, int ystart, int yend):
            xrange_{xstart, xend},
            yrange_{ystart, yend},
            x_{xstart},
            y_{ystart}
        {
        }

        scfx::valbox get_coord() {
            scfx::valbox res{};
            res.become_object();
            int x{};
            int y{};
            std::unique_lock l{coords_mtp_};
            if(x_ > xrange_.second) {
                x_ = xrange_.first;
                ++y_;
            }
            if(y_ > yrange_.second) {
                res["result"] = "failure";
                return res;
            }
            x = x_; y = y_;
            ++x_;
            l.unlock();
            res["result"].assign(std::string{"success"});
            res["x"].assign(x);
            res["y"].assign(y);
            return res;
        }

        void set_at(int x, int y, char c) {
            std::unique_lock l{field_mtp_};
            field_[y][x] = c;
        }

        std::vector<std::vector<char>> get_field() const {
            std::vector<std::vector<char>> res{};
            std::shared_lock l{field_mtp_};
            for(int y{yrange_.first}; y < yrange_.second; ++y) {
                res.push_back({});
                for(int x{xrange_.first}; x < xrange_.second; ++x) {
                    try {
                        res[res.size() - 1].push_back(field_.at(y).at(x));
                    } catch (...) {
                        res[res.size() - 1].push_back(' ');
                    }
                }
            }
            return res;
        }

        bool printed() const {
            std::unique_lock l1{prn_mtp_};
            return printed_;
        }

        void print() const {
            auto fld{get_field()};
            std::unique_lock l1{prn_mtp_};
            if(!printed_) {
                for(auto &&xa: fld) {
                    for (auto &&c: xa) {
                        std::cout << c;
                    }
                    std::cout << std::endl;
                }
                printed_ = true;
            }
        }

        void set_range(int xstart, int xend, int ystart, int yend) {
            std::shared_lock l1{coords_mtp_};
            std::shared_lock l2{field_mtp_};
            xrange_ = {xstart, xend};
            yrange_ = {ystart, yend};
            x_ = xstart;
            y_ = ystart;
            field_.clear();
        }

        void unprint() {
            std::shared_lock l3{prn_mtp_};
            printed_ = false;
        }

        void reset() {
            std::shared_lock l1{coords_mtp_};
            std::shared_lock l2{field_mtp_};
            std::shared_lock l3{prn_mtp_};
            x_ = xrange_.first;
            y_ = yrange_.first;
            printed_ = false;
            field_.clear();
        }

        int x_begin() const { std::shared_lock l1{coords_mtp_}; return xrange_.first; }
        int x_end() const { std::shared_lock l1{coords_mtp_}; return xrange_.second; }
        int y_begin() const { std::shared_lock l1{coords_mtp_}; return yrange_.first; }
        int y_end() const { std::shared_lock l1{coords_mtp_}; return yrange_.second; }
        char at(int x, int y) const { std::unique_lock l{field_mtp_}; try { return field_.at(y).at(x); } catch (...) {} return ' '; }

        void set_ranges(int xmin, int xmax, int ymin, int ymax) {
            std::shared_lock l1{coords_mtp_};
            std::shared_lock l2{field_mtp_};
            std::shared_lock l3{prn_mtp_};
            xrange_.first = xmin;
            xrange_.second = xmax;
            yrange_.first = ymin;
            yrange_.second = ymax;
            x_ = xrange_.first;
            y_ = yrange_.first;
            printed_ = false;
            field_.clear();
        }

    private:
        mutable std::shared_mutex prn_mtp_{};
        mutable bool printed_{false};
        mutable std::shared_mutex coords_mtp_{};
        std::pair<int, int> xrange_{};
        std::pair<int, int> yrange_{};
        int x_{};
        int y_{};
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
            rt->add_method("fractal", "get_coord", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(1)
                return args[0].as_class<fractal *>()->get_coord();
            });
        } catch (...) {
        }
        try {
            rt->add_method("fractal", "set_ranges", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(5)
                args[0].as_class<fractal *>()->set_ranges(args[1].cast_to_s32(), args[2].cast_to_s32(), args[3].cast_to_s32(), args[4].cast_to_s32());
                return true;
            });
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
            rt->add_method("fractal", "print", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(1)
                args[0].as_class<fractal *>()->print();
                return true;
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
        try { rt_->remove_method("fractal", "get_coord"); } catch(...) {}
        try { rt_->remove_method("fractal", "set_at"); } catch(...) {}
        try { rt_->remove_method("fractal", "print"); } catch(...) {}
        rt_ = nullptr;
    }

private:
    std::shared_mutex rt_mtp_{};
    scfx::runtime_interface *rt_{nullptr};

    fractal frc{-88, 387, -96, 135};
};

static fractal_ext extinst{};

scfx::extension_interface *create_scfx_extension() {
    return &extinst;
}

void remove_scfx_extension(scfx::extension_interface *) {
}
