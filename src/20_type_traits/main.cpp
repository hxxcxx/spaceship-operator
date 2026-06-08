// ============================================================
// 第20章: 新的类型特性 (New Type Traits)
// 参考: C++20 Complete Guide Chapter 20
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
#include <type_traits>
#include <functional>
#include <iterator>

// 辅助: 打印类型名称(简化)
template<typename T>
const char* typeName() {
    // 使用编译器特定的名称
#ifdef _MSC_VER
    return typeid(T).name();
#else
    return typeid(T).name();
#endif
}

// ============================================================
// 20.1 用于类型分类的新类型特性
// ============================================================

void demo_array_classification() {
    std::cout << "--- 20.1 有界/无界数组分类 ---\n";

    // is_bounded_array_v: 已知大小的数组
    // is_unbounded_array_v: 未知大小的数组
    std::cout << std::boolalpha;
    std::cout << "  int[5] bounded: " << std::is_bounded_array_v<int[5]> << "\n";
    std::cout << "  int[5] unbounded: " << std::is_unbounded_array_v<int[5]> << "\n";

    extern int unb[];  // 无界数组声明
    std::cout << "  int[] bounded: " << std::is_bounded_array_v<decltype(unb)> << "\n";
    std::cout << "  int[] unbounded: " << std::is_unbounded_array_v<decltype(unb)> << "\n";

    std::cout << "  int bounded: " << std::is_bounded_array_v<int> << "\n";
    std::cout << "  int unbounded: " << std::is_unbounded_array_v<int> << "\n";
    std::cout << "\n";
}

int unb[10];  // 定义

// ============================================================
// 20.2 用于类型检查的新类型特性
// ============================================================

void demo_nothrow_convertible() {
    std::cout << "--- 20.2 is_nothrow_convertible ---\n";
    std::cout << std::boolalpha;

    // char* -> string: 可以转换但可能抛异常
    std::cout << "  char* -> string convertible: "
              << std::is_convertible_v<char*, std::string> << "\n";
    std::cout << "  char* -> string nothrow: "
              << std::is_nothrow_convertible_v<char*, std::string> << "\n";

    // string -> string_view: 不抛异常
    std::cout << "  string -> string_view convertible: "
              << std::is_convertible_v<std::string, std::string_view> << "\n";
    std::cout << "  string -> string_view nothrow: "
              << std::is_nothrow_convertible_v<std::string, std::string_view> << "\n";

    // int -> double: 不抛异常
    std::cout << "  int -> double nothrow: "
              << std::is_nothrow_convertible_v<int, double> << "\n";
    std::cout << "\n";
}

// ============================================================
// 20.3 用于类型转换的新类型特性
// ============================================================

void demo_type_transformations() {
    std::cout << "--- 20.3 类型转换特性 ---\n";

    // 20.3.1 remove_cvref_t: 去掉引用+cv
    using T1 = std::remove_cvref_t<const std::string&>;      // std::string
    using T2 = std::remove_cvref_t<const char* const>;        // const char*
    using T3 = std::remove_cvref_t<volatile int&&>;           // int
    std::cout << "  remove_cvref_t<const string&> is string: "
              << std::is_same_v<T1, std::string> << "\n";
    std::cout << "  remove_cvref_t<const char* const> is const char*: "
              << std::is_same_v<T2, const char*> << "\n";
    std::cout << "  remove_cvref_t<volatile int&&> is int: "
              << std::is_same_v<T3, int> << "\n";

    // 20.3.2 unwrap_reference_t / unwrap_ref_decay_t
    std::string s = "hello";
    using U1 = std::unwrap_reference_t<decltype(std::ref(s))>;     // string&
    using U2 = std::unwrap_reference_t<decltype(std::cref(s))>;    // const string&
    using U3 = std::unwrap_reference_t<decltype(s)>;               // string
    std::cout << "  unwrap_ref(ref(s)) is string&: "
              << std::is_same_v<U1, std::string&> << "\n";
    std::cout << "  unwrap_ref(cref(s)) is const string&: "
              << std::is_same_v<U2, const std::string&> << "\n";

    using D1 = std::unwrap_ref_decay_t<decltype(std::ref(s))>;     // string&
    using D2 = std::unwrap_ref_decay_t<decltype(s)>;              // string (decay)
    std::cout << "  unwrap_ref_decay(ref(s)) is string&: "
              << std::is_same_v<D1, std::string&> << "\n";
    std::cout << "  unwrap_ref_decay(s&) is string: "
              << std::is_same_v<D2, std::string> << "\n";

    // 20.3.4 type_identity_t: 返回类型本身
    using I1 = std::type_identity_t<int>;      // int
    using I2 = std::type_identity_t<double>;   // double
    std::cout << "  type_identity_t<int> is int: "
              << std::is_same_v<I1, int> << "\n";

    // type_identity 用途: 禁止模板参数推导
    // void insert(vector<T>&, const type_identity_t<T>& val)
    // 调用 insert(vec_double, 42) 时 T不会被推导为int
    std::cout << "  (type_identity用于禁止模板参数推导)\n";
    std::cout << "\n";
}

// type_identity 实际应用: 禁止推导
template<typename T>
void insertTypeSafe(std::vector<T>& coll, const std::type_identity_t<T>& value) {
    coll.push_back(value);
}

void demo_type_identity_usage() {
    std::cout << "--- 20.3.4 type_identity实际应用 ---\n";
    std::vector<double> coll;
    insertTypeSafe(coll, 42);   // OK: 42不会把T推导为int, T仍是double
    std::cout << "  inserted " << coll[0] << " into vector<double>\n";
    std::cout << "\n";
}

// ============================================================
// 20.3.3 common_reference_t
// ============================================================

void demo_common_reference() {
    std::cout << "--- 20.3.3 common_reference ---\n";

    // common_reference_t: 找到多个类型的公共引用类型
    using CR1 = std::common_reference_t<int&, int>;       // int
    using CR2 = std::common_reference_t<int&, int&>;      // int&
    using CR3 = std::common_reference_t<int&, double>;    // double
    using CR4 = std::common_reference_t<int&&, int&&>;    // int&&

    std::cout << std::boolalpha;
    std::cout << "  int& + int => int: " << std::is_same_v<CR1, int> << "\n";
    std::cout << "  int& + int& => int&: " << std::is_same_v<CR2, int&> << "\n";
    std::cout << "  int& + double => double: " << std::is_same_v<CR3, double> << "\n";
    std::cout << "  int&& + int&& => int&&: " << std::is_same_v<CR4, int&&> << "\n";
    std::cout << "\n";
}

// ============================================================
// 20.4 迭代器新型特性
// ============================================================

void demo_iterator_traits() {
    std::cout << "--- 20.4 迭代器类型特性 ---\n";

    // iter_difference_t: 迭代器差值类型
    using Diff1 = std::iter_difference_t<int*>;           // ptrdiff_t
    using Diff2 = std::iter_difference_t<std::vector<long>>;  // ptrdiff_t
    std::cout << "  iter_difference_t<int*> is ptrdiff_t: "
              << std::is_same_v<Diff1, std::ptrdiff_t> << "\n";

    // iter_value_t: 迭代器指向的值类型
    using Val1 = std::iter_value_t<int*>;                 // int
    using Val2 = std::iter_value_t<const int* const>;     // int
    using Val3 = std::iter_value_t<std::vector<long>>;    // long
    std::cout << "  iter_value_t<int*> is int: "
              << std::is_same_v<Val1, int> << "\n";
    std::cout << "  iter_value_t<const int* const> is int: "
              << std::is_same_v<Val2, int> << "\n";
    std::cout << "  iter_value_t<vector<long>> is long: "
              << std::is_same_v<Val3, long> << "\n";

    // iter_reference_t: 解引用类型
    using Ref1 = std::iter_reference_t<int*>;             // int&
    using Ref2 = std::iter_reference_t<const int*>;       // const int&
    std::cout << "  iter_reference_t<int*> is int&: "
              << std::is_same_v<Ref1, int&> << "\n";
    std::cout << "  iter_reference_t<const int*> is const int&: "
              << std::is_same_v<Ref2, const int&> << "\n";

    // iter_rvalue_reference_t: 右值引用类型
    using RRef1 = std::iter_rvalue_reference_t<int*>;     // int&&
    std::cout << "  iter_rvalue_reference_t<int*> is int&&: "
              << std::is_same_v<RRef1, int&&> << "\n";
    std::cout << "\n";
}

// ============================================================
// 20.5 布局兼容性
// ============================================================

struct Point2D { int a; int b; };
struct Point3D { int x; int y; int z; };
struct Type1 { const int id; int val; std::string name; };

// 标准布局类型
struct B1 { int x; };
struct D1 : B1 { int y; };

void demo_layout_compatibility() {
    std::cout << "--- 20.5 布局兼容性 ---\n";
    std::cout << std::boolalpha;

    // is_layout_compatible_v
    std::cout << "  Point2D/Point3D layout_compatible: "
              << std::is_layout_compatible_v<Point2D, Point3D> << "\n";
    std::cout << "  char/char layout_compatible: "
              << std::is_layout_compatible_v<char, char> << "\n";
    std::cout << "  char/signed char layout_compatible: "
              << std::is_layout_compatible_v<char, signed char> << "\n";

    // is_pointer_interconvertible_base_of_v
    std::cout << "  B1 is base of D1 (ptr interconvertible): "
              << std::is_pointer_interconvertible_base_of_v<B1, D1> << "\n";
    // static_assert(std::is_standard_layout_v<D1>);
    static_assert(std::is_standard_layout_v<B1>);     // 应 true
    static_assert(!std::is_pointer_interconvertible_base_of_v<B1, D1>);
    // is_corresponding_member
    std::cout << "  Point2D::b & Point3D::y corresponding: "
              << std::is_corresponding_member(&Point2D::b, &Point3D::y) << "\n";
    std::cout << "  Point2D::b & Point3D::z corresponding: "
              << std::is_corresponding_member(&Point2D::b, &Point3D::z) << "\n";

    // is_pointer_interconvertible_with_class
    std::cout << "  D1::y ptr_interconvertible: "
              << std::is_pointer_interconvertible_with_class(&D1::y) << "\n";

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第20章: 新的类型特性 ===\n\n";

    demo_array_classification();
    demo_nothrow_convertible();
    demo_type_transformations();
    demo_type_identity_usage();
    demo_common_reference();
    demo_iterator_traits();
    demo_layout_compatibility();

    return 0;
}
