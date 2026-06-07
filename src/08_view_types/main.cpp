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
#include <sstream>
#include <string_view>
#include <type_traits>
#include <concepts>
#include <numbers>
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第8章：视图类型详解
// 参考: C++20 Complete Guide Chapter 8
// 注: 仅实现ch06/ch07中未覆盖的新内容
// ============================================================

void print(auto&& rg, const char* delim = " ", const char* end = "\n") {
    for (const auto& elem : rg) {
        std::cout << elem << delim;
    }
    std::cout << end;
}

// --- 8.2 view_interface 基类 API ---
// ch06/ch07 未覆盖: view_interface 提供的成员函数

void demo_view_interface() {
    std::cout << "--- 8.2 view_interface 基类 API ---\n";

    // view_interface 为所有标准视图提供基础成员函数
    // 根据迭代器类别，自动提供不同的成员:

    // empty() - 至少前向迭代器
    auto v1 = std::views::iota(1, 6);
    std::cout << std::format("  iota empty: {}\n", v1.empty());

    // operator bool - 至少前向迭代器
    if (v1) {
        std::cout << "  iota bool: true (非空)\n";
    }

    // size() - 能计算 begin/end 差值
    std::cout << std::format("  iota size: {}\n", v1.size());

    // front() - 至少前向迭代器
    std::cout << std::format("  iota front: {}\n", v1.front());

    // back() - 双向且 common
    std::cout << std::format("  iota back: {}\n", v1.back());

    // operator[] - 至少随机访问
    std::cout << std::format("  iota[2]: {}\n", v1[2]);

    // data() - 连续内存
    std::vector<int> vec{10, 20, 30, 40, 50};
    auto sv = std::views::all(vec);
    std::cout << std::format("  ref_view data: {}\n", *sv.data());

    // empty_view 的特殊行为
    auto ev = std::views::empty<int>;
    std::cout << std::format("  empty_view empty: {}\n", ev.empty());
    std::cout << std::format("  empty_view size: {}\n", ev.size());
    std::cout << std::format("  empty_view bool: {}\n", static_cast<bool>(ev));

    std::cout << "\n";
}

// --- 8.3.1 子范围 (subrange) 从迭代器对创建 ---
// ch06 有哨兵+子范围，但未展示 equal_range 等模式

void demo_subrange_from_iterators() {
    std::cout << "--- 8.3.1 子范围从迭代器对创建 ---\n";

    // 从 find 结果创建子范围
    std::vector<int> coll{0, 8, 15, 47, 11, -1, 13};
    auto s1 = std::ranges::subrange{
        std::ranges::find(coll, 15),
        std::ranges::find(coll, -1)
    };
    std::cout << "  subrange(find,find): "; print(s1);

    // equal_range 示例: 获取 multimap 中某 key 的所有值
    std::unordered_multimap<std::string, std::string> dict = {
        {"strange", "fremd"},
        {"smart", "klug"},
        {"car", "Auto"},
        {"smart", "raffiniert"},
        {"trait", "Merkmal"},
        {"smart", "elegant"},
    };
    auto range = dict.equal_range("smart");
    auto smartRange = std::ranges::subrange(range.first, range.second);
    std::cout << "  smart 翻译:\n";
    for (const auto& [key, val] : smartRange) {
        std::cout << std::format("    {} = {}\n", key, val);
    }

    // 从迭代器对 + 管道
    std::list<int> lst{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto s2 = std::ranges::subrange{std::ranges::find(lst, 4), lst.end()}
        | std::views::take(3);
    std::cout << "  subrange+take: "; print(s2);

    // 子范围的大小可能因插入/删除而改变
    std::list<int> lst21{1, 2, 3, 4, 5, 6, 7, 8};
    
    auto sr = std::ranges::subrange(lst21.begin(), lst21.end());
    std::cout << "  插入前: "; print(sr);
    lst21.insert(++lst21.begin(), 0);
    std::cout << "  插入后: "; print(sr);  // 范围增大了

    std::cout << "\n";
}

// --- 8.4.4 输入流视图 (istream_view) ---
// 全新内容: 从流中读取元素

void demo_istream_view() {
    std::cout << "--- 8.4.4 输入流视图 (istream_view) ---\n";

    // 从字符串流读取 string
    std::string s{"2 4 6 8 Motorway 1977 by Tom Robinson"};
    std::istringstream strm1{s};
    std::cout << "  读取string: ";
    for (const auto& elem : std::ranges::istream_view<std::string>{strm1}) {
        std::cout << elem << " | ";
    }
    std::cout << "\n";

    // 从字符串流读取 int (遇到非数字停止)
    std::istringstream strm2{s};
    std::cout << "  读取int:    ";
    for (const auto& elem : std::views::istream<int>(strm2)) {
        std::cout << elem << " ";
    }
    std::cout << "\n";

    // 从字符串流读取 double
    std::istringstream strm3{"3.14 2.718 1.618 0.577"};
    std::cout << "  读取double: ";
    for (const auto& elem : std::views::istream<double>(strm3)) {
        std::cout << elem << " ";
    }
    std::cout << "\n";

    // 注意: istream_view 不能用 const 引用遍历
    // 必须用万能引用 (auto&&)
    auto printStream = [](auto&& rg) {
        for (const auto& elem : rg) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    };
    std::istringstream strm4{"hello world foo bar"};
    printStream(std::views::istream<std::string>(strm4));

    // end() 返回 default_sentinel
    std::istringstream strm5{"1 2 3"};
    auto iv = std::views::istream<int>(strm5);
    std::cout << "  end类型: " << typeid(decltype(iv.end())).name() << "\n";

    std::cout << "\n";
}

// --- 8.4.5/8.4.6 string_view 和 span 作为视图 ---
// 全新内容: 展示 string_view 和 span 的视图特性

void demo_string_span_as_views() {
    std::cout << "--- 8.4.5/8.4.6 string_view 和 span 作为视图 ---\n";

    // string_view 是借用范围 (borrowed_range)
    std::string str = "Hello C++20 Views";
    std::string_view sv{str};
    std::cout << std::format("  string_view borrowed: {}\n",
        std::ranges::borrowed_range<std::string_view>);
    std::cout << "  string_view: ";
    for (char c : sv | std::views::take(5)) std::cout << c;
    std::cout << "\n";

    // span 是借用范围
    int arr[] = {10, 20, 30, 40, 50};
    std::span<int> sp{arr};
    std::cout << std::format("  span borrowed: {}\n",
        std::ranges::borrowed_range<std::span<int>>);

    // span::subspan / first / last (ch06 未展示)
    std::vector<std::string> vec{"New York", "Tokyo", "Rio", "Berlin", "Sydney"};

    // 对中间三个元素排序 (subspan)
    std::ranges::sort(std::span{vec}.subspan(1, 3));
    std::cout << "  subspan(1,3)排序: "; print(vec);

    // last: 最后三个
    std::cout << "  last(3): ";
    for (const auto& e : std::span{vec}.last(3)) std::cout << e << " ";
    std::cout << "\n";

    // first: 前两个
    std::cout << "  first(2): ";
    for (const auto& e : std::span{vec}.first(2)) std::cout << e << " ";
    std::cout << "\n";

    // span 也可用于管道
    auto v = std::span{arr} | std::views::filter([](int x) { return x > 20; });
    std::cout << "  span|filter: "; print(v);

    std::cout << "\n";
}

// --- 8.5.5 过滤视图写入问题 ---
// 全新内容: 通过 filter 修改元素时破坏谓词的 UB

void demo_filter_write_issues() {
    std::cout << "--- 8.5.5 过滤视图写入问题 ---\n";

    // 危险: 修改元素使其不再满足过滤谓词
    std::vector<int> coll{1, 4, 7, 10, 13, 16, 19, 22, 25};
    auto isEven = [](int i) { return i % 2 == 0; };
    auto collEven = coll | std::views::filter(isEven);

    std::cout << "  初始coll: "; print(coll);

    // 第一次修改: 缓存了第一个匹配位置
    std::cout << "  第一次递增偶数:\n";
    for (int& i : collEven) {
        std::cout << std::format("    递增 {}\n", i);
        i += 1;  // 修改后不再满足 isEven!
    }
    std::cout << "  修改后coll: "; print(coll);

    // 第二次修改: 缓存的 begin 仍指向旧位置
    // 但那个位置已不再是偶数 -> UB / 重复处理
    std::cout << "  第二次递增偶数:\n";
    for (int& i : collEven) {
        std::cout << std::format("    递增 {}\n", i);
        i += 1;
    }
    std::cout << "  再次后coll: "; print(coll);

    // 安全做法: 使用普通循环
    std::cout << "  安全做法: 普通循环\n";
    std::vector<int> coll2{1, 4, 7, 10, 13, 16};
    for (auto& m : coll2) {
        if (m % 2 == 0) {
            m += 100;  // 安全: 不破坏谓词检查逻辑
        }
    }
    std::cout << "  安全修改后: "; print(coll2);

    // 建议: 用 filter 只读过滤，写入用普通循环
    std::cout << "  建议: filter 仅用于读取，修改用普通循环\n";
    std::cout << "\n";
}

// --- 8.6.1 转换视图返回引用 (写入) ---
// 全新内容: transform 可返回引用实现写入

void demo_transform_ref_write() {
    std::cout << "--- 8.6.1 转换视图返回引用 ---\n";

    // transform 可以返回引用，从而允许写入
    std::vector<std::pair<int, int>> coll{{1, 9}, {9, 1}, {2, 2}, {4, 1}, {2, 7}};
    std::cout << "  初始: ";
    for (const auto& [a, b] : coll) std::cout << a << "/" << b << " ";
    std::cout << "\n";

    // 返回 pair 中较小的成员的引用
    auto minMember = [](std::pair<int, int>& elem) -> int& {
        return elem.second < elem.first ? elem.second : elem.first;
    };

    // 通过 transform 视图写入
    for (auto&& member : coll | std::views::transform(minMember)) {
        ++member;  // 修改原始 pair 中较小的值
    }
    std::cout << "  递增较小值: ";
    for (const auto& [a, b] : coll) std::cout << a << "/" << b << " ";
    std::cout << "\n";

    // 注意: lambda 必须显式声明 -> int&
    // 否则返回副本，修改不影响原始数据

    std::cout << "\n";
}

// --- 8.6.2 元素视图 (elements_view) ---
// 全新内容: elements<N> 访问 tuple 的第 N 个元素

void demo_elements_view() {
    std::cout << "--- 8.6.2 元素视图 (elements_view) ---\n";

    // elements<N>: 获取 tuple 的第 N 个元素
    std::vector<std::tuple<int, std::string, double>> coll{
        {1, "pi", std::numbers::pi},
        {2, "e", std::numbers::e},
        {3, "golden-ratio", std::numbers::phi},
        {4, "euler-constant", std::numbers::egamma},
    };

    // elements<0>: 获取第一个元素 (int)
    std::cout << "  elements<0>: ";
    print(coll | std::views::elements<0>);

    // elements<1>: 获取第二个元素 (string)
    std::cout << "  elements<1>: ";
    print(coll | std::views::elements<1>);

    // elements<2>: 获取第三个元素 (double)
    std::cout << "  elements<2>: ";
    print(coll | std::views::elements<2>);

    // keys 和 values 是 elements 的特化
    // keys = elements<0>, values = elements<1>
    std::map<std::string, int> scores{{"alice", 90}, {"bob", 85}, {"charlie", 95}};
    std::cout << "  keys:   "; print(scores | std::views::keys);
    std::cout << "  values: "; print(scores | std::views::values);

    // 按 elements 排序 (按第三个元素排序)
    std::ranges::sort(coll, std::ranges::less{},
        [](const auto& e) { return std::get<2>(e); });
    std::cout << "  按double排序后 elements<1>: ";
    print(coll | std::views::elements<1>);
    std::cout << "  按double排序后 elements<2>: ";
    print(coll | std::views::elements<2>);

    // 注意: 不应该直接对 elements 视图排序
    // std::ranges::sort(coll | std::views::elements<2>);  // 错误! 只排序值，不排序原始元素

    std::cout << "\n";
}

// --- 8.7.1 反向视图详细 API ---
// ch06 有基本用法，这里补充 base() 和缓存细节

void demo_reverse_detail() {
    std::cout << "--- 8.7.1 反向视图详细 API ---\n";

    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto rv = vec | std::views::reverse;

    // base() 返回底层范围
    std::cout << "  reverse base类型: " << typeid(decltype(rv.base())).name() << "\n";

    // 反向视图的迭代器特性
    std::cout << std::format("  size: {}, front: {}, back: {}\n",
        rv.size(), rv.front(), rv.back());
    std::cout << std::format("  [0]: {}, [3]: {}\n", rv[0], rv[3]);

    // 双重 reverse 返回原始范围 (优化)
    auto rv2 = vec | std::views::reverse | std::views::reverse;
    std::cout << "  reverse|reverse: "; print(rv2);
    // 注意: 适配器优化，双重reverse可能返回原始范围

    // reverse 与 take 组合
    auto rv3 = vec | std::views::take(5) | std::views::reverse;
    std::cout << "  take(5)|reverse: "; print(rv3);

    // 反向遍历需要底层是双向范围
    // list 也可以
    std::list<int> lst{1, 2, 3, 4, 5};
    std::cout << "  list|reverse:   "; print(lst | std::views::reverse);

    std::cout << "\n";
}

// --- 8.8.1 拆分视图 (多值分隔符) ---
// ch06 有单字符 split，这里补充多值分隔符和 lazy_split

void demo_split_detail() {
    std::cout << "--- 8.8.1 拆分视图详细 ---\n";

    // 多值分隔符
    std::vector<int> rg{1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3};
    std::cout << "  按{5,1}拆分:\n";
    for (const auto& sub : rg | std::views::split(std::array{5, 1})) {
        std::cout << "    "; print(sub);
    }

    // 用 pattern 数组
    std::array pattern{4, 5};
    std::cout << "  按{4,5}拆分:\n";
    for (const auto& sub : std::views::split(rg, std::views::all(pattern))) {
        std::cout << "    "; print(sub);
    }

    // 字符串拆分 (string_view 作为分隔符)
    std::string str{"No problem can withstand the assault of sustained thinking"};
    std::cout << "  按\"th\"拆分:\n";
    for (auto sub : std::views::split(str, std::string_view{"th"})) {
        std::cout << "    " << std::string_view{sub.begin(), sub.end()} << "\n";
    }

    // 连续分隔符产生空子范围
    std::vector<int> rg2{5, 5, 1, 2, 3, 4, 5, 6, 5, 5, 4, 3, 2, 1, 5, 5};
    std::cout << "  连续分隔符:\n";
    int idx = 0;
    for (const auto& sub : std::views::split(rg2, 5)) {
        std::cout << "    [" << idx++ << "]: ";
        if (std::ranges::empty(sub)) {
            std::cout << "(empty)";
        } else {
            print(sub);
        }
    }

    // split_view 的 base()
    auto sv = rg | std::views::split(5);
    std::cout << "  split base类型: " << typeid(decltype(sv.base())).name() << "\n";

    std::cout << "\n";
}

// --- 8.8.2 连接视图 (join_view) 连接多个子范围 ---
// ch06 有基本 join，这里补充连接多个数组等高级用法

void demo_join_detail() {
    std::cout << "--- 8.8.2 连接视图详细 ---\n";

    // 用 subrange 连接多个数组
    int arr1[] = {1, 2, 3, 4, 5};
    int arr2[] = {0, 8, 15};
    int arr3[] = {10, 20, 30};

    std::array<std::ranges::subrange<int*>, 3> coll{
        std::ranges::subrange{arr1},
        std::ranges::subrange{arr2},
        std::ranges::subrange{arr3}
    };
    std::cout << "  join subranges: "; print(coll | std::views::join);

    // join 嵌套 string (二维展平)
    std::vector<std::vector<std::string>> nested{
        {"he", "hi", "ho"},
        {"---", "|", "---"},
        {"he", "hi", "ho"}
    };
    std::cout << "  join嵌套: ";
    print(nested | std::views::join);

    // 双层 join: 展平二维字符
    std::vector<std::string> words{"hello", "world", "cpp"};
    std::cout << "  双层join字符: ";
    for (char c : words | std::views::join) std::cout << c << " ";
    std::cout << "\n";

    // join + transform 组合
    std::vector<std::vector<int>> data{{1, 2, 3}, {4, 5}, {6, 7, 8, 9}};
    auto result = data | std::views::join | std::views::filter([](int x) { return x % 2 != 0; });
    std::cout << "  join+filter奇数: "; print(result);

    // join_view 的 base() 和 size
    auto jv = coll | std::views::join;
    std::cout << "  join base类型: " << typeid(decltype(jv.base())).name() << "\n";

    // 注意: 内层迭代器不支持 iterator_traits
    // 应使用 std::ranges::next() 而非 std::next()
    std::cout << "  提示: join_view 内层迭代器不支持 iterator_traits\n";
    std::cout << "  应使用 std::ranges::next() 而非 std::next()\n";

    std::cout << "\n";
}

// --- 详细视图 API 演示 ---
// 展示各视图的 base(), pred() 等成员函数

void demo_view_api_details() {
    std::cout << "--- 视图详细 API ---\n";

    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto isEven = [](int x) { return x % 2 == 0; };

    // take_view: base() 返回底层范围
    auto tv = vec | std::views::take(5);
    std::cout << "  take_view:\n";
    std::cout << "    size: " << tv.size() << "\n";
    std::cout << "    front: " << tv.front() << ", back: " << tv.back() << "\n";
    std::cout << "    [2]: " << tv[2] << "\n";

    // filter_view: base() + pred()
    auto fv = vec | std::views::filter(isEven);
    std::cout << "  filter_view:\n";
    std::cout << "    base类型: " << typeid(decltype(fv.base())).name() << "\n";
    // pred() 返回谓词的引用
    auto& pred = fv.pred();
    std::cout << "    pred()(4): " << pred(4) << "\n";

    // transform_view: base()
    auto tfv = vec | std::views::transform([](int x) { return x * x; });
    std::cout << "  transform_view:\n";
    std::cout << "    base类型: " << typeid(decltype(tfv.base())).name() << "\n";
    std::cout << "    size: " << tfv.size() << "\n";
    std::cout << "    front: " << tfv.front() << ", back: " << tfv.back() << "\n";

    // drop_view
    auto dv = vec | std::views::drop(3);
    std::cout << "  drop_view:\n";
    std::cout << "    size: " << dv.size() << "\n";
    std::cout << "    front: " << dv.front() << "\n";

    // take_while_view: base() + pred()
    auto twv = vec | std::views::take_while([](int x) { return x < 6; });
    std::cout << "  take_while_view:\n";
    std::cout << "    pred()(3): " << twv.pred()(3) << "\n";

    // drop_while_view: base() + pred()
    auto dwv = vec | std::views::drop_while([](int x) { return x < 4; });
    std::cout << "  drop_while_view:\n";
    std::cout << "    front: " << dwv.front() << "\n";

    // single_view: 特殊接口
    auto sv = std::views::single(42);
    std::cout << "  single_view:\n";
    std::cout << "    size: " << sv.size() << "\n";
    std::cout << "    front: " << sv.front() << ", back: " << sv.back() << "\n";
    std::cout << "    [0]: " << sv[0] << "\n";
    std::cout << "    data: " << sv.data() << "\n";
    // 可修改 single_view 的值
    sv.front() = 99;
    std::cout << "    修改后: " << sv.front() << "\n";

    // owning_view 接口
    auto ov = std::ranges::owning_view<std::vector<int>>{std::vector<int>{10, 20, 30}};
    std::cout << "  owning_view:\n";
    std::cout << "    size: " << ov.size() << "\n";
    std::cout << "    front: " << ov.front() << "\n";
    std::cout << "    base(): "; print(ov.base());

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第8章：视图类型详解 ===\n";
    std::cout << "注: 仅包含 ch06/ch07 未覆盖的新内容\n\n";

    demo_view_interface();
    demo_subrange_from_iterators();
    demo_istream_view();
    demo_string_span_as_views();
    demo_filter_write_issues();
    demo_transform_ref_write();
    demo_elements_view();
    demo_reverse_detail();
    demo_split_detail();
    demo_join_detail();
    demo_view_api_details();

    return 0;
}
