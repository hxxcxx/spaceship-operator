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
#include <set>
#include <format>
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

// convertible_to: 检查 From 是否能隐式转换到 To
// 注意: convertible_to<int, double> 为 true，但 convertible_to<double, int> 也为 true (窄化转换在隐式中允许)
// same_as 更严格: 要求完全相同的类型

// 示例: 通用 to_string，要求 T 能转换为 string
template<typename T>
    requires std::convertible_to<T, std::string>
std::string as_string(const T& val) {
    return std::string(val);
}

// 示例: 通用数值函数，要求能转换为 double
template<typename T>
    requires std::convertible_to<T, double>
double to_double(const T& val) {
    return static_cast<double>(val);
}

// 对比 same_as vs convertible_to vs common_reference_with
template<typename T, typename U>
void print_convertibility() {
    std::cout << std::format("    convertible_to: {}\n", std::convertible_to<T, U>);
    std::cout << std::format("    same_as: {}\n", std::same_as<T, U>);
    std::cout << std::format("    common_reference_with: {}\n", std::common_reference_with<T, U>);
}

// ranges 概念: range, forward_range, random_access_range, sized_range 等
template<std::ranges::range R>
void print_range_concept_info(const R&) {
    using T = std::ranges::range_value_t<R>;
    std::cout << std::format("    forward_range: {}\n", std::ranges::forward_range<R>);
    std::cout << std::format("    bidirectional_range: {}\n", std::ranges::bidirectional_range<R>);
    std::cout << std::format("    random_access_range: {}\n", std::ranges::random_access_range<R>);
    std::cout << std::format("    contiguous_range: {}\n", std::ranges::contiguous_range<R>);
    std::cout << std::format("    sized_range: {}\n", std::ranges::sized_range<R>);
    std::cout << std::format("    common_range: {}\n", std::ranges::common_range<R>);
    std::cout << std::format("    view: {}\n", std::ranges::view<R>);
}

// --- 3.2 自定义概念 (Custom Concepts) ---

// 简单约束：要求有 size() 方法
template<typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

// 检测成员函数是否存在: push_back, push_front, insert, find, sort 等
template<typename Coll, typename T>
concept SupportsPushBack = requires(Coll c, T v) {
    c.push_back(v);
};

template<typename Coll, typename T>
concept SupportsPushFront = requires(Coll c, T v) {
    c.push_front(v);
};

template<typename Coll>
concept SupportsSort = requires(Coll c) {
    c.sort();
};

// 禁止窄化转换的概念
// std::convertible_to 允许窄化 (如 long long -> int)，
// ConvertsWithoutNarrowing 通过数组初始化检查来禁止窄化
template<typename From, typename To>
concept ConvertsWithoutNarrowing =
    std::convertible_to<From, To> && requires(From&& x) {
        { std::type_identity_t<To[]>{std::forward<From>(x)} }
        -> std::same_as<To[1]>;
};

// 通用 add(单值): 自动选择 push_back 或 insert，并禁止窄化
template<typename Coll, typename T>
    requires ConvertsWithoutNarrowing<T, typename Coll::value_type>
void add(Coll& coll, const T& val) {
    if constexpr (SupportsPushBack<Coll, typename Coll::value_type>) {
        coll.push_back(val);
    } else {
        coll.insert(val);
    }
}

// 通用 add(范围): 插入整个范围，并禁止窄化
template<typename Coll, std::ranges::input_range R>
    requires ConvertsWithoutNarrowing<std::ranges::range_value_t<R>, typename Coll::value_type>
void add(Coll& coll, const R& range) {
    if constexpr (SupportsPushBack<Coll, typename Coll::value_type>) {
        coll.insert(coll.end(), std::ranges::begin(range), std::ranges::end(range));
    } else {
        coll.insert(std::ranges::begin(range), std::ranges::end(range));
    }
}

// 通用 add_front: 只对支持 push_front 的容器生效
template<typename Coll, typename T>
    requires SupportsPushFront<Coll, T>
void add_front(Coll& coll, const T& val) {
    coll.push_front(val);
}

// 通用 sort_if_can: 只对支持自身 sort() 的容器生效
template<typename Coll>
    requires SupportsSort<Coll>
void sort_if_can(Coll& coll) {
    coll.sort();
}

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
    std::cout << std::format("  pointer to: {}\n", typeid(*ptr).name());
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
        std::cout << std::format("  insert {} (hash={})\n", val, h);
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
    std::cout << std::format("  gcd(12, 8) = {}\n", gcd(12, 8));
    std::cout << std::format("  gcd(100, 75) = {}\n", gcd(100, 75));
    std::cout << std::format("  normalize(-3.14) = {}\n", normalize(-3.14));

    // totally_ordered_with: 跨类型比较
    std::cout << std::format("  compare_cross(42, 42.0) = {}\n", compare_cross(42, 42.0));
    std::cout << std::format("  compare_cross(1, 2.0) = {}\n", compare_cross(1, 2.0));
    std::cout << std::format("  compare_cross(3.0, 2) = {}\n", compare_cross(3.0, 2));
    std::cout << "\n";

    // convertible_to 示例
    std::cout << std::format("  as_string(\"hello\") = {}\n", as_string("hello"));
    std::cout << std::format("  as_string(std::string(\"world\")) = {}\n", as_string(std::string("world")));
    std::cout << std::format("  to_double(42) = {}\n", to_double(42));
    std::cout << std::format("  to_double(3.14f) = {}\n", to_double(3.14f));

    // same_as vs convertible_to vs common_reference_with
    std::cout << "  [int vs double]:\n";    print_convertibility<int, double>();
    std::cout << "  [int vs int]:\n";       print_convertibility<int, int>();
    std::cout << "  [int vs string]:\n";    print_convertibility<int, std::string>();
    std::cout << "  [const char* vs string]:\n"; print_convertibility<const char*, std::string>();
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
    std::cout << std::format("  vector HasSize: {}\n", HasSize<std::vector<int>>);
    std::cout << std::format("  string HasSize: {}\n", HasSize<std::string>);
    std::cout << std::format("  int HasSize: {}\n", HasSize<int>);
    std::cout << std::format("  vector Container: {}\n", Container<std::vector<int>>);
    std::cout << std::format("  vector NumericContainer: {}\n", NumericContainer<std::vector<int>>);
    std::cout << std::format("  vector<double> NumericContainer: {}\n", NumericContainer<std::vector<double>>);

    // SupportsPushBack / SupportsPushFront / SupportsSort
    std::cout << std::format("  vector SupportsPushBack<int>: {}\n", SupportsPushBack<std::vector<int>, int>);
    std::cout << std::format("  list SupportsPushFront<int>: {}\n", SupportsPushFront<std::list<int>, int>);
    std::cout << std::format("  vector SupportsPushFront<int>: {}\n", SupportsPushFront<std::vector<int>, int>);
    std::cout << std::format("  list SupportsSort: {}\n", SupportsSort<std::list<int>>);
    std::cout << std::format("  vector SupportsSort: {}\n", SupportsSort<std::vector<int>>);

    // ConvertsWithoutNarrowing + add() 完整示例
    std::vector<int> iVec;
    add(iVec, 42);          // OK: push_back
    std::set<int> iSet;
    add(iSet, 42);           // OK: insert
    short s = 42;
    add(iVec, s);            // OK: short -> int 不窄化
    // add(iVec, 7.7);       // 编译错误: double -> int 窄化
    // long long ll = 42; add(iVec, ll);  // 编译错误: long long -> int 窄化

    std::vector<double> dVec;
    add(dVec, 0.7);          // OK: double
    add(dVec, 0.7f);         // OK: float -> double 不窄化
    // add(dVec, 7);         // 编译错误: int -> double 窄化

    // 范围插入
    add(iVec, iSet);         // OK: set<int> -> vector<int>
    int vals[] = {0, 8, 18};
    add(iVec, vals);         // OK: int[] -> vector<int>
    // add(dVec, vals);      // 编译错误: int[] -> vector<double> 窄化

    std::cout << "  iVec after all add(): ";
    for (auto& e : iVec) std::cout << e << " ";
    std::cout << "\n";

    std::list<int> lst;
    add(lst, 100);           // OK: push_back
    add_front(lst, 0);       // OK: push_front
    sort_if_can(lst);        // OK: list 有 .sort()
    std::cout << "  list after add/add_front/sort: ";
    for (auto& e : lst) std::cout << e << " ";
    std::cout << "\n\n";

    // 3.3 requires 四种形式
    std::cout << "--- 3.3 requires 四种形式 ---\n";
    std::cout << std::format("  vector HasValueType: {}\n", HasValueType<std::vector<int>>);
    std::cout << std::format("  int Hashable: {}\n", Hashable<int>);
    std::cout << std::format("  string Hashable: {}\n", Hashable<std::string>);
    std::cout << std::format("  double Hashable: {}\n\n", Hashable<double>);

    // 3.4 requires 子句
    std::cout << "--- 3.4 requires 子句 ---\n";
    int x = 42;
    print_pointer_info(&x);
    std::cout << std::format("  doubler(21) = {}\n", doubler(21));
    std::cout << std::format("  doubler(3.14) = {}\n", doubler(3.14));
    std::cout << std::format("  square(5) = {}\n", square(5));
    std::cout << std::format("  square(2.5) = {}\n\n", square(2.5));

    // 3.5 约束与重载
    std::cout << "--- 3.5 约束与重载 ---\n";
    std::cout << std::format("  describe_type(42) = {}\n", describe_type(42));
    std::cout << std::format("  describe_type(3.14) = {}\n", describe_type(3.14));
    std::cout << std::format("  describe_type_detail(42) = {}\n", describe_type_detail(42));
    std::cout << std::format("  describe_type_detail(42LL) = {}\n\n", describe_type_detail(42LL));

    // 3.6 约束与类模板
    std::cout << "--- 3.6 约束与类模板 ---\n";
    HashSet<std::string> str_set;
    str_set.insert("hello");
    str_set.insert("world");
    HashSet<int> int_set;
    int_set.insert(42);
    std::cout << "\n";

    // 3.7 约束偏特化
    std::cout << "--- 3.7 约束偏特化 ---\n";
    std::vector<int> v2{1, 2, 3};
    std::cout << "  count_elements(vector) = " << count_elements(v2) << "\n";
    std::cout << "  count_elements(7) = " << count_elements(7) << "\n";
    std::cout << "\n";

    return 0;
}
