#pragma once

#include "ktxvector.h"


namespace ktx {



template <typename T, typename Allocator>
template <std::input_iterator InputIt,
         std::forward_iterator NoThrowForwardIt>
auto vector<T, Allocator>::uninitialized_move(
        InputIt first,
        InputIt last,
        NoThrowForwardIt d_first) -> NoThrowForwardIt {
    NoThrowForwardIt current = d_first;
    try {
        for(; first != last; ++first, ++current) {
            alloc_traits::construct(alloc_,
                            std::addressof(*current),
                            std::move_if_noexcept(*first));
        }
        return current;
    } catch (...) {
        for (; d_first != current; ++d_first) {
            alloc_traits::destroy(alloc_, d_first);
        }
        throw;
    }
}

template <typename T, typename Allocator>
vector<T, Allocator>::vector(
        const std::initializer_list<value_type> items, Allocator alloc
        ) : sz_(items.size()), cap_(sz_*2), alloc_(alloc) {
    data_ = alloc_traits::allocate(alloc_, cap_);
    try {
        uninitialized_move(items.begin(), items.end(), data_);
    } catch (...) {
        alloc_traits::deallocate(alloc_, data_, cap_);
        throw;
    }
}

template <typename T, typename Allocator>
template <typename... Args>
void vector<T, Allocator>::emplace_back(Args&&... args) {
    if (sz_ < cap_) {
        alloc_traits::construct(
                alloc_,
                data_ + sz_,
                std::forward<Args>(args)...);
        ++sz_;
        return;
    }

    auto newcap = cap_ > 0 ? cap_ * expansion : 2;
    auto newdata = alloc_traits::allocate(alloc_, newcap);
    try {
        alloc_traits::construct(
                alloc_,
                newdata + sz_,
                std::forward<Args>(args)...);
        uninitialized_move(data_, data_ + sz_, newdata);
    } catch(...) {
        alloc_traits::deallocate(alloc_, newdata, newcap);
        throw;
    }

    clear();
    alloc_traits::deallocate(alloc_, data_, cap_);

    ++sz_;
    cap_ = newcap;
    data_ = newdata;
}

template<typename T, typename Allocator>
void vector<T, Allocator>::reserve(size_type newcap) {
    if (newcap <= cap_) {
        return;
    }

    auto newdata = alloc_traits::allocate(alloc_, newcap);
    try {
        uninitialized_move(data_, data_ + sz_, newcap);
    } catch(...) {
        alloc_traits::deallocate(alloc_, newdata, newcap);
        throw;
    }
    cap_ = newcap;
}

template<typename T, typename Allocator>
void vector<T, Allocator>::push_back(value_type value) { 
    emplace_back(std::move(value));
}

template<typename T, typename Allocator>
void vector<T, Allocator>::clear() {
    if (0 == cap_) {
        return;
    }
    for (auto current = data_; current != data_ + sz_; ++current) {
        alloc_traits::destroy(alloc_, current);
    }
    sz_ = 0;
}

template<typename T, typename Allocator>
vector<T, Allocator>::~vector() {
    clear();
    alloc_traits::deallocate(alloc_, data_, cap_);
}



};


