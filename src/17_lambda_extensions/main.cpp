// ============================================================
// 第17章: Lambda扩展 (Lambda Extensions)
// 参考: C++20 Complete Guide Chapter 17
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
#include <set>
#include <variant>
#include <type_traits>
#include <utility>

// ============================================================
// 17.1 带模板参数的泛型Lambda
// ============================================================

void demo_template_lambda() {
    std::cout << "--- 17.1 带模板参数的泛型Lambda ---\n";

    // C++20: 泛型lambda可以有显式模板参数
    auto printType = []<typename T>(const T& val) {
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "  int: " << val << "\n";
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "  string: \"" << val << "\"\n";
        } else if constexpr (std::is_same_v<T, const char*>) {
            std::cout << "  cstr: \"" << val << "\"\n";
        } else {
            std::cout << "  other: " << val << "\n";
        }
    };
    printType(42);
    printType(std::string{"hello"});
    printType("world");

    // 模板参数: 匹配特定容器类型
    auto printVector = []<typename T>(const std::vector<T>& vec) {
        std::cout << "  vector(" << vec.size() << "): ";
        for (const auto& v : vec) std::cout << v << " ";
        std::cout << "\n";
    };
    std::vector<int> vi{1, 2, 3};
    std::vector<double> vd{1.1, 2.2};
    printVector(vi);
    printVector(vd);

    // 模板参数: 数组大小
    auto printArray = []<typename T, int N>(T (&arr)[N]) {
        std::cout << "  array[" << N << "]: ";
        for (int i = 0; i < N; ++i) std::cout << arr[i] << " ";
        std::cout << "\n";
    };
    int arr[] = {10, 20, 30, 40};
    printArray(arr);

    // 模板参数: 完美转发参数包
    auto forwardCall = []<typename... Types>(Types&&... args) {
        ([](auto&& x) { std::cout << "  fwd: " << x << "\n"; }(std::forward<Types>(args)), ...);
    };
    forwardCall(1, 2.5, std::string{"ok"});
    std::cout << "\n";
}

// ============================================================
// 17.1.1 泛型Lambda中使用模板参数替代decltype
// ============================================================

void demo_template_vs_decltype() {
    std::cout << "--- 17.1.1 模板参数替代decltype ---\n";

    std::variant<int, std::string> var = std::string{"hello"};

    // C++20方式: 用模板参数, 更清晰
    std::visit([]<typename T>(const T& val) {
        if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "  (C++20) string: \"" << val << "\"\n";
        } else {
            std::cout << "  (C++20) other: " << val << "\n";
        }
    }, var);
    std::cout << "\n";
}

// ============================================================
// 17.2 Lambda默认构造函数和赋值运算符
// ============================================================

class Customer {
    std::string name;
public:
    Customer() = default;
    Customer(std::string n) : name(std::move(n)) {}
    const std::string& getName() const { return name; }
    friend bool operator==(const Customer& a, const Customer& b) {
        return a.name == b.name;
    }
};

void demo_default_construct() {
    std::cout << "--- 17.2 Lambda默认构造和容器使用 ---\n";

    // C++20: 无捕获的lambda可以默认构造
    auto cmp1 = [](const auto& x, const auto& y) {
        return x > y;
    };
    decltype(cmp1) cmp2;    // C++20: OK (默认构造)
    cmp2 = cmp1;             // C++20: OK (赋值)

    std::cout << "  cmp2(3,5) = " << cmp2(3, 5) << "\n";
    std::cout << "  cmp2(5,3) = " << cmp2(5, 3) << "\n";

    // 用于容器: 只需传递lambda类型
    auto lessName = [](const Customer& c1, const Customer& c2) {
        return c1.getName() < c2.getName();
    };
    std::set<Customer, decltype(lessName)> custSet;
    custSet.insert(Customer("Alice"));
    custSet.insert(Customer("Bob"));
    custSet.insert(Customer("Charlie"));
    std::cout << "  set: ";
    for (const auto& c : custSet) std::cout << c.getName() << " ";
    std::cout << "\n\n";
}

// ============================================================
// 17.3 Lambda作为非类型模板参数
// 注: MSVC v14.44 对 lambda as NTTP 可能触发ICE

void demo_lambda_nttp() {
    std::cout << "--- 17.3 Lambda作为非类型模板参数 ---\n";
    // C++20: 无捕获lambda可作为非类型模板参数
    // template<auto GetVat> int addTax(int value) { ... }
    // auto tax = [] { return 0.19; };
    // addTax<tax>(100);  // OK in C++20
    std::cout << "  (lambda as NTTP: 见文档示例, MSVC可能触发ICE)\n";
    std::cout << "\n";
}

// ============================================================
// 17.4 consteval Lambda
// ============================================================

void demo_consteval_lambda() {
    std::cout << "--- 17.4 consteval Lambda ---\n";

    // consteval lambda: 必须在编译时调用
    auto hashed = [](const char* str) consteval {
        std::size_t hash = 5381;
        while (*str != '\0') {
            hash = hash * 33 ^ static_cast<unsigned char>(*str);
            ++str;
        }
        return hash;
    };

    // 编译时调用:
    constexpr auto h1 = hashed("beer");
    constexpr auto h2 = hashed("wine");
    constexpr auto h3 = hashed("water");
    std::cout << "  hashes: " << h1 << ", " << h2 << ", " << h3 << "\n\n";
}

// ============================================================
// 17.5.2 捕获结构化绑定
// ============================================================

void demo_capture_structured_binding() {
    std::cout << "--- 17.5.2 捕获结构化绑定 ---\n";

    std::pair<int, std::string> pairs[] = {{1, "one"}, {2, "two"}, {3, "three"}};
    for (const auto& [key, val] : pairs) {
        // C++20: 允许捕获结构化绑定
        auto l = [key, &val] {
            std::cout << "  [" << key << "] = " << val << "\n";
        };
        l();
    }
    std::cout << "\n";
}

// ============================================================
// 17.5.3 捕获参数包
// ============================================================

void demo_capture_pack() {
    std::cout << "--- 17.5.3 捕获参数包 ---\n";
    // C++20: [...args = std::move(args)]
    // 注: MSVC v14.44 对pack init-capture支持不佳
    // 标准语法: auto l = [op, ...args = std::move(args)] { return op(args...); };
    std::cout << "  (pack init-capture [...args=std::move(args)] 需GCC/Clang)\n";
    std::cout << "\n";
}

// ============================================================
// 17.5.1 捕获this的变化
// ============================================================

void demo_capture_this() {
    std::cout << "--- 17.5.1 捕获this的变化 ---\n";
    std::cout << "  C++20: [=, this] 合法, [=, *this] 按值捕获对象副本\n";
    std::cout << "  隐式捕获 *this 已弃用\n\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第17章: Lambda扩展 ===\n\n";

    demo_template_lambda();
    demo_template_vs_decltype();
    demo_default_construct();
    demo_lambda_nttp();
    demo_consteval_lambda();
    demo_capture_structured_binding();
    demo_capture_pack();
    demo_capture_this();

    return 0;
}
