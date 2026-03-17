#pragma once

#include "commondefs.hpp"
#include "str_util.hpp"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
#if !defined(_XOPEN_SOURCE) && _XOPEN_SOURCE < 500
#define _XOPEN_SOURCE 500
#endif
#include <ftw.h>
#include <unistd.h>
#include <pwd.h>
#elif defined(PLATFORM_WINDOWS)
#include <WinSock2.h>
#include <Shlobj.h>
#include <Shlobj_core.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")
#elif defined(PLATFORM_APPLE)
#include <unistd.h>
#include <removefile.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <fstream>

#ifdef USE_FILE_MAGIC
#include <magic.h>
#endif

#include "timespec_wrapper.hpp"
#include <sys/types.h>
#ifdef PLATFORM_WINDOWS
#else
#include <dirent.h>
#endif
#include <cerrno>
#include <string>
#include <vector>
#include <iostream>
#include <istream>
#include <ostream>

namespace scfx::file_util {

    template<typename T>
    struct native_path_separator;

    template<>
    struct native_path_separator<std::string> {
        static std::string val() {
#if defined(PLATFORM_WINDOWS)
            return "\\";
#else
            return "/";
#endif
        }
        static char sym() {
#if defined(PLATFORM_WINDOWS)
            return '\\';
#else
            return '/';
#endif
        }
    };

    template<>
    struct native_path_separator<std::wstring> {
        static std::wstring val() {
#if defined(PLATFORM_WINDOWS)
            return L"\\";
#else
            return L"/";
#endif
        }
        static wchar_t sym() {
#if defined(PLATFORM_WINDOWS)
            return '\\';
#else
            return '/';
#endif
        }
    };

    DEFINE_RUNTIME_ERROR_CLASS_MSG(directory_opening_error, "failed to open directory")
    DEFINE_RUNTIME_ERROR_CLASS_MSG(file_loading_error, "failed to load file")
    DEFINE_RUNTIME_ERROR_CLASS_MSG(file_opening_error, "failed to open file")
    DEFINE_RUNTIME_ERROR_CLASS_MSG(dir_not_exists_error, "directory not exists")
    DEFINE_RUNTIME_ERROR_CLASS_MSG(not_a_dir_or_dir_not_exists_error, "not a directory or not exists")
    DEFINE_RUNTIME_ERROR_CLASS_MSG(file_not_exists_error, "file not exists")

    static std::string extract_file_ext(const std::string &file_name) {
#if (__cplusplus < 201700L)
        std::vector<std::string> tokens = scfx::str_util::str_tok<std::string>(file_name, ".");
        if(tokens.size() < 2) {
            return "";
        }
        return tokens[tokens.size() - 1];
#else
        std::string res{std::filesystem::path{file_name}.extension().string()};
        while(!res.empty() && res[0] == '.') { res = res.substr(1); }
        return res;
#endif
    }

    static std::string extract_file_name(const std::string &path) {
#if (__cplusplus < 201700L)
        std::vector<std::string> tokens = scfx::str_util::str_tok<std::string>(path, native_path_separator<std::string>::val(), true);
        while(tokens.size() && tokens[tokens.size() - 1].empty()) {
            auto end_it{tokens.end()}; --end_it;
            tokens.erase(end_it);
        }
        if(tokens.size() >= 1) {
            return tokens[tokens.size() - 1];
        } else {
            return std::string{};
        }
#else
        return std::filesystem::path{path}.filename().string();
#endif
    }

    static std::string extract_file_dir(const std::string &path) {
#if (__cplusplus < 201700L)
        std::vector<std::string> tokens = scfx::str_util::str_tok<std::string>(path, native_path_separator<std::string>::val(), true);
        while(tokens.size() && tokens[tokens.size() - 1].empty()) {
            auto end_it{tokens.end()}; --end_it;
            tokens.erase(end_it);
        }
        if(tokens.size() > 1) {
            std::string out;
            for(int i = 0; i < (int)tokens.size() - 1; ++i) {
                if(i == 0 || tokens[(std::size_t)i].size()) {
                    out += tokens[(std::size_t)i] + native_path_separator<std::string>::val();
                }
            }
            return out;
        } else {
            return std::string{};
        }
#else
        return std::filesystem::path{path}.parent_path().string();
#endif
    }

    namespace detail {

        static int64_t last_error() {
#if defined(PLATFORM_WINDOWS)
            return GetLastError();
#else
            return errno;
#endif
        }

        std::string error_str(int64_t e) {
#if defined(PLATFORM_WINDOWS)
            DWORD errorMessageID{(DWORD)e};
            if(errorMessageID == 0) {
                return std::string{};
            }
            LPWSTR messageBuffer{nullptr};
            size_t size = FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                errorMessageID,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&messageBuffer,
                0,
                nullptr
            );
            std::wstring message(messageBuffer, size);
            LocalFree(messageBuffer);
            return str_util::to_utf8(message);
#elif defined(PLATFORM_LINUX)
            std::system_error se{(int)e, std::system_category()};
            std::stringstream ss{};
            ss << se.what();
            return ss.str();
#endif
        }
    }

    static bool file_exists(const std::string &file_name) {
#if (__cplusplus < 201700L)
    #if defined(PLATFORM_LINUX) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
        struct stat sb;
        int stres{stat(file_name.c_str(), &sb)};
        if(stres != 0) {
            return false;
        }
        switch (sb.st_mode & S_IFMT) {
            case S_IFBLK:  return true; /* printf("block device\n"); */
            case S_IFCHR:  return true; /* printf("character device\n"); */
            case S_IFDIR:  return false; /* printf("directory\n"); */
            case S_IFIFO:  return true; /* printf("FIFO/pipe\n"); */
            case S_IFLNK:  return true; /* printf("symlink\n"); */
            case S_IFREG:  return true; /* printf("regular file\n"); */
            case S_IFSOCK: return true; /* printf("socket\n"); */
            default:       return false; /*printf("unknown?\n");*/
        }
    #elif defined(PLATFORM_WINDOWS)
        DWORD dwAttrib = GetFileAttributesA(file_name.c_str());
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0));
    #endif
#else
        return std::filesystem::exists(file_name) &&
               (
                std::filesystem::is_regular_file(file_name)
                ||
                std::filesystem::is_block_file(file_name)
                ||
                std::filesystem::is_character_file(file_name)
                ||
                std::filesystem::is_fifo(file_name)
                ||
                std::filesystem::is_socket(file_name)
               );
#endif
    }

    timespec_wrapper get_file_creation_time(std::string const &file_path) {
#if defined(PLATFORM_WINDOWS)
        struct _stat attrib;
        if (_stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_ctime;
        } else {
            return {};
        }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        struct stat attrib;
        if (stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_ctim;
        } else {
            return {};
        }
#elif defined(PLATFORM_APPLE)
        struct stat attrib;
        if (stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_ctimespec;
        } else {
            return {};
        }
#else
        return {};
#endif
    }

    timespec_wrapper get_file_access_time(std::string const &file_path) {
#if defined(PLATFORM_WINDOWS)
        struct _stat attrib;
        if (_stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_atime;
        } else {
            return {};
        }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        struct stat attrib;
        if (stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_atim;
        } else {
            return {};
        }
#elif defined(PLATFORM_APPLE)
        struct stat attrib;
        if (stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_atimespec;
        } else {
            return {};
        }
#else
        return {};
#endif
    }

    timespec_wrapper get_file_modification_time(std::string const &file_path) {
#if defined(PLATFORM_WINDOWS)
        struct _stat attrib;
        if(_stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_mtime;
        } else {
            return {};
        }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        struct stat attrib;
        if (stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_mtim;
        } else {
            return {};
        }
#elif defined(PLATFORM_APPLE)
        struct stat attrib;
        if (stat(file_path.c_str(), &attrib) == 0) {
            return attrib.st_mtimespec;
        } else {
            return {};
        }
#else
        return {};
#endif
    }

#ifdef USE_FILE_MAGIC
    namespace detail {

        class mgc_detector {
        public:
            mgc_detector(int flags = MAGIC_MIME_TYPE):
                magic_handle_{magic_open(flags), &magic_close}
            {
                if(magic_handle_) {
                    if(::magic_load(magic_handle_.get(), nullptr) != 0) {
                        magic_handle_.reset();
                    }
                }
            }

            bool ok() const {
                return magic_handle_.get() != nullptr;
            }

            std::string file(const std::string &pat_str) const {
                if(!magic_handle_) {
                    throw std::runtime_error{"magic library is not initialized."};
                }
                char const *res{::magic_file(magic_handle_.get(), pat_str.c_str())}; if(res) { return res; } return {};
            }

            std::string buffer(void const *data, std::size_t data_size) const {
                if(!magic_handle_) {
                    throw std::runtime_error{"magic library is not initialized."};
                }
                char const *res{::magic_buffer(magic_handle_.get(), data, data_size)}; if(res) { return res; } else { return {}; }
            }

        private:
            std::unique_ptr<magic_set, decltype(&magic_close)> magic_handle_;
        };

    }

    static std::string file_type(const std::string &pat_str, int flags = MAGIC_MIME_TYPE) {
        if(file_exists(pat_str)) { return detail::mgc_detector{flags}.file(pat_str); }
        return {};
    }

    static std::string data_type(void const *data, std::size_t data_size, int flags = MAGIC_MIME_TYPE) {
        if(data && data_size) { return detail::mgc_detector{flags}.buffer(data, data_size); }
        return {};
    }

    static std::string data_type(std::vector<std::uint8_t> const &data, int flags = MAGIC_MIME_TYPE) {
        return data_type(data.data(), data.size(), flags);
    }

    static std::string data_type(std::string const &data, int flags = MAGIC_MIME_TYPE) {
        return data_type(data.data(), data.size(), flags);
    }
#endif

    static bool dir_exists(const std::string &file_name) {
#if (__cplusplus < 201700L)
#if defined(PLATFORM_LINUX) || defined(PLATFORM_UNIXISH) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
        struct stat sb;

        if(stat(file_name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
            return true;
        }
        return false;
#elif defined(PLATFORM_WINDOWS)
        DWORD dwAttrib = GetFileAttributesA(file_name.c_str());

        return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
#else
        return std::filesystem::is_directory(file_name);
#endif

    }

    static std::string real_path(const std::string &p) {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
        std::string res;
        if(p.size()) {
            std::vector<char> resolved(p.size() * 2);
            res = realpath(p.c_str(), &resolved[0]);
        }
        return res;
#elif defined(PLATFORM_WINDOWS)
        std::string res;
        if (p.size()) {
            std::vector<char> resolved(p.size() * 2 + 1);
            res = _fullpath(&resolved[0], p.c_str(), p.size() * 2);
        }
        return res;
#endif
    }

    DEFINE_RUNTIME_ERROR_CLASS_MSG(stat_calling_error, "stat() calling error")
    DEFINE_RUNTIME_ERROR_CLASS_MSG(file_size_getting_error, "file size examining error")

    static bool copy_file(std::string const &from, std::string const &to) {
        std::ifstream sf(from, std::ios::binary);
        if(!sf) { return false; }
        std::ofstream df{to, std::ios::binary | std::ios::trunc};
        if(!df) { return false; }
        std::copy(
            std::istreambuf_iterator<char>{sf},
            std::istreambuf_iterator<char>{},
            std::ostreambuf_iterator<char>{df}
        );
        return true;
    }

    static bool rm_dir(const std::string &path) {
#if (__cplusplus < 201700L)
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        auto do_remove{[](const char *fpath, const struct stat64 */*sb*/, int tflag, struct FTW */*ftwbuf*/) -> int {
            if(tflag == FTW_F) {
                ::unlink(fpath);
            } else if(tflag == FTW_DP) {
                ::rmdir(fpath);
            }
            return 0;
        }};
        if(::nftw64(path.c_str(), do_remove, 100, FTW_DEPTH | FTW_PHYS) == -1) {
            return false;
        }
        return true;
#elif defined(PLATFORM_WINDOWS)
        return RemoveDirectoryA(path.c_str());
#elif defined(__APPLE__)
        removefile_state_t s;
        s = removefile_state_alloc();
        return removefile(path.c_str(), s, REMOVEFILE_RECURSIVE | REMOVEFILE_KEEP_PARENT) == 0;
#endif
#else
        if(std::filesystem::is_directory(path)) {
            return std::filesystem::remove_all(path);
        }
        return false;
#endif
    }

    static bool delete_fs_entry(const std::string &fn) {
#if (__cplusplus < 201700L)
        if(dir_exists(fn)) {
            return rm_dir(fn);
        } else if(file_exists(fn)) {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
            return ::unlink(fn.c_str()) == 0;
#elif defined(PLATFORM_WINDOWS)
            return DeleteFileA(fn.c_str());
#endif
            return false;
        }
#else
        return std::filesystem::remove(fn);
#endif
    }

    static std::int64_t file_size(const std::string &fname) {
#if (__cplusplus < 201700L)
        std::fstream f{fname, std::ifstream::ate | std::ifstream::binary};
        if(!f) {
            throw file_opening_error{};
        }
        return f.tellg();
#else
        if(!file_exists(fname)) {
            throw file_opening_error{};
        }
        return std::filesystem::file_size(fname);
#endif
    }

    static std::vector<std::uint8_t> load_from_file(const std::string &fn, std::uint64_t how_much = 0) {
        std::deque<std::uint8_t> result{};
        if(!file_exists(fn)) {
            throw file_loading_error{};
        }
        std::ifstream file{};
        file.open(fn.c_str(), std::ios_base::in | std::ios_base::binary);
        if(file.is_open()) {
            int c{};
            std::uint64_t total_read{0};
            while((c = file.get()) != -1) {
                result.push_back(static_cast<std::uint8_t>(c));
                ++total_read;
                if(how_much > 0 && total_read >= how_much) {
                    break;
                }
            }
            file.close();
        }
        return std::vector<std::uint8_t>{result.begin(), result.end()};
    }

    static std::string load_text_file(const std::string &fn) {
        if(!file_exists(fn)) {
            throw file_not_exists_error{};
        }
        std::ifstream sf(fn, std::ios::binary);
        if(!sf) { return {}; }
        std::stringstream ss{};
        ss << sf.rdbuf();
        return ss.str();
    }

    static bool save_to_file(const std::string &fn, const std::vector<std::uint8_t> &data) {
        std::ofstream ofs;
        ofs.open(fn.c_str(), std::ios_base::out | std::ios_base::binary);
        if(ofs.is_open()) {
            ofs.write(reinterpret_cast<const char *>(data.data()), static_cast<ssize_t>(data.size()));
            ofs.close();
            return true;
        } else {
            return false;
        }
    }

    static bool save_to_file(const std::string &fn, const std::string &data) {
        std::ofstream ofs;
        ofs.open(fn.c_str(), std::ios_base::out | std::ios_base::binary);
        if(ofs.is_open()) {
            ofs.write(reinterpret_cast<const char *>(data.data()), static_cast<ssize_t>(data.size()));
            ofs.close();
            return true;
        } else {
            return false;
        }
    }

    static bool save_to_file(const std::string &fn, const void *data, ptrdiff_t size) {
        std::ofstream ofs;
        ofs.open(fn.c_str(), std::ios_base::out | std::ios_base::binary);
        if(ofs.is_open()) {
            if(data && size) {
                ofs.write(reinterpret_cast<const char *>(data), size);
            }
            ofs.close();
            return true;
        } else {
            return false;
        }
    }

    static std::string suffix_file_name_number(std::string const &fn, int num) {
        std::string tfn{fn};
        std::string sn{scfx::str_util::itoa(static_cast<std::int64_t>(num))};
        auto dot_pos{tfn.find('.')};
        if(dot_pos != std::string::npos) {
            return tfn.substr(0, dot_pos) + "(" + sn + ")" + tfn.substr(dot_pos);
        } else {
            return tfn + "(" + sn + ")";
        }
    }

    class dir_entry {
    public:
        enum class kind {
            none, file, dir, sym,
        };

        dir_entry() = default;
        dir_entry(
            const std::string &path,
            std::size_t fsize = static_cast<std::size_t>(0),
            kind k = kind::file,
            scfx::timespec_wrapper mtime = {}
        ):
            path_{path},
            size_{fsize},
            kind_{k},
            modify_time_{mtime}
        {
        }
        std::string file_name() const { return std::filesystem::path{path_}.filename().string(); }
        std::string file_ext() const { return std::filesystem::path{path_}.extension().string(); }
        void set_path(const std::string &fp) { path_ = fp; }
        const std::string &full_path() const { return path_; }

        scfx::timespec_wrapper modify_time() const { return modify_time_; }
        bool is_file() const { return kind_ == kind::file; }
        bool is_dir() const { return kind_ == kind::dir; }
        bool is_sym() const { return kind_ == kind::sym; }
        uint64_t file_size() const { return size_; }

    public:
        std::string path_;
        std::size_t size_{0};
        kind kind_{kind::none};
        scfx::timespec_wrapper modify_time_{};
    };

#if (__cplusplus < 201700L)
#ifdef PLATFORM_WINDOWS
    template<typename T>
    void for_dir_tree(const std::string &dir_path, T apply, bool recursive = true) {
        std::string local_dir_path = dir_path;
        if(local_dir_path.size() == 0) {
            local_dir_path = std::string(".") + native_path_separator<std::string>::val();
        }
        if(local_dir_path[local_dir_path.size() - 1] != native_path_separator<std::string>::val()[0]) {
            local_dir_path += native_path_separator<std::string>::val();
        }
        if(!dir_exists(local_dir_path)) {
            return;
        }
        struct dirent *dir_entry_p;
        HANDLE dir_p{ INVALID_HANDLE_VALUE };
        WIN32_FIND_DATAA ffd;
        bool fr{true};
        for(
            dir_p = FindFirstFileA((local_dir_path + "*.*").c_str(), &ffd);
            dir_p != INVALID_HANDLE_VALUE && fr;
            fr = FindNextFileA(dir_p, &ffd)
            ) {
            std::string p = local_dir_path + ffd.cFileName;

            if (std::string{ ffd.cFileName } == "." || std::string{ ffd.cFileName } == "..") {
                continue;
            }

            if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                p += native_path_separator<std::string>::val();
                dir_entry aplly_entry{p, 0, dir_entry::kind::dir, get_file_modification_time(p)};
                apply(aplly_entry);
                if(recursive) {
                    for_dir_tree(p, apply, true);
                }
            } else {
                dir_entry aplly_entry{p, static_cast<std::size_t >(file_size(p)), dir_entry::kind::file, get_file_modification_time(p)};
                apply(aplly_entry);
            }
        }
        if(dir_p != INVALID_HANDLE_VALUE) {
            FindClose(dir_p);
        }
    }
#else
    template<typename T>
    void for_dir_tree(const std::string &dir_path, T apply, bool recursive = true) {
        std::string local_dir_path{dir_path};
        if(local_dir_path.size() == 0) {
            local_dir_path = std::string(".") + native_path_separator<std::string>::val();
        }
        if(local_dir_path[local_dir_path.size() - 1] != native_path_separator<std::string>::val()[0]) {
            local_dir_path += native_path_separator<std::string>::val();
        }
        if(!dir_exists(local_dir_path)) {
            return;
        }
        struct dirent *dir_entry_p{nullptr};
        DIR *dir_p{nullptr};
        for(dir_p = opendir(local_dir_path.c_str()); dir_p && (dir_entry_p = readdir(dir_p)); ) {
            if(memcmp(dir_entry_p->d_name, ".", 1) == 0 || memcmp(dir_entry_p->d_name, "..", 2) == 0) {
                continue;
            }
            std::string p{local_dir_path + dir_entry_p->d_name};
            if(dir_exists(p)) {
                p += native_path_separator<std::string>::val();
                dir_entry aplly_entry{p, 0, dir_entry::kind::dir, get_file_modification_time(p)};
                if(!apply(aplly_entry)) {
                    break;
                }
                if(recursive) {
                    for_dir_tree(p, apply, true);
                }
            } else {
                dir_entry aplly_entry{p, static_cast<std::size_t >(file_size(p)), dir_entry::kind::file, get_file_modification_time(p)};
                if(!apply(aplly_entry)) {
                    break;
                }
            }
        }
        if(dir_p) {
            closedir(dir_p);
        }
    }
#endif
#else
    template<typename T>
    void for_dir_tree(const std::string &dir_path, T apply, bool recursive = true) {
        std::filesystem::path p{dir_path};
        if(dir_path.size() == 0) {
            p = std::string{"."};
        }
        if(!dir_exists(p)) {
            return;
        }
        if(recursive) {
            for(auto const &entry: std::filesystem::recursive_directory_iterator{p}) {
                auto tt{entry.last_write_time()};
                std::stringstream ss{};
                ss << tt << "+0";
                scfx::timespec_wrapper mt{ss.str()};
                if(entry.is_regular_file()) {
                    if(!apply(dir_entry{entry.path().string(), entry.file_size(), dir_entry::kind::file, mt})) {
                        break;
                    }
                } else if(entry.is_directory()) {
                    if(!apply(dir_entry{entry.path().string(), 0, dir_entry::kind::dir, mt})) {
                        break;
                    }
                }
            }
        } else {
            for(auto const &entry: std::filesystem::directory_iterator{p}) {
                auto tt{entry.last_write_time()};
                std::stringstream ss{};
                ss << tt << "+0";
                scfx::timespec_wrapper mt{ss.str()};
                if(entry.is_regular_file()) {
                    if(!apply(dir_entry{entry.path().string(), entry.file_size(), dir_entry::kind::file, mt})) {
                        break;
                    }
                } else if(entry.is_directory()) {
                    if(!apply(dir_entry{entry.path().string(), 0, dir_entry::kind::dir,mt})) {
                        break;
                    }
                }
            }
        }
    }
#endif

    static bool is_empty(std::string const &dir) {
#if (__cplusplus < 201700L)
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        std::unique_ptr<DIR, decltype(&closedir)> dtr{opendir(dir.c_str()), closedir};
        if(dtr == nullptr) {
            throw not_a_dir_or_dir_not_exists_error{};
        }
        std::size_t n{0};
        struct dirent *d;
        while((d = readdir(dtr.get())) != nullptr) {
            if(++n > 2) {
                break;
            }
        }
        return n < 2;
#else
        bool contains{false};
        if(!dir_exists(dir)) {
            throw not_a_dir_or_dir_not_exists_error{};
        }
        for_dir_tree(dir, [&](dir_entry const &) { contains = true; return false; }, false);
        return !contains;
#endif
#else
        if(!std::filesystem::is_directory(dir)) {
            throw not_a_dir_or_dir_not_exists_error{};
        }
        return std::filesystem::is_empty(dir);
#endif
    }

}
