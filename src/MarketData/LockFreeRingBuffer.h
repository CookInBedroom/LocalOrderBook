#ifndef LOCKFREERINGBUFFER_H
#define LOCKFREERINGBUFFER_H

#include <atomic> // thread-safe atomic variables
#include <bit> // std::is_trivially_copyable_v
#include <vector>

namespace MarketData {
    template<typename T, std::size_t Size>
    class LockFreeRingBuffer {
        // sizing the buffer to a power of 2 is important for computational efficiency when it comes to calculating the next index in its circular structure.
        // Size = how many 8-byte items the buffer can hold
        static_assert(std::has_single_bit(Size), "Size must be a power of two.");
        // T has to be trivially copyable for an optimised memory management.
        // It ensures copying/moving an item involves nothing more than moving bits across CPU registers, guaranteed to be fast, safe, and thread friendly.
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable. (e.g., a fixed-size POD struct).");
        // Example: Size = 8 -> mask = 7, so 8 & 7 = 0 (index 8 wraps around to 0).
        // static -> one copy per class
        // constexpr -> computed at compile time, and immutable (read-only)
        static constexpr std::size_t kMask = Size - 1;
        // Cache line padding adding to thread variables' memory addresses, such as head_ and tail_ used by the ring buffer
        // Adding padding to the variables' memory addresses prevents false cache line sharing between producer/consumer cores
        // in Apple, CPU cache line size can be checked with command "sysctl hw.cachelinesize" thru terminal
        static constexpr std::size_t kCacheLine = 64;

    public:
        // Producer thread pushes stream data into the buffer
        // item is a read-only reference, so no copy is made when it's passed into the function
        bool try_push(const T& item) {
            // head_ is only producer thread modified, so reading it with memory_order_relaxed for the fastest performance
            const std::size_t head = head_.load(std::memory_order_relaxed);

            if (head - cached_tail_ == Size) {
                // buffer is full depending on the cached tail value
                // so update cached_tail_ with the latest tail_ using memory_order_acquire from the memory_order_release store
                // release-acquire makes sure thread operations are in strict order as how code is written without CPU aggressive optimisation
                // so when tail_ is loaded, it is guaranteed that the consumer has indeed consumed the slot before _tail is incremented.
                cached_tail_ = tail_.load(std::memory_order_acquire);

                if (head - cached_tail_ == Size) { return false; } // buffer is really full
            }

            buffer_[head & kMask] = item; // write the item into the buffer

            // Update head_ to the release store, so when head_ is loaded with acquire, the an item is guaranteed finished adding to the buffer.
            head_.store(head + 1, std::memory_order_release);

            return true;
        }

        // Consumer thread pops stream data from the buffer
        bool try_pop(T& out) {
            const std::size_t tail = tail_.load(std::memory_order_relaxed);

            if (tail == cached_head_) {
                cached_head_ = tail_.load(std::memory_order_acquire);

                if (tail == cached_tail_) { return false; } // buffer is really empty
            }

            out = buffer_[tail & kMask];

            // The pop doesn't remove the item from the buffer,
            // instead, it just moves the tail_ forward to the next slot, and the slot is treated empty.
            tail_.store(tail + 1, std::memory_order_release);

            return true;
        }

    private:
        // Atomic collapses the fetch from RAM -> execute in Core -> store back to RAM, three steps into one indivisible operation.
        // Upon modifying, it invalidates local copies in all other CPU cores
        // The hardware momentarily pause other cores' read/write until the atomic operation finishes.

        // alignas() tells the compiler to put this member at a memory address that's the start of a cache line.
        // Addresses like, 0x0064, 0x0128 etc..

        alignas(kCacheLine) std::atomic<std::size_t> head_ {0};
        std::size_t cached_tail_ {0}; // only assessed by the producer

        alignas(kCacheLine) std::atomic<std::size_t> tail_ {0};
        std::size_t cached_head_ {0}; // only accessed by the consumer

        std::vector<T> buffer_ = std::vector<T>(Size);
    };
}

#endif
