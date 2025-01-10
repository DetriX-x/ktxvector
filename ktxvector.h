#pragma once


#include <initializer_list>
#include <memory>
#include <iterator>
#include <iostream>

namespace ktx {



template <typename T, typename Allocator = std::allocator<T>>
class vector {
private:
    template <bool isConst>
    class base_iterator;

public:
    using value_type = T;
    using size_type = std::size_t;
    using allocator_type = Allocator;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename 
        std::allocator_traits<Allocator>::const_pointer;
    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using alloc_traits = std::allocator_traits<allocator_type>;
    

private:
    size_type sz_;
    size_type cap_;
    pointer data_;
    [[no_unique_address]] allocator_type alloc_;


public:
    // constructors
    vector() : sz_{0u}, cap_{0u}, data_{nullptr}, alloc_{} {}
    vector(std::initializer_list<value_type> list, 
            Allocator alloc = Allocator());

    // accessors and modifiers
    template <typename Self>
    constexpr auto operator[](this Self&& self, size_type index) -> 
    std::conditional_t<
        std::is_const_v<std::remove_reference_t<Self>>,
        const_reference,
        reference> {
        return std::forward<Self>(self).data_[index];
    }

private:
template <std::input_iterator InputIt,
         std::forward_iterator NoThrowForwardIt>
auto uninitialized_copy(
        InputIt first,
        InputIt last,
        NoThrowForwardIt d_first) -> NoThrowForwardIt;

};


}
