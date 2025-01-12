#pragma once

#include "ktxvector.h"
#include <exception>


namespace ktx {



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

template<typename T, typename Allocator>
vector<T, Allocator>::vector(const vector<T, Allocator>& other) {
    auto newdata = alloc_traits::allocate(alloc_, other.cap_);
    try {
        uninitialized_move(other.data_,
                           other.data_ + other.sz_,
                           newdata);
    } catch(...) {
        alloc_traits::deallocate(alloc_, newdata, other.cap_);
    }

    sz_ = other.sz_;
    cap_ = other.cap_;
    data_ = newdata;
}

template <typename T, typename Allocator>
template <std::input_iterator InputIt,
         std::forward_iterator NoThrowForwardIt>
auto vector<T, Allocator>::uninitialized_move(
        InputIt first,
        InputIt last,
        NoThrowForwardIt d_first) -> NoThrowForwardIt {
    auto current = d_first;
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
    bool isConstructed {false};
    try {
        alloc_traits::construct(
                alloc_,
                newdata + sz_,
                std::forward<Args>(args)...);
        isConstructed = true;
        uninitialized_move(data_, data_ + sz_, newdata);
    } catch(...) {
        if (isConstructed) {
            alloc_traits::destroy(alloc_, newdata + sz_);
        }
        alloc_traits::deallocate(alloc_, newdata, newcap);
        throw;
    }

    destroy_all();
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
        uninitialized_move(data_, data_ + sz_, newdata);
    } catch(...) {
        alloc_traits::deallocate(alloc_, newdata, newcap);
        throw;
    }

    destroy_all();
    alloc_traits::deallocate(alloc_, data_, cap_);
    cap_ = newcap;
    data_ = newdata;
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

    destroy_all();
    sz_ = 0;
}

template<typename T, typename Allocator>
vector<T, Allocator>::~vector() {
    clear();
    alloc_traits::deallocate(alloc_, data_, cap_);
}

template<typename T, typename Allocator>
void vector<T, Allocator>::destroy_all() {
    for (auto current = data_; current != data_ + sz_; ++current) {
        alloc_traits::destroy(alloc_, current);
    }
}

template<typename T, typename Allocator>
template <typename Self>
constexpr auto vector<T, Allocator>::at(this Self&& self,
                                        size_type index) -> 
std::conditional_t<
    std::is_const_v<std::remove_reference_t<Self>>,
    const_reference,
    reference> {
        if (index >= self.sz_) {
            throw std::out_of_range{"Index is out of range of vector"};
        }
        return std::forward<Self>(self).data_[index];
    }

template<typename T, typename Allocator>
void swap(vector<T, Allocator>& to, vector<T, Allocator>& from) {
    std::swap(to.sz_, from.sz_);
    std::swap(to.cap_, from.cap_);
    std::swap(to.data_, from.data_);
}



};


