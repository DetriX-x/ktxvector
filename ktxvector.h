#include <iterator>

namespace ktx {



template <typename T, typename AllocatorType>
class vector {
public:
    using value_type = T;
    using pointer = T*;
private:
    size_t sz_;
    size_t cap_;
    pointer data_;

public:

};



}

