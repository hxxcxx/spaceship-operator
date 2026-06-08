// ============================================================
// 第19章: 非类型模板参数(NTTP)扩展
// 参考: C++20 Complete Guide Chapter 19
// ============================================================

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <string_view>
#include <cmath>
#include <array>
#include <utility>
#include <type_traits>

// ============================================================
// 19.1.1 浮点值作为非类型模板参数
// ============================================================

template<double Vat>
int addTax(int value) {
    return static_cast<int>(std::round(value * (1 + Vat)));
}

// auto也支持浮点NTTP
template<auto Val>
void printNttp() {
    if constexpr (std::is_integral_v<decltype(Val)>) {
        std::cout << "  integral NTTP: " << Val << "\n";
    } else if constexpr (std::is_floating_point_v<decltype(Val)>) {
        std::cout << "  floating NTTP: " << Val << "\n";
    }
}

void demo_float_nttp() {
    std::cout << "--- 19.1.1 浮点值作为NTTP ---\n";

    // double作为NTTP
    std::cout << "  addTax<0.19>(100) = " << addTax<0.19>(100) << "\n";
    std::cout << "  addTax<0.19>(4199) = " << addTax<0.19>(4199) << "\n";
    std::cout << "  addTax<0.07>(1950) = " << addTax<0.07>(1950) << "\n";

    // auto NTTP
    printNttp<42>();
    printNttp<3.14>();

    // 浮点精度: 模板实例化可能因精度而异
    std::cout << "  注意: 浮点NTTP可能有精度问题\n";
    std::cout << "  is_same<42.0, 126.0/3> = "
              << std::is_same_v<decltype(addTax<42.0>), decltype(addTax<126.0 / 3>)>
              << "\n";
    std::cout << "\n";
}

// ============================================================
// 19.1.2 对象(结构体)作为非类型模板参数
// ============================================================

struct Tax {
    double value;
    constexpr Tax(double v) : value{v} {
        // 编译期断言
    }
    friend std::ostream& operator<<(std::ostream& strm, const Tax& t) {
        return strm << t.value;
    }
};

template<Tax Vat>
int addTaxStruct(int value) {
    return static_cast<int>(std::round(value * (1 + Vat.value)));
}

void demo_struct_nttp() {
    std::cout << "--- 19.1.2 结构体作为NTTP ---\n";

    constexpr Tax tax{0.19};
    std::cout << "  tax = " << tax << "\n";
    std::cout << "  addTaxStruct<tax>(100) = " << addTaxStruct<tax>(100) << "\n";
    std::cout << "  addTaxStruct<Tax{0.07}>(1950) = " << addTaxStruct<Tax{0.07}>(1950) << "\n";

    // 结构类型要求:
    // - 所有非静态成员是public的
    // - 类型是字面量类型(有constexpr构造函数)
    std::cout << "  (结构类型: public成员, 字面量类型)\n";
    std::cout << "\n";
}

// ============================================================
// 19.1.2 std::pair和std::array作为NTTP
// ============================================================

template<auto Val>
void printPairNttp() {
    std::cout << "  pair: (" << Val.first << ", " << Val.second << ")\n";
}

template<auto Val>
void printArrayNttp() {
    std::cout << "  array: ";
    for (const auto& v : Val) std::cout << v << " ";
    std::cout << "\n";
}

void demo_std_types_nttp() {
    std::cout << "--- 19.1.2 std::pair/array作为NTTP ---\n";

    printPairNttp<std::pair{47, 11}>();
    printArrayNttp<std::array{0, 8, 15, 42}>();
    std::cout << "\n";
}

// ============================================================
// 19.1.2 字符串作为非类型模板参数
// ============================================================

template<std::size_t N>
struct Str {
    char chars[N];

    constexpr Str(const char (&s)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            chars[i] = s[i];
        }
    }

    friend std::ostream& operator<<(std::ostream& strm, const Str& s) {
        return strm << s.chars;
    }
};

template<Str Prefix>
class Logger {
public:
    void log(std::string_view msg) const {
        std::cout << "  " << Prefix << msg << "\n";
    }
};

void demo_string_nttp() {
    std::cout << "--- 19.1.2 字符串作为NTTP ---\n";

    Logger<Str{"> "}> logger;
    logger.log("hello from NTTP string!");
    logger.log("字符串字面量作为模板参数");

    Logger<Str{"[LOG] "}> logger2;
    logger2.log("another prefix");
    std::cout << "\n";
}

// ============================================================
// 19.1.2 复合结构体作为NTTP
// ============================================================

struct Lit {
    int x = 42;
    int y;
    constexpr Lit(int i) : y{i} {}
};

struct Data {
    int i;
    std::array<double, 5> vals;
    Lit lit;
};

template<auto Obj>
void printObjType() {
    std::cout << "  type: " << typeid(Obj).name() << "\n";
    std::cout << "  Obj.i = " << Obj.i << "\n";
}

void demo_composite_nttp() {
    std::cout << "--- 19.1.2 复合结构体NTTP ---\n";

    constexpr Data d1{42, {1, 2, 3, 0, 0}, Lit{99}};
    std::cout << "  d1.i = " << d1.i << ", d1.lit.y = " << d1.lit.y << "\n";
    printObjType<Data{42, {1, 2, 3, 0, 0}, Lit{99}}>();

    constexpr Data d2{1, {2, 0, 0, 0, 0}, Lit{3}};
    printObjType<d2>();
    std::cout << "\n";
}

// ============================================================
// 19.1.3 Lambda作为NTTP
// 注: MSVC v14.44 对 lambda NTTP 触发ICE, 仅作文档说明

void demo_lambda_nttp() {
    std::cout << "--- 19.1.3 Lambda作为NTTP ---\n";
    // C++20: 无捕获lambda可作为NTTP
    // template<auto GetVat> int addTax(int v) { return round(v*(1+GetVat())); }
    // auto tax = []{return 0.19;}; addTax<tax>(100);
    // 注: MSVC v14.44 对lambda NTTP触发ICE
    std::cout << "  (lambda NTTP: 见文档示例, MSVC可能触发ICE)\n";
    std::cout << "  约束: 无捕获, 必须可编译期使用\n";
    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第19章: 非类型模板参数(NTTP)扩展 ===\n\n";

    demo_float_nttp();
    demo_struct_nttp();
    demo_std_types_nttp();
    demo_string_nttp();
    demo_composite_nttp();
    demo_lambda_nttp();

    return 0;
}
