#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <format>
#include <chrono>
#include <locale>
#include <sstream>
#include <iterator>
#include <cmath>
#include <cstddef>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第10章: 格式化输出
// 参考: C++20 Complete Guide Chapter 10
// 注: MSVC v14.44的std::format编译时检查对std::string参数有bug
//     (C7595), 因此部分演示使用vformat规避
// ============================================================

// --- 10.1 格式化输出示例 ---

// 10.1.1 使用 std::format()
void demo_format_basic() {
    std::cout << "--- 10.1.1 std::format() 基本用法 ---\n";

    std::string str{"hello"};

    // 基本替换: {} 按顺序使用参数
    // 注: MSVC bug, 用vformat处理string参数
    auto sz = str.size();
    std::cout << std::format("String '{}' has {} chars\n", str, str.size());

    std::cout << std::vformat("  String {} has {} chars\n",
        std::make_format_args(str, sz));

    // 参数索引: {n} 指定使用第n个参数(0开始)
    std::cout << std::vformat("  {1} is the size of {0}\n",
        std::make_format_args(str, sz));

    // 重复使用参数 (纯int, 无MSVC bug)
    std::cout << std::format("  Value: {0}, again: {0}\n", 42);

    // 泛型代码中使用
    auto print2 = [](const auto& arg1, const auto& arg2) {
        std::cout << std::format("    args: {} and {}\n", arg1, arg2);
    };
    print2(7.7, true);
    print2(42, '?');

    // chrono 类型也支持格式化
    std::cout << std::format("    now: {}\n", std::chrono::system_clock::now());
    std::cout << std::format("    duration: {}\n", std::chrono::seconds{13});

    // 输出 {{ 和 }}
    std::cout << std::format("  转义花括号: {{}}\n");

    std::cout << "\n";
}

// 10.1.2 使用 std::format_to_n()
void demo_format_to_n() {
    std::cout << "--- 10.1.2 std::format_to_n() ---\n";

    std::string str{"hello"};

    // 写入固定大小缓冲区
    char buffer[64];
    auto strSz = str.size();
    auto ret = std::format_to_n(buffer, std::size(buffer) - 1,
        "String {} has {} chars\n", str, strSz);
    *(ret.out) = '\0';
    std::cout << "  buffer: " << buffer;
    std::cout << std::format("  wrote {} chars, need {} total\n",
        (int)(ret.out - buffer), (int)ret.size);

    // 使用 std::array
    std::array<char, 64> arrBuf{};
    auto ret2 = std::format_to_n(arrBuf.begin(), arrBuf.size() - 1,
        "Pi is approximately {:.5}\n", 3.14159265358979);
    *(ret2.out) = '\0';
    std::cout << "  arrBuf: " << arrBuf.data();

    // 截断示例: 缓冲区不够大
    std::array<char, 8> smallBuf{};
    auto ret3 = std::format_to_n(smallBuf.begin(), smallBuf.size() - 1,
        "{}", 123456.78);
    *(ret3.out) = '\0';
    std::cout << std::format("  truncated: {} (wrote {}, need {})\n",
        smallBuf.data(), (int)std::distance(smallBuf.begin(), ret3.out), (int)ret3.size);

    std::cout << "\n";
}

// 10.1.3 使用 std::format_to()
void demo_format_to() {
    std::cout << "--- 10.1.3 std::format_to() ---\n";

    std::string str{"hello"};
    auto strSz = str.size();

    // 直接写入流 (通过 ostreambuf_iterator)
    std::cout << "  stream: ";
    std::format_to(std::ostreambuf_iterator<char>{std::cout},
        "String {} has {} chars\n", str, strSz);

    // 追加到字符串 (通过 back_inserter)
    std::string s;
    std::format_to(std::back_inserter(s), "Hello ");
    std::format_to(std::back_inserter(s), "{}!\n", 42);
    std::cout << "  append: " << s;

    std::cout << "\n";
}

// 10.1.4 使用 std::formatted_size()
void demo_formatted_size() {
    std::cout << "--- 10.1.4 std::formatted_size() ---\n";

    auto sz = std::formatted_size("{:#x} {:#o} {:#b}\n", 255, 255, 255);
    std::cout << std::format("  formatted_size: {} chars\n", (int)sz);

    // 用于预留空间
    std::string result;
    result.reserve(sz);
    std::format_to(std::back_inserter(result), "{:#x} {:#o} {:#b}\n", 255, 255, 255);
    std::cout << "  reserved: " << result;

    std::cout << "\n";
}

// --- 10.2 格式化库性能与vformat ---

// 10.2.1 std::vformat() 和运行时格式字符串
void demo_vformat() {
    std::cout << "--- 10.2.1 std::vformat() 运行时格式 ---\n";

    // std::format() 要求编译时已知的格式字符串
    // const char* fmt1 = "{}\n";           // 运行时字符串
    // std::format(fmt1, 42);               // 编译错误!

    constexpr const char* fmt2 = "{}\n";
    std::cout << "  constexpr: " << std::format(fmt2, 42);

    // 运行时格式字符串使用 vformat
    const char* fmt3 = "{} and {}\n";
    int iVal = 42;
    double d = 1.7;
    std::cout << "  vformat: " << std::vformat(fmt3, std::make_format_args(iVal, d));

    // 无效的格式在运行时抛出 format_error
    try {
        const char* badFmt = "{:s}\n";
        int badVal = 42;
        std::cout << "  bad: " << std::vformat(badFmt, std::make_format_args(badVal));
    } catch (const std::format_error& e) {
        std::cout << std::format("  caught: {}\n", e.what());
    }

    std::cout << "\n";
}

// --- 10.3 格式化输出详解 ---

// 10.3.2 标准格式说明符概览
void demo_format_spec_overview() {
    std::cout << "--- 10.3.2 标准格式说明符 ---\n";
    std::cout << "  format: fill align sign # 0 width .prec L type\n";
    std::cout << "  fill:   padding char (only with align)\n";
    std::cout << "  align:  <(left) >(right) ^(center)\n";
    std::cout << "  sign:   -(neg only) +(pos+neg) space(pos=space)\n";
    std::cout << "  #:      alternate form (0x/0b prefix, force dot)\n";
    std::cout << "  0:      zero-pad (arithmetic types)\n";
    std::cout << "  width:  minimum field width\n";
    std::cout << "  .prec:  precision (float/string)\n";
    std::cout << "  L:      locale-aware\n";
    std::cout << "  type:   notation (d,x,f,e,g,s,p...)\n";
    std::cout << "\n";
}

// 10.3.3 宽度、精度和填充字符
void demo_width_precision_fill() {
    std::cout << "--- 10.3.3 width, precision, fill ---\n";

    // 宽度
    std::cout << std::format("  {:7}|  right (num default)\n", 42);
    std::cout << std::format("  {:<7}|  left\n", 42);
    std::cout << std::format("  {:^7}|  center\n", 42);
    std::cout << std::format("  {:>7}|  explicit right\n", 42);

    // 填充字符
    std::cout << std::format("  {:*<7}|  left fill *\n", 42);
    std::cout << std::format("  {:*>7}|  right fill *\n", 42);
    std::cout << std::format("  {:*^7}|  center fill *\n", 42);

    // 0填充(仅算术类型)
    std::cout << std::format("  {:07}|    zero-pad\n", 42);
    std::cout << std::format("  {:^07}|   zero-pad+center=ignored\n", 42);

    // 0作为填充字符
    std::cout << std::format("  {:0^7}|   char 0 center fill\n", 42);

    // 精度 - 浮点
    std::cout << std::format("  {}|         default\n", 0.12345678);
    std::cout << std::format("  {:.5}|      prec 5\n", 0.12345678);
    std::cout << std::format("  {:10.5}|    width10+prec5\n", 0.12345678);
    std::cout << std::format("  {:^10.5}|   center+w10+p5\n", 0.12345678);

    // 精度 - 字符串(截断)
    std::cout << std::format("  {:.7}|      str prec 7\n", "counterproductive");
    std::cout << std::format("  {:20.7}|    str w20+p7\n", "counterproductive");
    std::cout << std::format("  {:^20.7}|   str center+w20+p7\n", "counterproductive");

    // 动态宽度和精度(从参数获取)
    int width = 10;
    int precision = 2;
    std::cout << "  dynamic width+prec:\n";
    for (double val : {1.0, 12.345678, -777.7}) {
        std::cout << std::format("    [{:+{}.{}f}]\n", val, width, precision);
    }

    std::cout << "\n";
}

// 10.3.4 整数类型说明符
void demo_integer_format() {
    std::cout << "--- 10.3.4 integer format specifiers ---\n";

    int val = 42;
    char ch = '@';
    bool b = true;

    std::cout << std::format("  default:  {} {} {}\n", val, ch, b);
    std::cout << std::format("  d(dec):   {:d} {:d} {:d}\n", val, (int)ch, (int)b);

    std::cout << std::format("  b(bin):   {:b}\n", val);
    std::cout << std::format("  #b(pre):  {:#b}\n", val);
    std::cout << std::format("  #B(upp):  {:#B}\n", val);

    std::cout << std::format("  o(oct):   {:o}\n", val);
    std::cout << std::format("  #o(pre):  {:#o}\n", val);

    std::cout << std::format("  x(hex):   {:x}\n", val);
    std::cout << std::format("  X(upp):   {:X}\n", val);
    std::cout << std::format("  #x(pre):  {:#x}\n", val);
    std::cout << std::format("  #X(upp):  {:#X}\n", val);

    std::cout << std::format("  c(char):  {:c} {:c}\n", val, ch);
    std::cout << std::format("  s(str):   {:s}\n", b);

    std::cout << std::format("  no sign:  {:5d}\n", 42);
    std::cout << std::format("  + sign:   {:+5d}\n", 42);
    std::cout << std::format("  space:    {: 5d}\n", 42);

    std::cout << std::format("  '?': {0:02X} {0:+4d} {0:03o}\n", (int)'?');
    std::cout << std::format("  'y': {0:02X} {0:+4d} {0:03o}\n", (int)'y');

    std::cout << "\n";
}

// 10.3.4 浮点类型说明符
void demo_float_format() {
    std::cout << "--- 10.3.4 float format specifiers ---\n";

    double v1 = -1.0;
    double v2 = 0.0009765625;
    double v3 = 1785856.0;

    std::cout << std::format("  default: {} {} {}\n", v1, v2, v3);

    std::cout << std::format("  f(fixed): {:f}\n", v1);
    std::cout << std::format("  F(upper): {:F}\n", v1);

    std::cout << std::format("  e(exp):   {:e}\n", v3);
    std::cout << std::format("  E(upper): {:E}\n", v3);

    std::cout << std::format("  g(gen):   {:g}\n", v1);
    std::cout << std::format("  g(gen):   {:g}\n", v2);
    std::cout << std::format("  g(gen):   {:g}\n", v3);

    std::cout << std::format("  a(hex):   {:a}\n", v1);
    std::cout << std::format("  A(upper): {:A}\n", v1);

    std::cout << std::format("  no #:  {}\n", -1.0);
    std::cout << std::format("  with #: {:#}\n", -1.0);
    std::cout << std::format("  #g:    {:#g}\n", -1.0);

    std::cout << std::format("  f default:  {:f}\n", 3.14159265358979);
    std::cout << std::format("  f prec2:    {:.2f}\n", 3.14159265358979);
    std::cout << std::format("  e prec2:    {:.2e}\n", 3.14159265358979);
    std::cout << std::format("  g prec3:    {:.3g}\n", 3.14159265358979);

    std::cout << std::format("  inf:  {}\n", std::numeric_limits<double>::infinity());
    std::cout << std::format("  nan:  {}\n", std::numeric_limits<double>::quiet_NaN());

    std::cout << "\n";
}

// 10.3.4 字符串和指针说明符
void demo_string_pointer_format() {
    std::cout << "--- 10.3.4 string and pointer specifiers ---\n";

    std::cout << std::format("  default:   {}\n", "counter");
    std::cout << std::format("  s(explicit): {:s}\n", "counter");
    std::cout << std::format("  prec5:     {:.5}\n", "counter");
    std::cout << std::format("  prec5short: {:.5}\n", "hi");
    std::cout << std::format("  w+prec:    {:10.5}\n", "counter");

    int x = 42;
    std::cout << std::format("  ptr:     {}\n", static_cast<void*>(&x));
    std::cout << std::format("  :p:      {:p}\n", static_cast<void*>(&x));
    std::cout << std::format("  nullptr: {}\n", nullptr);
    std::cout << std::format("  nullptr: {:p}\n", nullptr);

    std::cout << "\n";
}

// --- 10.4 国际化 ---
void demo_locale() {
    std::cout << "--- 10.4 locale ---\n";

#ifdef _MSC_VER
    std::locale locG{"deu_deu.1252"};
#else
    std::locale locG{"de_DE"};
#endif

    std::cout << std::format("  no locale:  {}\n", 1000.7);
    std::cout << std::format(locG, "  with locale: {:L}\n", 1000.7);

    std::cout << std::format("  bool default: {}\n", true);
    std::cout << std::format(locG, "  bool locale:  {:L}\n", true);

    std::cout << "\n";
}

// --- 10.5 错误处理 ---
void demo_error_handling() {
    std::cout << "--- 10.5 error handling ---\n";

    std::cout << std::format("  {:d}\n", 42);
    // std::cout << std::format("{:s}\n", 42);  // 编译错误

    try {
        const char* badFmt = "{:s}";
        int val1 = 42;
        auto result = std::vformat(badFmt, std::make_format_args(val1));
        std::cout << result << "\n";
    } catch (const std::format_error& e) {
        std::cout << std::format("  format_error: {}\n", e.what());
    }

    try {
        const char* fmt = "{} {} {}";
        int a = 1, b = 2;
        auto result = std::vformat(fmt, std::make_format_args(a, b));
        std::cout << result << "\n";
    } catch (const std::format_error& e) {
        std::cout << std::format("  missing arg: {}\n", e.what());
    }

    std::cout << "\n";
}

// --- 10.6 用户自定义格式化输出 ---

class Always40 {
public:
    int getValue() const { return 40; }
};

template<>
struct std::formatter<Always40> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(const Always40& obj, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", obj.getValue());
    }
};

class Always41 {
public:
    int getValue() const { return 41; }
};

template<>
class std::formatter<Always41> {
    int width = 0;
public:
    constexpr auto parse(std::format_parse_context& ctx) {
        auto pos = ctx.begin();
        while (pos != ctx.end() && *pos != '}') {
            if (*pos < '0' || *pos > '9') {
                throw std::format_error{"invalid format for Always41"};
            }
            width = width * 10 + *pos - '0';
            ++pos;
        }
        return pos;
    }
    auto format(const Always41& obj, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{:{}}", obj.getValue(), width);
    }
};

class Always42 {
public:
    int getValue() const { return 42; }
};

template<>
struct std::formatter<Always42> {
    int width_ = 0;
    char align_ = '\0';
    char fill_ = ' ';
    char type_ = 'd';
    bool alt_ = false;

    constexpr auto parse(std::format_parse_context& ctx) {
        auto pos = ctx.begin();
        // fill and align
        if (pos != ctx.end() && pos + 1 != ctx.end()) {
            char next = *(pos + 1);
            if (next == '<' || next == '>' || next == '^') {
                fill_ = *pos;
                align_ = next;
                pos += 2;
            }
        }
        if (pos != ctx.end() && (*pos == '<' || *pos == '>' || *pos == '^')) {
            align_ = *pos;
            ++pos;
        }
        // #
        if (pos != ctx.end() && *pos == '#') { alt_ = true; ++pos; }
        // width
        while (pos != ctx.end() && *pos >= '0' && *pos <= '9') {
            width_ = width_ * 10 + (*pos - '0');
            ++pos;
        }
        // type
        if (pos != ctx.end() && *pos != '}') { type_ = *pos; ++pos; }
        return pos;
    }
    auto format(const Always42& obj, std::format_context& ctx) const {
        int val = obj.getValue();
        // build format string manually to avoid MSVC C7595
        std::string spec;
        spec += "{:";
        if (fill_ != ' ' || align_) {
            if (fill_ != ' ') spec += fill_;
            if (align_) spec += align_;
        }
        if (alt_) spec += '#';
        if (width_ > 0) spec += std::to_string(width_);
        spec += type_;
        spec += '}';
        return std::vformat_to(ctx.out(), spec, std::make_format_args(val));
    }
};

enum class Color { red, green, blue };

template<>
struct std::formatter<Color> : public std::formatter<std::string> {
    auto format(Color c, std::format_context& ctx) const {
        std::string value;
        switch (c) {
            using enum Color;
            case red:   value = "red"; break;
            case green: value = "green"; break;
            case blue:  value = "blue"; break;
            default:    value = std::format("Color{}", static_cast<int>(c)); break;
        }
        return std::formatter<std::string>::format(value, ctx);
    }
};

void demo_custom_formatters() {
    std::cout << "--- 10.6 custom formatters ---\n";

    Always40 a40;
    std::cout << std::format("  Always40: {}\n", a40);
    std::cout << std::format("  repeat: {0} {0}\n", a40);

    Always41 a41;
    std::cout << std::format("  Always41 default: {}\n", a41);
    std::cout << std::format("  Always41 width7: '{:7}'\n", a41);
    try {
        // MSVC C7595: parse()中throw导致编译时检查失败, 用vformat规避
        std::vformat("{:f}", std::make_format_args(a41));
    } catch (const std::format_error& e) {
        std::cout << std::format("  format error: {}\n", e.what());
    }

    Always42 a42;
    std::cout << std::format("  Always42 default: {}\n", a42);
    std::cout << std::format("  width7: '{:7}'\n", a42);
    std::cout << std::format("  fill+center: '{: ^7}'\n", a42);
    std::cout << std::format("  hex: '{:x}'\n", a42);
    std::cout << std::format("  bin: '{:#b}'\n", a42);

    std::cout << "  Color enum:\n";
    for (Color c : {Color::red, Color::green, Color::blue, Color{13}}) {
        std::cout << std::format("    {:_>8} = {:02}\n", c, static_cast<int>(c));
    }

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第10章: 格式化输出 ===\n\n";

    demo_format_basic();
    demo_format_to_n();
    demo_format_to();
    demo_formatted_size();
    demo_vformat();
    demo_format_spec_overview();
    demo_width_precision_fill();
    demo_integer_format();
    demo_float_format();
    demo_string_pointer_format();
    demo_locale();
    demo_error_handling();
    demo_custom_formatters();

    return 0;
}
