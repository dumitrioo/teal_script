#pragma once

#include "commondefs.hpp"

namespace teal {

    class so {
    public:
        so() = default;
        so(std::string const &file_name) {
            this->open(file_name);
        }
        so(so const &) = delete;
        so &operator=(so const &) = delete;
        so(so &&) = delete;
        so &operator=(so &&) = delete;
        ~so() {
            close();
        }

        void open(std::string const &file_name) {
            std::unique_lock l{lib_handle_mtp_};
            if(lib_handle_ != nullptr) {
                return;
            }
            lib_handle_ =
#if defined(PLATFORM_WINDOWS)
                ::LoadLibraryA(file_name.c_str());
#elif defined(PLATFORM_LINUX)
                ::dlopen(file_name.c_str(), RTLD_NOW);
            if(lib_handle_ == nullptr) {
                throw std::runtime_error{dlerror()};
            }
#endif
        }

        void close() noexcept {
            std::unique_lock l{lib_handle_mtp_};
            if(lib_handle_ != nullptr) {
#if defined(PLATFORM_WINDOWS)
                ::FreeLibrary(lib_handle_);
#elif defined(PLATFORM_LINUX)
                ::dlclose(lib_handle_);
#endif
                lib_handle_ = nullptr;
            }
        }

        bool ok() const noexcept {
            std::shared_lock l{lib_handle_mtp_};
            return lib_handle_ != nullptr;
        }

        template<typename T>
        T symbol(std::string const &sym_name) const noexcept {
            void *res{nullptr};
            std::shared_lock l{lib_handle_mtp_};
            if(lib_handle_ != nullptr) {
                res =
#if defined(PLATFORM_WINDOWS)
                    ::GetProcAddress(lib_handle_, sym_name.c_str());
#elif defined(PLATFORM_LINUX)
                    ::dlsym(lib_handle_, sym_name.c_str());
#endif
            }
            return reinterpret_cast<T>(res);
        }

    private:
        mutable std::shared_mutex lib_handle_mtp_{};
#ifdef PLATFORM_WINDOWS
        HMODULE lib_handle_{nullptr};
#else
        void *lib_handle_{nullptr};
#endif
    };

}
