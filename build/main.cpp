#include "../ktxvector_realization.h"
#include <iostream>


int main() {
    ktx::vector<int> v({1, 2, 3, 4});
    v[2] = 0;
    std::cout << v[2] << '\n';


}
