#include <compare>
#include <iostream>
#include <string>

struct Version {
    int major;
    int minor;
    int patch;

    auto operator<=>(const Version&) const = default;
};

int main() {
    Version v1{1, 0, 0};
    Version v2{2, 0, 0};
    Version v3{1, 0, 0};

    std::cout << "spaceship-operator demo\n\n";

    // Three-way comparison
    auto result = v1 <=> v2;
    if (result == std::strong_ordering::less)
        std::cout << "v1 < v2\n";
    if (v1 == v3)
        std::cout << "v1 == v3\n";
    if (v2 > v1)
        std::cout << "v2 > v1\n";

    return 0;
}
