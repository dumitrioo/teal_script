#pragma once

#include "include/commondefs.hpp"
#include "include/file_util.hpp"
#include "include/str_util.hpp"
#include "include/base16.hpp"
#include "include/base64.hpp"
#include "include/base85.hpp"
#include "include/dlib.hpp"
#if defined(SCFX_USE_ASYNC_CONSOLE)
#include "include/containers/concurrentqueue.h"
#endif
#include "scaflux_util.hpp"
#include "scaflux_token.hpp"
#include "scaflux_lexer.hpp"
#include "scaflux_expr.hpp"
#include "scaflux_statement.hpp"
#include "scaflux_parser.hpp"
#include "scaflux_lexer.hpp"
#include "scaflux_cells.hpp"
#include "scaflux_codegen.hpp"
#include "scaflux_exec_ctx.hpp"
#include "scaflux_interfaces.hpp"

#include "ext/array_buffer_ext.hpp"
#include "ext/file_ext.hpp"
#include "ext/crypto_ext.hpp"
#include "ext/cpu_ext.hpp"
#include "ext/rand_ext.hpp"
#include "ext/time_ext.hpp"
#include "ext/math_ext.hpp"

#ifdef PLATFORM_WINDOWS
#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */
#endif

namespace scfx {

    namespace detail {

#if defined(SCFX_USE_ASYNC_CONSOLE)
        class console {
        public:
            console():
                out_buffers_{std::make_unique<buff>(), std::make_unique<buff>()},
                out_thread_{
                    [this]() {
                        while(!termination_) {
                            auto opt_pair{fetch_string_from_buffer()};
                            if(opt_pair) {
                                if(opt_pair->second) {
                                    std::cerr << opt_pair->first << std::flush;
                                } else {
                                    std::cout << opt_pair->first << std::flush;
                                }
                            }
                        }
                    }
                }
            {
            }

            ~console() {
                termination_ = true;
                if(out_thread_.joinable()) { out_thread_.join(); }
            }

            void info(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[34minfo\033[0m" : "info"), args);
            }
            void log(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[32mlog\033[0m" : "log"), args);
            }
            void warn(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[35mwarning\033[0m" : "warning"), args);
            }
            void debug(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[93mdebug\033[0m" : "debug"), args);
            }
            void error(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[91merror\033[0m" : "error"), args);
            }

            void print(std::vector<scfx::valbox> const &args) {
                std::stringstream out{};
                if(setfill_) { out << std::setfill(fill_char_); }
                if(setw_) { out << std::setw(w_); }
                if(setprec_) { out << std::setprecision(prec_); }
                if(fk_ != flt_kind::def) {
                    switch(fk_) {
                        case flt_kind::fix: out << std::fixed; break;
                        case flt_kind::sci: out << std::scientific; break;
                        case flt_kind::hex: out << std::hex; break;
                        default: out << std::defaultfloat; break;
                    }
                }
                for(auto &&v: args) { out << v; }
                put_string_to_buffer(out.str(), false);
            }

            void println(std::vector<scfx::valbox> const &args) {
                std::stringstream out{};
                if(setfill_) { out << std::setfill(fill_char_); }
                if(setw_) { out << std::setw(w_); }
                if(setprec_) { out << std::setprecision(prec_); }
                if(fk_ != flt_kind::def) {
                    switch(fk_) {
                        case flt_kind::fix: out << std::fixed; break;
                        case flt_kind::sci: out << std::scientific; break;
                        case flt_kind::hex: out << std::hex; break;
                        default: out << std::defaultfloat; break;
                    }
                }
                for(auto &&v: args) { out << v; }
                out << '\n';
                put_string_to_buffer(out.str(), false);
            }

            void flush() {}
            void fixed() { fk_ = flt_kind::fix; }
            void scientific() { fk_ = flt_kind::sci; }
            void hexfloat() { fk_ = flt_kind::hex; }
            void defaultfloat() { fk_ = flt_kind::def; }
            void setprecision(int prec) { setprec_ = true; prec_ = prec; }
            int precision() { return prec_; }
            void setw(int w) { setw_ = true; w_ = w; }
            void setfill(char arg) { setfill_ = true; fill_char_ = arg; }
            char fill() { return fill_char_; }
            bool colors_enabled() const { return terminal_colours_; }
            void enable_colors(bool v) { terminal_colours_ = v; }

        private:
            void cerr_out(std::string const &type, std::vector<scfx::valbox> const &args) {
                std::stringstream out{};
                if(setfill_) { out << std::setfill(fill_char_); }
                if(setw_) { out << std::setw(w_); }
                if(setprec_) { out << std::setprecision(prec_); }
                if(fk_ != flt_kind::def) {
                    switch(fk_) {
                    case flt_kind::fix: out << std::fixed; break;
                    case flt_kind::sci: out << std::scientific; break;
                    case flt_kind::hex: out << std::hex; break;
                    default: out << std::defaultfloat; break;
                    }
                }
                out << scfx::str_util::from_utf8(scfx::timespec_wrapper::now().as_iso_8601_str()) << " " << type << ": ";
                for(auto &&v: args) {
                    out << v;
                }
                out << '\n';
                put_string_to_buffer(out.str(), true);
            }

            void cout_out(std::string const &type, std::vector<scfx::valbox> const &args) {
                std::stringstream out{};
                if(setfill_) { out << std::setfill(fill_char_); }
                if(setw_) { out << std::setw(w_); }
                if(setprec_) { out << std::setprecision(prec_); }
                if(fk_ != flt_kind::def) {
                    switch(fk_) {
                        case flt_kind::fix: out << std::fixed; break;
                        case flt_kind::sci: out << std::scientific; break;
                        case flt_kind::hex: out << std::hex; break;
                        default: out << std::defaultfloat; break;
                    }
                }
                out << scfx::str_util::from_utf8(scfx::timespec_wrapper::now().as_iso_8601_str()) << " " << type << ": ";
                for(auto &&v: args) {
                    out << v;
                }
                out << '\n';
                put_string_to_buffer(out.str(), false);
            }

            class buff {
            public:
                void put_string(std::string const &s, bool is_err) {
                    out_buffer_.enqueue({s, is_err});
                }

                std::optional<std::pair<std::string, bool>> fetch_string() {
                    std::pair<std::string, bool> res{};
                    if(out_buffer_.try_dequeue(res)) {
                        return res;
                    }
                    return {};
                }

                std::size_t size() const {
                    return out_buffer_.size_approx();
                }

            private:
                moodycamel::ConcurrentQueue<std::pair<std::string, bool>> out_buffer_{};
            };

            void put_string_to_buffer(std::string const &s, bool is_err) {
                {
                    std::shared_lock l{out_index_mtp_};
                    out_buffers_[(out_index_.load(std::memory_order::acquire) + 1) % 2]->put_string(s, is_err);
                }
                {
                    std::unique_lock l{out_mtp_};
                    out_cvar_.notify_one();
                }
            }

            std::optional<std::pair<std::string, bool>> fetch_string_from_buffer() {
                std::optional<std::pair<std::string, bool>> res{out_buffers_[out_index_.load(std::memory_order::acquire) % 2]->fetch_string()};
                if(res) {
                    return res;
                } else {
                    bool need_switch_index{true};
                    std::unique_lock l{out_mtp_};
                    if(out_buffers_[(out_index_.load(std::memory_order::acquire) + 1) % 2]->size() == 0) {
                        std::cv_status wst{out_cvar_.wait_for(l, std::chrono::milliseconds{100})};
                        if(wst == std::cv_status::timeout) {
                            need_switch_index = false;
                        }
                    }
                    if(need_switch_index) {
                        std::unique_lock l{out_index_mtp_};
                        out_index_ = (out_index_ + 1) % 2;
                        out_index_switches_++;
                    }
                }
                return out_buffers_[out_index_.load(std::memory_order::acquire) % 2]->fetch_string();
            }

            std::mutex out_mtp_{};
            std::condition_variable out_cvar_{};
            std::atomic<std::size_t> out_index_switches_{0};
            mutable shared_mutex out_index_mtp_{};
            std::atomic<std::size_t> out_index_{0};
            std::array<std::unique_ptr<buff>, 2> out_buffers_{};
            std::thread out_thread_{};

            bool setfill_{false};
            char fill_char_{};
            bool setw_{false};
            int w_{};
            bool setprec_{false};
            int prec_{};
            enum class flt_kind{def, sci, fix, hex};
            flt_kind fk_{flt_kind::def};
            bool terminal_colours_{false};
            bool termination_{false};
        };
#else
        class console {
        public:
            void info(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[34minfo\033[0m" : "info"), args);
            }
            void log(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[32mlog\033[0m" : "log"), args);
            }
            void warn(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[35mwarning\033[0m" : "warning"), args);
            }
            void debug(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[93mdebug\033[0m" : "debug"), args);
            }
            void error(std::vector<scfx::valbox> const &args) {
                cout_out((terminal_colours_ ? "\033[91merror\033[0m" : "error"), args);
            }

            void print(std::vector<scfx::valbox> const &args) {
                std::stringstream out{};
                if(setfill_) { out << std::setfill(fill_char_); }
                if(setw_) { out << std::setw(w_); }
                if(setprec_) { out << std::setprecision(prec_); }
                if(fk_ != flt_kind::def) {
                    switch(fk_) {
                        case flt_kind::fix: out << std::fixed; break;
                        case flt_kind::sci: out << std::scientific; break;
                        case flt_kind::hex: out << std::hex; break;
                        default: out << std::defaultfloat; break;
                    }
                }
                for(auto &&v: args) { out << v; }
                std::unique_lock l{out_mtp_};
                std::cout << out.str() << std::flush;
            }

            void println(std::vector<scfx::valbox> const &args) {
                std::stringstream out{};
                if(setfill_) { out << std::setfill(fill_char_); }
                if(setw_) { out << std::setw(w_); }
                if(setprec_) { out << std::setprecision(prec_); }
                if(fk_ != flt_kind::def) {
                    switch(fk_) {
                        case flt_kind::fix: out << std::fixed; break;
                        case flt_kind::sci: out << std::scientific; break;
                        case flt_kind::hex: out << std::hex; break;
                        default: out << std::defaultfloat; break;
                    }
                }
                for(auto &&v: args) { out << v; }
                std::unique_lock l{out_mtp_};
                std::cout << out.str() << std::endl;
            }

            void flush() {
                std::unique_lock l{out_mtp_};
                std::cout.flush();
            }
            void fixed() { fk_ = flt_kind::fix; }
            void scientific() { fk_ = flt_kind::sci; }
            void hexfloat() { fk_ = flt_kind::hex; }
            void defaultfloat() { fk_ = flt_kind::def; }
            void setprecision(int prec) { setprec_ = true; prec_ = prec; }
            int precision() { return prec_; }
            void setw(int w) { setw_ = true; w_ = w; }
            void setfill(char arg) { setfill_ = true; fill_char_ = arg; }
            char fill() { return fill_char_; }
            bool colors_enabled() const { return terminal_colours_; }
            void enable_colors(bool v) { terminal_colours_ = v; }

        private:
            void cerr_out(std::string const &type, std::vector<scfx::valbox> const &args) {
                std::stringstream out{};
                if(setfill_) { out << std::setfill(fill_char_); }
                if(setw_) { out << std::setw(w_); }
                if(setprec_) { out << std::setprecision(prec_); }
                if(fk_ != flt_kind::def) {
                    switch(fk_) {
                    case flt_kind::fix: out << std::fixed; break;
                    case flt_kind::sci: out << std::scientific; break;
                    case flt_kind::hex: out << std::hex; break;
                    default: out << std::defaultfloat; break;
                    }
                }
                out << scfx::str_util::from_utf8(scfx::timespec_wrapper::now().as_iso_8601_str()) << " " << type << ": ";
                for(auto &&v: args) {
                    out << v;
                }
                std::unique_lock l{out_mtp_};
                std::cerr << out.str() << std::endl;
            }

            void cout_out(std::string const &type, std::vector<scfx::valbox> const &args) {
                std::stringstream out{};
                if(setfill_) { out << std::setfill(fill_char_); }
                if(setw_) { out << std::setw(w_); }
                if(setprec_) { out << std::setprecision(prec_); }
                if(fk_ != flt_kind::def) {
                    switch(fk_) {
                        case flt_kind::fix: out << std::fixed; break;
                        case flt_kind::sci: out << std::scientific; break;
                        case flt_kind::hex: out << std::hex; break;
                        default: out << std::defaultfloat; break;
                    }
                }
                out << scfx::str_util::from_utf8(scfx::timespec_wrapper::now().as_iso_8601_str()) << " " << type << ": ";
                for(auto &&v: args) {
                    out << v;
                }
                std::unique_lock l{out_mtp_};
                std::cout << out.str() << std::endl;
            }

        private:
            bool setfill_{false};
            char fill_char_{};
            bool setw_{false};
            int w_{};
            bool setprec_{false};
            int prec_{};
            shared_mutex out_mtp_{};
            enum class flt_kind{def, sci, fix, hex};
            flt_kind fk_{flt_kind::def};
            bool terminal_colours_{false};
        };
#endif
    }


    class runtime: public runtime_interface {
    public:
        runtime() {
            exctx_.set_runtime_interface(this);

            add_function("version_major", SCFXFUN(/*args*/) { return version_major_; });
            add_function("version_minor", SCFXFUN(/*args*/) { return version_minor_; });
            add_function("version_patch", SCFXFUN(/*args*/) { return version_patch_; });
            add_function("version_string", SCFXFUN(/*args*/) {
                return str_util::utoa(version_major_) + "." +
                       str_util::utoa(version_minor_) + "." +
                       str_util::utoa(version_patch_);
            });

            add_var("EPERM", EPERM);     // 1   Operation not permitted
            add_var("ENOENT", ENOENT);   // 2   No such file or directory
            add_var("ESRCH", ESRCH);     // 3   No such process
            add_var("EINTR", EINTR);     // 4   Interrupted system call
            add_var("EIO", EIO);         // 5   I/O error
            add_var("ENXIO", ENXIO);     // 6   No such device or address
            add_var("E2BIG", E2BIG);     // 7   Argument list too long
            add_var("ENOEXEC", ENOEXEC); // 8   Exec format error
            add_var("EBADF", EBADF);     // 9   Bad file number
            add_var("ECHILD", ECHILD);   // 10  No child processes
            add_var("EAGAIN", EAGAIN);   // 11  Try again
            add_var("ENOMEM", ENOMEM);   // 12  Out of memory
            add_var("EACCES", EACCES);   // 13  Permission denied
            add_var("EFAULT", EFAULT);   // 14  Bad address
            add_var("ENOTBLK", ENOTBLK); // 15  Block device required
            add_var("EBUSY", EBUSY);     // 16  Device or resource busy
            add_var("EEXIST", EEXIST);   // 17  File exists
            add_var("EXDEV", EXDEV);     // 18  Cross-device link
            add_var("ENODEV", ENODEV);   // 19  No such device
            add_var("ENOTDIR", ENOTDIR); // 20  Not a directory
            add_var("EISDIR", EISDIR);   // 21  Is a directory
            add_var("EINVAL", EINVAL);   // 22  Invalid argument
            add_var("ENFILE", ENFILE);   // 23  File table overflow
            add_var("EMFILE", EMFILE);   // 24  Too many open files
            add_var("ENOTTY", ENOTTY);   // 25  Not a typewriter
            add_var("ETXTBSY", ETXTBSY); // 26  Text file busy
            add_var("EFBIG", EFBIG);     // 27  File too large
            add_var("ENOSPC", ENOSPC);   // 28  No space left on device
            add_var("ESPIPE", ESPIPE);   // 29  Illegal seek
            add_var("EROFS", EROFS);     // 30  Read-only file system
            add_var("EMLINK", EMLINK);   // 31  Too many links
            add_var("EPIPE", EPIPE);     // 32  Broken pipe
            add_var("EDOM", EDOM);       // 33  Math argument out of domain of func
            add_var("ERANGE", ERANGE);   // 34  Math result not representable

            add_function("execute", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return std::system(args[0].cast_to_string().c_str());
            });
            add_function("load_from_file", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                auto fd{scfx::file_util::load_from_file(args[0].cast_to_string())};
                return std::string{fd.begin(), fd.end()};
            });
            add_function("save_to_file", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                return scfx::file_util::save_to_file(args[0].cast_to_string(), args[1].cast_to_string());
            });
            add_function("delete_filesystem_entry", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return scfx::file_util::delete_fs_entry(args[0].cast_to_string());
            });
            add_function("extract_file_dir", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return scfx::file_util::extract_file_dir(args[0].cast_to_string());
            });
            add_function("extract_file_name", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return scfx::file_util::extract_file_name(args[0].cast_to_string());
            });
            add_function("extract_file_ext", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return scfx::file_util::extract_file_ext(args[0].cast_to_string());
            });

            add_function("file_exists", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return scfx::file_util::file_exists(args[0].cast_to_string());
            });
            add_function("dir_exists", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return scfx::file_util::dir_exists(args[0].cast_to_string());
            });

            add_function("native_path_seperator", SCFXFUN() {
                return file_util::native_path_separator<std::string>{}.sym();
            });

            add_function("native_path_seperator_str", SCFXFUN() {
                return file_util::native_path_separator<std::string>{}.val();
            });

            add_function("temp_directory_path", SCFXFUN() {
                return std::filesystem::temp_directory_path().string();
            });

            add_function("list_directory", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2)
                bool recur{false};
                if(args.size() >= 2) {
                    recur = args[1].cast_to_bool();
                }
                valbox names{valbox_no_initialize::dont_do_it};
                names.become_array();
                scfx::file_util::for_dir_tree(
                    args[0].cast_to_string(),
                    [&](scfx::file_util::dir_entry const &de) {
                        names.as_array().push_back(de.full_path());
                        return true;
                    },
                    recur
                );
                return names;
            });

#ifdef USE_FILE_MAGIC
            add_function("data_type", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2);
                std::string res{};
                if(args[0].is_string_ref()) {
                    if(args.size() == 1) {
                        res = scfx::file_util::data_type(args[0].as_string());
                    } else {
                        res = scfx::file_util::data_type(args[0].as_string(), args[1].cast_to_s32());
                    }
                } else if(args[0].is_array_ref()) {
                    if(args.size() == 1) {
                        res = scfx::file_util::data_type(args[0].cast_to_string());
                    } else {
                        res = scfx::file_util::data_type(args[0].cast_to_string(), args[1].cast_to_s32());
                    }
                }
                return res;
            });
            add_function("file_type", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2);
                if(args.size() == 1) {
                    return scfx::file_util::file_type(args[0].cast_to_string());
                } else {
                    return scfx::file_util::file_type(args[0].cast_to_string(), args[1].cast_to_s32());
                }
            });
            add_var("MAGIC_NONE", MAGIC_NONE);
            add_var("MAGIC_DEBUG", MAGIC_DEBUG);
            add_var("MAGIC_SYMLINK", MAGIC_SYMLINK);
            add_var("MAGIC_COMPRESS", MAGIC_COMPRESS);
            add_var("MAGIC_DEVICES", MAGIC_DEVICES);
            add_var("MAGIC_MIME_TYPE", MAGIC_MIME_TYPE);
            add_var("MAGIC_CONTINUE", MAGIC_CONTINUE);
            add_var("MAGIC_CHECK", MAGIC_CHECK);
            add_var("MAGIC_PRESERVE_ATIME", MAGIC_PRESERVE_ATIME);
            add_var("MAGIC_RAW", MAGIC_RAW);
            add_var("MAGIC_ERROR", MAGIC_ERROR);
            add_var("MAGIC_MIME_ENCODING", MAGIC_MIME_ENCODING);
            add_var("MAGIC_MIME", MAGIC_MIME);
            add_var("MAGIC_APPLE", MAGIC_APPLE);
            add_var("MAGIC_EXTENSION", MAGIC_EXTENSION);
#endif
            add_function("errno", SCFXFUN() { return errno; });
            add_function("err_to_str", SCFXFUN(args) { return std::string{strerror(args[0].cast_num_to_num<int>())}; });

            array_buffer_ext_.register_runtime(this);
            math_ext_.register_runtime(this);
            time_ext_.register_runtime(this);
            crypt_.register_runtime(this);
            fpool_.register_runtime(this);
            perf_stat_.register_runtime(this);
            randlib_.register_runtime(this);

            add_var("console", scfx::valbox{&con_, "console"});
            add_method("console", "info", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_GE(args, 1) std::vector<scfx::valbox> args1{args.begin() + 1, args.end()}; SCFXTHIS(args, detail::console *)->info(args1); return {valbox_no_initialize::dont_do_it}; });
            add_method("console", "log", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_GE(args, 1) std::vector<scfx::valbox> args1{args.begin() + 1, args.end()}; SCFXTHIS(args, detail::console *)->log(args1); return {valbox_no_initialize::dont_do_it}; });
            add_method("console", "warn", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_GE(args, 1) std::vector<scfx::valbox> args1{args.begin() + 1, args.end()}; SCFXTHIS(args, detail::console *)->warn(args1); return {valbox_no_initialize::dont_do_it}; });
            add_method("console", "debug", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_GE(args, 1) std::vector<scfx::valbox> args1{args.begin() + 1, args.end()}; SCFXTHIS(args, detail::console *)->debug(args1); return {valbox_no_initialize::dont_do_it}; });
            add_method("console", "error", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_GE(args, 1) std::vector<scfx::valbox> args1{args.begin() + 1, args.end()}; SCFXTHIS(args, detail::console *)->error(args1); return {valbox_no_initialize::dont_do_it}; });
            add_method("console", "print", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_GE(args, 1) std::vector<scfx::valbox> args1{args.begin() + 1, args.end()}; SCFXTHIS(args, detail::console *)->print(args1); return {valbox_no_initialize::dont_do_it}; });
            add_method("console", "println", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_GE(args, 1) std::vector<scfx::valbox> args1{args.begin() + 1, args.end()}; SCFXTHIS(args, detail::console *)->println(args1); return {valbox_no_initialize::dont_do_it}; });
            add_method("console", "flush", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, detail::console *)->flush(); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "fixed", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, detail::console *)->fixed(); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "scientific", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, detail::console *)->scientific(); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "hexfloat", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, detail::console *)->hexfloat(); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "defaultfloat", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, detail::console *)->defaultfloat(); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "setprecision", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, detail::console *)->setprecision(args[1].cast_to_u64()); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "precision", SCFXFUN(args) { return SCFXTHIS(args, detail::console *)->precision(); });
            add_method("console", "setw", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, detail::console *)->setw(args[1].cast_to_u64()); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "setfill", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, detail::console *)->setfill(args[1].cast_to_char()); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "fill", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, detail::console *)->fill(); });
            add_method("console", "enable_colors", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, detail::console *)->enable_colors(args[1].cast_to_bool()); return scfx::valbox{valbox_no_initialize::dont_do_it}; });
            add_method("console", "colors_enabled", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, detail::console *)->colors_enabled(); });
            add_function("println", SCFXFUN(args) { con_.println(args); return {valbox_no_initialize::dont_do_it}; });
            add_function("print", SCFXFUN(args) { con_.print(args); return {valbox_no_initialize::dont_do_it}; });


            add_function("typeof", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                valbox::type t{args[0].deref().val_type()};
                return t == valbox::type::CLASS ? args[0].ref_class_name() : valbox::type_to_str(t);
            });

            add_function("is_i64", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_s64_ref(); });
            add_function("is_u64", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_u64_ref(); });
            add_function("is_i32", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_s32_ref(); });
            add_function("is_u32", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_u32_ref(); });
            add_function("is_i16", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_s16_ref(); });
            add_function("is_u16", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_u16_ref(); });
            add_function("is_i8", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_s8_ref(); });
            add_function("is_u8", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_u8_ref(); });
            add_function("is_f32", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_float_ref(); });
            add_function("is_f64", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_double_ref(); });
            add_function("is_float", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_long_double_ref(); });
            add_function("is_char", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_char_ref(); });
            add_function("is_wchar", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_wchar_ref(); });
            add_function("is_bool", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_bool_ref(); });
            add_function("is_string", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_string_ref(); });
            add_function("is_wstring", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_wstring_ref(); });
            add_function("is_mat4", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_mat4_ref(); });
            add_function("is_vec4", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_vec4_ref(); });
            add_function("is_array", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_array_ref(); });
            add_function("is_object", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return args[0].is_object_ref(); });

            add_function("hton", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_char_ref()) { return args[0].as_char(); }
                if(args[0].is_bool_ref()) { return args[0].as_bool(); }
                if(args[0].is_s64_ref()) { return bit_util::swap_on_le<int64_t>{args[0].as_s64()}.val; }
                if(args[0].is_u64_ref()) { return bit_util::swap_on_le<uint64_t>{args[0].as_u64()}.val; }
                if(args[0].is_s32_ref()) { return bit_util::swap_on_le<int32_t>{args[0].as_s32()}.val; }
                if(args[0].is_u32_ref()) { return bit_util::swap_on_le<uint64_t>{args[0].as_u32()}.val; }
                if(args[0].is_s16_ref()) { return bit_util::swap_on_le<int16_t>{args[0].as_s16()}.val; }
                if(args[0].is_u16_ref()) { return bit_util::swap_on_le<uint16_t>{args[0].as_u16()}.val; }
                if(args[0].is_s8_ref()) { return bit_util::swap_on_le<int8_t>{args[0].as_s8()}.val; }
                if(args[0].is_u8_ref()) { return bit_util::swap_on_le<uint8_t>{args[0].as_u8()}.val; }
                if(args[0].is_float_ref()) { return bit_util::swap_on_le<float>{args[0].as_float()}.val; }
                if(args[0].is_double_ref()) { return bit_util::swap_on_le<double>{args[0].as_double()}.val; }
                if(args[0].is_long_double_ref()) { return bit_util::swap_on_le<long double>{args[0].as_long_double()}.val; }
                if(args[0].is_wchar_ref()) { return bit_util::swap_on_le<wchar_t>{args[0].as_wchar()}.val; }
                throw std::runtime_error{"invalid argument type"};
            });
            add_function("ntoh", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_char_ref()) { return args[0].as_char(); }
                if(args[0].is_bool_ref()) { return args[0].as_bool(); }
                if(args[0].is_s64_ref()) { return bit_util::swap_on_le<int64_t>{args[0].as_s64()}.val; }
                if(args[0].is_u64_ref()) { return bit_util::swap_on_le<uint64_t>{args[0].as_u64()}.val; }
                if(args[0].is_s32_ref()) { return bit_util::swap_on_le<int32_t>{args[0].as_s32()}.val; }
                if(args[0].is_u32_ref()) { return bit_util::swap_on_le<uint64_t>{args[0].as_u32()}.val; }
                if(args[0].is_s16_ref()) { return bit_util::swap_on_le<int16_t>{args[0].as_s16()}.val; }
                if(args[0].is_u16_ref()) { return bit_util::swap_on_le<uint16_t>{args[0].as_u16()}.val; }
                if(args[0].is_s8_ref()) { return bit_util::swap_on_le<int8_t>{args[0].as_s8()}.val; }
                if(args[0].is_u8_ref()) { return bit_util::swap_on_le<uint8_t>{args[0].as_u8()}.val; }
                if(args[0].is_float_ref()) { return bit_util::swap_on_le<float>{args[0].as_float()}.val; }
                if(args[0].is_double_ref()) { return bit_util::swap_on_le<double>{args[0].as_double()}.val; }
                if(args[0].is_long_double_ref()) { return bit_util::swap_on_le<long double>{args[0].as_long_double()}.val; }
                if(args[0].is_wchar_ref()) { return bit_util::swap_on_le<wchar_t>{args[0].as_wchar()}.val; }
                throw std::runtime_error{"invalid argument type"};
            });

            add_function("tole", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_char_ref()) { return args[0].as_char(); }
                if(args[0].is_bool_ref()) { return args[0].as_bool(); }
                if(args[0].is_s64_ref()) { return bit_util::swap_on_be<int64_t>{args[0].as_s64()}.val; }
                if(args[0].is_u64_ref()) { return bit_util::swap_on_be<uint64_t>{args[0].as_u64()}.val; }
                if(args[0].is_s32_ref()) { return bit_util::swap_on_be<int32_t>{args[0].as_s32()}.val; }
                if(args[0].is_u32_ref()) { return bit_util::swap_on_be<uint64_t>{args[0].as_u32()}.val; }
                if(args[0].is_s16_ref()) { return bit_util::swap_on_be<int16_t>{args[0].as_s16()}.val; }
                if(args[0].is_u16_ref()) { return bit_util::swap_on_be<uint16_t>{args[0].as_u16()}.val; }
                if(args[0].is_s8_ref()) { return bit_util::swap_on_be<int8_t>{args[0].as_s8()}.val; }
                if(args[0].is_u8_ref()) { return bit_util::swap_on_be<uint8_t>{args[0].as_u8()}.val; }
                if(args[0].is_float_ref()) { return bit_util::swap_on_be<float>{args[0].as_float()}.val; }
                if(args[0].is_double_ref()) { return bit_util::swap_on_be<double>{args[0].as_double()}.val; }
                if(args[0].is_long_double_ref()) { return bit_util::swap_on_be<long double>{args[0].as_long_double()}.val; }
                if(args[0].is_wchar_ref()) { return bit_util::swap_on_be<wchar_t>{args[0].as_wchar()}.val; }
                throw std::runtime_error{"invalid argument type"};
            });
            add_function("tobe", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_char_ref()) { return args[0].as_char(); }
                if(args[0].is_bool_ref()) { return args[0].as_bool(); }
                if(args[0].is_s64_ref()) { return bit_util::swap_on_le<int64_t>{args[0].as_s64()}.val; }
                if(args[0].is_u64_ref()) { return bit_util::swap_on_le<uint64_t>{args[0].as_u64()}.val; }
                if(args[0].is_s32_ref()) { return bit_util::swap_on_le<int32_t>{args[0].as_s32()}.val; }
                if(args[0].is_u32_ref()) { return bit_util::swap_on_le<uint64_t>{args[0].as_u32()}.val; }
                if(args[0].is_s16_ref()) { return bit_util::swap_on_le<int16_t>{args[0].as_s16()}.val; }
                if(args[0].is_u16_ref()) { return bit_util::swap_on_le<uint16_t>{args[0].as_u16()}.val; }
                if(args[0].is_s8_ref()) { return bit_util::swap_on_le<int8_t>{args[0].as_s8()}.val; }
                if(args[0].is_u8_ref()) { return bit_util::swap_on_le<uint8_t>{args[0].as_u8()}.val; }
                if(args[0].is_float_ref()) { return bit_util::swap_on_le<float>{args[0].as_float()}.val; }
                if(args[0].is_double_ref()) { return bit_util::swap_on_le<double>{args[0].as_double()}.val; }
                if(args[0].is_long_double_ref()) { return bit_util::swap_on_le<long double>{args[0].as_long_double()}.val; }
                if(args[0].is_wchar_ref()) { return bit_util::swap_on_le<wchar_t>{args[0].as_wchar()}.val; }
                throw std::runtime_error{"invalid argument type"};
            });

            add_function("i64", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (std::int64_t)0; } return args[0].cast_to_s64(); });
            add_function("u64", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (std::uint64_t)0; } return args[0].cast_to_u64(); });
            add_function("i32", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (std::int32_t)0; } return args[0].cast_to_s32(); });
            add_function("u32", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (std::uint32_t)0; } return args[0].cast_to_u32(); });
            add_function("i16", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (std::int16_t)0; } return args[0].cast_to_s16(); });
            add_function("u16", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (std::uint16_t)0; } return args[0].cast_to_u16(); });
            add_function("i8", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (std::int8_t)0; } return args[0].cast_to_s8(); });
            add_function("u8", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (std::uint8_t)0; } return args[0].cast_to_u8(); });
            add_function("f32", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (float)0; } return args[0].cast_to_float(); });
            add_function("f64", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (double)0; } return args[0].cast_to_double(); });
            add_function("float", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return (long double)0; } return args[0].cast_num_to_num<long double>(); });
            add_function("char", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return char{}; } return args[0].cast_to_char(); });
            add_function("wchar", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return wchar_t{}; } return args[0].cast_to_wchar(); });
            add_function("bool", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return false; } return args[0].cast_to_bool(); });
            add_function("string", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return std::string{}; } return args[0].cast_to_string(); });
            add_function("wstring", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_LE(args, 1) if(args.empty()) { return std::wstring{}; } return args[0].cast_to_wstring(); });
            add_function("vec4", SCFXFUN(args) {
                valbox res{valbox::vec4_t{0, 0, 0, 1}};
                if(args.size() == 1) {
                    if(args[0].is_array_ref() || args[0].is_object_ref()) {
                        res = vec4_from_json<long double>(args[0].to_json());
                    } else if(args[0].is_string_ref() || args[0].is_wstring_ref()) {
                        if(args[0].is_string_ref()) {
                            res = vec4_from_str<long double>(args[0].as_string());
                        } else {
                            res = vec4_from_str<long double>(args[0].as_wstring());
                        }
                    } else {
                        res.as_vec4()[0] = args[0].cast_to_long_double();
                    }
                } else {
                    for(std::size_t i = 0; i < 4 && i < args.size(); ++i) {
                        res.as_vec4()[i] = args[i].cast_to_long_double();
                    }
                }
                return res;
            });
            add_function("mat4", SCFXFUN(args) {
                valbox res{valbox::mat4_t::identity()};
                if(args.size() == 16) {
                    for(std::size_t i{0}; i < 16 && i < args.size(); ++i) {
                        res.as_mat4().at_flat_index(i) = args[i].cast_to_long_double();
                    }
                } else if(args.size() < 16) {
                    std::size_t row_no{0};
                    for(std::size_t i{0}; row_no < 4 && i < args.size(); ++i) {
                        if(args[i].is_array_ref()) {
                            for(std::size_t j{0}; j < 4 && j < args[i].as_array().size(); ++j) {
                                res.as_mat4()[row_no][j] = args[i].as_array()[j].cast_to_long_double();
                            }
                        } else if(args[i].is_vec4_ref()) {
                            for(std::size_t j{0}; j < 4; ++j) {
                                res.as_mat4()[row_no][j] = args[i].as_vec4()[j];
                            }
                        } else {
                            for(std::size_t j{0}; j < 4 && i < args.size(); ++j) {
                                if(args[i].is_any_fp_number() || args[i].is_any_int_number()) {
                                    res.as_mat4()[row_no][j] = args[i].cast_to_long_double();
                                    ++i;
                                } else {
                                    break;
                                }
                            }
                        }
                        ++row_no;
                    }
                }
                return res;
            });
            add_function("array", SCFXFUN(args) {
                valbox res{valbox_no_initialize::dont_do_it};
                res.become_array();
                if(args.size() == 1 && (args[0].is_array_ref())) {
                    res.assign(args[0]);
                } else if(args.size() == 1 && (args[0].is_string_ref() || args[0].is_wstring_ref())) {
                    res.construct(args[0].cast_to_array());
                } else {
                    for(std::size_t i = 0; i < args.size(); ++i) {
                        res.as_array().push_back(args[i]);
                    }
                }
                return res;
            });
            add_function("object", SCFXFUN(args) {
                if(args.empty()) {
                    return valbox{valbox_no_initialize::dont_do_it}.become_object();
                }
                if(args[0].is_object_ref()) {
                    return args[0];
                }
                valbox res{valbox_no_initialize::dont_do_it};
                res.become_object();
                if(args[0].is_string_ref()) {
                    res.from_json(scfx::json::deserialize(args[0].as_string()));
                    return res;
                } else if(args[0].is_wstring_ref()) {
                    return scfx::json::deserialize(args[0].as_wstring());
                } else if(args[0].is_any_int_number()) {
                    return scfx::json{args[0].cast_num_to_num<int64_t>()};
                } else if(args[0].is_any_fp_number()) {
                    return scfx::json{args[0].cast_num_to_num<long double>()};
                } else if(args[0].is_bool_ref()) {
                    return scfx::json{args[0].cast_num_to_num<long double>()};
                }
                return res;
            });
            add_function("deserialize", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                valbox res{valbox_no_initialize::dont_do_it};
                if(args[0].is_string_ref()) {
                    res.from_json(scfx::json::deserialize(args[0].as_string()));
                    return res;
                } else if(args[0].is_wstring_ref()) {
                    res.from_json(scfx::json::deserialize(args[0].as_wstring()));
                }
                return res;
            });
            add_function("serialize", SCFXFUN(args) {
                valbox res{valbox_no_initialize::dont_do_it};
                if(args.size() == 1) {
                    res = args[0].to_json().serialize();
                } else if(args.size() == 2) {
                    res = args[0].to_json().serialize(args[1].cast_to_u64());
                }
                return res;
            });
            add_function("serialize5", SCFXFUN(args) {
                valbox res{valbox_no_initialize::dont_do_it};
                if(args.size() == 1) {
                    res = args[0].to_json().serialize5();
                } else if(args.size() == 2) {
                    res = args[0].to_json().serialize5(args[1].cast_to_u64());
                }
                return res;
            });

            add_function("contains", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                auto a1r{args[0].deref()};
                if(a1r.is_object_ref()) {
                    return a1r.as_object().find(args[1].cast_to_string()) != a1r.as_object().end();
                } else if(a1r.is_array_ref()) {
                    auto idx{args[1].cast_to_u64()};
                    return idx < a1r.as_array().size();
                }
                return false;
            });

            add_function("key_exists", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                auto a1r{args[0].deref()};
                if(a1r.is_object_ref()) {
                    return a1r.as_object().find(args[1].cast_to_string()) != a1r.as_object().end();
                }
                throw std::runtime_error{"not object"};
            });
            add_function("string_field_exists", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                auto a1r{args[0].deref()};
                if(a1r.is_object_ref()) {
                    return a1r.as_object().find(args[1].cast_to_string()) != a1r.as_object().end() &&
                           a1r.as_object().at(args[1].cast_to_string()).is_string_ref();
                }
                throw std::runtime_error{"not object"};
            });

            add_function("key_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                auto a1r{args[0].deref()};
                if(a1r.is_object_ref()) {
                    return a1r.object_key_at(args[1].cast_to_u64());
                }
                throw std::runtime_error{"not object"};
            });

            add_function("value_at", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                auto a1r{args[0].deref()};
                if(a1r.is_object_ref()) {
                    return a1r.object_value_at(args[1].cast_to_u64());
                }
                throw std::runtime_error{"not object"};
            });

            add_function("resize", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox &a1r{args[0].deref()};
                if(a1r.is_array_ref()) {
                    auto &vec{a1r.as_array()};
                    vec.resize(args[1].cast_to_u64());
                    return true;
                } else if(a1r.is_string_ref()) {
                    auto &str{a1r.as_string()};
                    str.resize(args[1].cast_to_u64());
                    return true;
                } else if(a1r.is_wstring_ref()) {
                    auto &str{a1r.as_wstring()};
                    str.resize(args[1].cast_to_u64());
                    return true;
                }
                return false;
            });
            add_function("push_back", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox &a1r{args[0].deref()};
                if(a1r.is_undefined_ref()) {
                    a1r.become_array();
                }
                a1r.as_array().push_back(args[1].clone());
                return args[0];
            });

#if 1
            add_function("push_front", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox &a1r{args[0].deref()};
                if(a1r.is_undefined_ref()) {
                    a1r.become_array();
                }
                a1r.as_array().insert(a1r.as_array().begin(), args[1].clone());
                return args[0];
            });
            add_function("pop_front", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                valbox &a1r{args[0].deref()};
                if(a1r.is_array_ref()) {
                    if(a1r.as_array().empty()) {
                        throw std::runtime_error{"array empty"};
                    }
                    valbox res{a1r.as_array().front()};
                    auto &vec{a1r.as_array()};
                    vec.erase(vec.begin());
                    return res;
                }
                throw std::runtime_error{"not array"};
            });
#endif

            add_function("pop_back", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                valbox &a1r{args[0].deref()};
                if(a1r.is_array_ref()) {
                    if(a1r.as_array().empty()) {
                        throw std::runtime_error{"array empty"};
                    }
                    valbox res{a1r.as_array().back()};
                    a1r.as_array().pop_back();
                    return res;
                }
                throw std::runtime_error{"not array"};
            });

            add_function("undefine", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                args[0].become_undefined();
                return args[0];
            });

            add_function("replace_substr", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                if(args.size() == 2) {
                    return scfx::str_util::to_utf8(scfx::str_util::replace_substring<std::wstring>(
                        args[0].cast_to_wstring(), args[1].cast_to_wstring(), std::wstring{}
                    ));
                }
                if(args.size() == 3) {
                    return scfx::str_util::to_utf8(scfx::str_util::replace_substring<std::wstring>(
                        args[0].cast_to_wstring(), args[1].cast_to_wstring(), args[2].cast_to_wstring()
                    ));
                }
                return std::string{};
            });


            add_function("substr", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                if(args.size() == 1) {
                    return args[0].cast_to_string();
                } else if(args.size() == 2) {
                    std::string s{args[0].cast_to_string()};
                    std::size_t from{args[1].cast_to_u64()};
                    return s.substr(from);
                } else if(args.size() == 3) {
                    std::string s{args[0].cast_to_string()};
                    std::size_t from{args[1].cast_to_u64()};
                    std::size_t num{args[2].cast_to_u64()};
                    return s.substr(from, num);
                } else if(args.size() > 3) {
                    return args[0].cast_to_string();
                }
                return std::string{};
            });

            add_function("slice", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                if(args[0].is_array_ref()) {
                    if(args.size() == 1) {
                        return args[0];
                    } else if(args.size() == 2) {
                        valbox res{valbox_no_initialize::dont_do_it};
                        res.become_array();
                        res = args[0].subarray(args[1].cast_to_u64());
                        return res;
                    } else if(args.size() == 3) {
                        valbox res{valbox_no_initialize::dont_do_it};
                        res.become_array();
                        res = args[0].subarray(args[1].cast_to_u64(), args[2].cast_to_u64());
                        return res;
                    }
                } else if(args[0].is_string_ref()) {
                    if(args.size() == 1) {
                        return args[0];
                    } else if(args.size() == 2) {
                        return args[0].as_string().substr(args[1].cast_to_u64());
                    } else if(args.size() == 3) {
                        return args[0].as_string().substr(args[1].cast_to_u64(), args[2].cast_to_u64());
                    } else if(args.size() > 3) {
                        return args[0].cast_to_string();
                    }
                } else if(args[0].is_wstring_ref()) {
                    if(args.size() == 1) {
                        return args[0];
                    } else if(args.size() == 2) {
                        return args[0].as_wstring().substr(args[1].cast_to_u64());
                    } else if(args.size() == 3) {
                        return args[0].as_wstring().substr(args[1].cast_to_u64(), args[2].cast_to_u64());
                    } else if(args.size() > 3) {
                        return args[0].cast_to_wstring();
                    }
                }
                return valbox{valbox_no_initialize::dont_do_it};
            });

            add_function("str_tok", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{valbox_no_initialize::dont_do_it};
                res.become_array();
                if(args[0].is_string_ref()) {
                    std::vector<std::string> sv{scfx::str_util::str_tok(args[0].cast_to_string(), args[1].cast_to_string())};
                    for(auto &&s: sv) {
                        res.as_array().push_back(s);
                    }
                } else if(args[0].is_wstring_ref()) {
                    std::vector<std::wstring> sv{scfx::str_util::str_tok(args[0].cast_to_wstring(), args[1].cast_to_wstring())};
                    for(auto &&s: sv) {
                        res.as_array().push_back(s);
                    }
                }
                return res;
            });

            add_function("ltrim", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string_ref()) {
                    return scfx::str_util::ltrim<std::string>(args[0].as_string());
                } else if(args[0].is_wstring_ref()) {
                    return scfx::str_util::ltrim<std::wstring>(args[0].as_wstring());
                } else {
                    return args[0];
                }
            });

            add_function("rtrim", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string_ref()) {
                    return scfx::str_util::rtrim<std::string>(args[0].as_string());
                } else if(args[0].is_wstring_ref()) {
                    return scfx::str_util::rtrim<std::wstring>(args[0].as_wstring());
                } else {
                    return args[0];
                }
            });
            add_function("trim", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string_ref()) {
                    return scfx::str_util::trim<std::string>(args[0].as_string());
                } else if(args[0].is_wstring_ref()) {
                    return scfx::str_util::trim<std::wstring>(args[0].as_wstring());
                } else {
                    return args[0];
                }
            });

            add_function("subarray", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                if(args[0].is_array_ref()) {
                    if(args.size() == 1) {
                        return args[0].subarray();
                    } else if(args.size() == 2) {
                        return args[0].subarray(args[1].cast_num_to_num<std::size_t>());
                    } else if(args.size() == 3) {
                        return args[0].subarray(args[1].cast_num_to_num<std::size_t>(), args[2].cast_num_to_num<std::size_t>());
                    }
                }
                throw std::runtime_error{"the first argument must be array"};
            });

            add_function("hexdump", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 4)
                if(args.size() == 1) {
                    return scfx::str_util::hexdump(args[0].cast_to_byte_array());
                } else if(args.size() == 2) {
                    return scfx::str_util::hexdump(
                        args[0].cast_to_byte_array(),
                        args[1].cast_to_u64()
                    );
                } else if(args.size() == 3) {
                    return scfx::str_util::hexdump(
                        args[0].cast_to_byte_array(),
                        args[1].cast_to_u64(),
                        args[2].cast_to_string()
                    );
                } else if(args.size() == 4) {
                    return scfx::str_util::hexdump(
                        args[0].cast_to_byte_array(),
                        args[1].cast_to_u64(),
                        args[2].cast_to_string(),
                        args[3].cast_to_bool()
                     );
                }
                return std::string{};
            });

            add_function("data_to_base85_str", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_byte_array()};
                return scfx::data_to_base85_str(src);
            });

            add_function("base85_str_to_data", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_string()};
                auto d{scfx::base85_str_to_data(src)};
                return std::string{d.begin(), d.end()};
            });

            add_function("data_to_base64_str", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_byte_array()};
                return scfx::data_to_base64_str(src);
            });

            add_function("base64_str_to_data", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_string()};
                auto d{scfx::base64_str_to_data(src)};
                return std::string{d.begin(), d.end()};
            });

            add_function("data_to_hex_str", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_byte_array()};
                return scfx::data_to_hex_str(src);
            });

            add_function("hex_str_to_data", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_string()};
                auto d{scfx::hex_str_to_data(src)};
                return std::string{d.begin(), d.end()};
            });

            add_function("getset", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{valbox_no_initialize::dont_do_it};
                res.assign(args[0]);
                args[0].assign(args[1]);
                return res;
            });


            add_function("atoi", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                if(args.size() == 1) {
                    return scfx::str_util::atoi(args[0].cast_to_string());
                } else if(args.size() == 2) {
                    return scfx::str_util::atoi(args[0].cast_to_string(), args[1].cast_to_u64());
                } else if(args.size() == 3) {
                    return scfx::str_util::atoi(args[0].cast_to_string(), args[1].cast_to_u64(), args[2].cast_to_bool());
                }
                return std::int64_t{0};
            });

            add_function("atoui", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2)
                if(args.size() == 1) {
                    return scfx::str_util::atoui(args[0].cast_to_string());
                } else if(args.size() == 2) {
                    return scfx::str_util::atoui(args[0].cast_to_string(), args[1].cast_to_u64());
                }
                return std::uint64_t{0};
            });

            add_function("atof", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return scfx::str_util::atof(args[0].cast_to_string());
            });

            add_function("itoa", SCFXFUN(args) {
                if(args.size() > 0) {
                    if(args.size() > 1) {
                        if(args.size() > 2) {
                            if(args.size() > 3) {
                                return scfx::str_util::itoa(args[0].cast_to_s64(), args[1].cast_to_s64(), args[2].cast_to_s64(), args[3].cast_to_bool());
                            } else {
                                return scfx::str_util::itoa(args[0].cast_to_s64(), args[1].cast_to_s64(), args[2].cast_to_s64());
                            }
                        } else {
                            return scfx::str_util::itoa(args[0].cast_to_s64(), args[1].cast_to_s64());
                        }
                    } else {
                        return scfx::str_util::itoa(args[0].cast_to_s64());
                    }
                }
                return std::string{};
            });

            add_function("utoa", SCFXFUN(args) {
                if(args.size() > 0) {
                    if(args.size() > 1) {
                        if(args.size() > 2) {
                            if(args.size() > 3) {
                                return scfx::str_util::utoa(args[0].cast_to_u64(), args[1].cast_to_s64(), args[2].cast_to_s64(), args[3].cast_to_bool());
                            } else {
                                return scfx::str_util::utoa(args[0].cast_to_u64(), args[1].cast_to_s64(), args[2].cast_to_s64());
                            }
                        } else {
                            return scfx::str_util::utoa(args[0].cast_to_u64(), args[1].cast_to_s64());
                        }
                    } else {
                        return scfx::str_util::utoa(args[0].cast_to_u64());
                    }
                }
                return std::string{};
            });

            add_function("ftoa", SCFXFUN(args) {
                if(args.size() > 0) {
                    if(args.size() > 1) {
                        return scfx::str_util::ftoa(args[0].cast_to_long_double(), args[1].cast_to_u64());
                    } else {
                        return scfx::str_util::ftoa(args[0].cast_to_long_double());
                    }
                }
                return std::string{"0.0"};
            });


            add_function("get_bit_field", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                if(args[0].is_u64_ref()) {
                    scfx::bit_util::bits<std::uint64_t> bf{args[0].cast_to_u64()};
                    return bf.get(args[1].cast_to_u64(), args[2].cast_to_u64());
                } else if(args[0].is_s64_ref()) {
                    scfx::bit_util::bits<std::uint64_t> bf{args[0].cast_to_u64()};
                    return bf.get(args[1].cast_to_u64(), args[2].cast_to_u64());
                } else if(args[0].is_u32_ref()) {
                    scfx::bit_util::bits<std::uint32_t> bf{args[0].cast_to_u32()};
                    return bf.get(args[1].cast_to_u64(), args[2].cast_to_u64());
                } else if(args[0].is_s32_ref()) {
                    scfx::bit_util::bits<std::uint32_t> bf{args[0].cast_to_u32()};
                    return bf.get(args[1].cast_to_u64(), args[2].cast_to_u64());
                } else if(args[0].is_u16_ref()) {
                    scfx::bit_util::bits<std::uint16_t> bf{args[0].cast_to_u16()};
                    return bf.get(args[1].cast_to_u64(), args[2].cast_to_u64());
                } else if(args[0].is_s16_ref()) {
                    scfx::bit_util::bits<std::uint16_t> bf{args[0].cast_to_u16()};
                    return bf.get(args[1].cast_to_u64(), args[2].cast_to_u64());
                } else if(args[0].is_u8_ref()) {
                    scfx::bit_util::bits<std::uint8_t> bf{args[0].cast_to_u8()};
                    return bf.get(args[1].cast_to_u64(), args[2].cast_to_u64());
                } else if(args[0].is_s8_ref()) {
                    scfx::bit_util::bits<std::uint8_t> bf{args[0].cast_to_u8()};
                    return bf.get(args[1].cast_to_u64(), args[2].cast_to_u64());
                }
                return args[0];
            });

            add_function("get_bit", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                scfx::bit_util::bits<std::uint64_t> bf{args[0].cast_to_u64()};
                auto bitpos{args[1].cast_to_u64()};
                return bf.get(bitpos);
            });

            add_function("update_bit", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
                scfx::bit_util::bits<std::uint64_t> bf{args[0].cast_to_u64()};
                auto bitpos{args[1].cast_to_u64()};
                auto bitval{args[2].cast_to_u64()};
                if(bitval != 0) {
                    bf.set(bitpos);
                } else {
                    bf.clr(bitpos);
                }
                valbox res{valbox_no_initialize::dont_do_it};
                res.become_same_type_as(args[0]);
                res.assign_preserving_type(bf.whole());
                return res;
            });
            add_function("set_bit", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                scfx::bit_util::bits<std::uint64_t> bf{args[0].cast_to_u64()};
                bf.set(args[1].cast_to_u64());
                valbox res{valbox_no_initialize::dont_do_it};
                res.become_same_type_as(args[0]);
                res.assign_preserving_type(bf.whole());
                return res;
            });
            add_function("clear_bit", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                scfx::bit_util::bits<std::uint64_t> bf{args[0].cast_to_u64()};
                bf.clr(args[1].cast_to_u64());
                valbox res{valbox_no_initialize::dont_do_it};
                res.become_same_type_as(args[0]);
                res.assign_preserving_type(bf.whole());
                return res;
            });


            add_function("sleep", SCFXFUN(args) {
                long double time_to_sleep{args[0].cast_to_long_double()};
                if(time_to_sleep > 0) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<std::int64_t>(time_to_sleep * 1000000000.0L)));
                }
                return time_to_sleep;
            });
            add_function("set_cycle_nanosleep", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1);
                uint64_t time_to_sleep{args[0].cast_to_u64()};
                set_nanoseconds_of_sleeping_between_cycles(time_to_sleep);
                return time_to_sleep;
            });
            add_function("cycle_nanosleep", SCFXFUN() {
                return sleep_between_cycles_nanoseconds();
            });
            add_function("exit", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2);
                if(programmatic_termination_enabled_ != 0) {
                    exit_status_ = args.size() == 2 ? args[1].cast_num_to_num<int>() : 0;
                    terminate();
                    execution_context *ctx{(execution_context *)args[0].as_ptr()};
                    ctx->request_return();
                }
                return programmatic_termination_enabled_ != 0;
            });

            add_function("assert", SCFXFUN(args) {
                SCFX_CHCK_FUN_PARMS_NUM_GE(args, 3);
                if(!args[2].cast_to_bool()) {
                    throw runtime_error{args[0].cast_to_s64(), args[0].cast_to_s64(), std::string{"assertion failed"}};
                }
                return true;
            });


            add_function("thread_id", SCFXFUN() {
                std::stringstream ss{};
                ss << std::this_thread::get_id();
                std::uint64_t res{};
                ss >> res;
                return res;
            });
            add_function("hardware_concurrency", SCFXFUN() {
                std::shared_lock l{threads_mtp_};
                auto ts{threads_.size()};
                return ts > 0 ? ts : 1;
            });


            add_function("size", SCFXFUN(args) {
                valbox &der{args[0].deref()};
                switch(der.val_type()) {
                    case valbox::type::STRING: return static_cast<uint64_t>(der.as_string().size());
                    case valbox::type::WSTRING: return static_cast<uint64_t>(der.as_wstring().size());
                    case valbox::type::ARRAY: return static_cast<uint64_t>(der.as_array().size());
                    case valbox::type::OBJECT: return static_cast<uint64_t>(der.as_object().size());
                    default: return static_cast<uint64_t>(1);
                }
            });

            add_function("empty", SCFXFUN(args) {
                valbox arg0{args[0]};
                return arg0.is_undefined_ref() ||
                       (arg0.is_object_ref() && arg0.as_object().empty()) ||
                       (arg0.is_array_ref() && arg0.as_array().empty()) ||
                       (arg0.is_string_ref() && arg0.as_string().empty()) ||
                       (arg0.is_wstring_ref() && arg0.as_wstring().empty());
            });
        }

        runtime(runtime const &) = delete;
        runtime(runtime &&) = delete;
        runtime &operator=(runtime const &) = delete;
        runtime &operator=(runtime &&) = delete;
        ~runtime() {
            stop_mt();

            worker_cells_templates_.clear();
            global_constants_dictionary_.clear();
            global_functions_dictionary_.clear();
            global_methods_dictionary_.clear();
            input_cells_.clear();
            input_names_to_instances_mapping_.clear();
            outputs_.clear();
            worker_cells_templates_.clear();
            worker_cells_.clear();
            worker_bodies_.clear();
            user_functions_.clear();

            unload_extensions();
            crypt_.unregister_runtime();
            fpool_.unregister_runtime();
            perf_stat_.unregister_runtime();
            randlib_.unregister_runtime();
            time_ext_.unregister_runtime();
            math_ext_.unregister_runtime();
            array_buffer_ext_.unregister_runtime();
        }

        scfx::timespec_wrapper valbox_to_timestamp(std::vector<scfx::valbox> const &args) const {
            if(args.size() >= 1) {
                if(args[0].is_string_ref()) {
                    return scfx::timespec_wrapper{args[0].as_string()};
                } else if(args[0].is_wstring_ref()) {
                    return scfx::timespec_wrapper{args[0].as_wstring()};
                } else if(args[0].is_any_fp_number()) {
                    return scfx::timespec_wrapper{args[0].cast_to_long_double()};
                } else if(args[0].is_any_int_number()) {
                    return scfx::timespec_wrapper{args[0].cast_to_s64()}; // milliseconds
                }
            }
            return scfx::timespec_wrapper::now();
        }

        void load_file(std::string const &filename) {
            if(!load_library(filename)) {
                if(scfx::file_util::file_exists(filename)) {
                    std::string src{scfx::file_util::load_text_file(filename)};
                    load_source_string(src);
                }
            }
        }

        void loading_complete() {
            std::unique_lock l{workers_mtp_};
            for(auto &&wcp: worker_cells_) {
                worker_cell_instance const &curr_cell{wcp.second};
                auto type_it{worker_cells_templates_.find(curr_cell.type_name())};
                if(type_it != worker_cells_templates_.end()) {
                    if(curr_cell. actual_args_info(). size() != static_cast<size_t>(type_it->second.num_args())) {
                        throw runtime_error{curr_cell.line(), curr_cell.col(), "actual arguments count for the cell mismatch"};
                    }
                }
            }
        }

        void add_function(std::string const &func_name, std::function<valbox(std::vector<valbox> &)> f) override {
            if(!is_identifier(func_name)) { throw std::runtime_error{std::string{"invalid identifier: \""} + func_name + "\""}; }
            if(global_functions_dictionary_.find(func_name) != global_functions_dictionary_.end()) {
                throw std::runtime_error{
                    std::string{"symbol already exists: \""} + func_name + "\""
                };
            }
            global_functions_dictionary_[func_name] = valbox{std::move(f), func_name, false};
        }

        void remove_function(std::string const &fname) override {
            auto it{global_functions_dictionary_.find(fname)};
            if(it != global_functions_dictionary_.end()) {
                global_functions_dictionary_.erase(it);
            }
        }

        void add_var(std::string const &var_name, valbox const &v) override {
            if(!is_identifier(var_name)) { throw std::runtime_error{std::string{"invalid identifier: \""} + var_name + "\""}; }
            if(global_constants_dictionary_.find(var_name) != global_constants_dictionary_.end()) {
                throw std::runtime_error{
                    std::string{"symbol already exists: \""} + var_name + "\""
                };
            }
            global_constants_dictionary_[var_name] = v;
        }

        void remove_var(std::string const &varname) override {
            auto it{global_constants_dictionary_.find(varname)};
            if(it != global_constants_dictionary_.end()) {
                global_constants_dictionary_.erase(it);
            }
        }

        void add_method(std::string const &class_name, std::string const &method_name, std::function<valbox(std::vector<valbox> &)> f) override {
            if(!is_identifier(class_name)) { throw std::runtime_error{std::string{"invalid identifier: \""} + class_name + "\""}; }
            if(!is_identifier(method_name)) { throw std::runtime_error{std::string{"invalid identifier: \""} + method_name + "\""}; }
            if(global_methods_dictionary_[class_name].find(method_name) != global_methods_dictionary_[class_name].end()) {
                throw std::runtime_error{std::string{"method already exists: \""} + method_name + "\""};
            }
            global_methods_dictionary_[class_name][method_name] = valbox{std::move(f), method_name, false};
        }

        void remove_method(std::string const &class_name, std::string const &method_name) override {
            auto class_name_it{global_methods_dictionary_.find(class_name)};
            if(class_name_it != global_methods_dictionary_.end()) {
                auto method_name_it{class_name_it->second.find(method_name)};
                if(method_name_it != class_name_it->second.end()) {
                    class_name_it->second.erase(method_name_it);
                    if(class_name_it->second.empty()) {
                        global_methods_dictionary_.erase(class_name_it);
                    }
                }
            }
        }

        valbox get_node_val(std::string const &name) {
            std::shared_lock l{workers_mtp_};
            auto it{worker_cells_.find(name)};
            if(it != worker_cells_.end()) {
                worker_cell_instance const &ci{it->second};
                return ci.curr_value();
            }
            throw std::runtime_error{std::string{"node not found: \""} + name + "\""};
        }

        size_t worker_cells_count() const {
            std::shared_lock l{workers_mtp_};
            return worker_cells_.size();
        }

        valbox get_external_value(std::string const &/*name*/) {
            return {};
        }

        void set_single_thread_mode() {
            stop_mt();
            unterminate();
            set_thread_mode_single();
        }

        void set_multi_thread_mode() {
            set_thread_mode_multi();
        }

        void run_cycles(std::size_t n) {
            for(std::size_t i{0}; i < n; ++i) {
                run_cycle();
                if(termination_requested()) {
                    break;
                }
            }
        }

        void run_cycle() {
            if(termination_requested()) {
                return;
            }
            if(!is_current_thread_mode_single()) {
                if(is_current_thread_mode_none()) {
                    set_thread_mode_single();
                } else {
                    throw std::runtime_error{"set single thread mode first"};
                }
            }
            exctx_.clear_all_jumps_request();
            for(auto &&w: worker_cells_) {
                worker_cell_instance &curr_cell{w.second};
                std::string const &curr_cell_type_name{curr_cell.type_name()};

                if(!curr_cell.type_info_transferred()) {
                    auto type_it{worker_cells_templates_.find(curr_cell_type_name)};
                    if(type_it != worker_cells_templates_.end()) {
                        if(curr_cell.actual_args_info().size() != static_cast<size_t>(type_it->second.num_args())) {
                            throw runtime_error{curr_cell.line(), curr_cell.col(), "actual arguments count for the cell mismatch"};
                        }
                        curr_cell.set_type_info(
                            type_it->second.num_args(),
                            type_it->second.arg_names()
                        );
                    }
                }

                exctx_.set_self_fields(curr_cell.cell_self_values_ptr());

                exctx_.clear_stack_soft();
                exctx_.new_stack_frame();

#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                bool undefineds{false};
#endif
                auto &&args_info{curr_cell.actual_args_info()};
                for(std::size_t curr_arg_number{0}; curr_arg_number < args_info.size(); ++curr_arg_number) {
                    auto &&ai{args_info[curr_arg_number]};
                    std::string curr_arg_name{ai.argname};
                    if(ai.cell) {
                        if(ai.w_cell_ptr != nullptr) {
                            exctx_.set_local_value(curr_arg_name, ai.w_cell_ptr->curr_value());
                        } else if(ai.in_cell_ptr != nullptr) {
                            exctx_.set_local_value(curr_arg_name, ai.in_cell_ptr->curr_value());
                        } else {
                            auto w_it{worker_cells_.find(ai.cell_name)};
                            if(w_it != worker_cells_.end()) {
#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                if(w_it->second.curr_value().is_undefined_ref()) {
                                    undefineds = true;
                                    break;
                                }
#endif
                                ai.w_cell_ptr = &(w_it->second);
                                exctx_.set_local_value(curr_arg_name, w_it->second.curr_value());
                            } else {
                                auto in_it{input_cells_.find(ai.cell_name)};
                                if(in_it != input_cells_.end()) {
#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                    if(in_it->second.curr_value().is_undefined_ref()) {
                                        undefineds = true;
                                        break;
                                    }
#endif
                                    ai.in_cell_ptr = &(in_it->second);
                                    exctx_.set_local_value(curr_arg_name, in_it->second.curr_value());
                                } else {
#ifdef SCFX_USE_EXTERNAL_VALUES
                                    valbox val{get_external_value(curr_arg_name)};
#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                    if(val.is_undefined()) {
                                        undefineds = true;
                                        break;
                                    }
#endif
                                    exctx_.set_local_value(curr_arg_name, val);
#else
                                    throw runtime_error{
                                        curr_cell.line(), curr_cell.col(),
                                        std::string{"input value not found for compute element \""} +
                                            curr_cell.inst_name() + "\""
                                    };
#endif
                                }
                            }
                        }
                    } else {
                        if(ai.expr_val.is_undefined_ref()) {
                            ai.expr_val = ai.expr->eval(&exctx_, eval_caller_type::no_matter, nullptr);
                        }
                        valbox vb{ai.expr_val};
#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                        if(vb.is_undefined_ref()) {
                            undefineds = true;
                            break;
                        }
#endif
                        exctx_.set_local_value(curr_arg_name, vb);
                    }
                }

#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                if(undefineds) {
                    curr_cell.set_curr_value(valbox{valbox_no_initialize::dont_do_it});
                    if(!curr_cell.out_name().empty()) {
                        exctx_.set_output(curr_cell.out_name(), valbox{valbox_no_initialize::dont_do_it});
                    }
                    continue;
                }
#endif
                if(!curr_cell.body()) {
                    auto body_it{worker_bodies_.find(curr_cell_type_name)};
                    if(body_it == worker_bodies_.end()) {
                        throw runtime_error{curr_cell.line(), curr_cell.col(), "cell not found"};
                    }
                    curr_cell.set_body(body_it->second);
                }
                curr_cell.body()->exec(&exctx_);

                if(termination_requested()) {
                    break;
                }
                if(exctx_.return_requested()) {
                    curr_cell.set_curr_value(exctx_.return_result());
                    if(!curr_cell.out_name().empty()) {
                        exctx_.set_output(curr_cell.out_name(), exctx_.return_result());
                    }
                }
                exctx_.clear_all_jumps_request();
            }
            if(sleep_between_cycles_nanoseconds_ > 0) {
                std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_between_cycles_nanoseconds_));
            }
        }

        void terminate() {
            termination_requested_.store(1, std::memory_order_release);
        }

        void unterminate() {
            termination_requested_.store(0, std::memory_order_release);
        }

        bool termination_requested() const {
            return termination_requested_.load(std::memory_order_acquire) != 0;
        }

        bool programmatic_termination_enabled() const {
            return programmatic_termination_enabled_ != 0;
        }

        void set_programmatic_termination_enabled(bool val) {
            programmatic_termination_enabled_ = (val ? 1 : 0);
        }

        int exit_status() const noexcept {
            return exit_status_;
        }

        std::uint64_t sleep_between_cycles_nanoseconds() const noexcept {
            return sleep_between_cycles_nanoseconds_;
        }

        void set_nanoseconds_of_sleeping_between_cycles(std::uint64_t val) noexcept {
            sleep_between_cycles_nanoseconds_ = val;
        }

        void stop_mt() {
            std::unique_lock l{threads_mtp_};
            if(!is_current_thread_mode_multi()) {
                return;
            }
            termination_requested_.store(1, std::memory_order_release);
            for(auto &&t: threads_) {
                if(t.joinable()) {
                    t.join();
                }
            }
            threads_.clear();
            set_thread_mode_none();
        }

        void run_mt(int thrd_cnt) {
            std::unique_lock l{threads_mtp_};
            if(!is_current_thread_mode_multi()) {
                if(is_current_thread_mode_none()) {
                    set_thread_mode_multi();
                } else {
                    throw std::runtime_error{"set multi thread mode first"};
                }
            }
            if(!threads_.empty()) {
                return;
            }
            {
                std::unique_lock l{failure_mtp_};
                failure_description_.clear();
                failure_.store(0, std::memory_order_release);
            }
            termination_requested_.store(0, std::memory_order_release);
            for(int i{0}; i < thrd_cnt; ++i) {
                threads_.emplace_back([this]() {
                    bool excepted{false};
                    std::string exbuf{};
#ifndef SCFX_DEBUGGING
                    try {
#endif
                        std::shared_ptr<execution_context> exctx{std::make_shared<execution_context>()};
                        execution_context *exctx_ptr{exctx.get()};
                        exctx_ptr->set_runtime_interface(this);
                        while(
                            termination_requested_.load(std::memory_order_acquire) == 0 &&
                            failure_.load(std::memory_order_acquire) == 0
                        ) {
                            exctx_ptr->clear_all_jumps_request();
                            for(auto &&wrkcl: worker_cells_) {
                                if(wrkcl.second.try_lock()) {
                                    worker_cell_instance &curr_cell{wrkcl.second};
                                    scfx::shut_on_destroy sod{[&]() { curr_cell.unlock(); }};
                                    std::string const &curr_cell_type_name{curr_cell.type_name()};

                                    if(!curr_cell.type_info_transferred()) {
                                        auto type_it{worker_cells_templates_.find(curr_cell_type_name)};
                                        if(type_it != worker_cells_templates_.end()) {
                                            if(curr_cell.actual_args_info().size() != static_cast<size_t>(type_it->second.num_args())) {
                                                throw runtime_error{curr_cell.line(), curr_cell.col(), "actual arguments count for the cell mismatch"};
                                            }
                                            curr_cell.set_type_info(type_it->second.num_args(), type_it->second.arg_names());
                                        }
                                    }

                                    exctx_ptr->set_self_fields(curr_cell.cell_self_values_ptr());

                                    exctx_ptr->clear_stack_soft();
                                    exctx_ptr->new_stack_frame();

#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                    bool undefineds{false};
#endif
                                    std::vector<worker_cell_instance::arg_info> &args_info{curr_cell.actual_args_info()};
                                    for(std::size_t curr_arg_number{0}; curr_arg_number < args_info.size(); ++curr_arg_number) {
                                        worker_cell_instance::arg_info &ai{args_info[curr_arg_number]};
                                        std::string curr_arg_name{ai.argname};
                                        if(ai.cell) {
                                            if(ai.w_cell_ptr != nullptr) {
                                                exctx_ptr->set_local_value(curr_arg_name, ai.w_cell_ptr->curr_value());
                                            } else if(ai.in_cell_ptr != nullptr) {
                                                exctx_ptr->set_local_value(curr_arg_name, ai.in_cell_ptr->curr_value());
                                            } else {
                                                auto w_it{worker_cells_.find(ai.cell_name)};
                                                if(w_it != worker_cells_.end()) {
#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                                    if(w_it->second.curr_value().is_undefined_ref()) {
                                                        undefineds = true;
                                                        break;
                                                    }
#endif
                                                    ai.w_cell_ptr = &(w_it->second);
                                                    exctx_ptr->set_local_value(curr_arg_name, ai.w_cell_ptr->curr_value());
                                                } else {
                                                    auto in_it{input_cells_.find(ai.cell_name)};
                                                    if(in_it != input_cells_.end()) {
#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                                        if(in_it->second.curr_value().is_undefined_ref()) {
                                                            undefineds = true;
                                                            break;
                                                        }
#endif
                                                        ai.in_cell_ptr = &(in_it->second);
                                                        exctx_ptr->set_local_value(curr_arg_name, ai.in_cell_ptr->curr_value());
                                                    } else {
#ifdef SCFX_USE_EXTERNAL_VALUES
                                                        valbox val{get_external_value(curr_arg_name)};
#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                                        if(val.is_undefined()) {
                                                            undefineds = true;
                                                            break;
                                                        }
#endif
                                                        exctx_ptr->set_local_value(curr_arg_name, val);
#else
                                                        throw runtime_error{
                                                            curr_cell.line(), curr_cell.col(),
                                                            std::string{"input value not found for compute element \""} +
                                                                curr_cell.inst_name() + "\""
                                                        };
#endif
                                                    }
                                                }
                                            }
                                        } else {
                                            if(ai.expr_val.is_undefined_ref()) {
                                                ai.expr_val = ai.expr->eval(exctx_ptr, eval_caller_type::no_matter, nullptr);
                                            }
                                            valbox vb{ai.expr_val};
#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                            if(vb.is_undefined_ref()) {
                                                undefineds = true;
                                                break;
                                            }
#endif
                                            exctx_ptr->set_local_value(curr_arg_name, vb);
                                        }
                                    }

#ifdef SCFX_DISABLE_UNDEFINED_CELL_ARGS
                                    if(undefineds) {
                                        curr_cell.set_curr_value(valbox{valbox_no_initialize::dont_do_it});
                                        if(!curr_cell.out_name().empty()) {
                                            exctx_ptr->set_output(curr_cell.out_name(), valbox{valbox_no_initialize::dont_do_it});
                                        }
                                        continue;
                                    }
#endif
                                    if(!curr_cell.body()) {
                                        auto body_it{worker_bodies_.find(curr_cell_type_name)};
                                        if(body_it == worker_bodies_.end()) {
                                            throw runtime_error{curr_cell.line(), curr_cell.col(), "cell not found"};
                                        }
                                        curr_cell.set_body(body_it->second);
                                    }
                                    curr_cell.body()->exec(exctx_ptr);

                                    if(termination_requested() || failure()) {
                                        break;
                                    }
                                    if(exctx_ptr->return_requested()) {
                                        curr_cell.set_curr_value(exctx_ptr->return_result());
                                        if(!curr_cell.out_name().empty()) {
                                            exctx_ptr->set_output(curr_cell.out_name(), exctx_ptr->return_result());
                                        }
                                    }
                                    exctx_ptr->clear_all_jumps_request();
                                }
                            }
                            if(sleep_between_cycles_nanoseconds_ > 0) {
                                std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_between_cycles_nanoseconds_));
                            }
                        }
#ifndef SCFX_DEBUGGING
                    } catch(std::exception const &e) {
                        excepted = true;
                        exbuf = e.what();
                    }
#endif
                    if(excepted) {
                        std::unique_lock l{failure_mtp_};
                        failure_description_ = exbuf;
                        failure_.store(1, std::memory_order_release);
                    }
                });
            }
        }

        bool failure() const noexcept {
            return failure_.load() != 0;
        }

        std::string failure_description() const noexcept {
            std::shared_lock l{failure_mtp_};
            return failure_description_;
        }

        bool wait(long double secnds) {
            if(thread_mode_ == thread_mode::multi) {
                auto slpfor{std::chrono::nanoseconds{
                        std::min<std::int64_t>(static_cast<std::int64_t>(secnds * 1'000'000'000.0L),
                        wait_granularity_nsec_)
                    }
                };
                auto future{
                    std::chrono::steady_clock::now() +
                    std::chrono::nanoseconds{static_cast<std::int64_t>(secnds * 1'000'000'000.0L)}
                };
                while(
                    termination_requested_.load(std::memory_order_acquire) == 0 &&
                    failure_.load(std::memory_order_acquire) == 0 &&
                    std::chrono::steady_clock::now() < future
                ) {
                    std::this_thread::sleep_for(slpfor);
                }
                std::shared_lock l{threads_mtp_};
                if(termination_requested_.load(std::memory_order_acquire) != 0 || failure_.load(std::memory_order_acquire) != 0) {
                    for(auto &&t: threads_) {
                        if(t.joinable()) {
                            t.join();
                        }
                    }
                }
                return termination_requested_.load(std::memory_order_acquire) != 0 || failure_.load(std::memory_order_acquire) != 0;
            }
            return true;
        }

        std::int64_t wait_granularity_nsec() const noexcept {
            return wait_granularity_nsec_;
        }

        void set_wait_granularity_nsec(std::int64_t val) noexcept {
            wait_granularity_nsec_ = val;
        }

        // runtime interface ---------------------------------------------------------------
        dict_map_t<std::string, valbox> *global_constants_dictionary() override {
            return &global_constants_dictionary_;
        }

        dict_map_t<std::string, valbox> *global_functions_dictionary() override {
            return &global_functions_dictionary_;
        }

        dict_map_t<std::string, dict_map_t<std::string, valbox>> *global_methods_dictionary() override {
            return &global_methods_dictionary_;
        }

        std::function<bool(std::string)> const &user_functions_search() override {
            return user_functions_search_;
        }

        std::function<valbox(std::vector<valbox> &)> const &user_function_selector() override {
            return user_function_selector_;
        }

        valbox get_input(std::string const &name) override {
            std::shared_lock l{input_cells_mtp_};
            return input_cells_[name].curr_value();
        }

        valbox const &get_output(std::string const &name) override {
            std::shared_lock l{outputs_mtp_};
            return outputs_[name];
        }

        void set_input(std::string const &name, valbox const &val) override {
            std::unique_lock l{input_cells_mtp_};
            auto it{input_names_to_instances_mapping_.find(name)};
            if(it == input_names_to_instances_mapping_.end()) {
                throw std::runtime_error{name + ": input name not found"};
            }
            input_cells_[it->second].set_value(val);
        }

        void set_output(std::string const &name, valbox const &val) override {
            std::unique_lock l{outputs_mtp_};
            outputs_[name] = val;
        }

        void clear_input(std::string const &name) override {
            std::unique_lock l{input_cells_mtp_};
            input_cells_[name].set_value({});
        }

        void clear_output(std::string const &name) override {
            std::unique_lock l{outputs_mtp_};
            outputs_.erase(name);
        }

        void clear_inputs() override {
            std::unique_lock l{input_cells_mtp_};
            input_cells_.clear();
        }

        void clear_outputs() override {
            std::unique_lock l{outputs_mtp_};
            outputs_.clear();
        }

    private:
        void load_source_string(std::string const &src) {
            lexer lxr{};
            parser prs{};
            lxr.set_callback([&](token const &tkn, bool /*space_with_nl*/) {
                if(tkn.tktype() != token::type::SPACE) {
                    prs.add_token(tkn);
                }
            });
            for(auto &&c: scfx::str_util::from_utf8(src)) {
                lxr.consume_char(c);
            }
            lxr.consume_eof();
            auto ast{prs.parse()};

            code_generator lj{};
            lj.chop(
                ast, input_cells_, input_names_to_instances_mapping_, worker_cells_templates_,
                worker_cells_, worker_bodies_, user_functions_, global_functions_dictionary_
            );
        }

        bool load_library(std::string const &fname) {
            bool res{false};
            try {
                std::shared_ptr<scfx::dlib> dll_ptr{std::make_shared<scfx::dlib>(fname)};
                if(dll_ptr->ok()) {
                    auto ld_fn{dll_ptr->symbol<scfx::extension_interface *(*)()>("create_scfx_extension")};
                    auto unld_fn{dll_ptr->symbol<void (*)(scfx::extension_interface *)>("remove_scfx_extension")};
                    if(ld_fn && unld_fn) {
                        scfx::extension_interface *ext{ld_fn()};
                        if(ext) {
                            ext->register_runtime(this);
                            std::unique_lock l{loaded_extensions_mtp_};
                            loaded_extensions_.emplace_back(std::move(dll_ptr), ext);
                            res = true;
                        }
                    }
                }
            } catch(...) {
            }
            return res;
        }

        void unload_extensions() {
            std::unique_lock l{loaded_extensions_mtp_};
            for(auto &&ep: loaded_extensions_) {
                ep.second->unregister_runtime();
                ep.first->symbol<void (*)(scfx::extension_interface *)>("remove_scfx_extension")(ep.second);
                ep.first->close();
            }
            loaded_extensions_.clear();
        }

    private:
        detail::console con_{};

        std::atomic<std::int64_t> failure_{0};
        mutable shared_mutex failure_mtp_{};
        std::string failure_description_{};

        std::atomic<std::int64_t> termination_requested_{0};
        enum class thread_mode{none, single, multi };
        thread_mode thread_mode_{thread_mode::none};
        void set_thread_mode_single() { thread_mode_ = thread_mode::single; }
        void set_thread_mode_multi() { thread_mode_ = thread_mode::multi; }
        void set_thread_mode_none() { thread_mode_ = thread_mode::none; }
        bool is_current_thread_mode_none() const { return thread_mode_ == thread_mode::none; }
        bool is_current_thread_mode_single() const { return thread_mode_ == thread_mode::single; }
        bool is_current_thread_mode_multi() const { return thread_mode_ == thread_mode::multi; }
        std::uint64_t sleep_between_cycles_nanoseconds_{0};

        execution_context exctx_{};
        shared_mutex threads_mtp_{};
        std::list<std::thread> threads_{};
        std::int64_t wait_granularity_nsec_{100LL};

        dict_map_t<std::string, valbox> global_constants_dictionary_{
            {"M_E", valbox{2.7182818284590452353602874713526624977572470936999595749669676277240766L}},
            {"M_PI", valbox{3.1415926535897932384626433832795028841971693993751058209749445923078164L}},
            {"M_PHI", valbox{1.6180339887498948482045868343656381177203091798057628621354486227052605L}},
            {"FP_INFINITE", valbox{FP_INFINITE}},
            {"FP_NAN", valbox{FP_NAN}},
            {"FP_ZERO", valbox{FP_ZERO}},
            {"FP_SUBNORMAL", valbox{FP_SUBNORMAL}},
            {"FP_NORMAL", valbox{FP_NORMAL}},
        };
        dict_map_t<std::string, valbox> global_functions_dictionary_{};
        dict_map_t<std::string, dict_map_t<std::string, valbox>> global_methods_dictionary_{};
        shared_mutex input_cells_mtp_{};
        map_t<std::string, input_cell> input_cells_{};
        map_t<std::string, std::string> input_names_to_instances_mapping_{};
        shared_mutex outputs_mtp_{};
        mutable map_t<std::string, valbox> outputs_{};
        mutable shared_mutex workers_mtp_{};
        map_t<std::string, worker_cell_definition_info> worker_cells_templates_{};
        map_t<std::string, worker_cell_instance> worker_cells_{};
        map_t<std::string, statement_ptr> worker_bodies_{};
        map_t<std::string, function_definition> user_functions_{};
        std::function<bool(std::string)> user_functions_search_{
            [this](std::string const &fname) -> bool {
                auto it{user_functions_.find(fname)};
                return it != user_functions_.end();
            }
        };
        std::function<valbox(std::vector<valbox> &)> user_function_selector_{
            [this](std::vector<valbox> &fargs) -> valbox {
                execution_context *exctx{reinterpret_cast<execution_context *>(fargs[0].as_ptr())};

                typename map_t<std::string, function_definition>::const_iterator it{user_functions_.find(fargs[1].as_string())};

                if(it == user_functions_.end()) {
                    throw std::runtime_error{"function not found"};
                }

                function_definition const &fdef{it->second};

                exctx->push_frame_ignore();
                scfx::shut_on_destroy leave_frame_ign{[exctx]() { exctx->pop_frame_ignore(); }};

                exctx->new_stack_frame();
                scfx::shut_on_destroy leave_frame{[exctx]() {
                    exctx->clear_return_request();
                    exctx->del_stack_frame();
                }};

                std::vector<std::string> const &anames{fdef.arg_names()};
                auto fargs_size{fargs.size()};
                for(std::size_t i{0}; i < anames.size(); ++i) {
                    if(i >= fargs_size) {
                        exctx->set_local_value(anames[i], valbox{valbox_no_initialize::dont_do_it});
                    } else {
                        exctx->set_local_value(anames[i], fargs[i + 2]);
                    }
                }

                fdef.body()->exec(exctx);
                if(exctx->return_requested()) {
                    return exctx->return_result();
                }

                return valbox{valbox_no_initialize::dont_do_it};
            }
        };
        int exit_status_{};
        std::size_t programmatic_termination_enabled_{1};

        math_ext math_ext_{};
        time_ext time_ext_{};
        crypto_ext crypt_{};
        file_ext fpool_{};
        cpu_ext perf_stat_{};
        rand_ext randlib_{};
        array_buffer_ext array_buffer_ext_{};

        shared_mutex loaded_extensions_mtp_{};
        std::list<std::pair<std::shared_ptr<dlib>, extension_interface *>> loaded_extensions_{};
        static std::size_t constexpr version_major_{1};
        static std::size_t constexpr version_minor_{2};
        static std::size_t constexpr version_patch_{115};
    };

}
