// ============================================================
// 第21章: 核心语言的小改进
// 参考: C++20 Complete Guide Chapter 21
// ============================================================

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <cstdint>

// ============================================================
// 21.1 带初始化的基于范围的for循环
// ============================================================

void demo_for_init() {
    std::cout << "--- 21.1 带初始化的for循环 ---\n";

    std::vector<std::string> coll{"hello", "world", "C++20"};

    // C++20: for循环带初始化语句
    for (int i = 1; const auto& elem : coll) {
        std::cout << "  " << i << ": " << elem << "\n";
        ++i;
    }

    // 解决临时对象生命周期问题
    // for (auto&& optColl = getValues(); int i : optColl) { ... }
    std::cout << "\n";
}

// ============================================================
// 21.2 对枚举值使用using
// ============================================================

enum class Status { open, progress, done = 9 };

void printStatus(Status s) {
    switch (s) {
        using enum Status;  // C++20: 使枚举值在当前作用域可用
        case open:
            std::cout << "  open\n"; break;
        case progress:
            std::cout << "  in progress\n"; break;
        case done:
            std::cout << "  done\n"; break;
    }
}

void demo_using_enum() {
    std::cout << "--- 21.2 using enum ---\n";

    printStatus(Status::open);
    printStatus(Status::progress);
    printStatus(Status::done);

    // using enum 也可以对特定值使用using声明
    // using Status::open, Status::done;
    std::cout << "\n";
}

// ============================================================
// 21.3 将枚举委托到不同作用域
// ============================================================

namespace MyProject {
    class Task {
    public:
        enum class Priority { low, medium, high };
    };
    using enum Task::Priority;  // 暴露Priority的值到MyProject作用域
}

void demo_enum_delegation() {
    std::cout << "--- 21.3 枚举委托 ---\n";

    // 可以直接使用MyProject作用域下的枚举值
    auto p = MyProject::low;
    std::cout << "  priority value: " << static_cast<int>(p) << "\n";
    std::cout << "\n";
}

// ============================================================
// 21.4 新字符类型char8_t
// ============================================================

void demo_char8_t() {
    std::cout << "--- 21.4 char8_t ---\n";

    // C++20: char8_t是新的UTF-8字符类型
    char8_t c = u8'@';
    const char8_t* s = u8"hello";

    std::cout << "  char8_t value: " << static_cast<int>(c) << "\n";
    // C++20: cout不直接支持char8_t*, 需要reinterpret_cast
    std::cout << "  u8 string: " << reinterpret_cast<const char*>(s) << "\n";

    // char8_t字符串类型
    std::u8string u8s = u8"C++20";
    std::cout << "  u8string size: " << u8s.size() << "\n";
    std::cout << "\n";
}

// ============================================================
// 21.5 聚合体的改进
// ============================================================

struct Value {
    double amount = 0;
    int precision = 2;
    std::string unit = "Dollar";
};

struct Aggr {
    std::string msg;
    int val;
};

void demo_aggregate() {
    std::cout << "--- 21.5 聚合体改进 ---\n";

    // 21.5.1 指定初始化器
    Value v1{100};
    Value v2{.amount = 100, .unit = "Euro"};
    Value v3{.precision = 8, .unit = "$"};
    std::cout << "  v1: " << v1.amount << " " << v1.unit << "\n";
    std::cout << "  v2: " << v2.amount << " " << v2.unit << "\n";
    std::cout << "  v3: " << v3.amount << " " << v3.precision << " " << v3.unit << "\n";

    // 21.5.2 括号聚合初始化
    Aggr a1{"hello", 42};    // 花括号
    Aggr a2("hi", 99);       // C++20: 括号也可以
    std::cout << "  a1: " << a1.msg << ", " << a1.val << "\n";
    std::cout << "  a2: " << a2.msg << ", " << a2.val << "\n";

    // C++20: make_unique/make_shared支持聚合体
    auto up = std::make_unique<Aggr>("Rome", 200);
    std::cout << "  unique_ptr: " << up->msg << ", " << up->val << "\n";
    std::cout << "\n";
}

// ============================================================
// 21.6 新属性
// ============================================================

int likely_demo(int n) {
    if (n <= 0) [[unlikely]] {
        return n;
    } else [[likely]] {
        return n * n;
    }
}

struct Empty {};

struct WithNoUniqueAddr {
    [[no_unique_address]] Empty e;
    int i;
};

struct WithoutNoUniqueAddr {
    Empty e;
    int i;
};

void demo_attributes() {
    std::cout << "--- 21.6 新属性 ---\n";

    // [[likely]]/[[unlikely]]
    std::cout << "  likely_demo(5) = " << likely_demo(5) << "\n";

    // [[no_unique_address]]
    std::cout << "  WithNoUniqueAddr: " << sizeof(WithNoUniqueAddr) << " bytes\n";
    std::cout << "  WithoutNoUniqueAddr: " << sizeof(WithoutNoUniqueAddr) << " bytes\n";

    // [[nodiscard("reason")]]
    // [[nodiscard("Did you mean clear()?")]] bool empty() const;
    std::cout << "\n";
}

// ============================================================
// 21.7 特性测试宏
// ============================================================

void demo_feature_test_macros() {
    std::cout << "--- 21.7 特性测试宏 ---\n";

#ifdef __cpp_char8_t
    std::cout << "  __cpp_char8_t = " << __cpp_char8_t << "\n";
#else
    std::cout << "  char8_t not supported\n";
#endif

#ifdef __cpp_generic_lambdas
    std::cout << "  __cpp_generic_lambdas = " << __cpp_generic_lambdas << "\n";
#endif

#ifdef __cpp_lib_ranges
    std::cout << "  __cpp_lib_ranges = " << __cpp_lib_ranges << "\n";
#endif

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第21章: 核心语言的小改进 ===\n\n";

    demo_for_init();
    demo_using_enum();
    demo_enum_delegation();
    demo_char8_t();
    demo_aggregate();
    demo_attributes();
    demo_feature_test_macros();

    return 0;
}
