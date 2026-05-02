#pragma once

#include "commondefs.hpp"

namespace teal {

    template<
        std::size_t BLOCK_SIZE = 256 * 1024 * 1024,
        std::size_t ALIGNMENT = alignof(std::max_align_t),
        std::size_t MAX_ALLOC_SIZE = 4096
    >
    class sorting_alloc {
        struct alloc_unit_hdr {
            alloc_unit_hdr *next;
            size_t size;
            void *user_ptr() {
                return ((uint8_t *)this) + ALIGNMENT;
            }
        };
        static_assert(ALIGNMENT >= sizeof(alloc_unit_hdr), "choose greater alignment");

        struct alignas(64) thread_safe_free_item {
            std::shared_mutex mtp{};
            alloc_unit_hdr *fc{nullptr};
        };


    public:
        sorting_alloc() = default;
        sorting_alloc(sorting_alloc const &) = delete;
        sorting_alloc& operator=(sorting_alloc const &) = delete;
        sorting_alloc(sorting_alloc &&) = delete;
        sorting_alloc& operator=(sorting_alloc &&) = delete;
        ~sorting_alloc() = default;

        void *allocate(size_t size) {
            size_t aligned_size{((size + ALIGNMENT - 1) & ~(ALIGNMENT - 1))};
            size_t full_size{aligned_size + ALIGNMENT};

            if(aligned_size > MAX_ALLOC_SIZE) {
                alloc_unit_hdr *full_chnk{(alloc_unit_hdr *)::malloc(full_size)};
                full_chnk->size = aligned_size;
                return full_chnk->user_ptr();
            }

            std::unique_lock l{free_lists_[aligned_size - 1].mtp};
            if(free_lists_[aligned_size - 1].fc == nullptr) {
                std::unique_lock ll{mtp_};
                l.unlock();
                if(offset_ + full_size > BLOCK_SIZE) {
                    return nullptr;
                }
                alloc_unit_hdr *fc_ptr{(alloc_unit_hdr *)(buffer_ + offset_)};
                fc_ptr->size = aligned_size;
                offset_ += full_size;
                return fc_ptr->user_ptr();
            } else {
                alloc_unit_hdr *fc_ptr{free_lists_[aligned_size - 1].fc};
                free_lists_[aligned_size - 1].fc = fc_ptr->next;
                return fc_ptr->user_ptr();
            }
        }

        bool deallocate(void *ptr, size_t size) {
            alloc_unit_hdr *fc_ptr{(alloc_unit_hdr *)(((uint8_t *)ptr) - ALIGNMENT)};
            size_t aligned_size{((size + ALIGNMENT - 1) & ~(ALIGNMENT - 1))};
            size_t stored_size{fc_ptr->size};
            if(stored_size != aligned_size) {
                return false;
            }
            if(aligned_size > MAX_ALLOC_SIZE) {
                ::free(fc_ptr);
                return true;
            }
            std::unique_lock l{free_lists_[aligned_size - 1].mtp};
            fc_ptr->next = free_lists_[aligned_size - 1].fc;
            free_lists_[aligned_size - 1].fc = fc_ptr;
            return true;
        }

        bool deallocate(void *ptr) {
            alloc_unit_hdr *fc_ptr{(alloc_unit_hdr *)(((uint8_t *)ptr) - ALIGNMENT)};
            size_t stored_size{fc_ptr->size};
            if(stored_size > MAX_ALLOC_SIZE) {
                ::free(fc_ptr);
                return true;
            }
            std::unique_lock l{free_lists_[stored_size - 1].mtp};
            fc_ptr->next = free_lists_[stored_size - 1].fc;
            free_lists_[stored_size - 1].fc = fc_ptr;
            return true;
        }

        void reset() {
            offset_ = 0;
            for(auto &&p: free_lists_) {
                p.fc = nullptr;
            }
        }

    private:
        std::shared_mutex mtp_{};
        alignas(ALIGNMENT) uint8_t buffer_[BLOCK_SIZE];
        size_t offset_{0};
        alignas(64) std::array<thread_safe_free_item, MAX_ALLOC_SIZE> free_lists_{};
    };

}
