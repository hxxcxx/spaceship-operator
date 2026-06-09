// ============================================================
// 第22章: 泛型编程的改进
// 参考: C++20 Complete Guide Chapter 22
// ============================================================

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <complex>
#include <cassert>

// ============================================================
// 22.1 模板中的隐式typename
// ============================================================

template<typename T>
struct MyType {
    using value_type = T;
};

// C++20: 在以下语境中可以省略typename
template<typename T>
// 隐式typename: 返回类型中的依赖类型不再需要typename
MyType<T>::value_type get_value() {
    return 42;
}

// C++17需要: typename MyType<T>::value_type get_value()
// C++20可以: MyType<T>::value_type get_value()

// 隐式typename: using声明中
template<typename Coll>
using ValueTypeOld = typename Coll::value_type;  // 仍然OK

template<typename Coll>
using ValueTypeNew = Coll::value_type;           // C++20也OK

// 隐式typename: 函数返回类型中的依赖类型
template<typename T>
using FirstTypeOld = typename T::value_type;   // C++17

template<typename T>
using FirstTypeNew = T::value_type;             // C++20

void demo_implicit_typename() {
    std::cout << "--- 22.1 隐式typename ---\n";

    auto val = get_value<int>();
    std::cout << "  get_value<int>() = " << val << "\n";

    using V1 = ValueTypeOld<std::vector<int>>;
    using V2 = ValueTypeNew<std::vector<int>>;
    std::cout << "  both types are same: " << std::is_same_v<V1, V2> << "\n";

    std::cout << "\n";
}

// ============================================================
// 22.2 聚合体的类模板参数推导(CTAD)
// ============================================================

// 简单聚合体
template<typename T>
struct Pair {
    T first;
    T second;
};

// C++20: 聚合体自动推导指引
// Pair p{1, 2};  // 推导为Pair<int>

// 带默认模板参数的聚合体
template<typename T = int>
struct ValWrap {
    T value;
};

void demo_aggregate_ctad() {
    std::cout << "--- 22.2 聚合体CTAD ---\n";

    // 聚合体CTAD
    Pair p1{1, 2};           // Pair<int>
    Pair p2{3.14, 2.72};     // Pair<double>
    std::cout << "  p1: " << p1.first << ", " << p1.second << "\n";
    std::cout << "  p2: " << p2.first << ", " << p2.second << "\n";

    // 带默认模板参数
    ValWrap v1{42};           // ValWrap<int>
    ValWrap<double> v2{3.14}; // ValWrap<double>
    std::cout << "  v1: " << v1.value << "\n";
    std::cout << "  v2: " << v2.value << "\n";

    // std::array也受益
    std::pair pr{1, 2.0};
    std::cout << "  pair CTAD: " << pr.first << ", " << pr.second << "\n";

    std::cout << "\n";
}

// ============================================================
// 22.3 条件explicit
// ============================================================

// C++20: explicit(bool) - 根据条件决定是否为explicit

class BoolString {
private:
    std::string value;
public:
    // 从任意可转换为string的类型: 隐式转换
    template<typename T,
             typename = std::enable_if_t<std::is_convertible_v<T, std::string>>>
    BoolString(T&& v) : value(std::forward<T>(v)) {}

    // 从bool: explicit(条件)
    // explicit(!std::is_convertible_v<U, std::string>)
    // 当U可以隐式转换为string时, 不是explicit; 否则是explicit
};

// 更实际的例子: 类型安全的构造函数
template<typename T>
class Wrapper {
    T val;
public:
    // 当T可以从U隐式构造时, 隐式构造; 否则显式构造
    template<typename U>
    explicit(!std::is_convertible_v<U, T>)
    Wrapper(U&& u) : val(std::forward<U>(u)) {}

    T get() const { return val; }
};

// 另一个例子: 窄化转换时explicit
class NarrowInt {
    int val;
public:
    // int隐式, double显式
    NarrowInt(int v) : val(v) {}
    explicit(!std::is_same_v<double, double>) // always explicit for demo
    NarrowInt(double d) : val(static_cast<int>(d)) {}
};

void demo_conditional_explicit() {
    std::cout << "--- 22.3 条件explicit ---\n";

    // Wrapper<int>: 从int隐式, 从double也是隐式(double->int)
    // Wrapper<std::string>: 从const char*隐式, 从int显式(int不能隐式转string)
    Wrapper<int> w1 = 42;            // 隐式: int -> int OK
    Wrapper<std::string> w2 = "hi";  // 隐式: const char* -> string OK
    // Wrapper<std::string> w3 = 42;  // 编译错误: int -> string 是explicit的

    std::cout << "  w1: " << w1.get() << "\n";
    std::cout << "  w2: " << w2.get() << "\n";

    // 直接初始化可以调用explicit构造
    Wrapper<std::string> w4{std::string("world")};
    std::cout << "  w4: " << w4.get() << "\n";

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第22章: 泛型编程的改进 ===\n\n";

    demo_implicit_typename();
    demo_aggregate_ctad();
    demo_conditional_explicit();

    return 0;
}
