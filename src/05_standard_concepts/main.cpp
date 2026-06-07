#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <array>
#include <span>
#include <concepts>
#include <type_traits>
#include <ranges>
#include <iterator>
#include <algorithm>
#include <functional>
#include <compare>
#include <random>
#include <complex>
#include <memory>
#include <format>
#include <numeric>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第5章：标准概念详解 (Standard Concepts in Detail)
// 参考: C++20 Complete Guide Chapter 5
// ============================================================

// --- 5.2 与语言相关的概念 ---

// 5.2.1 算术概念 (Arithmetic Concepts)

// integral: 包括 bool, char, short, int, long, long long 等所有整数类型
template<std::integral T>
T safe_mod(T a, T b) {
    return a % b;
}

// signed_integral: 有符号整型 (int, short, long, long long 等)
template<std::signed_integral T>
T abs_value(T val) {
    return val < 0 ? -val : val;
}

// unsigned_integral: 无符号整型 (unsigned int, size_t, uint64_t 等)
template<std::unsigned_integral T>
T wrap_add(T a, T b) {
    return a + b;  // 无符号溢出是良定义的
}

// floating_point: float, double, long double
template<std::floating_point T>
T lerp(T a, T b, T t) {
    return a + t * (b - a);
}

// 5.2.2 对象概念 (Object Concepts)
// movable -> copyable -> semiregular -> regular

// movable: 支持移动构造、移动赋值、交换
// copyable: 可复制（包含 movable）
// semiregular: 可默认构造 + copyable
// regular: semiregular + equality_comparable

template<std::movable T>
T move_value(T val) {
    return std::move(val);
}

template<std::copyable T>
T copy_value(const T& val) {
    return val;
}

template<std::semiregular T>
T make_default() {
    return T{};  // 要求默认构造
}

template<std::regular T>
bool is_default_val(const T& val) {
    return val == T{};  // 要求 == 比较
}

// 5.2.3 类型间关系的概念

// same_as: 类型完全相同（参数顺序无关）
template<typename T, typename U>
    requires std::same_as<T, U>
void must_be_same_type(const T&, const U&) {
    std::cout << "  类型完全相同\n";
}

// convertible_to: From 可隐式和显式转换为 To
template<typename From, typename To>
    requires std::convertible_to<From, To>
To implicit_convert(const From& val) {
    return val;  // 隐式转换
}

// derived_from: D 公开派生自 B
struct Base { virtual ~Base() = default; int id = 0; };
struct Derived : public Base { int extra = 42; };
struct NotDerived { int x = 0; };

template<typename D>
    requires std::derived_from<D, Base>
void process_base(const D& obj) {
    std::cout << "  process_base: id=" << obj.id << "\n";
}

// constructible_from: 可以用指定参数构造
template<typename T>
    requires std::constructible_from<T, int>
T make_from_int(int val) {
    return T(val);
}

// swappable: 可以交换两个值
template<std::swappable T>
void my_swap(T& a, T& b) {
    using std::swap;
    swap(a, b);
}

// common_with: 两种类型有公共类型
template<typename T, typename U>
    requires std::common_with<T, U>
auto add_mixed(const T& a, const U& b) {
    using Common = std::common_type_t<T, U>;
    return static_cast<Common>(a) + static_cast<Common>(b);
}

// common_reference_with: 两种类型有公共引用类型
template<typename T, typename U>
    requires std::common_reference_with<T, U>
void compare_refs(const T& a, const U& b) {
    using CR = std::common_reference_t<const T&, const U&>;
    std::cout << std::format("  common_ref compare: {} vs {}\n",
        static_cast<CR>(a), static_cast<CR>(b));
}

// 5.2.4 比较概念

// equality_comparable: 支持 == 和 !=
template<std::equality_comparable T>
bool is_equal(const T& a, const T& b) {
    return a == b;
}

// totally_ordered: 支持 <, <=, >, >=, ==, !=
template<std::totally_ordered T>
T min_val(const T& a, const T& b) {
    return a < b ? a : b;
}

// three_way_comparable: 支持 <=> (三路比较)
template<std::three_way_comparable T>
auto compare_3way(const T& a, const T& b) {
    return a <=> b;
}

// --- 5.3 迭代器和范围的概念 ---

// 5.3.1 范围和视图的概念

// range: 支持 begin/end 迭代
template<std::ranges::range R>
void print_range_type() {
    using Val = std::ranges::range_value_t<R>;
    std::cout << std::format("    range_value_t: {}\n", typeid(Val).name());
}

// input_range: 可读的范围（至少提供输入迭代器）
template<std::ranges::input_range R>
auto sum_range(const R& r) {
    using Val = std::ranges::range_value_t<R>;
    Val total{};
    for (const auto& elem : r) total += elem;
    return total;
}

// forward_range: 可多遍遍历的范围
template<std::ranges::forward_range R>
std::size_t count_unique(R r) {
    std::set<std::ranges::range_value_t<R>> seen;
    for (const auto& elem : r) seen.insert(elem);
    return seen.size();
}

// random_access_range: 支持随机访问
template<std::ranges::random_access_range R>
auto& element_at(R& r, std::size_t idx) {
    return r[idx];  // O(1) 随机访问
}

// contiguous_range: 元素在连续内存中
template<std::ranges::contiguous_range R>
auto raw_ptr(R& r) {
    return std::ranges::data(r);  // 返回原始指针
}

// sized_range: O(1) 获取大小
template<std::ranges::sized_range R>
void print_sized_info(const R& r) {
    std::cout << std::format("    size={}, empty={}\n",
        std::ranges::size(r), std::ranges::empty(r));
}

// common_range: 迭代器和哨兵类型相同
template<std::ranges::common_range R>
void process_common(R& r) {
    // 可以安全地用 begin/end 同类型做操作
    auto first = std::ranges::begin(r);
    auto last = std::ranges::end(r);
    std::cout << std::format("    common_range: same iterator/sentinel type: {}\n",
        std::is_same_v<decltype(first), decltype(last)>);
}

// borrowed_range: 迭代器不依赖范围生命周期
// 左值范围总是 borrowed_range；span, string_view 等也满足

// view: 轻量级范围（复制/移动成本很低）
template<std::ranges::view V>
void print_view_info() {
    std::cout << "    是一个 view（轻量级范围）\n";
}

// viewable_range: 可安全转换为 view 的范围
template<std::ranges::viewable_range R>
auto to_view(R&& r) {
    return std::views::all(std::forward<R>(r));
}

// 5.3.2 针对类似指针对象的概念

// indirectly_readable: 可通过 * 读取
// indirectly_writable: 可通过 * 写入
// indirectly_movable: 可间接移动
// indirectly_copyable: 可间接复制
// indirectly_swappable: 可间接交换
// indirectly_comparable: 可间接比较

// 使用 indirectly_readable 检查指针/迭代器可读
template<std::indirectly_readable P>
auto read_ptr(P p) -> std::iter_value_t<P> {
    return *p;
}

// 5.3.3 迭代器相关概念
// input_output_iterator < output_iterator
//                        < input_iterator < forward_iterator < bidirectional_iterator
//                        < random_access_iterator < contiguous_iterator

// input_iterator: 单遍读取
template<std::input_iterator Iter>
void read_all(Iter first, Iter last) {
    std::cout << "    [input_iterator] 读取: ";
    for (; first != last; ++first) std::cout << *first << " ";
    std::cout << "\n";
}

// forward_iterator: 多遍读取
template<std::forward_iterator Iter>
std::size_t count_matches(Iter first, Iter last,
                          const std::iter_value_t<Iter>& val) {
    std::size_t n = 0;
    for (; first != last; ++first)
        if (*first == val) ++n;
    return n;
}

// bidirectional_iterator: 支持反向遍历
template<std::bidirectional_iterator Iter>
void print_reverse(Iter first, Iter last) {
    std::cout << "    [bidirectional] 反向: ";
    if (first == last) { std::cout << "(空)\n"; return; }
    auto it = last;
    do { --it; std::cout << *it << " "; } while (it != first);
    std::cout << "\n";
}

// random_access_iterator: 支持跳跃访问
template<std::random_access_iterator Iter>
auto nth_element(Iter base, std::iter_difference_t<Iter> n) {
    return *(base + n);  // O(1) 跳跃
}

// contiguous_iterator: 连续内存迭代器
// vector 的迭代器满足此概念

// sentinel_for: S 可作为 Iter 的哨兵
// sized_sentinel_for: 哨兵与迭代器之间的距离可 O(1) 计算

// 5.3.4 算法相关的迭代器概念
// permutable: 可重新排列元素
// mergeable: 可合并两个已排序序列
// sortable: 可排序

// --- 5.4 可调用对象相关概念 ---

// 5.4.1 可调用对象的基本概念

// invocable: 可以用指定参数调用
template<typename Op, typename... Args>
    requires std::invocable<Op, Args...>
auto safe_invoke(Op&& op, Args&&... args) {
    return std::invoke(std::forward<Op>(op), std::forward<Args>(args)...);
}

// regular_invocable: 语义上不修改操作和参数（与 invocable 等价）
// predicate: 可调用且返回布尔值
template<typename Pred, typename T>
    requires std::predicate<Pred, T>
bool check_all(Pred pred, const std::vector<T>& vals) {
    for (const auto& v : vals)
        if (!pred(v)) return false;
    return true;
}

// relation: 二元关系（谓词接受两个参数）
template<typename Pred, typename T, typename U>
    requires std::relation<Pred, T, U>
bool check_relation(Pred pred, const T& a, const U& b) {
    return pred(a, b);
}

// strict_weak_order: 严格弱序（如 <）
template<typename Comp, typename T>
    requires std::strict_weak_order<Comp, T, T>
bool is_less(Comp comp, const T& a, const T& b) {
    return comp(a, b);
}

// uniform_random_bit_generator: 随机数生成器
// 要求：operator()() 返回无符号整数，有 min() 和 max() 静态方法
template<std::uniform_random_bit_generator Gen>
auto random_val(Gen& gen) {
    return gen();
}

// 5.4.2 迭代器使用的可调用对象概念
// indirectly_unary_invocable, indirectly_unary_predicate 等
// 这些通常用于算法内部约束

// --- 5.5 辅助概念 ---

// default_initializable: 可默认构造 T x; 或 T{}
// move_constructible: 可移动构造 T t2{std::move(t1)}
// copy_constructible: 可复制构造 T t2{t1}
// destructible: 可销毁（析构不抛异常）
// weakly_incrementable: 支持递增（不要求保持相等性）
// incrementable: 支持递增且保持相等性

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第5章：标准概念详解 ===\n\n";

    // 5.2.1 算术概念
    std::cout << "--- 5.2.1 算术概念 ---\n";
    std::cout << std::format("  safe_mod(17, 5) = {}\n", safe_mod(17, 5));
    std::cout << std::format("  abs_value(-42) = {}\n", abs_value(-42));
    std::cout << std::format("  wrap_add(250u, 10u) = {}\n", wrap_add(250u, 10u));
    std::cout << std::format("  lerp(0.0, 10.0, 0.3) = {}\n", lerp(0.0, 10.0, 0.3));

    std::cout << "  [integral 检查]:\n";
    std::cout << std::format("    int: {}, bool: {}, char: {}\n",
        std::integral<int>, std::integral<bool>, std::integral<char>);
    std::cout << std::format("    float: {}, double: {}, string: {}\n",
        std::integral<float>, std::integral<double>, std::integral<std::string>);

    std::cout << "  [signed/unsigned 区分]:\n";
    std::cout << std::format("    int: signed={}, unsigned={}\n",
        std::signed_integral<int>, std::unsigned_integral<int>);
    std::cout << std::format("    unsigned int: signed={}, unsigned={}\n",
        std::signed_integral<unsigned>, std::unsigned_integral<unsigned>);
    std::cout << std::format("    bool: signed={}, unsigned={}\n",
        std::signed_integral<bool>, std::unsigned_integral<bool>);
    std::cout << std::format("    char: signed={}, unsigned={}\n",
        std::signed_integral<char>, std::unsigned_integral<char>);

    std::cout << "  [floating_point 检查]:\n";
    std::cout << std::format("    float: {}, double: {}, long double: {}\n",
        std::floating_point<float>, std::floating_point<double>, std::floating_point<long double>);
    std::cout << "\n";

    // 5.2.2 对象概念
    std::cout << "--- 5.2.2 对象概念 ---\n";
    std::cout << "  [对象概念层级 movable < copyable < semiregular < regular]:\n";
    auto print_obj_concepts = []<typename T>(const char* name) {
        std::cout << std::format("    {:<12} movable={} copyable={} semiregular={} regular={}\n",
            name,
            std::movable<T>, std::copyable<T>,
            std::semiregular<T>, std::regular<T>);
    };
    print_obj_concepts.operator()<int>("int");
    print_obj_concepts.operator()<double>("double");
    print_obj_concepts.operator()<std::string>("string");
    print_obj_concepts.operator()<std::vector<int>>("vector<int>");
    print_obj_concepts.operator()<std::unique_ptr<int>>("unique_ptr");
    // move_only 类型
    struct MoveOnly {
        MoveOnly() = default;
        MoveOnly(MoveOnly&&) = default;
        MoveOnly& operator=(MoveOnly&&) = default;
        void swap(MoveOnly&) {}
    };
    std::cout << std::format("    {:<12} movable={} copyable={} semiregular={} regular={}\n",
        "MoveOnly", std::movable<MoveOnly>, std::copyable<MoveOnly>,
        std::semiregular<MoveOnly>, std::regular<MoveOnly>);

    std::cout << std::format("  copy_value(42) = {}\n", copy_value(42));
    std::cout << std::format("  make_default<int>() = {}\n", make_default<int>());
    std::cout << std::format("  is_default_val(0) = {}\n", is_default_val(0));
    std::cout << std::format("  is_default_val(42) = {}\n", is_default_val(42));
    std::cout << "\n";

    // 5.2.3 类型间关系
    std::cout << "--- 5.2.3 类型间关系 ---\n";

    // same_as
    std::cout << "  [same_as]:\n";
    must_be_same_type(1, 2);
    // must_be_same_type(1, 2.0);  // 编译错误

    // convertible_to
    std::cout << "  [convertible_to]:\n";
    std::cout << std::format("    int->double: {}, double->int: {}\n",
        std::convertible_to<int, double>, std::convertible_to<double, int>);
    std::cout << std::format("    int->string: {}, const char*->string: {}\n",
        std::convertible_to<int, std::string>, std::convertible_to<const char*, std::string>);
    auto d = implicit_convert<int, double>(42);
    std::cout << std::format("    implicit_convert<int,double>(42) = {}\n", d);

    // derived_from
    std::cout << "  [derived_from]:\n";
    Derived der;
    process_base(der);
    // process_base(NotDerived{});  // 编译错误
    std::cout << std::format("    Derived->Base: {}, NotDerived->Base: {}\n",
        std::derived_from<Derived, Base>, std::derived_from<NotDerived, Base>);

    // constructible_from
    std::cout << "  [constructible_from]:\n";
    std::cout << std::format("    string(const char*): {}, string(int, char): {}\n",
        std::constructible_from<std::string, const char*>,
        std::constructible_from<std::string, int, char>);
    std::cout << std::format("    complex(double): {}, complex(double,double): {}\n",
        std::constructible_from<std::complex<double>, double>,
        std::constructible_from<std::complex<double>, double, double>);
    auto cplx = make_from_int<std::complex<double>>(42);
    std::cout << "    make_from_int<complex<double>>(42) = (" << cplx.real() << "," << cplx.imag() << ")\n";

    // common_with / common_type
    std::cout << "  [common_with / common_type]:\n";
    std::cout << std::format("    int+double common_type: {}\n",
        std::is_same_v<std::common_type_t<int, double>, double>);
    std::cout << std::format("    add_mixed(1, 2.5) = {}\n", add_mixed(1, 2.5));

    // swappable
    std::cout << "  [swappable]:\n";
    int a = 10, b = 20;
    my_swap(a, b);
    std::cout << std::format("    after swap: a={}, b={}\n", a, b);
    std::cout << "\n";

    // 5.2.4 比较概念
    std::cout << "--- 5.2.4 比较概念 ---\n";
    std::cout << "  [比较概念检查]:\n";
    std::cout << std::format("    int  equality_comparable={}, totally_ordered={}, three_way={}\n",
        std::equality_comparable<int>, std::totally_ordered<int>,
        std::three_way_comparable<int>);
    std::cout << std::format("    complex<int> totally_ordered={}, three_way={}\n",
        std::totally_ordered<std::complex<int>>, std::three_way_comparable<std::complex<int>>);

    std::cout << std::format("  is_equal(42, 42) = {}\n", is_equal(42, 42));
    std::cout << std::format("  min_val(3.14, 2.72) = {}\n", min_val(3.14, 2.72));

    // three_way_comparable
    auto cmp = compare_3way(42, 42);
    std::cout << std::format("  42 <=> 42: ", "");
    if (cmp < 0) std::cout << "less\n";
    else if (cmp > 0) std::cout << "greater\n";
    else std::cout << "equal\n";

    std::cout << "  [比较类别]:\n";
    std::cout << std::format("    int  <=> int   → {}\n",
        typeid(decltype(42 <=> 42)).name());
    std::cout << std::format("    double <=> double → {}\n",
        typeid(decltype(3.14 <=> 2.72)).name());
    std::cout << "\n";

    // 5.3.1 范围和视图的概念
    std::cout << "--- 5.3.1 范围和视图的概念 ---\n";
    std::cout << "  [范围概念层级检查]:\n";
    auto print_range_concepts = []<typename R>(const char* name) {
        std::cout << std::format("    {:<16} range={} input={} fwd={} bidir={} rand={} contig={} sized={}\n",
            name,
            std::ranges::range<R>,
            std::ranges::input_range<R>,
            std::ranges::forward_range<R>,
            std::ranges::bidirectional_range<R>,
            std::ranges::random_access_range<R>,
            std::ranges::contiguous_range<R>,
            std::ranges::sized_range<R>);
    };
    print_range_concepts.operator()<std::vector<int>>("vector<int>");
    print_range_concepts.operator()<std::list<int>>("list<int>");
    print_range_concepts.operator()<std::deque<int>>("deque<int>");
    print_range_concepts.operator()<std::set<int>>("set<int>");
    print_range_concepts.operator()<std::string>("string");
    print_range_concepts.operator()<int[5]>("int[5]");

    // view 相关
    std::cout << "  [view / common_range / borrowed_range]:\n";
    auto take_v = std::views::take(std::vector<int>{}, 3);
    std::cout << std::format("    take_view is view: {}\n", std::ranges::view<decltype(take_v)>);
    std::cout << std::format("    vector is view: {}\n", std::ranges::view<std::vector<int>>);
    std::cout << std::format("    vector common_range: {}\n", std::ranges::common_range<std::vector<int>>);
    std::cout << std::format("    span borrowed_range: {}\n", std::ranges::borrowed_range<std::span<int>>);
    std::cout << std::format("    vector borrowed_range (左值): {}\n", std::ranges::borrowed_range<std::vector<int>&>);

    // sized_range 示例
    std::vector<int> sv{1, 2, 3, 4, 5};
    std::cout << "  [sized_range 信息]:\n";
    print_sized_info(sv);

    // sum_range 示例
    std::vector<int> nums{1, 2, 3, 4, 5};
    std::cout << std::format("  sum_range(vector) = {}\n", sum_range(nums));
    int arr[] = {10, 20, 30};
    std::cout << std::format("  sum_range(array) = {}\n", sum_range(arr));

    // count_unique 示例
    std::vector<int> dup{1, 2, 2, 3, 3, 3, 4};
    std::cout << std::format("  count_unique({{1,2,2,3,3,3,4}}) = {}\n", count_unique(dup));

    // random_access 和 contiguous
    std::vector<int> rav{10, 20, 30, 40, 50};
    std::cout << std::format("  element_at(rav, 2) = {}\n", element_at(rav, 2));
    auto* raw = raw_ptr(rav);
    std::cout << std::format("  raw_ptr(rav)[0] = {}\n", raw[0]);

    // viewable_range -> to_view
    auto v = to_view(std::vector<int>{1, 2, 3});
    std::cout << std::format("  to_view(vector) size: {}\n", std::ranges::size(v));
    std::cout << "\n";

    // 5.3.3 迭代器相关概念
    std::cout << "--- 5.3.3 迭代器相关概念 ---\n";
    std::cout << "  [迭代器概念层级]:\n";
    auto print_iter_concepts = []<typename Iter>(const char* name) {
        std::cout << std::format("    {:<20} input={} fwd={} bidir={} rand={} contig={}\n",
            name,
            std::input_iterator<Iter>,
            std::forward_iterator<Iter>,
            std::bidirectional_iterator<Iter>,
            std::random_access_iterator<Iter>,
            std::contiguous_iterator<Iter>);
    };
    print_iter_concepts.operator()<std::vector<int>::iterator>("vector::iter");
    print_iter_concepts.operator()<std::list<int>::iterator>("list::iter");
    print_iter_concepts.operator()<std::set<int>::iterator>("set::iter");
    print_iter_concepts.operator()<int*>("int*");
    print_iter_concepts.operator()<std::istream_iterator<int>>("istream_iter");

    // 读取示例
    std::cout << "  [迭代器操作]:\n";
    std::vector<int> read_v{5, 10, 15, 20};
    read_all(read_v.begin(), read_v.end());

    // 反向遍历
    std::list<int> rev_list{1, 2, 3, 4, 5};
    print_reverse(rev_list.begin(), rev_list.end());

    // 随机访问
    std::vector<int> rand_v{100, 200, 300, 400, 500};
    std::cout << std::format("    nth_element(rand_v, 2) = {}\n",
        nth_element(rand_v.begin(), 2));

    // sentinel_for 检查
    std::cout << "  [sentinel_for]:\n";
    std::cout << std::format("    vector iterator sentinel_for self: {}\n",
        std::sentinel_for<std::vector<int>::iterator, std::vector<int>::iterator>);

    // sized_sentinel_for
    std::cout << std::format("    vector iter sized_sentinel: {}\n",
        std::sized_sentinel_for<std::vector<int>::iterator, std::vector<int>::iterator>);
    std::cout << "\n";

    // 5.4 可调用对象相关概念
    std::cout << "--- 5.4 可调用对象相关概念 ---\n";

    // invocable 检查
    std::cout << "  [invocable 检查]:\n";
    auto lambda_int = [](int x) { return x * 2; };
    auto lambda_bool = [](int x) { return x > 0; };
    std::cout << std::format("    lambda(int): invocable<int>={}, invocable<string>={}\n",
        std::invocable<decltype(lambda_int), int>,
        std::invocable<decltype(lambda_int), std::string>);

    // safe_invoke 示例
    auto add_func = [](int a, int b) { return a + b; };
    std::cout << std::format("    safe_invoke(add, 3, 4) = {}\n",
        safe_invoke(add_func, 3, 4));

    // 成员指针的 invocable
    struct S { int member = 99; int mfunc(int x) { return x + member; } };
    std::cout << std::format("    invocable member ptr: {}\n",
        std::invocable<decltype(&S::member), S>);
    std::cout << std::format("    invocable member func: {}\n",
        std::invocable<decltype(&S::mfunc), S, int>);

    // predicate 检查
    std::cout << "  [predicate 检查]:\n";
    std::cout << std::format("    lambda(int)->bool is predicate<int>: {}\n",
        std::predicate<decltype(lambda_bool), int>);
    std::cout << std::format("    lambda(int)->int is predicate<int>: {}\n",
        std::predicate<decltype(lambda_int), int>);

    // check_all 示例
    auto is_positive = [](int x) { return x > 0; };
    std::vector<int> pos_vals{1, 2, 3};
    std::vector<int> mixed_vals{1, -2, 3};
    std::cout << std::format("    check_all(is_positive, {{1,2,3}}) = {}\n",
        check_all(is_positive, pos_vals));
    std::cout << std::format("    check_all(is_positive, {{1,-2,3}}) = {}\n",
        check_all(is_positive, mixed_vals));

    // strict_weak_order
    std::cout << "  [strict_weak_order / relation]:\n";
    auto less_than = [](int a, int b) { return a < b; };
    std::cout << std::format("    less_than is strict_weak_order<int>: {}\n",
        std::strict_weak_order<decltype(less_than), int, int>);
    std::cout << std::format("    less_than is relation<int>: {}\n",
        std::relation<decltype(less_than), int, int>);
    std::cout << std::format("    is_less(3, 5) = {}\n", is_less(std::less<int>{}, 3, 5));
    std::cout << std::format("    check_relation(std::less, 3, 5) = {}\n",
        check_relation(std::less<int>{}, 3, 5));

    // uniform_random_bit_generator
    std::cout << "  [uniform_random_bit_generator]:\n";
    std::mt19937 gen(42);
    std::cout << std::format("    mt19937 is uniform_random_bit_generator: {}\n",
        std::uniform_random_bit_generator<std::mt19937>);
    std::cout << std::format("    random_val(mt19937) = {}\n", random_val(gen));
    std::cout << std::format("    min={}, max={}\n", (gen.min)(), (gen.max)());
    std::cout << "\n";

    // 5.5 辅助概念
    std::cout << "--- 5.5 辅助概念 ---\n";
    std::cout << "  [构造/析构/交换概念]:\n";
    auto print_aux_concepts = []<typename T>(const char* name) {
        std::cout << std::format("    {:<16} default_init={} move_ctor={} copy_ctor={} destructible={} swappable={}\n",
            name,
            std::default_initializable<T>,
            std::move_constructible<T>,
            std::copy_constructible<T>,
            std::destructible<T>,
            std::swappable<T>);
    };
    print_aux_concepts.operator()<int>("int");
    print_aux_concepts.operator()<std::string>("string");
    print_aux_concepts.operator()<std::unique_ptr<int>>("unique_ptr");
    print_aux_concepts.operator()<void(*)()>("fn_ptr");

    // incrementable vs weakly_incrementable
    std::cout << "  [incrementable vs weakly_incrementable]:\n";
    std::cout << std::format("    int*        weakly_inc={} inc={}\n",
        std::weakly_incrementable<int*>, std::incrementable<int*>);
    std::cout << std::format("    vector::itr weakly_inc={} inc={}\n",
        std::weakly_incrementable<std::vector<int>::iterator>,
        std::incrementable<std::vector<int>::iterator>);
    std::cout << std::format("    istream_itr weakly_inc={} inc={}\n",
        std::weakly_incrementable<std::istream_iterator<int>>,
        std::incrementable<std::istream_iterator<int>>);

    // indirectly_readable / indirectly_writable
    std::cout << "  [indirect 概念]:\n";
    std::cout << std::format("    int*  readable={} writable={}\n",
        std::indirectly_readable<int*>, std::indirectly_writable<int*, int>);
    std::cout << std::format("    const int* readable={} writable={}\n",
        std::indirectly_readable<const int*>, std::indirectly_writable<const int*, int>);
    int x = 42;
    std::cout << std::format("    read_ptr(&x) = {}\n", read_ptr(&x));

    // indirectly_copyable / movable / swappable
    std::cout << std::format("    int*->int* copyable={} movable={} swappable={}\n",
        std::indirectly_copyable<int*, int*>,
        std::indirectly_movable<int*, int*>,
        std::indirectly_swappable<int*, int*>);

    // permutable / sortable / mergeable
    std::cout << "  [算法迭代器概念]:\n";
    std::cout << std::format("    vector::iter permutable={} sortable={} mergeable={}\n",
        std::permutable<std::vector<int>::iterator>,
        std::sortable<std::vector<int>::iterator>,
        std::mergeable<std::vector<int>::iterator,
                       std::vector<int>::iterator,
                       std::vector<int>::iterator>);
    std::cout << std::format("    list::iter   permutable={}\n",
        std::permutable<std::list<int>::iterator>);

    std::cout << "\n";
    return 0;
}
