#include <fstream>
#include <iostream>

int main() {
    std::ifstream file("wspr_normal.bits", std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }

    for (int i = 0; i < 14; ++i) {
        char c;
        file.read(&c, 1);
        std::cout << static_cast<int>(c) << " ";
    }

    std::cout << "\n";
    return 0;
}

