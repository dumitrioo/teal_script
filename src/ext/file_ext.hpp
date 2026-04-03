#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/sequence_generator.hpp"
#include "../inc/str_util.hpp"

#include "../tealscript_value.hpp"
#include "../tealscript_util.hpp"
#include "../tealscript_interfaces.hpp"

namespace teal {

    class file_ext: public extension_interface {
    public:
        file_ext() = default;
        ~file_ext() {
            unregister_runtime();
        }
        file_ext(file_ext const &) = delete;
        file_ext &operator=(file_ext const &) = delete;
        file_ext(file_ext &&) = delete;
        file_ext &operator=(file_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt->add_function("file", TEALFUN(args) {
                if(args.size() == 1) {
                    return teal::valbox{std::make_shared<teal_file>(args[0].cast_to_string()), "file"};
                } else if(args.size() == 2) {
                    return teal::valbox{std::make_shared<teal_file>(args[0].cast_to_string(), args[1].cast_to_bool()), "file"};
                } else if(args.size() == 3) {
                    return teal::valbox{std::make_shared<teal_file>(args[0].cast_to_string(), args[1].cast_to_bool(), args[2].cast_to_bool()), "file"};
                } else if(args.size() == 4) {
                    return teal::valbox{std::make_shared<teal_file>(args[0].cast_to_string(), args[1].cast_to_bool(), args[2].cast_to_bool(), args[3].cast_to_bool()), "file"};
                }
                return teal::valbox{std::make_shared<teal_file>(), "file"};
            });
            rt->add_method("file", "open", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->open(args[1].cast_to_string());
                } else if(args.size() == 3) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->open(args[1].cast_to_string(), args[2].cast_to_bool());
                } else if(args.size() == 4) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->open(args[1].cast_to_string(), args[2].cast_to_bool(), args[3].cast_to_bool());
                } else if(args.size() == 5) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->open(args[1].cast_to_string(), args[2].cast_to_bool(), args[3].cast_to_bool(), args[4].cast_to_bool());
                }
                return false;
            });
            rt->add_method("file", "to_string", TEALFUN(args) {
                return std::string{"class \"file\", instance 0x"} +
                        teal::str_util::utoa<std::string>(reinterpret_cast<std::uintptr_t>(TEALTHIS(args, std::shared_ptr<teal_file>).get()), 16) +
                        ", status: " + (TEALTHIS(args, std::shared_ptr<teal_file>)->is_open() ? "opened" : "closed");
            });
            rt->add_method("file", "ok", TEALFUN(args) {
                return TEALTHIS(args, std::shared_ptr<teal_file>)->is_open();
            });
            rt->add_method("file", "read", TEALFUN(args) {
                if(args.size() == 2) {
                    std::vector<std::uint8_t> rdres{TEALTHIS(args, std::shared_ptr<teal_file>)->read(args[1].cast_to_u64())};
                    return std::string{rdres.begin(), rdres.end()};
                }
                return std::string{};
            });
            rt->add_method("file", "write", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->write(args[1].cast_to_byte_array());
                }
                return -1LL;
            });
            rt->add_method("file", "close", TEALFUN(args) {
                if(args.size() == 1) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->close();
                }
                return false;
            });
            rt->add_method("file", "seekr", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->seek_rd(args[1].cast_to_s64());
                }
                return false;
            });
            rt->add_method("file", "seekw", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->seek_wr(args[1].cast_to_s64());
                }
                return false;
            });
            rt->add_method("file", "seekr_end", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->seek_rd_from_end(args[1].cast_to_s64());
                }
                return false;
            });
            rt->add_method("file", "seekw_end", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->seek_wr_from_end(args[1].cast_to_s64());
                }
                return false;
            });
            rt->add_method("file", "seekr_begin", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->seek_rd_from_begin(args[1].cast_to_s64());
                }
                return false;
            });
            rt->add_method("file", "seekw_begin", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->seek_wr_from_begin(args[1].cast_to_s64());
                }
                return false;
            });
            rt->add_method("file", "seekr_cur", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->seek_rd_from_curr(args[1].cast_to_s64());
                }
                return false;
            });
            rt->add_method("file", "seekw_cur", TEALFUN(args) {
                if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<teal_file>)->seek_wr_from_curr(args[1].cast_to_s64());
                }
                return false;
            });
            rt->add_method("file", "tellr", TEALFUN(args) {
                if(args.size() == 1) {
                    auto res{TEALTHIS(args, std::shared_ptr<teal_file>)->tell_rd()};
                    return (std::int64_t)res;
                }
                return -1LL;
            });
            rt->add_method("file", "tellw", TEALFUN(args) {
                if(args.size() == 1) {
                    auto res{TEALTHIS(args, std::shared_ptr<teal_file>)->tell_wr()};
                    return (std::int64_t)res;
                }
                return -1LL;
            });
            rt->add_method("file", "size", TEALFUN(args) {
                if(args.size() == 1) {
                    auto org_pos{TEALTHIS(args, std::shared_ptr<teal_file>)->tell_rd()};
                    TEALTHIS(args, std::shared_ptr<teal_file>)->seek_rd_from_end(0);
                    auto res{TEALTHIS(args, std::shared_ptr<teal_file>)->tell_rd()};
                    TEALTHIS(args, std::shared_ptr<teal_file>)->seek_rd(org_pos);
                    return (std::int64_t)res;
                }
                return static_cast<std::int64_t>(0);
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
        class teal_file {
        public:
            teal_file() = default;
            teal_file(teal_file const &) = delete;
            teal_file(teal_file &&) = default;
            teal_file &operator=(teal_file const &) = delete;
            teal_file &operator=(teal_file &&) = default;
            ~teal_file() = default;

            teal_file(std::string const &path, bool for_write = false, bool create_if_ne = false, bool truncate_on_open = false) {
                open(path, for_write, create_if_ne, truncate_on_open);
            }

            bool open(std::string const &path, bool for_write = false, bool create_if_ne = false, bool truncate_on_open = false) {
                if(fd_ && fd_->good()) {
                    return true;
                }
                w_ = false;
                if(for_write) {
                    if(!create_if_ne) {
                        std::ifstream fdi{};
                        fdi.open(path, std::ios::binary | std::ios::in);
                        if(!fdi.is_open()) {
                            return false;
                        }
                        fdi.close();
                    }
                    fd_ = std::make_shared<std::fstream>();
                    w_ = true;
                    if(truncate_on_open) {
                        dynamic_cast<std::fstream *>(fd_.get())->open(path, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
                    } else {
                        dynamic_cast<std::fstream *>(fd_.get())->open(path, std::ios::binary | std::ios::in | std::ios::out);
                    }
                    if(!fd_->good()) {
                        std::ofstream fdo{};
                        fdo.open(path);
                        fdo.close();

                        if(truncate_on_open) {
                            dynamic_cast<std::fstream *>(fd_.get())->open(path, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
                        } else {
                            dynamic_cast<std::fstream *>(fd_.get())->open(path, std::ios::binary | std::ios::in | std::ios::out);
                        }
                    }
                } else {
                    fd_ = std::make_shared<std::ifstream>();
                    dynamic_cast<std::ifstream *>(fd_.get())->open(path, std::ios::binary | std::ios::in);
                }
                return fd_->good();
            }

            bool close() {
                if(fd_) {
                    fd_.reset();
                }
                return true;
            }

            bool is_open() const {
                return fd_ && fd_->good();
            }

            bool seek_rd(std::ios::pos_type pos) {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekg(pos, std::ios::beg);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(pos, std::ios::beg);
                    }
                    return true;
                }
                return false;
            }

            bool seek_wr(std::ios::pos_type pos) {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekp(pos, std::ios::beg);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(pos, std::ios::beg);
                    }
                    return true;
                }
                return false;
            }

            std::ios::pos_type tell_rd() {
                if(!is_open()) { return -1; }
                return w_ ?
                    dynamic_cast<std::fstream *>(fd_.get())->tellg():
                    dynamic_cast<std::ifstream *>(fd_.get())->tellg();
            }

            std::ios::pos_type tell_wr() {
                if(!is_open()) { return -1; }
                return w_ ?
                    dynamic_cast<std::fstream *>(fd_.get())->tellp():
                    dynamic_cast<std::ifstream *>(fd_.get())->tellg();
            }

            bool seek_rd_from_curr(std::ios::off_type offs) {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekg(offs, std::ios::cur);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(offs, std::ios::cur);
                    }
                    return true;
                }
                return false;
            }

            bool seek_rd_from_begin(std::ios::off_type offs) {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekg(offs, std::ios::beg);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(offs, std::ios::beg);
                    }
                    return true;
                }
                return false;
            }

            bool seek_rd_to_begin() {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekg(0);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(0);
                    }
                    return true;
                }
                return false;
            }

            bool seek_rd_from_end(std::ios::off_type offs) {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekg(offs, std::ios::end);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(offs, std::ios::end);
                    }
                    return true;
                }
                return false;
            }

            bool seek_rd_to_end() {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekg(0, std::ios::end);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(0, std::ios::end);
                    }
                    return true;
                }
                return false;
            }

            bool seek_wr_from_curr(std::ios::off_type offs) {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekp(offs, std::ios::cur);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(offs, std::ios::cur);
                    }
                    return true;
                }
                return false;
            }

            bool seek_wr_from_begin(std::ios::off_type offs) {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekp(offs, std::ios::beg);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(offs, std::ios::beg);
                    }
                    return true;
                }
                return false;
            }

            bool seek_wr_to_begin() {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekp(0);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(0);
                    }
                    return true;
                }
                return false;
            }

            bool seek_wr_from_end(std::ios::off_type offs) {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekp(offs, std::ios::end);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(offs, std::ios::end);
                    }
                    return true;
                }
                return false;
            }

            bool seek_wr_to_end() {
                if(is_open()) {
                    if(w_) {
                        dynamic_cast<std::fstream *>(fd_.get())->seekp(0, std::ios::end);
                    } else {
                        dynamic_cast<std::ifstream *>(fd_.get())->seekg(0, std::ios::end);
                    }
                    return true;
                }
                return false;
            }

            std::vector<std::uint8_t> read(size_t n) {
                std::vector<std::uint8_t> res{};
                if(is_open() && n > 0) {
                    res.resize(n);
                    try {
                        std::uint64_t rdcnt{};
                        if(w_) {
                            dynamic_cast<std::fstream *>(fd_.get())->read((std::fstream::char_type *)&res[0], n);
                            rdcnt = dynamic_cast<std::fstream *>(fd_.get())->gcount();
                        } else {
                            dynamic_cast<std::ifstream *>(fd_.get())->read((std::fstream::char_type *)&res[0], n);
                            rdcnt = dynamic_cast<std::ifstream *>(fd_.get())->gcount();
                        }
                        if(rdcnt != n) {
                            res.resize(rdcnt);
                        }
                    } catch (...) {
                        res.clear();
                    }
                }
                return res;
            }

            std::ios::off_type write(std::vector<std::uint8_t> const &d) {
                if(is_open() && d.size() > 0) {
                    try {
                        if(w_) {
                            dynamic_cast<std::fstream *>(fd_.get())->write((std::fstream::char_type const *)d.data(), d.size());
                        } else {
                            return -1;
                        }
                    } catch (...) {
                        return -1;
                    }
                }
                return d.size();
            }

        private:
            std::shared_ptr<std::basic_ios<char>> fd_{};
            bool w_{false};
        };

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
