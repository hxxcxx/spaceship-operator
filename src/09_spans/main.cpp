#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <array>
#include <span>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <iterator>
#include <functional>
#include <compare>
#include <format>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <concepts>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第9章: 跨度 (Spans)
// 参考: C++20 Complete Guide Chapter 9
// 注: ch08已覆盖span基本视图属性/subspan/first/last
//     本章覆盖: 固定/动态范围、引用语义、常量正确性、
//               构造函数细节、子跨度模板参数版本、as_bytes
// ============================================================

void print(auto&& rg, const char* delim = " ", const char* end = "\n") {
    for (const auto& elem : rg) {
        std::cout << elem << delim;
    }
    std::cout << end;
}

// 泛型打印span函数(适用于固定和动态范围)
template<typename T, std::size_t Sz>
void printSpan(std::span<T, Sz> sp) {
    if constexpr (Sz == std::dynamic_extent) {
        std::cout << "  [dynamic " << sp.size() << "]: ";
    } else {
        std::cout << "  [fixed " << Sz << "]:       ";
    }
    for (const auto& elem : sp) {
        std::cout << elem << " ";
    }
    std::cout << "\n";
}

// --- 9.1 使用跨度 ---

// 9.1.1 固定和动态范围
void demo_fixed_vs_dynamic() {
    std::cout << "--- 9.1.1 固定和动态范围 ---\n";

    int a5[5] = {1, 2, 3, 4, 5};
    std::array arr5{1, 2, 3, 4, 5};
    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8};

    // 固定范围: 从数组推导出固定大小
    std::span sp1 = a5;                   // span<int, 5>
    std::span sp2{arr5};                  // span<int, 5>
    std::cout << "  sp1 fixed: "; printSpan(sp1);
    std::cout << "  sp2 fixed: "; printSpan(sp2);

    // 显式指定固定范围
    std::span<int, 5> sp3{arr5};          // 固定5个元素
    std::span<int, 3> sp4{vec.data(), 3}; // 固定3个元素(从vector)
    std::cout << "  sp3 fixed: "; printSpan(sp3);
    std::cout << "  sp4 fixed: "; printSpan(sp4);

    // 动态范围: 从vector推导出动态大小
    std::span<int> sp5;                   // 动态范围(初始0个元素)
    std::span sp6{vec};                   // span<int> 动态8个元素
    std::span<int> sp7{arr5};             // 动态范围(显式指定元素类型)
    std::cout << "  sp6 dynamic: "; printSpan(sp6);
    std::cout << "  sp7 dynamic: "; printSpan(sp7);

    // 固定 vs 动态: 类型不同
    std::cout << std::format("  span<int,5> == span<int>: {}\n",
        std::same_as<std::span<int, 5>, std::span<int>>);

    // 默认构造(只有动态范围或大小为0可以)
    std::span<int> dynEmpty;              // OK: 动态范围
    std::span<int, 0> fixEmpty;           // OK: 大小为0
    // std::span<int, 5> fixNo;           // 错误: 固定范围不能默认构造
    std::cout << std::format("  dynEmpty size: {}, fixEmpty size: {}\n",
        dynEmpty.size(), fixEmpty.size());

    std::cout << "\n";
}

// 9.1.2 使用动态范围span的示例
void demo_dynamic_span() {
    std::cout << "--- 9.1.2 动态范围span示例 ---\n";

    std::vector<std::string> vec{"New York", "Tokyo", "Rio", "Berlin", "Sydney"};

    // 指向前3个元素的视图
    std::span<const std::string> sp{vec.data(), 3};
    std::cout << "  前3个: ";
    for (const auto& e : sp) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 排序vector会影响span引用的元素
    std::ranges::sort(vec);
    std::cout << "  sort后: ";
    for (const auto& e : sp) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // push_back可能导致vector重新分配 -> span失效!
    auto oldCapa = vec.capacity();
    vec.push_back("Cairo");
    if (oldCapa != vec.capacity()) {
        // 必须重新初始化span
        sp = std::span{vec.data(), 3};
        std::cout << "  [重新分配, 已更新span]\n";
    }

    // 将整个vector赋给span
    sp = vec;
    std::cout << "  全部:   ";
    for (const auto& e : sp) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 引用最后几个元素
    sp = std::span{vec.end() - 3, vec.end()};
    std::cout << "  最后3个: ";
    for (const auto& e : sp) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 使用last()
    sp = std::span{vec}.last(4);
    std::cout << "  last(4): ";
    for (const auto& e : sp) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 赋值不同容器
    std::array<std::string, 3> arr{"Tic", "Tac", "Toe"};
    sp = arr;  // OK: array也可以赋给动态span
    std::cout << "  赋值array: ";
    for (const auto& e : sp) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    std::cout << "\n";
}

// 9.1.3 非const元素span
void demo_mutable_span() {
    std::cout << "--- 9.1.3 非const元素span ---\n";

    std::vector<std::string> vec{"New York", "Tokyo", "Rio", "Berlin", "Sydney"};
    std::cout << "  初始: ";
    print(vec);

    // 对中间3个元素排序(通过span修改原容器)
    std::ranges::sort(std::span{vec}.subspan(1, 3));
    std::cout << "  subspan(1,3)排序: ";
    print(vec);

    // 打印最后3个
    std::cout << "  last(3): ";
    print(std::span{vec}.last(3));

    // span可以直接修改元素
    std::span<std::string> sp{vec};
    sp[0] = "CHANGED";
    std::cout << "  修改sp[0]: ";
    print(vec);

    // views::counted 对连续内存返回span
    auto countedSpan = std::views::counted(vec.begin() + 1, 3);
    std::cout << "  counted(1,3): ";
    print(countedSpan);
    std::cout << "    类型: " << typeid(countedSpan).name() << "\n";

    std::cout << "\n";
}

// 9.1.4 固定范围span
void demo_fixed_span() {
    std::cout << "--- 9.1.4 固定范围span ---\n";

    std::vector<std::string> vec{"New York", "Tokyo", "Rio", "Berlin", "Sydney"};

    // 固定范围: 大小是类型的一部分
    std::span<const std::string, 3> sp3{vec.data(), 3};
    std::cout << "  fixed<3>: ";
    for (const auto& e : sp3) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 排序后引用也改变
    std::ranges::sort(vec);
    std::cout << "  sort后:   ";
    for (const auto& e : sp3) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 重新分配检查
    auto oldCapa = vec.capacity();
    vec.push_back("Cairo");
    if (oldCapa != vec.capacity()) {
        sp3 = std::span<const std::string, 3>{vec.data(), 3};
    }

    // 赋值: 必须是相同大小的span
    sp3 = std::span<const std::string, 3>{vec.end() - 3, vec.end()};
    std::cout << "  last<3>:  ";
    for (const auto& e : sp3) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 使用 last<3>() 模板参数版本
    sp3 = std::span{vec}.last<3>();  // OK: 返回固定范围span
    std::cout << "  last<3>(): ";
    for (const auto& e : sp3) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 赋值array(必须大小匹配)
    std::array<std::string, 3> arr{"Tic", "Tac", "Toe"};
    sp3 = arr;  // OK: array大小也是3
    std::cout << "  array:    ";
    for (const auto& e : sp3) std::cout << "\"" << e << "\" ";
    std::cout << "\n";

    // 动态last(3)不能赋给固定范围 -> 编译错误
    // sp3 = std::span{vec}.last(3);  // 错误: 动态范围

    std::cout << "\n";
}

// 9.1.5 固定 vs 动态范围对比
void demo_fixed_vs_dynamic_comparison() {
    std::cout << "--- 9.1.5 固定 vs 动态范围对比 ---\n";

    std::vector vec{1, 2, 3};
    std::array arr{1, 2, 3, 4, 5, 6};

    // 固定范围: 编译时检查大小
    std::span<int, 3> sp3{vec};
    // sp3 = arr;  // 编译错误: array有6个元素, != 3

    // 动态范围: 运行时灵活
    std::span<int> spDyn;
    spDyn = vec;   // OK: 3个元素
    std::cout << std::format("  动态span赋vec: size={}\n", spDyn.size());
    spDyn = arr;   // OK: 6个元素
    std::cout << std::format("  动态span赋arr: size={}\n", spDyn.size());

    // 内存大小: 固定范围不需要存储大小
    std::cout << std::format("  sizeof(span<int,3>):  {} bytes\n", sizeof(std::span<int, 3>));
    std::cout << std::format("  sizeof(span<int>):    {} bytes\n", sizeof(std::span<int>));
    std::cout << "  (固定范围不需要存储size成员)\n";

    std::cout << "\n";
}

// --- 9.2 跨度的潜在问题 ---
void demo_span_pitfalls() {
    std::cout << "--- 9.2 跨度的潜在问题 ---\n";

    // 问题1: 引用临时对象(悬空引用)
    auto getData = []() -> std::vector<int> { return {1, 2, 3, 4, 5}; };
    // 以下都是危险的:
    // std::span<int, 3> first3{getData()};   // UB: 引用临时对象
    // std::span sp{getData().begin(), 3};     // UB: 引用临时对象
    // for (auto s : std::span{getData()}.last(3)) ... // UB: 临时对象在循环前销毁
    std::cout << "  [警告] 引用临时对象的span是未定义行为\n";

    // 安全方式: 先存储到变量
    auto data = getData();
    std::span<int> safeSp{data};
    std::cout << "  安全方式: "; print(safeSp);

    // 或使用带初始化的for循环(C++20)
    std::cout << "  for循环带初始化: ";
    for (auto&& d = getData(); auto elem : std::span{d}.first(3)) {
        std::cout << elem << " ";
    }
    std::cout << "\n";

    // 问题2: vector重新分配使span失效
    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8};
    std::span sp{vec};
    std::cout << "  初始sp[0]: " << sp[0] << "\n";
    // vec.push_back(9);  // 可能重新分配 -> sp失效!
    // std::cout << sp[0]; // 可能UB!
    std::cout << "  [警告] vector重新分配后span失效\n";

    // 问题3: list等非连续容器不能用span
    std::list<int> lst{1, 2, 3};
    // std::span<int> bad{lst.begin(), lst.end()};  // 编译错误: 非连续迭代器
    std::cout << "  [注意] list等非连续容器不能创建span\n";

    std::cout << "\n";
}

// --- 9.3 跨度的设计要点 ---

// 9.3.1 生命周期依赖
void demo_lifetime() {
    std::cout << "--- 9.3.1 生命周期依赖 ---\n";

    // span 是借用范围: 迭代器不依赖于span的生命周期
    std::vector<int> vec{25, 42, 2, 0, 122, 5, 7};
    auto sp3 = std::span{vec.data(), 3};
    auto pos1 = std::ranges::find(sp3, 42);
    if (pos1 != sp3.end()) {
        std::cout << std::format("  find临时span: {} (借用范围安全)\n", *pos1);
    }

    // span的迭代器就是原始指针
    std::array arr{1, 2, 3, 4, 5};
    std::span sp{arr};
    auto it = sp.begin();
    std::cout << std::format("  span迭代器类型: {}\n", typeid(it).name());
    std::cout << "  span迭代器就是原始指针(零开销)\n";

    // span作为view
    std::cout << std::format("  span是view: {}\n", std::ranges::view<std::span<int>>);
    std::cout << std::format("  span是borrowed_range: {}\n",
        std::ranges::borrowed_range<std::span<int>>);
    std::cout << std::format("  span是contiguous_range: {}\n",
        std::ranges::contiguous_range<std::span<int>>);

    std::cout << "\n";
}

// 9.3.2 类型擦除
void demo_type_erasure() {
    std::cout << "--- 9.3.2 类型擦除 ---\n";

    std::array arr{1, 2, 3, 4, 5};
    std::vector vec{1, 2, 3, 4, 5};

    // 动态span类型相同(擦除了来源信息)
    std::span<int> arrSpanDyn{arr};
    std::span<int> vecSpanDyn{vec};
    std::cout << std::format("  array动态span == vector动态span: {}\n",
        std::same_as<decltype(arrSpanDyn), decltype(vecSpanDyn)>);

    // 但CTAD推导不同(固定 vs 动态)
    std::span arrSpan{arr};       // span<int, 5> 固定
    std::span vecSpan{vec};       // span<int> 动态
    std::cout << std::format("  CTAD array: span<int,5>, CTAD vector: span<int>\n");
    std::cout << std::format("  固定!=动态: {}\n",
        !std::same_as<decltype(arrSpan), decltype(vecSpan)>);

    // span vs subrange
    std::cout << "  span vs subrange:\n";
    std::cout << "    span: 连续内存, 零开销, 迭代器是原始指针\n";
    std::cout << "    subrange: 任意迭代器, 可能有额外开销\n";

    // 性能: span只是指针+大小
    std::cout << std::format("  sizeof(span<int>): {} bytes (指针+大小)\n",
        sizeof(std::span<int>));
    std::cout << std::format("  sizeof(span<int,5>): {} bytes (仅指针)\n",
        sizeof(std::span<int, 5>));

    std::cout << "\n";
}

// 9.3.3 常量正确性
void demo_const_correctness() {
    std::cout << "--- 9.3.3 常量正确性 ---\n";

    std::array a1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // const span<int> vs span<const int>
    const std::span<int> sp1{a1};      // span本身const, 但元素可修改!
    std::span<const int> sp2{a1};      // 元素是const的

    sp1[0] = 42;     // OK: span const, 但元素不是
    // sp2[0] = 42;  // 错误: 元素是const

    // sp1 = std::span<int>{};  // 错误: span本身是const
    sp2 = std::span<const int>{};  // OK: span本身不是const

    std::cout << "  const span<int>: span不可重新赋值, 但元素可修改\n";
    std::cout << "  span<const int>: 元素不可修改, span可重新赋值\n";
    std::cout << std::format("  sp1[0]={}\n", sp1[0]);

    // const span的陷阱: cbegin/cend仍可能修改元素
    // 在C++20中, span不提供cbegin/cend成员
    std::cout << "  [注意] C++20 span不提供cbegin/cend成员(常量正确性问题)\n";

    // 泛型函数中的陷阱
    auto modifyElemsOfConstColl = [](const auto& coll) {
        // 如果T是span<int>, coll[0] = {} 仍然可以编译!
        // 如果T是vector<int>, coll[0] = {} 编译失败
        using ElemRef = decltype(*coll.begin());
        if constexpr (!std::is_const_v<std::remove_reference_t<ElemRef>>) {
            std::cout << "    元素非const -> 可修改(即使集合是const)\n";
        } else {
            std::cout << "    元素是const\n";
        }
    };

    std::cout << "  对vector:\n";
    modifyElemsOfConstColl(a1);
    std::cout << "  对span:\n";
    modifyElemsOfConstColl(std::span{a1});

    // 安全方式: 检查解引用后是否是const
    std::cout << "  安全检查: 要求 begin() 返回指向const元素的迭代器\n";

    std::cout << "\n";
}

// 9.3.4 泛型代码中使用span作为参数
void demo_span_generic() {
    std::cout << "--- 9.3.4 泛型代码中使用span ---\n";

    std::vector vec{1, 2, 3, 4, 5};
    std::array arr{10, 20, 30, 40, 50};

    // 泛型函数接受所有span
    // printSpan已在上面定义, 接受 span<T, Sz>

    // 对固定和动态范围都有效
    std::span spVec{vec};    // 动态
    std::span spArr{arr};    // 固定<5>

    std::cout << "  vector span:\n";
    printSpan(spVec);
    std::cout << "  array span:\n";
    printSpan(spArr);

    // 注意: 不能直接传vector给printSpan
    // printSpan(vec);              // 错误: 模板推导失败
    printSpan(std::span{vec});      // OK: 显式转span

    // 传递const元素版本
    std::span<const int> constSp{vec};
    // printSpan(constSp) 能工作, 因为 T=const int

    // 混合处理: 用 concept 分派
    auto process = []<typename T>(const T& coll) {
        if constexpr (std::ranges::contiguous_range<T>) {
            std::cout << "    连续内存: 用span处理\n";
            std::span sp{coll};
            std::cout << "    size=" << sp.size() << "\n";
        } else {
            std::cout << "    非连续: 通用处理\n";
            std::cout << "    distance=" << std::ranges::distance(coll) << "\n";
        }
    };
    std::cout << "  vector: \n";
    process(vec);
    std::list<int> lst{1, 2, 3};
    std::cout << "  list: \n";
    process(lst);

    std::cout << "\n";
}

// --- 9.4 跨度操作 ---

// 9.4.1 构造函数细节
void demo_constructors() {
    std::cout << "--- 9.4.1 构造函数 ---\n";

    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int raw[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::array arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    // CTAD推导规则
    std::span sp1a{raw};                       // span<int, 10> 固定
    std::span sp1b{arr};                       // span<int, 15> 固定
    std::span<int> sp1c{arr};                  // span<int> 动态(显式类型)
    std::span sp1d{arr.begin() + 5, 5};        // span<int> 动态
    auto sp1e = std::views::counted(arr.data() + 5, 5);  // span<int> 动态
    std::cout << "  CTAD raw array -> fixed\n";
    std::cout << "  CTAD std::array -> fixed\n";
    std::cout << "  CTAD vector -> dynamic\n";

    // 迭代器对构造
    std::span<int> sp2a{vec.begin(), vec.end()};     // 两个迭代器
    std::span<int> sp2b{vec.data(), vec.size()};     // 指针+大小
    std::span<int> sp2c{vec.data(), 5};              // 前5个元素
    std::cout << "  迭代器对: "; print(sp2a);
    std::cout << "  指针+5:   "; print(sp2c);

    // 隐式转换: 只能加const, 不能改变元素类型
    std::span<const int> ok1{vec};            // OK: 加const
    // std::span<long> bad1{vec};            // 错误: int != long
    std::cout << "  span<const int> OK\n";
    std::cout << "  span<long> -> 编译错误(类型不匹配)\n";

    // 固定大小构造: 条件explicit
    std::span<int> dynSp{vec.begin(), 5};           // OK: 隐式
    std::span<int> dynSp2 = {vec.begin(), 5};       // OK: 拷贝初始化
    std::span<int, 5> fixSp{vec.begin(), 5};        // OK: 直接初始化
    // std::span<int, 5> fixSp2 = {vec.begin(), 5}; // 错误: explicit
    std::cout << "  动态: 拷贝初始化OK, 固定: 拷贝初始化错误(explicit)\n";

    // 固定大小赋值限制
    std::span<int, 5> fixSpA{vec.data(), 5};
    std::span<int> dynSpA{vec};
    // fixSpA = dynSpA;  // 错误: 动态不能赋给固定
    dynSpA = fixSpA;     // OK: 固定可以赋给动态
    std::cout << "  固定->动态赋值OK, 动态->固定赋值错误\n";

    std::cout << "\n";
}

// 9.4.2 子跨度操作(模板参数版本)
void demo_subspan_operations() {
    std::cout << "--- 9.4.2 子跨度操作 ---\n";

    std::vector vec{1.1, 2.2, 3.3, 4.4, 5.5};
    std::span spDyn{vec};

    // first(n) vs first<n>()
    auto sp1 = spDyn.first(2);     // 动态大小
    auto sp2 = spDyn.first<2>();   // 固定大小
    std::cout << std::format("  first(2) size类型: dynamic={}\n",
        sp1.size() == 2);
    std::cout << std::format("  first<2>() 是固定: {}\n",
        std::same_as<decltype(sp2), std::span<double, 2>>);

    // last(n) vs last<n>()
    auto sp3 = spDyn.last(2);      // 动态大小
    auto sp4 = spDyn.last<2>();    // 固定大小
    std::cout << "  last(2):  "; print(sp3);
    std::cout << "  last<2>(): "; print(sp4);

    // subspan(offset, count)
    auto s1 = spDyn.subspan(2);                  // 从第3个到末尾, 动态
    auto s2 = spDyn.subspan(2, 2);               // 第3-4个, 动态
    std::cout << "  subspan(2):   "; print(s1);
    std::cout << "  subspan(2,2): "; print(s2);

    // subspan<Off, Count>() 模板参数版本
    auto s3 = spDyn.subspan<2>();                 // 动态(无Count)
    auto s4 = spDyn.subspan<2, 2>();              // 固定<2>(有Count)
    auto s5 = spDyn.subspan<2, std::dynamic_extent>();  // 动态
    std::cout << std::format("  subspan<2,2>(): fixed={}\n",
        std::same_as<decltype(s4), std::span<double, 2>>);

    // 对固定范围span的subspan
    std::array arr{1.1, 2.2, 3.3, 4.4, 5.5};
    std::span spFix{arr};  // span<double, 5>

    auto f1 = spFix.first(2);       // 动态!
    auto f2 = spFix.first<2>();     // 固定<2>
    auto f3 = spFix.subspan<2>();   // 固定<3>! (5-2=3)
    auto f4 = spFix.subspan<2, 2>();  // 固定<2>
    std::cout << std::format("  fixed.first(2):  dynamic={}\n",
        std::same_as<decltype(f1), std::span<double>>);
    std::cout << std::format("  fixed.first<2>(): fixed<2>={}\n",
        std::same_as<decltype(f2), std::span<double, 2>>);
    std::cout << std::format("  fixed.subspan<2>(): fixed<3>={}\n",
        std::same_as<decltype(f3), std::span<double, 3>>);

    std::cout << "\n";
}

// 9.4.3 其他成员操作
void demo_member_operations() {
    std::cout << "--- 9.4.3 其他成员操作 ---\n";

    std::vector<int> vec{10, 20, 30, 40, 50};
    std::span sp{vec};

    // 基本操作
    std::cout << std::format("  size(): {}\n", sp.size());
    std::cout << std::format("  size_bytes(): {} bytes\n", sp.size_bytes());
    std::cout << std::format("  empty(): {}\n", sp.empty());
    std::cout << std::format("  front(): {}\n", sp.front());
    std::cout << std::format("  back(): {}\n", sp.back());
    std::cout << std::format("  operator[](2): {}\n", sp[2]);

    // data(): 获取底层数据指针
    int* ptr = sp.data();
    std::cout << std::format("  data()[0]: {}\n", ptr[0]);

    // 迭代器
    std::cout << "  正向: ";
    for (auto it = sp.begin(); it != sp.end(); ++it) std::cout << *it << " ";
    std::cout << "\n  反向: ";
    for (auto it = sp.rbegin(); it != sp.rend(); ++it) std::cout << *it << " ";
    std::cout << "\n";

    // 成员类型
    std::cout << "  类型:\n";
    std::cout << std::format("    extent: {} (dynamic={})\n",
        decltype(sp)::extent,
        decltype(sp)::extent == std::dynamic_extent);
    std::cout << std::format("    element_type: {}\n",
        typeid(decltype(sp)::element_type).name());
    std::cout << std::format("    value_type: {}\n",
        typeid(decltype(sp)::value_type).name());

    // as_bytes / as_writable_bytes
    std::span<const std::byte> byteView = std::as_bytes(sp);
    std::cout << std::format("  as_bytes: {} bytes\n", byteView.size());
    std::span<std::byte> writableBytes = std::as_writable_bytes(sp);
    std::cout << std::format("  as_writable_bytes: {} bytes\n", writableBytes.size());

    // span 不支持的操作
    std::cout << "  [不支持] ==, !=, swap(), at(), cbegin/cend\n";

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第9章: 跨度 (Spans) ===\n\n";

    demo_fixed_vs_dynamic();
    demo_dynamic_span();
    demo_mutable_span();
    demo_fixed_span();
    demo_fixed_vs_dynamic_comparison();
    demo_span_pitfalls();
    demo_lifetime();
    demo_type_erasure();
    demo_const_correctness();
    demo_span_generic();
    demo_constructors();
    demo_subspan_operations();
    demo_member_operations();

    return 0;
}
