#pragma once

#include "ktxvector.h"


namespace ktx {



// public

template<typename T, typename Allocator>
vector<T, Allocator>::vector(
        const std::initializer_list<value_type> items, Allocator alloc
        ) : sz_(items.size()), cap_(sz_*expansion), alloc_(alloc) {
    data_ = create_from(alloc_, cap_, items.begin(), items.end());
}

template<typename T, typename Allocator>
vector<T, Allocator>::vector(size_type n, const T& val, Allocator a) 
    : sz_{n}, cap_{n ? n * expansion : 2}, alloc_(a) {
    auto newdata = alloc_traits::allocate(alloc_, cap_);
    auto current = newdata;
    try {
        for (; current != newdata + sz_; ++current) {
            alloc_traits::construct(alloc_, current, val);
        }
    } catch (...) {
        for (auto first = newdata; first != current; ++first) {
            alloc_traits::destroy(alloc_, first);
        }
        alloc_traits::deallocate(alloc_, newdata, cap_);
        throw;
    }

    data_ = newdata;
}

template<typename T, typename Allocator>
vector<T, Allocator>::vector(const vector<T, Allocator>& other) {
    auto newdata = create_from(
            alloc_,
            other.cap_,
            other.data_,
            other.data_ + other.sz_);

    sz_ = other.sz_;
    cap_ = other.cap_;
    data_ = newdata;
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

    auto newdata = create_from(
            alloc_,
            newcap,
            data_,
            data_ + sz_);

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
void vector<T, Allocator>::resize(size_type count, const value_type& val) {
    auto size = sz_;
    if (count > cap_) {
        reserve(count);
    }
    if (count < size) {
        for (auto i = 0; i != size - count; ++i) {
            pop_back();
        }
    } else if (count > sz_) {
        for (auto i = 0; i != count - size; ++i) {
            push_back(val);
        }
    }
}

template<typename T, typename Allocator>
void vector<T, Allocator>::resize(size_type count) {
    resize(count, T());
}

template<typename T, typename Allocator>
void vector<T, Allocator>::shrink_to_fit() {
    if (0 == sz_) {
        destroy_all();
        sz_ = cap_ = 0;
        return;
    }
    if (cap_ == sz_) {
        return;
    }
    auto newdata = create_from(alloc_, sz_, data_, data_ + sz_);

    destroy_all();
    alloc_traits::deallocate(alloc_, data_, cap_);

    data_ = newdata;
    cap_ = sz_;
}

template <typename T, typename Allocator>
void vector<T, Allocator>::pop_back() {
    alloc_traits::destroy(alloc_, data_ + sz_ - 1);
    --sz_;
}

// private

template<typename T, typename Allocator>
void vector<T, Allocator>::destroy_all() {
    std::for_each(data_, data_ + sz_, [this](auto&& obj) {
                alloc_traits::destroy(
                        alloc_,
                        &std::forward<decltype(obj)>(obj));
            });
}

template <typename T, typename Allocator>
template <std::input_iterator InputIt>
vector<T, Allocator>::pointer vector<T, Allocator>::create_from(
        Allocator alloc,
        size_type count,
        InputIt first,
        InputIt last) {
    auto newdata = alloc_traits::allocate(alloc, count);
    try {
        uninitialized_move(first,
                           last,
                           newdata);
    } catch(...) {
        alloc_traits::deallocate(alloc, newdata, count);
        throw;
    }
    return newdata;
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

template<typename T, typename Allocator>
template<typename... Args>
void vector<T, Allocator>::emplace(const_iterator pos, Args&&... args) {
    if (pos == end()) {
        emplace_back(std::forward<Args>(args)...);
        return;
    }
    if (sz_ < cap_) {
        alloc_traits::construct(
                alloc_,
                data_ + sz_,
                std::move_if_noexcept(data_[sz_ - 1])
                );
        auto it = data_ + sz_ - 1;
        for (; it != &*pos; --it) {
            *it = *(it - 1);
        }
        value_type value {std::forward<Args>(args)...};
        *it = std::move(value);
        ++sz_;
    }
    // TODO:
}

template<typename T, typename Allocator>
void vector<T, Allocator>::insert(const_iterator pos, value_type value) {
    emplace(pos, std::move(value));
}

template<typename T, typename Allocator>
vector<T, Allocator>::iterator vector<T, Allocator>::erase(const_iterator pos) {

}

// friend

template<typename T, typename Allocator>
void swap(vector<T, Allocator>& to, vector<T, Allocator>& from) {
    std::swap(to.sz_, from.sz_);
    std::swap(to.cap_, from.cap_);
    std::swap(to.data_, from.data_);
}


};


