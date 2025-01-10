#pragma once

#include "ktxvector.h"


namespace ktx {



template <typename T, typename Allocator>
template <std::input_iterator InputIt,
         std::forward_iterator NoThrowForwardIt>
auto vector<T, Allocator>::uninitialized_copy(
        InputIt first,
        InputIt last,
        NoThrowForwardIt d_first) -> NoThrowForwardIt {
    NoThrowForwardIt current = d_first;
    try {
        for(; first != last; ++first, ++current) {
            alloc_traits::construct(alloc_,
                            std::addressof(*current),
                            *first);
        }
        return current;
    } catch (...) {
        for (; d_first != current; ++d_first) {
            d_first->~T();
        }
        throw;
    }
}

template <typename T, typename Allocator>
vector<T, Allocator>::vector(
        const std::initializer_list<value_type> items, Allocator alloc
        ) : sz_(items.size()), cap_(sz_*2), alloc_(alloc) {
    data_ = alloc_traits::allocate(alloc_, cap_);
    uninitialized_copy(items.begin(), items.end(), data_);
}



};


