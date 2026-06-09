// ============================================================
// 第23章: 标准库的改进
// 参考: C++20 Complete Guide Chapter 23
// ============================================================

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <source_location>
#include <bit>
#include <compare>
#include <numbers>
#include <algorithm>
#include <cstdint>
#include <bitset>
#include <cassert>

// ============================================================
// 23.1 字符串的starts_with()和ends_with()
// ============================================================

void demo_starts_ends_with() {
    std::cout << "--- 23.1 starts_with / ends_with ---\n";

    std::string s = "hello world";

    // starts_with
    std::cout << "  \"" << s << "\" starts with \"hello\": "
              << s.starts_with("hello") << "\n";
    std::cout << "  \"" << s << "\" starts with 'h': "
              << s.starts_with('h') << "\n";
    std::cout << "  \"" << s << "\" starts with \"world\": "
              << s.starts_with("world") << "\n";

    // ends_with
    std::cout << "  \"" << s << "\" ends with \"world\": "
              << s.ends_with("world") << "\n";
    std::cout << "  \"" << s << "\" ends with 'd': "
              << s.ends_with('d') << "\n";
    std::cout << "  \"" << s << "\" ends with \"hello\": "
              << s.ends_with("hello") << "\n";

    // string_view也支持
    std::string_view sv = "test.cpp";
    std::cout << "  \"" << sv << "\" ends with \".cpp\": "
              << sv.ends_with(".cpp") << "\n";

    std::cout << "\n";
}

// ============================================================
// 23.2 source_location
// ============================================================

void log_with_location(const std::string& message,
                       const std::source_location& loc = std::source_location::current()) {
    std::cout << "  [" << loc.file_name() << ":" << loc.line()
              << " `" << loc.function_name() << "`] "
              << message << "\n";
}

void demo_source_location() {
    std::cout << "--- 23.2 source_location ---\n";

    // 基本用法
    auto loc = std::source_location::current();
    std::cout << "  file: " << loc.file_name() << "\n";
    std::cout << "  line: " << loc.line() << "\n";
    std::cout << "  function: " << loc.function_name() << "\n";

    // 作为日志函数的默认参数
    log_with_location("something happened");
    log_with_location("another event");

    std::cout << "\n";
}

// ============================================================
// 23.3 安全的整数比较
// ============================================================

void demo_safe_cmp() {
    std::cout << "--- 23.3 安全的整数比较 ---\n";

    // 经典问题: int与unsigned比较产生错误结果
    int i = -1;
    unsigned int u = 10;
    std::cout << "  -1 < 10u (builtin): " << (i < u) << "  (错误!)\n";

    // C++20: std::cmp_* 系列函数
    std::cout << "  -1 < 10u (cmp_less): " << std::cmp_less(i, u) << "  (正确!)\n";
    std::cout << "  -1 cmp_equal 10u: " << std::cmp_equal(i, u) << "\n";
    std::cout << "  -1 cmp_greater 10u: " << std::cmp_greater(i, u) << "\n";

    // cmp_less_equal, cmp_greater_equal
    std::cout << "  10 cmp_less_equal 10u: " << std::cmp_less_equal(10, 10u) << "\n";

    // in_range: 检查值是否在目标类型范围内
    std::cout << "  in_range<int>(-1): " << std::in_range<int>(-1) << "\n";
    std::cout << "  in_range<unsigned>(-1): " << std::in_range<unsigned>(-1) << "\n";

    std::cout << "\n";
}

// ============================================================
// 23.4 ssize()
// ============================================================

void demo_ssize() {
    std::cout << "--- 23.4 ssize() ---\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // size() 返回 size_t (无符号)
    // ssize() 返回有符号类型, 避免无符号运算陷阱
    std::cout << "  size(): " << v.size() << " (type: unsigned)\n";
    std::cout << "  ssize(): " << std::ssize(v) << " (type: signed)\n";

    // 可以安全地做减法
    for (auto i = std::ssize(v) - 1; i >= 0; --i) {
        std::cout << "  v[" << i << "] = " << v[static_cast<size_t>(i)] << "\n";
    }

    std::cout << "\n";
}

// ============================================================
// 23.5 数学常量
// ============================================================

void demo_math_constants() {
    std::cout << "--- 23.5 数学常量 (std::numbers) ---\n";

    // <numbers> 头文件
    std::cout << "  pi: " << std::numbers::pi << "\n";
    std::cout << "  e: " << std::numbers::e << "\n";
    std::cout << "  ln2: " << std::numbers::ln2 << "\n";
    std::cout << "  ln10: " << std::numbers::ln10 << "\n";
    std::cout << "  sqrt2: " << std::numbers::sqrt2 << "\n";
    std::cout << "  sqrt3: " << std::numbers::sqrt3 << "\n";
    std::cout << "  inv_pi: " << std::numbers::inv_pi << "\n";
    std::cout << "  inv_sqrtpi: " << std::numbers::inv_sqrtpi << "\n";
    std::cout << "  egamma: " << std::numbers::egamma << "\n";
    std::cout << "  phi: " << std::numbers::phi << "\n";

    // 使用float版本
    std::cout << "  pi_v<float>: " << std::numbers::pi_v<float> << "\n";

    std::cout << "\n";
}

// ============================================================
// 23.6 位操作
// ============================================================

void demo_bit_ops() {
    std::cout << "--- 23.6 位操作 (std::bit) ---\n";

    // rotl / rotr: 循环移位
    uint8_t val = 0b00001101;
    std::cout << "  rotl(0b00001101, 2): " << std::bitset<8>(std::rotl(val, 2)) << "\n";
    std::cout << "  rotr(0b00001101, 2): " << std::bitset<8>(std::rotr(val, 2)) << "\n";

    // popcount: 计算1的个数
    std::cout << "  popcount(0b00001101): " << std::popcount(val) << "\n";
    std::cout << "  popcount(0xFF): " << std::popcount(static_cast<uint8_t>(0xFF)) << "\n";

    // countl_zero: 从高位开始连续0的个数
    std::cout << "  countl_zero(0b00001101): " << std::countl_zero(val) << "\n";
    // countl_one: 从高位开始连续1的个数
    std::cout << "  countl_one(0b11110000): "
              << std::countl_one(static_cast<uint8_t>(0xF0)) << "\n";
    // countr_zero: 从低位开始连续0的个数
    std::cout << "  countr_zero(0b00001101): " << std::countr_zero(val) << "\n";
    // countr_one
    std::cout << "  countr_one(0b00001101): " << std::countr_one(val) << "\n";

    // has_single_bit: 是否只有1个1(2的幂)
    std::cout << "  has_single_bit(8): " << std::has_single_bit(static_cast<uint8_t>(8)) << "\n";
    std::cout << "  has_single_bit(6): " << std::has_single_bit(static_cast<uint8_t>(6)) << "\n";

    // bit_width: 表示该值需要的位数
    std::cout << "  bit_width(13): " << std::bit_width(static_cast<uint8_t>(13)) << "\n";
    std::cout << "  bit_width(16): " << std::bit_width(static_cast<uint8_t>(16)) << "\n";

    // bit_floor / bit_ceil: 最近的不大于/不小于的2的幂
    std::cout << "  bit_floor(13): " << (int)std::bit_floor(static_cast<uint8_t>(13)) << "\n";
    std::cout << "  bit_ceil(13): " << (int)std::bit_ceil(static_cast<uint8_t>(13)) << "\n";

    std::cout << "\n";
}

// ============================================================
// 23.7 bit_cast
// ============================================================

void demo_bit_cast() {
    std::cout << "--- 23.7 bit_cast ---\n";

    // bit_cast: 在不同类型间做位级转换
    float f = 3.14f;
    auto bits = std::bit_cast<uint32_t>(f);
    std::cout << "  float 3.14f -> bits: 0x" << std::hex << bits << std::dec << "\n";

    auto back = std::bit_cast<float>(bits);
    std::cout << "  bits -> float: " << back << "\n";

    // double <-> uint64_t
    double d = 2.718281828;
    auto dbits = std::bit_cast<uint64_t>(d);
    std::cout << "  double 2.718... -> bits: 0x" << std::hex << dbits << std::dec << "\n";
    std::cout << "  bits -> double: " << std::bit_cast<double>(dbits) << "\n";

    std::cout << "\n";
}

// ============================================================
// 23.8 std::endian
// ============================================================

void demo_endian() {
    std::cout << "--- 23.8 std::endian ---\n";

    if constexpr (std::endian::native == std::endian::little) {
        std::cout << "  本机字节序: little-endian\n";
    } else if constexpr (std::endian::native == std::endian::big) {
        std::cout << "  本机字节序: big-endian\n";
    } else {
        std::cout << "  本机字节序: mixed-endian\n";
    }

    std::cout << "\n";
}

// ============================================================
// 23.9 shift_left / shift_right算法
// ============================================================

void demo_shift_algorithms() {
    std::cout << "--- 23.9 shift_left / shift_right ---\n";

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9};

    // shift_left: 向左移动元素, 左边的被丢弃
    auto v1 = v;
    auto it1 = std::shift_left(v1.begin(), v1.end(), 3);
    std::cout << "  shift_left(3): ";
    for (int x : v1) std::cout << x << " ";
    std::cout << "(new end value: " << *it1 << ")\n";

    // shift_right: 向右移动元素, 左边填充未指定值
    auto v2 = v;
    auto it2 = std::shift_right(v2.begin(), v2.end(), 3);
    std::cout << "  shift_right(3): ";
    for (int x : v2) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第23章: 标准库的改进 ===\n\n";

    demo_starts_ends_with();
    demo_source_location();
    demo_safe_cmp();
    demo_ssize();
    demo_math_constants();
    demo_bit_ops();
    demo_bit_cast();
    demo_endian();
    demo_shift_algorithms();

    return 0;
}
