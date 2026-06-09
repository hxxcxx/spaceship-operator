// ============================================================
// 第24章: 已弃用和移除的特性
// 参考: C++20 Complete Guide Chapter 24
// ============================================================

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include <memory>

// ============================================================
// 24.1 已弃用和移除的核心语言特性
// ============================================================

void demo_core_deprecations() {
    std::cout << "--- 24.1 核心语言弃用/移除 ---\n";

    // 1. 隐式捕获 *this 已被弃用 (C++20)
    //    旧的写法: [=] 中隐式捕获 *this (按引用捕获)
    //    新的写法: 显式使用 [=, this] 或 [=, *this]
    struct S {
        int val = 42;
        void demo() {
            // C++20之前: [=] 隐式按引用捕获this (已弃用)
            // auto f = [=] { return val; };  // 警告: 隐式捕获this已弃用

            // C++20推荐: 显式按值捕获 *this
            auto f1 = [*this] { return val; };          // 按值捕获副本
            auto f2 = [=, this] { return val; };        // 显式按引用捕获
            auto f3 = [=, *this] { return val; };       // 显式按值捕获

            std::cout << "  [*this]: " << f1() << "\n";
            std::cout << "  [=, this]: " << f2() << "\n";
            std::cout << "  [=, *this]: " << f3() << "\n";
        }
    };
    S{}.demo();

    // 2. 聚合体不能再有用户声明的构造函数
    //    C++17: 以下结构体是聚合体(构造函数是用户声明但非用户提供的=default)
    //    C++20: 不再是聚合体
    // struct Aggr {
    //     Aggr() = default;  // 用户声明, 非用户提供
    //     int x;
    // };
    // C++20中: Aggr{} 不再是聚合初始化, 而是调用默认构造函数

    struct WithDefaultCtor {
        WithDefaultCtor() = default;
        int x = 0;
    };
    WithDefaultCtor wdc{};
    std::cout << "  WithDefaultCtor: " << wdc.x << "\n";

    std::cout << "\n";
}

// ============================================================
// 24.2.1 已弃用的库特性
// ============================================================

void demo_deprecated_lib() {
    std::cout << "--- 24.2.1 已弃用的库特性 ---\n";

    // 1. is_pod<> 已弃用, 应使用 is_trivial<> 或 is_standard_layout<>
    struct MyStruct {
        int a;
        double b;
    };

    // C++20中 is_pod 已弃用
    // std::cout << "  is_pod: " << std::is_pod_v<MyStruct> << "\n";  // 弃用

    // 替代方案:
    std::cout << "  is_trivial: " << std::is_trivial_v<MyStruct> << "\n";
    std::cout << "  is_standard_layout: " << std::is_standard_layout_v<MyStruct> << "\n";

    // is_trivially_copyable, is_trivially_default_constructible 等
    std::cout << "  is_trivially_copyable: " << std::is_trivially_copyable_v<MyStruct> << "\n";

    // 2. 对普通 shared_ptr 的原子操作已弃用
    //    应使用 std::atomic<std::shared_ptr<T>>
    //    旧的: std::atomic_load(&sp), std::atomic_store(&sp, newp) 等
    //    新的: std::atomic<std::shared_ptr<int>> asp;

    auto sp = std::make_shared<int>(42);
    // 旧方式 (已弃用):
    // std::atomic_store(&sp, std::make_shared<int>(99));

    // C++20 新方式:
    std::atomic<std::shared_ptr<int>> asp = std::make_shared<int>(42);
    asp.store(std::make_shared<int>(99));
    auto loaded = asp.load();
    std::cout << "  atomic<shared_ptr>: " << *loaded << "\n";

    std::cout << "\n";
}

// ============================================================
// 24.2.2 已移除的库特性
// ============================================================

void demo_removed_lib() {
    std::cout << "--- 24.2.2 已移除的库特性 ---\n";

    // 1. reserve() 不再无参调用来缩减容量
    //    C++17: s.reserve() 会缩减到合适大小
    //    C++20: reserve() 必须带参数, 无参版本已移除
    //    使用 shrink_to_fit() 代替

    std::string s = "hello world";
    s.reserve(1000);
    std::cout << "  after reserve(1000): capacity=" << s.capacity() << "\n";
    // s.reserve();  // C++20: 编译错误!
    s.shrink_to_fit();
    std::cout << "  after shrink_to_fit: capacity=" << s.capacity() << "\n";

    // 2. 不能再将 UTF-8 字符串写入标准输出流
    //    C++17: std::cout << u8"text" 是合法的 (输出为 const char*)
    //    C++20: char8_t 与 char 不兼容, 不能直接输出
    //    u8字符串现在是 const char8_t*, 不能直接传给 char* 参数的流

    // C++17: std::cout << u8"hello";  // OK, const char*
    // C++20: 上述代码编译错误, 因为 u8"hello" 是 const char8_t*
    // 解决方案: 使用 reinterpret_cast 或 u8string 转 string
    const char8_t* u8str = u8"UTF-8 text";
    std::cout << "  UTF-8 output: " << reinterpret_cast<const char*>(u8str) << "\n";

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第24章: 已弃用和移除的特性 ===\n\n";

    demo_core_deprecations();
    demo_deprecated_lib();
    demo_removed_lib();

    return 0;
}
