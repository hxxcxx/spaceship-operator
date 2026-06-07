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
#include <type_traits>
#include <concepts>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第7章：范围和视图的实用工具
// 参考: C++20 Complete Guide Chapter 7
// ============================================================

// 辅助打印函数
void print(auto&& rg, const char* delim = " ", const char* end = "\n") {
    for (const auto& elem : rg) {
        std::cout << elem << delim;
    }
    std::cout << end;
}

// --- 7.1 将范围用作视图的关键实用工具 ---

// 7.1.1 std::views::all()
void demo_views_all() {
    std::cout << "--- 7.1.1 std::views::all() ---\n";

    std::vector<int> coll{1, 2, 3, 4, 5};

    // all() 对左值容器返回 ref_view
    auto v1 = std::views::all(coll);
    std::cout << "  all(coll):         "; print(v1);
    std::cout << "    类型: " << typeid(v1).name() << "\n";

    // all() 对右值容器返回 owning_view
    auto getColl = []() -> std::vector<int> { return {10, 20, 30}; };
    auto v2 = std::views::all(getColl());
    std::cout << "  all(临时vector):   "; print(v2);
    std::cout << "    类型: " << typeid(v2).name() << "\n";

    // all() 对已经是视图的返回副本
    auto iotaView = std::views::iota(1, 6);
    auto v3 = std::views::all(iotaView);
    std::cout << "  all(iota_view):    "; print(v3);
    std::cout << "    类型: " << typeid(v3).name() << "\n";

    // all() 用于按值传递参数（避免拷贝容器）
    auto processRange = [](std::ranges::input_range auto rg) {
        std::cout << "    按值接收: ";
        for (const auto& elem : rg) std::cout << elem << " ";
        std::cout << "\n";
    };
    std::vector<std::string> names{"Alice", "Bob", "Charlie"};
    std::cout << "  按值传递容器拷贝开销大:\n";
    processRange(std::views::all(names));  // 轻量级传递

    // all_t<> 类型
    using T1 = std::views::all_t<decltype(coll)&>;       // ref_view
    using T2 = std::views::all_t<decltype(coll)>;        // owning_view (右值)
    std::cout << "  all_t<vector&>:    " << typeid(T1).name() << "\n";
    std::cout << "  all_t<vector>:     " << typeid(T2).name() << "\n";

    // viewable_range 检查
    std::cout << std::format("  viewable_range<vector<int>&>: {}\n",
        std::ranges::viewable_range<std::vector<int>&>);
    std::cout << std::format("  viewable_range<vector<int>>:   {}\n",
        std::ranges::viewable_range<std::vector<int>>);
    std::cout << std::format("  viewable_range<iota_view>:     {}\n",
        std::ranges::viewable_range<std::ranges::iota_view<int>>);
    std::cout << "\n";
}

// 7.1.2 std::views::counted()
void demo_views_counted() {
    std::cout << "--- 7.1.2 std::views::counted() ---\n";

    // vector 迭代器 -> span（连续内存）
    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto vecIt = std::ranges::find(vec, 5);
    auto v1 = std::views::counted(vecIt, 3);
    std::cout << "  counted(vec_iter,3): "; print(v1);
    std::cout << "    类型(span): " << typeid(v1).name() << "\n";

    // deque 迭代器 -> subrange（随机访问）
    std::deque<int> deq{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto deqIt = std::ranges::find(deq, 5);
    auto v2 = std::views::counted(deqIt, 3);
    std::cout << "  counted(deq_iter,3): "; print(v2);
    std::cout << "    类型(subrange): " << typeid(v2).name() << "\n";

    // list 迭代器 -> subrange<counted_iterator, default_sentinel>
    std::list<int> lst{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto lstIt = std::ranges::find(lst, 5);
    auto v3 = std::views::counted(lstIt, 3);
    std::cout << "  counted(lst_iter,3): "; print(v3);
    std::cout << "    类型(counted): " << typeid(v3).name() << "\n";

    // 原始数组指针 -> span
    int arr[] = {10, 20, 30, 40, 50};
    auto v4 = std::views::counted(arr, 3);
    std::cout << "  counted(arr,3):     "; print(v4);
    std::cout << "    类型(span): " << typeid(v4).name() << "\n";

    // counted vs take 对比
    std::cout << "  take(vec,3):        "; print(vec | std::views::take(3));
    std::cout << "  counted更灵活: 可从任意迭代器开始\n";

    // counted: count is stable
    std::list<int> lst21{1, 2, 3, 4, 5, 6, 7, 8};
    std::ptrdiff_t count5 = 5;
    auto c = std::views::counted(lst21.begin(), count5);
    std::cout << "  插入前: "; 
    print(c);
    lst21.insert(++lst21.begin(), 0);  // insert 0 as 2nd element
    std::cout << "  插入后: "; print(c);  // count still 5, content changed
    std::cout << "\n";
}

// 7.1.3 std::views::common()
void demo_views_common() {
    std::cout << "--- 7.1.3 std::views::common() ---\n";

    std::vector<int> vec{1, 2, 3, 4, 5};

    // 对已经是 common_range 的范围，返回 ref_view
    auto v1 = std::views::common(vec);
    std::cout << "  common(vector):     "; print(v1);
    std::cout << "    类型: " << typeid(v1).name() << "\n";

    // iota_view 本身就是 common_range（有界时）
    auto iv = std::views::iota(1, 6);
    auto v2 = std::views::common(iv);
    std::cout << "  common(iota):       "; print(v2);

    // take_view 对 list 可能不是 common_range
    std::list<int> lst{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto vt = std::views::take(lst, 5);
    std::cout << std::format("  take(list) common:  {}\n",
        std::ranges::common_range<decltype(vt)>);
    auto vc = std::views::common(vt);
    std::cout << std::format("  common后:           {}\n",
        std::ranges::common_range<decltype(vc)>);
    std::cout << "  common后的类型:     " << typeid(vc).name() << "\n";

    // 使用场景: 与传统算法配合
    auto legacyAlgo = [](auto beg, auto end) {
        std::cout << "    legacy: ";
        for (auto it = beg; it != end; ++it) std::cout << *it << " ";
        std::cout << "\n";
    };

    // 直接用 take_view 的 begin/end（类型不同）可能不行
    // 使用 common 包装后安全
    auto sub = std::views::take(lst, 4) | std::views::common;
    legacyAlgo(sub.begin(), sub.end());

    std::cout << "\n";
}

// --- 7.2 新的迭代器类别 ---
void demo_iterator_categories() {
    std::cout << "--- 7.2 新的迭代器类别 ---\n";

    // vector 迭代器: contiguous_iterator (C++20新增)
    std::vector<int> vec{1, 2, 3, 4};
    using VecIter = decltype(vec.begin());
    std::cout << std::format("  vector迭代器:\n");
    std::cout << std::format("    contiguous:  {}\n", std::contiguous_iterator<VecIter>);
    std::cout << std::format("    random_access: {}\n", std::random_access_iterator<VecIter>);
    std::cout << std::format("    bidirectional: {}\n", std::bidirectional_iterator<VecIter>);
    std::cout << std::format("    forward:     {}\n", std::forward_iterator<VecIter>);
    std::cout << std::format("    input:       {}\n", std::input_iterator<VecIter>);

    // array 迭代器: contiguous
    std::array<int, 4> arr{1, 2, 3, 4};
    using ArrIter = decltype(arr.begin());
    std::cout << std::format("  array迭代器 contiguous: {}\n", std::contiguous_iterator<ArrIter>);

    // deque 迭代器: random_access（不是 contiguous）
    std::deque<int> deq{1, 2, 3};
    using DeqIter = decltype(deq.begin());
    std::cout << std::format("  deque迭代器:\n");
    std::cout << std::format("    contiguous:    {}\n", std::contiguous_iterator<DeqIter>);
    std::cout << std::format("    random_access: {}\n", std::random_access_iterator<DeqIter>);

    // list 迭代器: bidirectional
    std::list<int> lst{1, 2, 3};
    using LstIter = decltype(lst.begin());
    std::cout << std::format("  list迭代器:\n");
    std::cout << std::format("    random_access: {}\n", std::random_access_iterator<LstIter>);
    std::cout << std::format("    bidirectional: {}\n", std::bidirectional_iterator<LstIter>);

    // set 迭代器: bidirectional
    std::set<int> s{1, 2, 3};
    using SetIter = decltype(s.begin());
    std::cout << std::format("  set迭代器 bidirectional: {}\n", std::bidirectional_iterator<SetIter>);

    // iota_view 迭代器: random_access（不连续）
    auto iota = std::views::iota(1, 10);
    using IotaIter = decltype(iota.begin());
    std::cout << std::format("  iota迭代器:\n");
    std::cout << std::format("    contiguous:    {}\n", std::contiguous_iterator<IotaIter>);
    std::cout << std::format("    random_access: {}\n", std::random_access_iterator<IotaIter>);

    // iterator_category vs iterator_concept
    // C++20新增 iterator_concept
    std::cout << "  iterator_concept (C++20新增):\n";
    // VecIter 有 iterator_concept = contiguous_iterator_tag
    // IotaIter 有 iterator_concept = random_access_iterator_tag
    // 但 iterator_category 可能不同
    std::cout << "    vector::iterator_concept 是 contiguous\n";
    std::cout << "    iota_view::iterator_concept 是 random_access\n";

    // 新的类型特性（优先使用）
    std::cout << "  新迭代器类型特性:\n";
    std::cout << std::format("    iter_value_t<vector::iter>:  {}\n", typeid(std::iter_value_t<VecIter>).name());
    std::cout << std::format("    iter_reference_t<vector::iter>: {}\n", typeid(std::iter_reference_t<VecIter>).name());
    std::cout << std::format("    iter_difference_t<vector::iter>: {}\n", typeid(std::iter_difference_t<VecIter>).name());
    std::cout << "\n";
}

// --- 7.3 新的迭代器和哨兵类型 ---

// 7.3.1 std::counted_iterator
void demo_counted_iterator() {
    std::cout << "--- 7.3.1 std::counted_iterator ---\n";

    std::vector<int> coll{1, 2, 3, 4, 5, 6, 7, 8, 9};

    // 方式1: 使用 count() 检查剩余
    std::cout << "  count()方式: ";
    for (std::counted_iterator pos{coll.begin(), 5}; pos.count() > 0; ++pos) {
        std::cout << *pos << " ";
    }
    std::cout << "\n";

    // 方式2: 使用 default_sentinel
    std::cout << "  default_sentinel方式: ";
    for (std::counted_iterator pos{coll.begin(), 5}; pos != std::default_sentinel; ++pos) {
        std::cout << *pos << " ";
    }
    std::cout << "\n";

    // counted_iterator 的API
    std::counted_iterator ci{coll.begin() + 2, 4};
    std::cout << std::format("  *ci={}, count={}, base指向: {}\n",
        *ci, ci.count(), *ci.base());

    // 距离计算
    std::counted_iterator ciEnd{coll.begin() + 2, 0};
    // auto dist = ci - std::default_sentinel;  // count差值
    std::cout << std::format("  count剩余: {}\n", ci.count());

    // counted_iterator 适用于非随机访问迭代器
    std::list<int> lst{10, 20, 30, 40, 50, 60, 70};
    std::counted_iterator li{lst.begin(), 4};
    std::cout << "  list counted: ";
    for (; li != std::default_sentinel; ++li) {
        std::cout << *li << " ";
    }
    std::cout << "\n";
    std::cout << "\n";
}

// 7.3.2 std::common_iterator
void demo_common_iterator() {
    std::cout << "--- 7.3.2 std::common_iterator ---\n";

    std::list<int> lst{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto vt = std::views::take(lst, 5);

    // take_view<list> 的 begin/end 类型可能不同
    // 使用 common_iterator 统一类型
    using BegT = decltype(vt.begin());
    using EndT = decltype(vt.end());

    // 如果类型不同，用 common_iterator 包装
    if constexpr (!std::same_as<BegT, EndT>) {
        std::cout << "  take(list) begin/end 类型不同\n";
        auto beg = std::common_iterator<BegT, EndT>{vt.begin()};
        auto end = std::common_iterator<BegT, EndT>{vt.end()};
        std::cout << "  common_iterator遍历: ";
        for (auto it = beg; it != end; ++it) {
            std::cout << *it << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "  take(list) begin/end 类型相同\n";
    }

    // 更简单的方式: 使用 views::common
    auto cv = vt | std::views::common;
    std::cout << "  views::common:       ";
    for (auto it = cv.begin(); it != cv.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // 模板辅助函数示例
    auto callLegacyAlgo = []<typename B, typename E>(B beg, E end) {
        if constexpr (std::same_as<B, E>) {
            std::cout << "    直接调用: ";
            for (auto it = beg; it != end; ++it) std::cout << *it << " ";
        } else {
            auto cbeg = std::common_iterator<B, E>{beg};
            auto cend = std::common_iterator<B, E>{end};
            std::cout << "    common_iterator包装: ";
            for (auto it = cbeg; it != cend; ++it) std::cout << *it << " ";
        }
        std::cout << "\n";
    };
    std::cout << "  通用算法包装:\n";
    callLegacyAlgo(vt.begin(), vt.end());
    std::cout << "\n";
}

// 7.3.3 std::default_sentinel
void demo_default_sentinel() {
    std::cout << "--- 7.3.3 std::default_sentinel ---\n";

    // default_sentinel 是一个空类型，用于与 counted_iterator 等配合
    std::vector<int> coll{1, 2, 3, 4, 5};

    // 与 counted_iterator 配合
    std::counted_iterator ci{coll.begin(), 3};
    std::cout << "  counted_iterator vs default_sentinel:\n";
    std::cout << "    ";
    while (ci != std::default_sentinel) {
        std::cout << *ci << " ";
        ++ci;
    }
    std::cout << "\n";

    // default_sentinel 的类型
    std::cout << std::format("  default_sentinel类型: {}\n",
        typeid(decltype(std::default_sentinel)).name());

    // std::default_sentinel_t 是空类
    std::cout << "  default_sentinel_t 是空类，无成员\n";
    std::cout << "\n";
}

// 7.3.4 std::unreachable_sentinel
void demo_unreachable_sentinel() {
    std::cout << "--- 7.3.4 std::unreachable_sentinel ---\n";

    std::vector<int> coll{5, 8, 42, 15, 7, 3, 9};

    // 如果确定42存在，可以用 unreachable_sentinel 优化
    // 编译器可以省略与 end 的比较
    auto pos42 = std::ranges::find(coll.begin(), std::unreachable_sentinel, 42);
    if (pos42 != std::unreachable_sentinel) {  // 永远为 true（除非到末尾）
        std::cout << std::format("  find(unreachable, 42): 找到 {}\n", *pos42);
    }

    // unreachable_sentinel 与任何迭代器比较总是 false
    // 意味着"永远不到末尾"
    std::cout << "  unreachable_sentinel 表示\"永远无法到达的末尾\"\n";
    std::cout << "  编译器可优化: 省略与 end 的比较检查\n";

    // iota 无限视图内部也使用类似机制
    auto infinite = std::views::iota(1);  // 无限 iota
    // 找前10个偶数
    std::cout << "  无限iota前5个偶数: ";
    int count = 0;
    for (int val : infinite | std::views::filter([](int x) { return x % 2 == 0; })) {
        std::cout << val << " ";
        if (++count >= 5) break;
    }
    std::cout << "\n";
    std::cout << "\n";
}

// 7.3.5 std::move_sentinel
void demo_move_sentinel() {
    std::cout << "--- 7.3.5 std::move_sentinel ---\n";

    std::list<std::string> coll{"tic", "tac", "toe"};
    std::vector<std::string> moved;

    // 使用 move_iterator + move_sentinel 移动元素
    std::cout << "  移动前 coll: ";
    print(coll);

    for (std::move_iterator pos{coll.begin()};
         pos != std::move_sentinel{coll.end()}; ++pos) {
        moved.push_back(*pos);  // *pos 触发移动
    }

    std::cout << "  移动后 moved: ";
    print(moved);
    std::cout << "  移动后 coll:  ";
    print(coll);  // 元素已被移走（空字符串或未定义）

    // 注意: move_iterator 和 move_sentinel 类型不同
    // 如果需要同类型，用 move_iterator 包装 end:
    std::list<std::string> coll2{"a", "b", "c"};
    std::vector<std::string> v2{
        std::move_iterator{coll2.begin()},
        std::move_iterator{coll2.end()}  // 同类型
    };
    std::cout << "  move_iterator对: ";
    print(v2);

    std::cout << "\n";
}

// --- 7.4 处理范围的新函数 ---

// 7.4.1 处理范围元素的函数
void demo_range_functions() {
    std::cout << "--- 7.4.1 处理范围元素的函数 ---\n";

    std::vector<int> vec{5, 3, 1, 4, 2};
    int arr[] = {10, 20, 30, 40, 50};

    // std::ranges 版本的函数
    std::cout << std::format("  empty(vec): {}\n", std::ranges::empty(vec));
    std::cout << std::format("  size(vec):  {}\n", std::ranges::size(vec));
    std::cout << std::format("  ssize(vec): {} (有符号)\n", std::ranges::ssize(vec));
    std::cout << std::format("  empty(arr): {}\n", std::ranges::empty(arr));
    std::cout << std::format("  size(arr):  {}\n", std::ranges::size(arr));

    // 迭代器函数
    auto it = std::ranges::begin(vec);
    std::cout << std::format("  *begin(vec): {}\n", *it);
    std::cout << std::format("  distance(vec): {}\n", std::ranges::distance(vec));

    // data(): 获取底层数据指针（连续内存容器）
    int* dataPtr = std::ranges::data(vec);
    std::cout << std::format("  data(vec)[0]: {}\n", dataPtr[0]);
    std::cout << std::format("  data(arr)[0]: {}\n", std::ranges::data(arr)[0]);

    // cdata: const 版本
    const auto& cvec = vec;
    std::cout << std::format("  cdata(cvec)[0]: {}\n", std::ranges::cdata(cvec)[0]);

    // ADL 问题的解决
    // std::ranges::begin() 不需要 using 声明
    std::cout << "  std::ranges::begin() 解决ADL问题\n";
    std::cout << "  优先使用 std::ranges 命名空间的函数\n";
    std::cout << "\n";
}

// 7.4.2 处理迭代器的函数
void demo_iterator_functions() {
    std::cout << "--- 7.4.2 处理迭代器的函数 ---\n";

    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto it = vec.begin();

    // next: 前进
    auto it3 = std::ranges::next(it, 3);  // 前进3步
    std::cout << std::format("  next(begin,3): {}\n", *it3);

    auto itEnd = std::ranges::next(it, vec.end());  // 到 end
    std::cout << std::format("  next(begin,end): 到末尾\n");

    // next 带边界
    auto itSafe = std::ranges::next(it, 100, vec.end());  // 不超过end
    std::cout << std::format("  next(begin,100,end): 到{}（安全边界）\n",
        itSafe == vec.end() ? "end" : std::to_string(*itSafe));

    // prev: 后退
    auto itPrev = std::ranges::prev(vec.end(), 2);
    std::cout << std::format("  prev(end,2): {}\n", *itPrev);

    // advance: 就地前进
    auto itAdv = vec.begin();
    std::ranges::advance(itAdv, 5);
    std::cout << std::format("  advance(begin,5): {}\n", *itAdv);

    // distance
    std::list<int> lst{1, 2, 3, 4, 5};
    std::cout << std::format("  distance(list): {}\n", std::ranges::distance(lst));
    std::cout << std::format("  distance(3,end): {}\n",
        std::ranges::distance(it3, vec.end()));

    // 对范围使用 distance（即使没有 size()）
    auto v = lst | std::views::filter([](int x) { return x % 2 == 1; });
    std::cout << std::format("  distance(filter视图): {}\n", std::ranges::distance(v));

    std::cout << "\n";
}

// 7.4.3 交换和移动元素/值的函数
void demo_swap_move() {
    std::cout << "--- 7.4.3 交换和移动元素/值的函数 ---\n";

    // std::ranges::swap
    int a = 10, b = 20;
    std::cout << std::format("  swap前: a={}, b={}\n", a, b);
    std::ranges::swap(a, b);
    std::cout << std::format("  swap后: a={}, b={}\n", a, b);

    // std::ranges::swap 解决泛型代码中的ADL问题
    std::vector<int> v1{1, 2, 3}, v2{4, 5, 6};
    std::ranges::swap(v1, v2);
    std::cout << "  swap(v1,v2): "; print(v1);

    // iter_swap: 交换迭代器指向的元素
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::ranges::iter_swap(vec.begin(), vec.begin() + 4);
    std::cout << "  iter_swap首尾: "; print(vec);

    // iter_move: 移动迭代器指向的元素
    std::vector<std::string> src{"hello", "world"};
    auto moved = std::ranges::iter_move(src.begin());
    std::cout << std::format("  iter_move: \"{}\"\n", moved);

    // iter_move/iter_swap 对代理迭代器友好
    std::cout << "  iter_move/iter_swap 对代理迭代器友好\n";
    std::cout << "  优先使用 ranges 版本而非 std::move(*it) + swap\n";
    std::cout << "\n";
}

// 7.4.4 值比较函数
void demo_comparison_functions() {
    std::cout << "--- 7.4.4 值比较函数 ---\n";

    // std::ranges 命名空间中的比较函数对象
    std::cout << std::format("  equal_to(1,1):     {}\n", std::ranges::equal_to{}(1, 1));
    std::cout << std::format("  not_equal_to(1,2): {}\n", std::ranges::not_equal_to{}(1, 2));
    std::cout << std::format("  less(1,2):         {}\n", std::ranges::less{}(1, 2));
    std::cout << std::format("  greater(2,1):      {}\n", std::ranges::greater{}(2, 1));
    std::cout << std::format("  less_equal(1,1):   {}\n", std::ranges::less_equal{}(1, 1));
    std::cout << std::format("  greater_equal(2,1): {}\n", std::ranges::greater_equal{}(2, 1));

    // 用作算法的排序标准
    std::vector<int> vec{5, 2, 8, 1, 9, 3};
    std::ranges::sort(vec, std::ranges::greater{});  // 降序
    std::cout << "  sort(greater): "; print(vec);

    // compare_three_way
    auto ordering = std::compare_three_way{}(3, 5);
    std::cout << std::format("  compare_three_way(3,5): {}\n",
        ordering == std::strong_ordering::less ? "less" :
        ordering == std::strong_ordering::greater ? "greater" : "equal");

    std::cout << "\n";
}

// --- 7.5 处理范围的新类型函数/实用工具 ---

// 7.5.1 范围的通用类型
void demo_range_type_traits() {
    std::cout << "--- 7.5.1 范围的通用类型 ---\n";

    using Vec = std::vector<int>;

    std::cout << std::format("  iterator_t<vector>:       {}\n",
        typeid(std::ranges::iterator_t<Vec>).name());
    std::cout << std::format("  sentinel_t<vector>:       {}\n",
        typeid(std::ranges::sentinel_t<Vec>).name());
    std::cout << std::format("  range_value_t<vector>:    {}\n",
        typeid(std::ranges::range_value_t<Vec>).name());
    std::cout << std::format("  range_reference_t<vector>: {}\n",
        typeid(std::ranges::range_reference_t<Vec>).name());
    std::cout << std::format("  range_difference_t<vector>: {}\n",
        typeid(std::ranges::range_difference_t<Vec>).name());
    std::cout << std::format("  range_size_t<vector>:     {}\n",
        typeid(std::ranges::range_size_t<Vec>).name());

    // 对数组也能用
    using Arr = int[5];
    std::cout << std::format("  range_value_t<int[5]>:    {}\n",
        typeid(std::ranges::range_value_t<Arr>).name());

    // 对视图也能用
    auto v = std::views::iota(1, 10);
    using View = decltype(v);
    std::cout << std::format("  range_value_t<iota>:      {}\n",
        typeid(std::ranges::range_value_t<View>).name());

    // borrowed_iterator_t: 借用范围返回迭代器，否则返回 dangling
    std::cout << std::format("  vector& 是借用范围: {}\n",
        std::ranges::borrowed_range<std::vector<int>&>);
    std::cout << std::format("  vector  是借用范围: {}\n",
        std::ranges::borrowed_range<std::vector<int>>);

    std::cout << "\n";
}

// 7.5.2 迭代器的通用类型
void demo_iterator_type_traits() {
    std::cout << "--- 7.5.2 迭代器的通用类型 ---\n";

    std::vector<int> vec{1, 2, 3};
    using Iter = decltype(vec.begin());

    // C++20 新的迭代器类型特性（优先使用）
    std::cout << std::format("  iter_value_t:         {}\n",
        typeid(std::iter_value_t<Iter>).name());
    std::cout << std::format("  iter_reference_t:     {}\n",
        typeid(std::iter_reference_t<Iter>).name());
    std::cout << std::format("  iter_rvalue_reference_t: {}\n",
        typeid(std::iter_rvalue_reference_t<Iter>).name());
    std::cout << std::format("  iter_difference_t:    {}\n",
        typeid(std::iter_difference_t<Iter>).name());

    // 对指针也能用
    using Ptr = int*;
    std::cout << std::format("  iter_value_t<int*>:   {}\n",
        typeid(std::iter_value_t<Ptr>).name());
    std::cout << std::format("  iter_reference_t<int*>: {}\n",
        typeid(std::iter_reference_t<Ptr>).name());

    // 对视图迭代器也能用
    auto v = std::views::iota(1, 10);
    using VIter = decltype(v.begin());
    std::cout << std::format("  iter_value_t<iota_iter>: {}\n",
        typeid(std::iter_value_t<VIter>).name());

    std::cout << "  优先使用 iter_value_t 而非 iterator_traits<>::value_type\n";
    std::cout << "\n";
}

// 7.5.3 新的函数类型
void demo_new_function_types() {
    std::cout << "--- 7.5.3 新的函数类型 ---\n";

    // std::identity: 返回自身
    std::identity id;
    std::cout << std::format("  identity(42): {}\n", id(42));
    std::cout << std::format("  identity(\"hi\"): {}\n", id(std::string("hi")));

    // identity 用作"无投影"
    std::vector<int> vec{5, 3, 1, 4, 2};
    auto pos = std::ranges::find(vec, 3, std::identity{});  // 等价于直接查找
    if (pos != vec.end()) {
        std::cout << std::format("  find(identity, 3): {}\n", *pos);
    }

    // identity 是很多算法的默认投影
    std::ranges::sort(vec, std::ranges::less{}, std::identity{});
    std::cout << "  sort(less, identity): "; print(vec);

    // compare_three_way
    auto result = std::compare_three_way{}(10, 20);
    if (result < 0) std::cout << "  compare_three_way(10,20): less\n";
    else if (result > 0) std::cout << "  compare_three_way(10,20): greater\n";
    else std::cout << "  compare_three_way(10,20): equal\n";

    std::cout << "\n";
}

// --- 7.6 范围算法 ---

// 7.6.1 范围算法的返回类型
void demo_range_algorithm_results() {
    std::cout << "--- 7.6 范围算法返回类型 ---\n";

    // in_out_result 示例 (transform)
    std::vector<int> inColl{1, 2, 3, 4, 5, 6, 7};
    std::vector<int> outColl(10, 0);

    auto result = std::ranges::transform(inColl, outColl.begin(),
        [](int val) { return val * val; });

    std::cout << "  transform结果:\n";
    std::cout << "    处理了输入: "; print(std::ranges::subrange{inColl.begin(), result.in});
    std::cout << "    写入到输出: "; print(std::ranges::subrange{outColl.begin(), result.out});
    std::cout << "    输出剩余:   "; print(std::ranges::subrange{result.out, outColl.end()});

    // in_fun_result 示例 (for_each)
    int sum = 0;
    auto [pos, func] = std::ranges::for_each(inColl, [&sum](int val) {
        sum += val;
    });
    std::cout << std::format("  for_each求和: {}\n", sum);

    // min_max_result 示例 (minmax_element)
    std::vector<int> vec{5, 2, 8, 1, 9, 3};
    auto [minIt, maxIt] = std::ranges::minmax_element(vec);
    std::cout << std::format("  minmax_element: min={}, max={}\n", *minIt, *maxIt);

    // in_found_result 示例
    // C++20 的 is_sorted_until
    std::vector<int> sorted{1, 2, 3, 5, 4, 7, 6};
    auto sortEnd = std::ranges::is_sorted_until(sorted);
    std::cout << "  is_sorted_until: ";
    for (auto it = sorted.begin(); it != sortEnd; ++it) std::cout << *it << " ";
    std::cout << "| (剩余未排序)\n";

    // mismatch 示例 (in_in_result)
    std::vector<int> v1{1, 2, 3, 4, 5};
    std::vector<int> v2{1, 2, 9, 4, 5};
    auto [it1, it2] = std::ranges::mismatch(v1, v2);
    if (it1 != v1.end()) {
        std::cout << std::format("  mismatch: v1[{}]={}, v2[{}]={}\n",
            std::ranges::distance(v1.begin(), it1), *it1,
            std::ranges::distance(v2.begin(), it2), *it2);
    }

    std::cout << "\n";
}

// 7.6.2 算法分类示例
void demo_algorithm_overview() {
    std::cout << "--- 7.6.2 算法分类示例 ---\n";

    std::vector<int> vec{5, 3, 1, 4, 2, 8, 7, 6, 9, 0};

    // 非修改算法
    std::cout << "  [非修改算法]\n";
    auto cnt = std::ranges::count(vec, 5);
    std::cout << std::format("    count(vec,5): {}\n", cnt);

    bool allPos = std::ranges::all_of(vec, [](int x) { return x >= 0; });
    std::cout << std::format("    all_of(>=0): {}\n", allPos);

    auto [minIt, maxIt] = std::ranges::minmax_element(vec);
    std::cout << std::format("    minmax: [{}, {}]\n", *minIt, *maxIt);

    // 修改算法
    std::cout << "  [修改算法]\n";
    std::vector<int> dest(10);
    auto copyResult = std::ranges::copy(vec, dest.begin());
    std::cout << "    copy: "; print(dest);

    std::ranges::replace(vec, 5, 50);
    std::cout << "    replace(5->50): "; print(vec);

    // 移除算法
    std::cout << "  [移除算法]\n";
    std::vector<int> remVec{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto [newBegin, newEnd] = std::ranges::remove(remVec, 5);
    // 注意: remove 不改变size，只是移动元素
    std::cout << "    remove(5)后: ";
    for (auto it = remVec.begin(); it != newEnd; ++it) std::cout << *it << " ";
    std::cout << "\n";

    // 变异算法
    std::cout << "  [变异算法]\n";
    std::vector<int> revVec{1, 2, 3, 4, 5};
    std::ranges::reverse(revVec);
    std::cout << "    reverse: "; print(revVec);

    // 排序算法
    std::cout << "  [排序算法]\n";
    std::vector<int> sortVec{5, 3, 1, 4, 2};
    std::ranges::sort(sortVec);
    std::cout << "    sort: "; print(sortVec);
    std::ranges::stable_sort(sortVec, std::ranges::greater{});
    std::cout << "    stable_sort(greater): "; print(sortVec);

    // 数值算法注意: 不支持范围传参
    std::cout << "  [数值算法]\n";
    std::cout << "    accumulate等不支持范围传参，需传迭代器\n";
    int total = std::accumulate(vec.begin(), vec.end(), 0);
    std::cout << std::format("    accumulate: {}\n", total);

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第7章：范围和视图的实用工具 ===\n\n";

    demo_views_all();
    demo_views_counted();
    demo_views_common();
    demo_iterator_categories();
    demo_counted_iterator();
    demo_common_iterator();
    demo_default_sentinel();
    demo_unreachable_sentinel();
    demo_move_sentinel();
    demo_range_functions();
    demo_iterator_functions();
    demo_swap_move();
    demo_comparison_functions();
    demo_range_type_traits();
    demo_iterator_type_traits();
    demo_new_function_types();
    demo_range_algorithm_results();
    demo_algorithm_overview();

    return 0;
}
