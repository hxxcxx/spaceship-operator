#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <concepts>
#include <type_traits>
#include <ranges>
#include <format>
#include <bit>
#include <functional>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第4章：概念、需求和约束详解 (Concepts, Requirements, Constraints in Detail)
// 参考: C++20 Complete Guide Chapter 4
// ============================================================

// --- 4.1 约束 (Constraints) ---
// 约束用于给泛型参数指定需求，在编译时决定是否实例化模板

// 方式1: requires 子句
template<typename T>
    requires std::integral<T>
T constrained_add(T a, T b) {
    return a + b;
}

// 方式2: 概念作为类型约束 (模板参数)
template<std::integral T>
T constrained_mul(T a, T b) {
    return a * b;
}

// 方式3: 概念约束 auto 参数
std::integral auto constrained_square(std::integral auto val) {
    return val * val;
}

// --- 4.2 requires 子句详解 ---
// requires 子句使用 requires 关键字 + 编译期布尔表达式

// 4.2.1 使用 && 组合多个约束
template<typename T>
    requires (sizeof(T) > 1)                          // 临时布尔表达式
    && requires { typename T::value_type; }           // requires 表达式
    && std::input_iterator<T>                         // 概念
void print_iterator_info(T) {
    std::cout << "  (满足: sizeof>1, 有 value_type, 是 input_iterator)\n";
}

// 4.2.2 使用 || 表达"可选"约束
template<typename T>
    requires std::integral<T> || std::floating_point<T>
T safe_div(T a, T b) {
    if constexpr (std::integral<T>) {
        return b != 0 ? a / b : T{0};
    } else {
        return a / b;
    }
}

// 4.2.3 跨类型约束
template<typename T, typename U>
    requires std::convertible_to<T, U>
U convert_value(const T& from) {
    return static_cast<U>(from);
}

// --- 4.3 临时布尔表达式 (Ad Hoc Boolean Expressions) ---
// requires 子句可以使用任何编译期布尔表达式

// 仅当 sizeof(T) 不太大时可用
template<typename T>
    requires (sizeof(T) <= 64)
void process_small_type(const T&) {
    std::cout << "  处理小型类型 (sizeof=" << sizeof(T) << ")\n";
}

// 仅当非类型模板参数 > 0 时可用
template<typename T, std::size_t N>
    requires (N > 0)
void process_array(const T (&arr)[N]) {
    std::cout << "  数组大小: " << N << "\n";
}

// 仅当参数不能用作字符串时可用
template<typename T>
    requires (!std::convertible_to<T, std::string>)
void non_string_process(const T& val) {
    std::cout << "  处理非字符串值: " << val << "\n";
}

// 仅对原始指针和 nullptr 可用
template<typename T>
    requires (std::is_pointer_v<T> || std::same_as<T, std::nullptr_t>)
void print_if_pointer(T ptr) {
    if constexpr (std::is_pointer_v<T>) {
        std::cout << "  是指针，指向类型: " << typeid(*ptr).name() << "\n";
    } else {
        std::cout << "  是 nullptr\n";
    }
}

// 使用 constexpr 函数作为约束
template<typename T>
constexpr bool is_power_of_2(T val) {
    return val > 0 && std::has_single_bit(static_cast<unsigned>(val));
}

template<typename T, int N>
    requires (is_power_of_2(N))
class AlignedBuffer {
public:
    alignas(N) T data[N];
    void info() const {
        std::cout << "  AlignedBuffer: " << N << " 个元素, 对齐 " << N << "\n";
    }
};

// 禁用模板的技巧
// template<typename T>
//     requires false       // 永远不会被实例化
// void disabled_function(T) {}

// --- 4.4 requires 表达式 (requires Expressions) ---
// requires 表达式不同于 requires 子句

// 4.4.1 简单需求 (Simple Requirements) — 只检查表达式是否合法
template<typename T>
concept SupportsAddSub = requires(T a, T b) {
    a + b;     // 支持 +
    a - b;     // 支持 -
};

template<typename T>
concept SupportsDeref = requires(T p) {
    *p;                     // 支持 operator*
    p[0];                   // 支持 operator[]
    p == nullptr;           // 可与 nullptr 比较
};

// 使用 || 的正确方式: 在两个 requires 表达式之间
template<typename T>
concept ComparableOrNullptr = requires(T a, T b) {
    a < b;
} || requires(T p) {
    p == nullptr;
};

// 注意: requires 表达式内使用概念并不约束类型！
// 错误示例: requires { std::integral<T>; } — 只检查表达式有效，不检查T是否是整数
// 正确做法: 用嵌套 requires (见 4.4.4) 或在 && 中使用概念

// 4.4.2 类型要求 (Type Requirements)
template<typename T>
concept HasFirstAndSecond = requires {
    typename T::first_type;     // 需要有 first_type
    typename T::second_type;    // 需要有 second_type
};

template<typename T>
concept HasValueType = requires {
    typename T::value_type;
};

// 类型要求也检查模板实例化是否有效
template<std::integral T>
class NumberWrapper {
    T value_;
public:
    NumberWrapper(T v) : value_(v) {}
    T get() const { return value_; }
};

template<typename T>
concept WrappableAsNumber = requires {
    typename NumberWrapper<T>;   // 检查 NumberWrapper<T> 是否可以实例化
};

// 注意: typename std::hash<T> 总是有效的（类型存在），但不代表 hash 可用
// 正确做法:
template<typename T>
concept TrulyHashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;  // 实际调用
};

// 4.4.3 复合要求 (Compound Requirements)
template<typename T>
concept EqualityComparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;           // 结果可转为 bool
};

template<typename T>
concept NoThrowSwappable = requires(T& a, T& b) {
    { a.swap(b) } noexcept;                             // 不抛异常
};

template<typename T>
concept IteratorConcept = requires(T it) {
    { *it } -> std::input_or_output_iterator;           // *it 的结果类型满足迭代器概念
    // 注意: -> 后面的概念，表达式的结果类型作为第一个参数
};

// 复合要求中不能使用类型特性嵌套
// 错误: { *p } -> std::integral<std::remove_reference_t<>>;
// 解决方案: 先定义辅助概念
template<typename T>
concept UnrefIntegral = std::integral<std::remove_reference_t<T>>;

template<typename T>
concept PointsToInteger = requires(T p) {
    { *p } -> UnrefIntegral;
};

// 4.4.4 嵌套要求 (Nested Requirements)
template<typename T>
concept DerefsMatch = requires(T p) {
    requires std::same_as<decltype(*p), decltype(p[0])>;  // *p 和 p[0] 类型相同
};

// 嵌套要求用于确保编译期表达式产生特定结果
template<typename T>
concept NonConstType = requires {
    requires !std::is_const_v<T>;    // 正确: 检查 T 不是 const
    // !std::is_const_v<T>;          // 错误! 只检查表达式有效，不检查结果
};

// 嵌套要求可以限制后续的类型操作
template<typename T>
concept SafeMakeUnsigned = requires {
    requires std::integral<T> && !std::same_as<T, bool>;  // 先约束 T
    // 然后可以安全使用 std::make_unsigned<T>
};

// --- 4.5 概念详解 (Concepts in Detail) ---

// 4.5.1 定义概念
template<typename T>
concept SortableContainer = requires(T& c) {
    { c.begin() } -> std::input_or_output_iterator;
    { c.end() } -> std::input_or_output_iterator;
    c.sort();  // 要求有 sort() 成员函数
};

// 概念本质上是编译期 bool 值（纯右值）
static_assert(std::is_same_v<decltype(std::integral<int>), bool>);

// 4.5.2 概念的特殊能力
// - 可用作类型约束: template<MyConcept T>
// - 具有包含关系（用于重载解析）
// - 不需要 inline（隐式内联）

// 4.5.3 用于非类型模板参数的概念
template<auto Val>
concept LessThan10 = Val < 10;

template<auto Val>
concept PowerOf2 = std::has_single_bit(static_cast<unsigned>(Val));

template<typename T, auto Val>
    requires PowerOf2<Val>
class FixedMemory {
public:
    T data[static_cast<std::size_t>(Val)];
    void info() const {
        std::cout << "  FixedMemory: " << Val << " 个 " << typeid(T).name() << "\n";
    }
};

// 注意: 不能用 template<typename T, PowerOf2 auto Val> 因为 PowerOf2 约束的是值不是类型

// --- 4.6 使用概念作为类型约束 ---

// 4.6.1 模板参数的类型约束
template<std::integral T>
class IntegralCounter {
    T count_;
public:
    IntegralCounter(T init = T{0}) : count_(init) {}
    void increment() { ++count_; }
    T get() const { return count_; }
};

// 4.6.2 auto 参数的类型约束
void print_integral(const std::integral auto& val) {
    std::cout << "  整数值: " << val << "\n";
}

// 4.6.3 多参数约束（参数类型作为第一个参数）
template<std::convertible_to<int> T>
void accept_convertible_to_int(const T& val) {
    std::cout << "  可转为 int: " << static_cast<int>(val) << "\n";
}

// 4.6.4 约束返回类型
std::copyable auto make_copyable(auto val) {
    return val;
}

// 4.6.5 约束非类型模板参数
template<typename T, std::integral auto Max>
class BoundedArray {
public:
    T data[static_cast<std::size_t>(Max)];
    constexpr std::size_t size() const { return static_cast<std::size_t>(Max); }
};

// --- 4.7 约束涵盖 (Constraint Subsumption) ---

// 基础概念
template<typename T>
concept GeoObject = requires(T obj) {
    { obj.width() } -> std::integral;
    { obj.height() } -> std::integral;
    obj.draw();
};

// 派生概念（涵盖 GeoObject）
struct Color { int r, g, b; };

template<typename T>
concept ColoredGeoObject = GeoObject<T> && requires(T obj) {
    obj.setColor(Color{});
    { obj.getColor() } -> std::convertible_to<Color>;
};

// 测试用的图形类
class SimpleRect {
    int w_, h_;
public:
    SimpleRect(int w, int h) : w_(w), h_(h) {}
    int width() const { return w_; }
    int height() const { return h_; }
    void draw() const {}
};

class ColoredRect {
    int w_, h_;
    Color c_;
public:
    ColoredRect(int w, int h, Color c = {0,0,0}) : w_(w), h_(h), c_(c) {}
    int width() const { return w_; }
    int height() const { return h_; }
    void draw() const {}
    void setColor(Color c) { c_ = c; }
    Color getColor() const { return c_; }
};

// 重载1: 基础 GeoObject
template<GeoObject T>
void process_geo(T& obj) {
    std::cout << "  process_geo(GeoObject): " << obj.width() << "x" << obj.height() << "\n";
}

// 重载2: ColoredGeoObject 涵盖 GeoObject，优先级更高
template<ColoredGeoObject T>
void process_geo(T& obj) {
    std::cout << "  process_geo(ColoredGeoObject): " << obj.width() << "x" << obj.height()
              << " color=(" << obj.getColor().r << "," << obj.getColor().g << "," << obj.getColor().b << ")\n";
}

// 4.7.1 间接涵盖示例
template<typename T>
concept RgSwap = std::ranges::input_range<T> && std::swappable<T>;

template<typename T>
concept ContCopy = std::ranges::contiguous_range<T> && std::copyable<T>;

template<RgSwap T>
void foo_subsumption(T&) {
    std::cout << "  foo(RgSwap)\n";
}

template<ContCopy T>
void foo_subsumption(T&) {
    std::cout << "  foo(ContCopy)\n";  // vector<int> 优先匹配这个
}

// 4.7.2 可交换概念 (SameAs 设计)
// 标准库的 std::same_as 是可交换的
template<typename T, typename U>
    requires std::same_as<T, U>
void same_type_func(T, U) {
    std::cout << "  参数类型相同\n";
}

template<typename T, typename U>
    requires std::same_as<U, T> && std::integral<T>  // 注意顺序相反
void same_type_func(T, U) {
    std::cout << "  参数类型相同且为整数\n";  // 优先匹配
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第4章：概念、需求和约束详解 ===\n\n";

    // 4.1 约束
    std::cout << "--- 4.1 约束 ---\n";
    std::cout << std::format("  constrained_add(3, 4) = {}\n", constrained_add(3, 4));
    std::cout << std::format("  constrained_mul(3, 4) = {}\n", constrained_mul(3, 4));
    std::cout << std::format("  constrained_square(5) = {}\n", constrained_square(5));
    std::cout << "\n";

    // 4.2 requires 子句
    std::cout << "--- 4.2 requires 子句 ---\n";
    std::vector<int> v{1, 2, 3};
    print_iterator_info(v.begin());
    std::cout << std::format("  safe_div(10, 3) = {}\n", safe_div(10, 3));
    std::cout << std::format("  safe_div(3.14, 2.0) = {}\n", safe_div(3.14, 2.0));
    std::cout << std::format("  convert_value<int, double>(42) = {}\n", convert_value<int, double>(42));
    std::cout << "\n";

    // 4.3 临时布尔表达式
    std::cout << "--- 4.3 临时布尔表达式 ---\n";
    process_small_type(42);
    process_small_type(3.14);
    int arr[] = {10, 20, 30};
    process_array(arr);
    non_string_process(42);
    // non_string_process("hello");  // 编译错误: const char* 可转为 string
    int x = 42;
    print_if_pointer(&x);
    print_if_pointer(nullptr);
    AlignedBuffer<int, 4> buf;
    buf.info();
    // AlignedBuffer<int, 3> bad_buf;  // 编译错误: 3 不是 2 的幂
    std::cout << "\n";

    // 4.4 requires 表达式
    std::cout << "--- 4.4 requires 表达式 ---\n";

    // 4.4.1 简单需求
    std::cout << "  [简单需求]\n";
    std::cout << std::format("    int SupportsAddSub: {}\n", SupportsAddSub<int>);
    std::cout << std::format("    string SupportsAddSub: {}\n", SupportsAddSub<std::string>);
    std::cout << std::format("    int* SupportsDeref: {}\n", SupportsDeref<int*>);
    std::cout << std::format("    int SupportsDeref: {}\n", SupportsDeref<int>);

    // 4.4.2 类型要求
    std::cout << "  [类型要求]\n";
    std::cout << std::format("    pair HasFirstAndSecond: {}\n", HasFirstAndSecond<std::pair<int,double>>);
    std::cout << std::format("    vector HasValueType: {}\n", HasValueType<std::vector<int>>);
    std::cout << std::format("    int WrappableAsNumber: {}\n", WrappableAsNumber<int>);
    std::cout << std::format("    double WrappableAsNumber: {}\n", WrappableAsNumber<double>);
    // WrappableAsNumber<std::string>  // false: string 不是 integral
    std::cout << std::format("    int TrulyHashable: {}\n", TrulyHashable<int>);
    std::cout << std::format("    string TrulyHashable: {}\n", TrulyHashable<std::string>);

    // 4.4.3 复合要求
    std::cout << "  [复合要求]\n";
    std::cout << std::format("    int EqualityComparable: {}\n", EqualityComparable<int>);
    std::cout << std::format("    string EqualityComparable: {}\n", EqualityComparable<std::string>);
    std::cout << std::format("    int* PointsToInteger: {}\n", PointsToInteger<int*>);
    std::cout << std::format("    double* PointsToInteger: {}\n", PointsToInteger<double*>);

    // 4.4.4 嵌套要求
    std::cout << "  [嵌套要求]\n";
    std::cout << std::format("    int NonConstType: {}\n", NonConstType<int>);
    std::cout << std::format("    const int NonConstType: {}\n", NonConstType<const int>);
    std::cout << std::format("    int SafeMakeUnsigned: {}\n", SafeMakeUnsigned<int>);
    std::cout << std::format("    bool SafeMakeUnsigned: {}\n", SafeMakeUnsigned<bool>);
    std::cout << "\n";

    // 4.5 概念详解
    std::cout << "--- 4.5 概念详解 ---\n";
    std::cout << std::format("  list SortableContainer: {}\n", SortableContainer<std::list<int>>);
    std::cout << std::format("  vector SortableContainer: {}\n", SortableContainer<std::vector<int>>);
    std::cout << std::format("  LessThan10<5>: {}\n", LessThan10<5>);
    std::cout << std::format("  LessThan10<15>: {}\n", LessThan10<15>);
    std::cout << std::format("  PowerOf2<8>: {}\n", PowerOf2<8>);
    std::cout << std::format("  PowerOf2<6>: {}\n", PowerOf2<6>);
    FixedMemory<int, 8> fm;
    fm.info();
    std::cout << "\n";

    // 4.6 使用概念作为类型约束
    std::cout << "--- 4.6 使用概念作为类型约束 ---\n";
    IntegralCounter<int> counter(0);
    counter.increment();
    counter.increment();
    counter.increment();
    std::cout << std::format("  counter.get() = {}\n", counter.get());
    print_integral(42);
    // print_integral(3.14);  // 编译错误
    accept_convertible_to_int(42);
    accept_convertible_to_int(true);  // bool 可转为 int
    auto val = make_copyable(42);
    std::cout << std::format("  make_copyable(42) = {}\n", val);
    BoundedArray<double, 5> ba;
    std::cout << std::format("  BoundedArray<double,5>.size() = {}\n", ba.size());
    std::cout << "\n";

    // 4.7 约束涵盖
    std::cout << "--- 4.7 约束涵盖 ---\n";

    // 基础 vs 派生概念重载
    SimpleRect sr(10, 20);
    ColoredRect cr(30, 40, {255, 0, 0});
    process_geo(sr);   // 调用 GeoObject 版本
    process_geo(cr);   // 调用 ColoredGeoObject 版本（涵盖优先）

    // 间接涵盖
    std::vector<int> vec_for_sub{1, 2, 3};
    foo_subsumption(vec_for_sub);  // ContCopy 优先（间接涵盖 RgSwap）

    std::list<int> lst_for_sub{4, 5, 6};
    foo_subsumption(lst_for_sub);  // 只匹配 RgSwap

    // 可交换概念
    same_type_func(1, 2);     // 优先匹配 integral 版本
    same_type_func(1.0, 2.0); // 匹配基础版本

    std::cout << "\n";

    // 补充: 概念在 if constexpr 中的使用
    std::cout << "--- 补充: 概念作为编译期布尔值 ---\n";
    auto describe = [](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::integral<T>) {
            std::cout << std::format("  {} 是整数类型\n", val);
        } else if constexpr (std::floating_point<T>) {
            std::cout << std::format("  {} 是浮点类型\n", val);
        } else if constexpr (std::same_as<T, std::string>) {
            std::cout << std::format("  \"{}\" 是字符串类型\n", val);
        } else {
            std::cout << std::format("  {} 是其他类型\n", val);
        }
    };
    describe(42);
    describe(3.14);
    describe(std::string("hello"));
    describe(true);

    return 0;
}
