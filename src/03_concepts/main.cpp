#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>
#include <iterator>
#include <memory>
#include <ranges>
#include <array>
#include <list>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第3章：概念、要求和约束 (Concepts, Requires, Constraints)
// ============================================================

// --- 3.1 标准库概念 (Standard Concepts) ---
// C++20 <concepts> 头文件提供了大量内置概念

template<std::integral T>
T gcd(T a, T b) {
    while (b != 0) { T t = b; b = a % b; a = t; }
    return a;
}

template<std::floating_point T>
T normalize(T val) {
    return val / std::abs(val);
}

// totally_ordered_with: 要求 T 和 U 之间可以进行全序比较
// 即支持 <, <=, >, >=, ==, != 交叉比较
template<typename T, typename U>
    requires std::totally_ordered_with<T, U>
int compare_cross(const T& a, const U& b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// ranges 概念: range, forward_range, random_access_range, sized_range 等
template<std::ranges::range R>
void print_range_concept_info(const R&) {
    using T = std::ranges::range_value_t<R>;

    // 编译期布尔值：检查更细粒度的 range 概念
    std::cout << "    forward_range: " << std::ranges::forward_range<R> << "\n";
    std::cout << "    bidirectional_range: " << std::ranges::bidirectional_range<R> << "\n";
    std::cout << "    random_access_range: " << std::ranges::random_access_range<R> << "\n";
    std::cout << "    contiguous_range: " << std::ranges::contiguous_range<R> << "\n";
    std::cout << "    sized_range: " << std::ranges::sized_range<R> << "\n";
    std::cout << "    common_range: " << std::ranges::common_range<R> << "\n";
    std::cout << "    view: " << std::ranges::view<R> << "\n";
}

// --- 3.2 自定义概念 (Custom Concepts) ---

// 简单约束：要求有 size() 方法
template<typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

// 复合约束：要求是容器（有 begin/end/size）
template<typename T>
concept Container = requires(T t) {
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
    { t.size() } -> std::convertible_to<std::size_t>;
};

// 嵌套约束：要求元素可打印
template<typename T>
concept PrintableContainer = Container<T> && requires(T t) {
    { *t.begin() } -> std::convertible_to<std::string>;
    // 或更通用的: 需要 operator<<
};

// --- 3.3 requires 表达式的四种形式 ---

// 形式1: 简单要求 (simple requirement)
template<typename T>
concept Swappable = requires(T a, T b) {
    a.swap(b);  // 检查表达式是否合法
};

// 形式2: 类型要求 (type requirement)
template<typename T>
concept HasValueType = requires {
    typename T::value_type;  // 检查嵌套类型是否存在
};

// 形式3: 复合要求 (compound requirement)
template<typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

// 形式4: 嵌套要求 (nested requirement)
template<typename T>
concept NumericContainer = Container<T> && requires {
    requires std::is_arithmetic_v<typename T::value_type>;
};

// --- 3.4 requires 子句 (requires clause) ---
// 用于模板约束，不同于 requires 表达式

// 方式1: requires 子句 + 概念
template<typename T>
    requires std::is_pointer_v<T>
void print_pointer_info(T ptr) {
    std::cout << "  pointer to: " << typeid(*ptr).name() << "\n";
}

// 方式2: requires 子句 + 表达式
template<typename T>
    requires requires(T t) { t + t; }  // 注意两个 requires
auto doubler(T val) {
    return val + val;
}

// 方式3: 尾置 requires
template<typename T>
auto square(T val) -> T requires std::is_arithmetic_v<T> {
    return val * val;
}

// --- 3.5 约束与函数重载 ---
// 概念越"精确"的优先级越高

template<typename T>
    requires std::integral<T>
std::string describe_type(T) { return "integer"; }

template<typename T>
    requires std::floating_point<T>
std::string describe_type(T) { return "floating point"; }

template<typename T>
    requires std::integral<T> && (sizeof(T) <= 4)
std::string describe_type_detail(T) { return "small integer (<=32bit)"; }

template<typename T>
    requires std::integral<T> && (sizeof(T) > 4)
std::string describe_type_detail(T) { return "large integer (>32bit)"; }

// --- 3.6 约束与类模板 ---
template<typename T>
    requires Hashable<T>
class HashSet {
public:
    void insert(const T& val) {
        std::size_t h = std::hash<T>{}(val);
        std::cout << "  insert " << val << " (hash=" << h << ")\n";
    }
};

// --- 3.7 约束的偏特化 ---
template<typename T>
    requires Container<T>
std::size_t count_elements(const T& c) {
    return c.size();
}

// 原始指针的偏特化：不是容器，走另一个路径
template<std::integral T>
std::size_t count_elements(T val) {
    return static_cast<std::size_t>(val);
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第3章：概念、要求和约束 ===\n\n";

    // 3.1 标准库概念
    std::cout << "--- 3.1 标准库概念 ---\n";
    std::cout << "  gcd(12, 8) = " << gcd(12, 8) << "\n";
    std::cout << "  gcd(100, 75) = " << gcd(100, 75) << "\n";
    std::cout << "  normalize(-3.14) = " << normalize(-3.14) << "\n";
    // gcd(3.14, 2.0);  // 编译错误: 不满足 std::integral

    // totally_ordered_with: 跨类型比较
    std::cout << "  compare_cross(42, 42.0) = " << compare_cross(42, 42.0) << "\n";
    std::cout << "  compare_cross(1, 2.0) = " << compare_cross(1, 2.0) << "\n";
    std::cout << "  compare_cross(3.0, 2) = " << compare_cross(3.0, 2) << "\n";
    // int 和 string 不满足 totally_ordered_with，编译错误:
    // compare_cross(1, std::string("hello"));
    std::cout << "\n";

    // ranges 概念检查
    std::cout << "  [vector<int> range properties]\n";
    print_range_concept_info(std::vector<int>{});
    std::cout << "  [list<int> range properties]\n";
    print_range_concept_info(std::list<int>{});
    std::cout << "  [string range properties]\n";
    print_range_concept_info(std::string{});
    auto view = std::vector<int>{1,2,3} | std::views::take(2);
    std::cout << "  [take_view range properties]\n";
    print_range_concept_info(view);
    std::cout << "\n";

    // 3.2 自定义概念
    std::cout << "--- 3.2 自定义概念 ---\n";
    std::vector<int> v{1, 2, 3, 4, 5};
    std::string s = "hello";
    std::cout << "  vector HasSize: " << HasSize<std::vector<int>> << "\n";
    std::cout << "  string HasSize: " << HasSize<std::string> << "\n";
    std::cout << "  int HasSize: " << HasSize<int> << "\n";
    std::cout << "  vector Container: " << Container<std::vector<int>> << "\n";
    std::cout << "  vector NumericContainer: " << NumericContainer<std::vector<int>> << "\n";
    std::cout << "  vector<double> NumericContainer: " << NumericContainer<std::vector<double>> << "\n";
    std::cout << "\n";

    // 3.3 requires 四种形式
    std::cout << "--- 3.3 requires 四种形式 ---\n";
    std::cout << "  vector HasValueType: " << HasValueType<std::vector<int>> << "\n";
    std::cout << "  int Hashable: " << Hashable<int> << "\n";
    std::cout << "  string Hashable: " << Hashable<std::string> << "\n";
    std::cout << "  double Hashable: " << Hashable<double> << "\n";
    std::cout << "\n";

    // 3.4 requires 子句
    std::cout << "--- 3.4 requires 子句 ---\n";
    int x = 42;
    print_pointer_info(&x);
    std::cout << "  doubler(21) = " << doubler(21) << "\n";
    std::cout << "  doubler(3.14) = " << doubler(3.14) << "\n";
    std::cout << "  square(5) = " << square(5) << "\n";
    std::cout << "  square(2.5) = " << square(2.5) << "\n";
    std::cout << "\n";

    // 3.5 约束与重载
    std::cout << "--- 3.5 约束与重载 ---\n";
    std::cout << "  describe_type(42) = " << describe_type(42) << "\n";
    std::cout << "  describe_type(3.14) = " << describe_type(3.14) << "\n";
    std::cout << "  describe_type_detail(42) = " << describe_type_detail(42) << "\n";
    std::cout << "  describe_type_detail(42LL) = " << describe_type_detail(42LL) << "\n";
    std::cout << "\n";

    // 3.6 约束与类模板
    std::cout << "--- 3.6 约束与类模板 ---\n";
    HashSet<std::string> str_set;
    str_set.insert("hello");
    str_set.insert("world");
    HashSet<int> int_set;
    int_set.insert(42);
    // HashSet<struct Foo> 不满足 Hashable，编译错误
    std::cout << "\n";

    // 3.7 约束偏特化
    std::cout << "--- 3.7 约束偏特化 ---\n";
    std::vector<int> v2{1, 2, 3};
    std::cout << "  count_elements(vector) = " << count_elements(v2) << "\n";
    std::cout << "  count_elements(7) = " << count_elements(7) << "\n";
    std::cout << "\n";

    return 0;
}
