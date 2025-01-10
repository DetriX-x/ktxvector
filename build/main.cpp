#include "../ktxvector_realization.h"
#include <iostream>



struct problematic {
    static int count;
    problematic(int val = 0) {
        if(val != 0) {
            std::cerr << "BAD CTOR\n";
            throw 1;
        }
        // if((rand() % (3 - 1 + 1)) + 1 == 2) {
        //     std::cerr << "BAD CTOR\n";
        //     throw 2;
        // }
        count++;
        std::cerr << "SUCCESSFUL CTOR " << count << "\n";
    }

    problematic(const problematic&) {
        if((rand() % (3 - 1 + 1)) + 1 == 2) {
            std::cerr << "BAD COPY CTOR\n";
            throw 2;
        }
        count++;
        std::cerr << "SUCCESSFUL COPY CTOR " << count << "\n";
    }
    
    ~problematic() {
        std::cerr << "SUCCESSFUL DTOR " << count <<  "\n";
        count--;
    }

    friend std::ostream& operator<<(std::ostream& os, 
            const problematic& pb)
    {
        os << pb.s;
        return os;
    }

    std::string s = "problematic #" + std::to_string(count + 1);
};

int problematic::count = 0;


int main() {
    std::srand(std::time(nullptr));
    try {
        ktx::vector<problematic> v({0, 0, 0, 0});
    } catch (...) {
        std::cout << "err\n";
        return 0;
    }
}
