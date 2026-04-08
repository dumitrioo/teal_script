#pragma once

#include "commondefs.hpp"
#include "file_util.hpp"
#include "str_util.hpp"

#include "str_util.hpp"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
#include <mntent.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#ifdef USE_BACK_TRACE
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>
#endif
#elif defined(PLATFORM_WINDOWS)
#include <WinSock2.h>
#include <wincrypt.h>
#endif

namespace teal::sys_util {

    class endian {
    private:
        static inline constexpr uint32_t uint32_ = 0x01020304;
        static inline constexpr uint8_t magic_ = (const uint8_t&)uint32_;

    public:
        endian() = delete;
        static inline constexpr bool little{magic_ == 0x04};
        static inline constexpr bool middle{magic_ == 0x02};
        static inline constexpr bool big{magic_ == 0x01};
        static_assert(
            (little && !middle && !big) ||
            (!little && middle && !big) ||
            (!little && !middle && big)
            ,
            "Cannot determine endianness!"
        );
    };

    static constexpr bool little_endian() {
        return endian::little;
    }

    static constexpr bool big_endian() {
        return endian::big;
    }

    static constexpr bool pdp_endian() {
        return endian::middle;
    }

    enum {
        HOST_ORDER_LITTLE_ENDIAN = 0x03020100ul,
        HOST_ORDER_BIG_ENDIAN = 0x00010203ul,
        HOST_ORDER_PDP_ENDIAN = 0x01000302ul
    };

    static const union {
        unsigned char bytes[4];
        std::uint32_t value;
    } host_order = { { 0, 1, 2, 3 } };

#define HOST_ORDER (teal::sys_util::host_order.value)
#define HOST_ORDER_LE (teal::sys_util::HOST_ORDER_LITTLE_ENDIAN)
#define HOST_ORDER_BE (teal::sys_util::HOST_ORDER_BIG_ENDIAN)
#define HOST_ORDER_PDP (teal::sys_util::HOST_ORDER_PDP_ENDIAN)

    template<typename S_T>
    static S_T check_dir_slash(const S_T &path) {
        if (path.size()) {
            if (path[path.size() - 1] != file_util::native_path_separator<S_T>{}.sym()) {
                return path + file_util::native_path_separator<S_T>{}.val();
            } else {
                return path;
            }
        }
        return file_util::native_path_separator<S_T>{}.val();
    }

    static int64_t last_error() {
#if defined(PLATFORM_WINDOWS)
        return GetLastError();
#else
        return errno;
#endif
    }

    static std::string error_str(int64_t e) {
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

#ifdef PLATFORM_WINDOWS
    template<typename FUNC_T>
    void for_reg_key(HKEY hk,
                    const teal::str_util::tstring &path,
                    FUNC_T apply,
                    REGSAM sam = KEY_ALL_ACCESS,
                    std::size_t max_nesting = -1,
                    bool values = true)
    {
        HKEY key;
        LONG status = RegOpenKeyEx(hk, path.c_str(), 0, sam, &key);
        if(status == ERROR_SUCCESS) {
            str_util::tstring path_st = check_dir_slash(path);

            if(values) {
                std::vector<TCHAR> val_name(8192);
                DWORD val_name_size = val_name.size();
                DWORD val_type;
                std::vector<std::uint8_t> val_content(65536);
                DWORD val_content_size = val_content.size();
                for(DWORD i = 0;; i++) {
                    LONG status;
                    val_name_size = val_name.size();
                    val_content_size = val_content.size();
                    for(status = RegEnumValue(key, i, &val_name[0], &val_name_size, 0, &val_type, &val_content[0], &val_content_size); status != ERROR_SUCCESS; status = RegEnumValue(key, i, &val_name[0], &val_name_size, 0, &val_type, &val_content[0], &val_content_size)) {
                        if(ERROR_NO_MORE_ITEMS == status) {
                            break;
                        }
                        if(val_name_size > val_name.size()) {
                            val_name.resize(val_name_size);
                        }
                        else {
                            val_name_size = val_name.size();
                        }
                        if(ERROR_MORE_DATA == status) {
                            if(val_content.size() < val_content_size) {
                                val_content.resize(val_content_size);
                            }
                            else {
                                val_content_size = val_content.size();
                            }
                        }
                    }
                    if(ERROR_NO_MORE_ITEMS == status) {
                        break;
                    }
                    if(status == ERROR_SUCCESS) {
                        apply(key, path_st + val_name.data(), val_name.data(), false, std::vector<std::uint8_t>(val_content.data(), val_content.data() + val_content_size), val_type);
                    }
                }
            }

            if(max_nesting) {
                std::vector<str_util::tstring> subkeys;

                std::vector<TCHAR> subkey_name(255);
                DWORD index = 0;
                for(LONG status = RegEnumKey(key, index++, &subkey_name[0], subkey_name.size()); status == ERROR_SUCCESS; status = RegEnumKey(key, index++, &subkey_name[0], subkey_name.size())) {
                    str_util::tstring apply_fn = path_st + subkey_name.data();
                    for_reg_key(hk, apply_fn, apply, sam, max_nesting - 1, values);
                    subkeys.push_back(subkey_name.data());
                }

                for(std::size_t i = 0; i < subkeys.size(); i++) {
                    apply(key, path_st + subkeys[i].data(), subkeys[i].data(), true, std::vector<std::uint8_t>(), 0);
                }
            }

            RegCloseKey(key);
        }
    }
#endif

#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
    static std::string backtrace() {
#ifdef USE_BACK_TRACE
        unw_cursor_t cursor;
        unw_context_t context;

        unw_getcontext(&context);
        unw_init_local(&cursor, &context);

        std::stringstream ss;

        while(unw_step(&cursor) > 0) {
            unw_word_t offset, pc;
            unw_get_reg(&cursor, UNW_REG_IP, &pc);
            if(pc == 0) {
                break;
            }
            ss << "0x" << teal::str_util::utoa<std::string>(pc, 16);

            char sym[256];
            if(unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
                std::string sym_str{sym};
                int status;
                char *demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
                if (status == 0) {
                    sym_str = demangled;
                }
                ss << ' ' << sym_str << "+0x" << teal::str_util::utoa<std::string>(offset, 16) << '\n';
            } else {
                ss << " -- error: unable to obtain symbol name for this frame\n";
            }
        }

        return ss.str();
#else
        return std::string{};
#endif
    }
#else
    static std::string backtrace() {
        return {};
    }
#endif

    static std::string current_executable_dir() {
        std::string module_dir;
#if defined(PLATFORM_WINDOWS)
        std::vector<char> path;
        path.resize(MAX_PATH + 1);
        if(GetModuleFileNameA(0, &path[0], MAX_PATH)) {
            std::string module_name = (CHAR *)&path[0];
            size_t slash_pos = module_name.find_last_of("\\");
            if(slash_pos != std::string::npos) {
                module_dir = module_name.substr(0, slash_pos);
            }
        }
#else
#endif
        return module_dir;
    }

    static std::string current_executable_path() {
        std::string module_path;
#if defined(PLATFORM_WINDOWS)
        std::vector<char> path;
        path.resize(MAX_PATH + 1);
        if(GetModuleFileNameA(0, &path[0], MAX_PATH)) {
            module_path = (char *) &path[0];
        }
#else
#endif
        return module_path;
    }

    static bool is_root() {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_UNIXISH) || defined(PLATFORM_POSIX) || defined(PLATFORM_ANDROID)
        return getuid() == 0;
#elif defined(PLATFORM_WINDOWS)
        BOOL fIsElevated{FALSE};
        HANDLE hToken{NULL};
        TOKEN_ELEVATION elevation{};
        DWORD dwSize{};

        do {
            if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
                break;
            }
            if(!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
                break;
            }
            fIsElevated = elevation.TokenIsElevated;
        } while(false);

        if(hToken) {
            CloseHandle(hToken);
        }

        return fIsElevated != 0;
#else
        return false;
#endif
    }

    static void set_high_priority() {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        struct sched_param param;
        param.sched_priority = 99; // Maximum priority

        int result = sched_setscheduler(0, SCHED_FIFO, &param);
        if (result != 0) {
            std::stringstream ss{};
            ss << "Error setting thread high priority: " << result;
            throw std::runtime_error{ss.str()};
        }
#elif defined(PLATFORM_WINDOWS)
        if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL)) {
            DWORD dwError{GetLastError()};
            std::stringstream ss{};
            ss << "Error setting thread high priority: " << dwError;
            throw std::runtime_error{ss.str()};
        }
#endif
    }

    static uint64_t multiplier(const std::string &s) {
        if((s == "kB") || (s == "KB") || (s == "Kb") || (s == "kb") || (s == "K") || (s == "k")) return 1024;
        if((s == "MB") || (s == "mB") || (s == "Mb") || (s == "mb") || (s == "M") || (s == "m")) return 1024 * 1024;
        if((s == "GB") || (s == "gB") || (s == "Gb") || (s == "gb") || (s == "G") || (s == "g")) return 1024 * 1024 * 1024;
        if((s == "B") || (s == "b") || s.empty()) return 1;
        return 1;
    }

    static void mem_info(uint64_t &total_mem, uint64_t &free_mem) {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        using std::ios_base;

        total_mem = 0ll;
        free_mem = 0ll;

        std::ifstream stat_stream("/proc/meminfo", std::ios_base::in);

        std::string mname, memunits;
        uint64_t memsize;

        stat_stream >> mname >> memsize >> memunits;
        total_mem = memsize * multiplier(memunits);

        stat_stream >> mname >> memsize >> memunits;
        free_mem = memsize * multiplier(memunits);
        stat_stream.close();
#else
        total_mem = 0ULL;
        free_mem = 0ULL;
#endif
    }

    static void mem_usage(uint64_t &vm_usage, uint64_t &resident_set) {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        using std::ios_base;
        using std::string;

        vm_usage     = 0ll;
        resident_set = 0ll;

        std::ifstream stat_stream("/proc/self/stat", std::ios_base::in);

        std::string pid, comm, state, ppid, pgrp, session, tty_nr;
        std::string tpgid, flags, minflt, cminflt, majflt, cmajflt;
        std::string utime, stime, cutime, cstime, priority, nice;
        std::string O, itrealvalue, starttime;

        uint64_t vsize;
        long rss;

        stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
            >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
            >> utime >> stime >> cutime >> cstime >> priority >> nice
            >> O >> itrealvalue >> starttime >> vsize >> rss;

        stat_stream.close();

        long page_size = sysconf(_SC_PAGE_SIZE);
        vm_usage     = vsize;
        resident_set = rss * page_size;
#elif defined(PLATFORM_WINDOWS)
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        resident_set = status.ullTotalPhys;
        vm_usage = status.ullTotalPhys - status.ullAvailPhys;
#else
        vm_usage     = 0ll;
        resident_set = 0ll;
#endif
    }

    static std::string get_exec_path() {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        std::size_t dest_len = 1024;
        char path[1024];
        path[0] = 0;
        if(readlink("/proc/self/exe", path, dest_len) != -1) {
            return teal::file_util::extract_file_dir(path);
        }
        return path;
#elif defined(PLATFORM_WINDOWS)
        TCHAR szFileName[MAX_PATH];
        GetModuleFileName(0, szFileName, MAX_PATH);
#ifdef _TCHAR_DEFINED
        return teal::str_util::to_utf8(szFileName);
#else
        return szFileName;
#endif
#else
        return {};
#endif
    }

    static std::string host_name() {
        std::string result{};
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        struct addrinfo hints, *info, *p;
        int gai_result{0};

        std::vector<char> hostname{};
        hostname.resize(2048);
        gethostname(&hostname[0], hostname.size() - 1);

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;

        if((gai_result = getaddrinfo(hostname.data(), "http", &hints, &info)) != 0) {
            return {};
        }

        for(p = info; p; p = p->ai_next) {
            result = p->ai_canonname;
            break;
        }
        freeaddrinfo(info);
#elif defined(PLATFORM_WINDOWS)
        std::vector<TCHAR> infoBuf;
        infoBuf.resize(32768);
        DWORD  bufCharCount = infoBuf.size();
        if(GetComputerName(infoBuf.data, &bufCharCount)) {
            result =
#ifdef(_UNICODE)
            result = str_util::to_utf8(std::wstring{(wchar_t const *)infoBuf.data()})
#else
            result = std::string{(wchar_t const *)infoBuf.data()}
#endif
            ;
        }
#endif
        return result;
    }


    static std::string system_name() {
#if defined(_WIN64)
        return "win64";
#elif defined(_WIN32)
        return "win32";
#elif defined(PLATFORM_APPLE)
#if TARGET_OS_IPHONE
        return "iphone";
#elif TARGET_IPHONE_SIMULATOR
        return "simulator";
#elif TARGET_OS_MAC
        return "macos";
#else
        return "apple";
#endif
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        struct utsname buf;
        uname(&buf);
        std::string result(buf.sysname);
        result += std::string(" ") + buf.release;
        result += std::string(" ") + buf.version;
        return result;
#elif defined(PLATFORM_UNIXISH)
        return "unix";
#elif defined(PLATFORM_POSIX)
        return "posix";
#else
        return "unknown";
#endif
    }

    static std::string system_vendor() {
        std::string res{};
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        auto vendor_info{teal::file_util::load_from_file("/sys/devices/virtual/dmi/id/sys_vendor")};
        res = teal::str_util::trim(std::string{vendor_info.begin(), vendor_info.end()});
#endif
        return res;
    }

    static std::string system_uuid() {
        std::string res{};
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        bool found{false};
        struct mntent *ent;
        FILE *mounts_file;
        mounts_file = setmntent("/proc/mounts", "r");
        if(mounts_file) {
            std::string root_fs{};
            while(nullptr != (ent = getmntent(mounts_file))) {
                if(std::string{ent->mnt_dir} == "/") {
                    root_fs = teal::file_util::extract_file_name(ent->mnt_fsname);
                    found = true;
                    break;
                }
            }
            endmntent(mounts_file);
            if(found) {
                for(
                    auto it{std::filesystem::directory_iterator("/dev/disk/by-uuid")};
                    it != std::filesystem::directory_iterator();
                    ++it
                    ) {
                    if(std::filesystem::is_symlink(it->symlink_status())) {
                        std::string fname{teal::file_util::extract_file_name(it->path())};
                        std::string slnk{teal::file_util::extract_file_name(std::filesystem::read_symlink(*it))};
                        if(slnk == root_fs) {
                            res = fname;
                            break;
                        }
                    }
                }
            }
        }
#endif
        return res;
    }

    static ssize_t gen_cs_rand(void *buff, std::size_t buff_size) {
        int result{0};
        if(buff && buff_size) {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            result = ::getrandom(buff, buff_size, GRND_RANDOM);
#elif defined(PLATFORM_APPLE)
        arc4random_buf(buff, buff_size);
        result = buff_size;
#elif defined(PLATFORM_WINDOWS)
        HCRYPTPROV hCryptProv{0};
        if(CryptAcquireContextW(&hCryptProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
            if(CryptGenRandom(hCryptProv, buff_size, (unsigned char *)buff)) {
                result = buff_size;
            }
        }
#endif
        }
        return result;
    }

    static bool increase_stack(rlim_t const requiredStackSize) {
        bool res{false};
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        struct rlimit currentLimit;
        int status;
        status = getrlimit(RLIMIT_STACK, &currentLimit);
        if (status == 0) {
            if (currentLimit.rlim_cur < requiredStackSize) {
                currentLimit.rlim_cur = requiredStackSize;
                status = setrlimit(RLIMIT_STACK, &currentLimit);
                if (status == 0) {
                    res = true;
                }
            }
        }
#elif defined(PLATFORM_APPLE)
#elif defined(PLATFORM_WINDOWS)
#endif
        return res;
    }

}
