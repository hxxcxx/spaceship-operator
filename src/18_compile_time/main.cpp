// ============================================================
// 第18章: 编译期计算 (Compile-Time Computation)
// 参考: C++20 Complete Guide Chapter 18
// ============================================================

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <cassert>
#include <type_traits>
#include <cstring>
#include <cmath>

// ============================================================
// 18.1 关键字 constinit
// ============================================================

constexpr int getMagicVal() { return 42; }

// constinit: 强制编译期初始化(可变)
constinit int g_magicVal = getMagicVal();

void demo_constinit() {
    std::cout << "--- 18.1 constinit ---\n";

    // constinit变量可修改(不像constexpr)
    std::cout << "  g_magicVal = " << g_magicVal << "\n";
    g_magicVal *= 2;
    std::cout << "  g_magicVal*2 = " << g_magicVal << "\n";
    g_magicVal = getMagicVal(); // 重置

    // 局部静态变量也可以constinit
    // static constinit int counter = 0;  // 保证编译期初始化

    // constinit 与 constexpr 的区别:
    // constexpr => const + 编译期初始化
    // constinit => 编译期初始化(可变)
    // const = constinit - 编译期保证
    std::cout << "  constexpr = const + 编译期初始化\n";
    std::cout << "  constinit = 编译期初始化(可变)\n";
    std::cout << "\n";
}

// ============================================================
// 18.2 关键字 consteval
// ============================================================

// constexpr: 可在编译期或运行时调用
constexpr bool isPrime(int value) {
    for (int i = 2; i <= value / 2; ++i) {
        if (value % i == 0) return false;
    }
    return value > 1;
}

// consteval: 必须在编译期调用(立即函数)
consteval int squareC(int x) {
    return x * x;
}

// constexpr: 编译期和运行时都可以
constexpr int squareCR(int x) {
    return x * x;
}

template<int Num>
consteval std::array<int, Num> primeNumbers() {
    std::array<int, Num> primes{};
    int idx = 0;
    for (int val = 2; idx < Num; ++val) {
        if (isPrime(val)) {
            primes[idx++] = val;
        }
    }
    return primes;
}

void demo_consteval() {
    std::cout << "--- 18.2 consteval ---\n";

    // consteval函数: 必须用编译期值调用
    constexpr auto sq = squareC(7);
    std::cout << "  squareC(7) = " << sq << "\n";

    // constexpr函数: 可以用运行时值调用
    int x = 5;
    std::cout << "  squareCR(" << x << ") = " << squareCR(x) << "\n";

    // consteval函数计算质数
    constexpr auto primes = primeNumbers<10>();
    std::cout << "  first 10 primes: ";
    for (auto p : primes) std::cout << p << " ";
    std::cout << "\n";

    // consteval vs constexpr:
    // - consteval: 只能编译时调用
    // - constexpr: 编译时和运行时都可以
    // - 普通: 只能运行时
    std::array<int, squareC(5)> arr{};  // OK: 编译时计算
    std::cout << "  array size = " << arr.size() << "\n";
    std::cout << "\n";
}

// ============================================================
// 18.2.3 consteval限制和调用链
// ============================================================

consteval int nextValue(int val) {
    return val + 1;
}

void demo_consteval_chain() {
    std::cout << "--- 18.2.3 consteval调用链 ---\n";

    // consteval可以调用consteval
    constexpr auto v1 = nextValue(41);
    std::cout << "  nextValue(41) = " << v1 << "\n";

    // consteval可以调用constexpr
    constexpr bool check = isPrime(7);  // constexpr被consteval上下文调用
    std::cout << "  isPrime(7) in consteval context = " << check << "\n";
    std::cout << "\n";
}

// ============================================================
// 18.4 std::is_constant_evaluated()
// ============================================================

constexpr int strLen(const char* s) {
    if (std::is_constant_evaluated()) {
        // 编译时: 手动计算
        int idx = 0;
        while (s[idx] != '\0') ++idx;
        return idx;
    } else {
        // 运行时: 使用strlen
        return static_cast<int>(std::strlen(s));
    }
}

void demo_is_constant_evaluated() {
    std::cout << "--- 18.4 std::is_constant_evaluated() ---\n";

    // 编译时上下文: 使用then分支
    constexpr int len1 = strLen("hello");
    std::cout << "  constexpr len(\"hello\") = " << len1 << "\n";

    // 运行时上下文: 使用else分支
    int len2 = strLen("world");
    std::cout << "  runtime len(\"world\") = " << len2 << "\n";

    // 注意事项:
    // - 不要在 if constexpr 中使用(总是true)
    // - 不要在 consteval 函数中使用(总是true)
    // - 只在 constexpr 函数中有意义
    std::cout << "  (只在constexpr函数中有意义)\n";
    std::cout << "\n";
}

// ============================================================
// 18.5 编译期使用vector和string
// ============================================================

// constexpr函数中使用vector(编译期分配,编译期释放)
constexpr double computeAvg() {
    std::vector<int> v{0, 8, 15, 42, 4, 77};
    v.push_back(0);
    std::ranges::sort(v);
    auto newEnd = std::unique(v.begin(), v.end());
    auto sum = std::accumulate(v.begin(), newEnd, 0);
    return static_cast<double>(sum) / static_cast<double>(v.size());
}

// consteval: 合并值并排序后返回固定大小数组
template<int Sz>
consteval auto mergeSorted(std::array<int, Sz> input, int extra) {
    // 使用编译时vector操作
    std::vector<int> v(input.begin(), input.end());
    v.push_back(extra);
    std::ranges::sort(v);

    // 转为array返回
    std::array<int, Sz + 1> arr{};
    for (int i = 0; i < static_cast<int>(v.size()) && i < Sz + 1; ++i) {
        arr[i] = v[i];
    }
    return arr;
}

// 编译时字符串操作
consteval std::size_t hashString(const char* str) {
    std::size_t hash = 5381;
    while (*str != '\0') {
        hash = hash * 33 ^ static_cast<unsigned char>(*str);
        ++str;
    }
    return hash;
}

void demo_compile_time_containers() {
    std::cout << "--- 18.5 编译期使用vector ---\n";

    // 编译时计算: vector操作
    constexpr auto avg = computeAvg();
    std::cout << "  compile-time avg = " << avg << "\n";

    // 编译时合并和排序
    constexpr std::array<int, 5> orig{8, 3, 15, 42, 4};
    constexpr auto merged = mergeSorted(orig, 7);
    std::cout << "  merged & sorted: ";
    for (auto v : merged) std::cout << v << " ";
    std::cout << "\n";

    // 编译时字符串哈希
    constexpr auto h1 = hashString("hello");
    constexpr auto h2 = hashString("world");
    std::cout << "  hash(\"hello\") = " << h1 << "\n";
    std::cout << "  hash(\"world\") = " << h2 << "\n";
    std::cout << "\n";
}

// ============================================================
// 18.5.2 编译时字符串导出到运行时
// ============================================================

// 编译时: 整数转字符串
constexpr std::string intToString(long long value) {
    if (std::is_constant_evaluated()) {
        if (value == 0) return "0";
        if (value < 0) return "-" + intToString(-value);
        auto higher = intToString(value / 10);
        if (value < 10) return std::string(1, '0' + value);
        return higher + std::string(1, '0' + value % 10);
    }
    return std::to_string(value);
}

// 编译时字符串转array导出
template<int MaxSize>
consteval auto toRuntimeString(std::string_view s) {
    std::array<char, MaxSize + 1> arr{};
    for (std::size_t i = 0; i < s.size() && i < MaxSize; ++i) {
        arr[i] = s[i];
    }
    return std::pair{arr, s.size()};
}

void demo_compile_time_string() {
    std::cout << "--- 18.5.2 编译时字符串 ---\n";

    // 编译时字符串导出: 使用简单字符串
    constexpr auto result = toRuntimeString<50>("C++20 compile-time");
    std::string s(result.first.data(), result.second);
    std::cout << "  exported: \"" << s << "\"\n";
    std::cout << "\n";
}

// ============================================================
// 18.6 constexpr扩展
// ============================================================

void demo_constexpr_extensions() {
    std::cout << "--- 18.6 constexpr扩展 ---\n";

    // C++20放宽的constexpr限制:
    // - 编译期可以使用堆内存(vector, string)
    // - 可以使用虚函数
    // - 可以使用dynamic_cast和typeid
    // - 可以使用try-catch(但不可throw)
    // - 可以使用union

    // 演示: constexpr算法
    constexpr auto sorted = []() consteval {
        std::array<int, 6> a{8, 3, 15, 42, 4, 77};
        std::ranges::sort(a);
        return a;
    }();
    std::cout << "  sorted: ";
    for (auto v : sorted) std::cout << v << " ";
    std::cout << "\n";
    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第18章: 编译期计算 ===\n\n";

    demo_constinit();
    demo_consteval();
    demo_consteval_chain();
    demo_is_constant_evaluated();
    demo_compile_time_containers();
    demo_compile_time_string();
    demo_constexpr_extensions();

    return 0;
}
