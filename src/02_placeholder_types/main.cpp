#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第2章：函数参数占位符类型 (Placeholder Types)
// ============================================================

// --- 2.1 auto 作为函数参数 (C++20) ---
// 在 C++20 之前，auto 不能用作函数参数
// C++20 允许 auto 作为函数参数，等同于模板参数

auto add_auto(auto a, auto b) {
    return a + b;
}

// 等价的模板写法:
// template<typename T, typename U>
// auto add_template(T a, U b) { return a + b; }

// --- 2.2 混合 auto 和具体类型 ---
void print_twice(const auto& value) {
    std::cout << value << " " << value << "\n";
}

// --- 2.3 auto 参数 + 约束 (concepts) ---
// 只接受整数类型
auto int_only(std::integral auto x) {
    std::cout << "  int_only: " << x << " (integral)\n";
    return x * 2;
}

// 只接受浮点类型
auto float_only(std::floating_point auto x) {
    std::cout << "  float_only: " << x << " (floating point)\n";
    return x * 2.0;
}

// 自定义 concept
template<typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

auto print_size(const HasSize auto& container) {
    std::cout << "  size = " << container.size() << "\n";
    return container.size();
}

// --- 2.4 decltype(auto) 占位符 ---
// decltype(auto) 保留引用和 cv 限定符

decltype(auto) get_element(auto& container, auto index) {
    return container[index]; // 返回引用
}

// --- 2.5 多个 auto 参数 ---
// 每个 auto 是独立的模板参数
auto multiply(auto a, auto b, auto c) {
    return a * b * c;
}

// --- 2.6 auto 参数与 lambda ---
// C++14 就支持 lambda 参数用 auto，C++20 普通函数也支持了

void demo_lambda_auto() {
    auto processor = [](auto first, auto second) {
        std::cout << "  processing: " << first << " and " << second << "\n";
    };

    processor(1, 2);
    processor(3.14, "hello");
    processor(std::string("world"), 'X');
}

// --- 2.7 auto 参数的函数重载 ---
// 可以基于约束进行重载 (不靠 auto 参数数量重载)

void describe(std::integral auto val) {
    std::cout << "  integer: " << val << "\n";
}

void describe(std::floating_point auto val) {
    std::cout << "  floating: " << val << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第2章：函数参数占位符类型 ===\n\n";

    // 2.1 auto 作为函数参数
    std::cout << "--- 2.1 auto 函数参数 ---\n";
    std::cout << "  add_auto(1, 2) = " << add_auto(1, 2) << "\n";
    std::cout << "  add_auto(1.5, 2.5) = " << add_auto(1.5, 2.5) << "\n";
    std::cout << "  add_auto(1, 2.5) = " << add_auto(1, 2.5) << " (int + double)\n\n";

    // 2.2 混合 auto 和具体类型
    std::cout << "--- 2.2 const auto& 参数 ---\n";
    print_twice(42);
    print_twice(std::string("hello"));
    print_twice(3.14);
    std::cout << "\n";

    // 2.3 auto + concepts 约束
    std::cout << "--- 2.3 auto + concepts 约束 ---\n";
    int_only(42);
    float_only(3.14);
    // int_only(3.14);  // 编译错误: 不满足 std::integral
    // float_only(42);  // 编译错误: 不满足 std::floating_point

    std::vector<int> v{1, 2, 3, 4, 5};
    std::string s = "hello";
    std::cout << "  vector: ";
    print_size(v);
    std::cout << "  string: ";
    print_size(s);
    std::cout << "\n";

    // 2.4 decltype(auto) 保留引用
    std::cout << "--- 2.4 decltype(auto) ---\n";
    std::vector<std::string> names{"Alice", "Bob", "Charlie"};
    decltype(auto) elem = get_element(names, 1);
    std::cout << "  element[1] = " << elem << "\n";
    elem = "David"; // 修改原容器
    std::cout << "  after modify: names[1] = " << names[1] << "\n\n";

    // 2.5 多个 auto 参数
    std::cout << "--- 2.5 多个 auto 参数 ---\n";
    std::cout << "  multiply(2, 3, 4) = " << multiply(2, 3, 4) << "\n";
    std::cout << "  multiply(2, 3.0, 4) = " << multiply(2, 3.0, 4) << "\n\n";

    // 2.6 lambda auto
    std::cout << "--- 2.6 lambda auto ---\n";
    demo_lambda_auto();
    std::cout << "\n";

    // 2.7 基于约束重载
    std::cout << "--- 2.7 基于约束重载 ---\n";
    describe(42);
    describe(3.14);
    std::cout << "\n";

    return 0;
}
