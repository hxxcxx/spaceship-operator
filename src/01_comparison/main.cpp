#include <compare>
#include <iostream>
#include <string>
#include <cmath>
#include <set>
#include <algorithm>
#include <format>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第1章：C++20 比较与运算符 (Spaceship Operator <=>)
// ============================================================

// --- 1.1 默认三路比较 (defaulted <=>) ---
struct Point {
    int x, y;
    auto operator<=>(const Point&) const = default;
};

// --- 1.2 强排序 vs 弱排序 vs 偏序 ---
// strong_ordering: 等价即相等 (如 int, string)
// weak_ordering:   等价但不一定相等 (如大小写不敏感比较)
// partial_ordering: 可能有不可比较值 (如 NaN)

struct CaseInsensitiveString {
    std::string value;

    explicit CaseInsensitiveString(std::string s) : value(std::move(s)) {}

    // 弱排序：大小写不敏感比较，"Hello" 等价于 "hello"
    std::weak_ordering operator<=>(const CaseInsensitiveString& other) const {
        auto to_lower = [](unsigned char c) { return std::tolower(c); };
        std::string a = value, b = other.value;
        std::transform(a.begin(), a.end(), a.begin(), to_lower);
        std::transform(b.begin(), b.end(), b.begin(), to_lower);
        if (a < b) return std::weak_ordering::less;
        if (a > b) return std::weak_ordering::greater;
        return std::weak_ordering::equivalent;
    }

    // 手动提供 == 因为弱排序中等价不代表相等
    bool operator==(const CaseInsensitiveString& other) const {
        return (*this <=> other) == std::weak_ordering::equivalent;
    }
};

// --- 1.3 偏排序 (partial_ordering) ---
struct FloatWrap {
    double value;

    auto operator<=>(const FloatWrap& other) const {
        // double 可以有 NaN，使用 partial_ordering
        if (std::isnan(value) || std::isnan(other.value))
            return std::partial_ordering::unordered;
        if (value < other.value) return std::partial_ordering::less;
        if (value > other.value) return std::partial_ordering::greater;
        return std::partial_ordering::equivalent;
    }

    bool operator==(const FloatWrap& other) const {
        return (*this <=> other) == std::partial_ordering::equivalent;
    }
};

// --- 1.4 自定义比较逻辑 ---
struct Person {
    std::string name;
    int age;

    // 只按年龄比较（演示自定义排序）
    std::strong_ordering operator<=>(const Person& other) const {
        return age <=> other.age;
    }

    // == 需要同时满足 name 和 age
    bool operator==(const Person& other) const {
        return name == other.name && age == other.age;
    }
};

// --- 1.5 反向迭代器自动支持 ---
// 定义了 <=> 后，编译器自动生成 <, <=, >, >=
// 定义了 == 后，编译器自动生成 !=

// --- 1.6 容器中直接使用 ---
void demo_set() {
    std::set<Point> points;
    points.insert({1, 2});
    points.insert({3, 4});
    points.insert({1, 2}); // 重复，不会插入
    std::cout << std::format("  set size: {} (expect 2)\n", points.size());
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第1章：C++20 比较与运算符 ===\n\n";

    // 1.1 默认三路比较
    std::cout << "--- 1.1 默认三路比较 ---\n";
    Point p1{1, 2}, p2{3, 4}, p3{1, 2};
    std::cout << std::format("  p1 < p2: {}\n", p1 < p2);
    std::cout << std::format("  p1 == p3: {}\n", p1 == p3);
    std::cout << std::format("  p1 <=> p2 is less: {}\n\n", (p1 <=> p2) < 0);

    // 1.2 弱排序
    std::cout << "--- 1.2 弱排序 (CaseInsensitive) ---\n";
    CaseInsensitiveString a("Hello"), b("hello"), c("World");
    std::cout << std::format("  \"Hello\" <=> \"hello\": {}\n",
        (a <=> b) == std::weak_ordering::equivalent ? "equivalent" : "different");
    std::cout << std::format("  \"Hello\" <=> \"World\": {}\n\n",
        (a <=> c) == std::weak_ordering::less ? "less" : "not less");

    // 1.3 偏排序
    std::cout << "--- 1.3 偏排序 (Float with NaN) ---\n";
    FloatWrap fw1{1.0}, fw2{2.0}, fw3{std::numeric_limits<double>::quiet_NaN()};
    std::cout << std::format("  1.0 <=> 2.0: {}\n",
        (fw1 <=> fw2) == std::partial_ordering::less ? "less" : "not less");
    std::cout << std::format("  NaN <=> 1.0: {}\n\n",
        (fw3 <=> fw1) == std::partial_ordering::unordered ? "unordered" : "ordered");

    // 1.4 自定义比较逻辑
    std::cout << "--- 1.4 自定义比较逻辑 (Person) ---\n";
    Person alice{"Alice", 30}, bob{"Bob", 25}, alice2{"Alice", 30};
    std::cout << std::format("  Alice(30) < Bob(25) [by age]: {}\n", alice < bob);
    std::cout << std::format("  Alice(30) > Bob(25) [by age]: {}\n", alice > bob);
    std::cout << std::format("  Alice == Alice2 [by name+age]: {}\n\n", alice == alice2);

    // 1.5 自动生成的运算符
    std::cout << "--- 1.5 自动生成的运算符 ---\n";
    std::cout << std::format("  p1 != p2: {}\n", p1 != p2);
    std::cout << std::format("  p1 <= p3: {}\n", p1 <= p3);
    std::cout << std::format("  p2 >= p1: {}\n\n", p2 >= p1);

    // 1.6 容器中使用
    std::cout << "--- 1.6 容器中使用 ---\n";
    demo_set();

    std::cout << "\n";
    return 0;
}
