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
#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// 第6章：范围与视图 (Ranges and Views)
// 参考: C++20 Complete Guide Chapter 6
// ============================================================

// 辅助打印函数（使用万能引用以兼容所有视图）
void print(auto&& rg, const char* delim = " ", const char* end = "\n") {
    for (const auto& elem : rg) {
        std::cout << elem << delim;
    }
    std::cout << end;
}

// --- 6.1 通过示例了解范围和视图 ---

// 6.1.1 将容器作为范围传递给算法
void demo_range_algorithms() {
    std::cout << "--- 6.1.1 范围算法 ---\n";

    // 旧方式: 传递两个迭代器
    std::vector<int> coll1{25, 42, 2, 0, 122, 5, 7};
    std::sort(coll1.begin(), coll1.end());
    std::cout << "  旧方式 sort: "; print(coll1);

    // 新方式: 直接传递范围
    std::vector<int> coll2{25, 42, 2, 0, 122, 5, 7};
    std::ranges::sort(coll2);
    std::cout << "  新方式 sort: "; print(coll2);

    // 原始数组也支持
    int arr[] = {42, 0, 8, 15, 7};
    std::ranges::sort(arr);
    std::cout << "  数组 sort:   "; print(arr);

    // 对 string 元素排序
    std::vector<std::string> names{"Rio", "Tokyo", "New York", "Berlin"};
    std::ranges::sort(names);
    std::cout << "  string sort: "; print(names);

    // 对 string 的字符排序
    std::string s = "hello";
    std::ranges::sort(s);
    std::cout << "  char sort:   " << s << "\n";

    // list 不满足 random_access_range，不能用 ranges::sort
    // std::list<int> lst{3, 1, 2};
    // std::ranges::sort(lst);  // 编译错误: 不满足 random_access_range
    std::cout << "\n";
}

// 6.1.3 视图和管道
void demo_views_pipeline() {
    std::cout << "--- 6.1.3 视图和管道 ---\n";
    std::vector<int> coll{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

    // 函数调用语法
    std::cout << "  take(coll, 5): ";
    print(std::views::take(coll, 5));

    // 管道语法（推荐）
    std::cout << "  coll|take(5):  ";
    print(coll | std::views::take(5));

    // 组合管道: 3的倍数 -> 平方 -> 前3个
    auto v = coll
        | std::views::filter([](int e) { return e % 3 == 0; })
        | std::views::transform([](int e) { return e * e; })
        | std::views::take(3);
    std::cout << "  filter(3x) -> square -> take(3): ";
    print(v);

    // map 管道示例
    std::map<std::string, int> composers{
        {"Bach", 1685}, {"Mozart", 1756}, {"Beethoven", 1770},
        {"Tchaikovsky", 1840}, {"Chopin", 1810}, {"Vivaldi", 1678},
    };
    std::cout << "  1700年后出生的前3位作曲家:\n";
    for (const auto& name : composers
        | std::views::filter([](const auto& p) { return p.second >= 1700; })
        | std::views::take(3)
        | std::views::keys) {
        std::cout << "    - " << name << "\n";
    }

    // iota 视图: 生成值序列
    std::cout << "  iota(1,11):   ";
    print(std::views::iota(1, 11));

    // 视图用于写入: 排序前5个元素
    std::vector<int> writable{5, 3, 1, 4, 2, 9, 8, 7, 6};
    std::ranges::sort(writable | std::views::take(5));
    std::cout << "  sort前5个:    "; print(writable);
    std::cout << "\n";
}

// 6.1.4 哨兵 (Sentinels)
struct NullTerm {
    bool operator==(auto pos) const {
        return *pos == '\0';
    }
};

// 泛型结束哨兵
template<auto End>
struct EndValue {
    bool operator==(auto pos) const {
        return *pos == End;
    }
};

void demo_sentinels() {
    std::cout << "--- 6.1.4 哨兵 ---\n";
    const char* rawString = "hello world";

    // 使用哨兵遍历 C 字符串（无需先计算长度）
    std::cout << "  NullTerm 遍历: ";
    for (auto pos = rawString; pos != NullTerm{}; ++pos) {
        std::cout << *pos << " ";
    }
    std::cout << "\n";

    // 使用 ranges::for_each + 哨兵
    std::cout << "  for_each+哨兵: ";
    std::ranges::for_each(rawString, NullTerm{}, [](char c) {
        std::cout << c << " ";
    });
    std::cout << "\n";

    // 子范围 + 哨兵
    std::ranges::subrange rawRange{rawString, NullTerm{}};
    std::cout << "  subrange+哨兵: ";
    for (char c : rawRange) std::cout << c << " ";
    std::cout << "\n";

    // EndValue 哨兵: 以特定值结束
    std::vector<int> coll{42, 8, 0, 15, 7, -1};
    std::cout << "  EndValue<-1>:  ";
    std::ranges::for_each(coll.begin(), EndValue<-1>{}, [](int v) {
        std::cout << v << " ";
    });
    std::cout << "\n";

    // 子范围 + EndValue: 可排序的范围
    std::vector<int> coll2{42, 8, 0, 15, 7, -1};
    std::ranges::subrange rng{coll2.begin(), EndValue<7>{}};
    std::ranges::sort(rng);
    std::cout << "  sort到7之前:   ";
    std::ranges::for_each(rng, [](int v) { std::cout << v << " "; });
    std::cout << "\n  完整coll2:     "; print(coll2);
    std::cout << "\n";
}

// 6.1.5 带计数的范围
void demo_counted_range() {
    std::cout << "--- 6.1.5 带计数的范围 ---\n";
    std::vector<int> coll{1, 2, 3, 4, 5, 6, 7, 8, 9};

    // views::counted: 从迭代器+计数创建视图
    auto pos5 = std::ranges::find(coll, 5);
    if (std::ranges::distance(pos5, coll.end()) >= 3) {
        std::cout << "  counted(pos5,3): ";
        print(std::views::counted(pos5, 3));
    }

    // counted 可以为0（空范围）
    std::cout << "  counted(begin,0): ";
    print(std::views::counted(coll.begin(), 0));

    // 注意: take vs counted
    // take 用于范围, counted 用于迭代器+计数
    std::cout << "  take(coll,3):    "; print(coll | std::views::take(3));
    std::cout << "\n";
}

// 6.1.6 投影 (Projections)
void demo_projections() {
    std::cout << "--- 6.1.6 投影 ---\n";
    std::vector<int> coll{-8, 15, -7, 0, -9, 42, -1};

    // 投影: 按绝对值排序
    std::ranges::sort(coll, std::ranges::less{}, [](int v) { return std::abs(v); });
    std::cout << "  按绝对值排序: "; print(coll);

    // 不用投影的方式（等价但不够清晰）
    std::vector<int> coll2{-8, 15, -7, 0, -9, 42, -1};
    std::ranges::sort(coll2, [](int a, int b) { return std::abs(a) < std::abs(b); });
    std::cout << "  等价方式:     "; print(coll2);

    // 投影: 查找（按绝对值匹配）
    std::vector<int> coll3{1, -2, 3, -4, 5};
    auto it = std::ranges::find(coll3, 4, [](int v) { return std::abs(v); });
    if (it != coll3.end()) {
        std::cout << std::format("  find(abs=4): 找到 {}\n", *it);
    }

    // 投影: 对 map 的 value 排序（需要复制到 vector）
    std::map<std::string, int> scores{{"alice", 90}, {"bob", 85}, {"charlie", 95}};
    std::cout << "  scores:\n";
    for (const auto& [name, score] : scores) {
        std::cout << std::format("    {}={}\n", name, score);
    }

    // 投影: transform 对元素取成员
    std::vector<std::pair<std::string, int>> pairs{{"a", 1}, {"b", 2}, {"c", 3}};
    std::cout << "  pairs | keys: ";
    print(pairs | std::views::keys);
    std::cout << "  pairs | values: ";
    print(pairs | std::views::values);
    std::cout << "\n";
}

// 6.1.7 范围实用工具
template<std::ranges::input_range Range>
std::ranges::range_value_t<Range> maxValue(Range&& rg_val) {
    if (std::ranges::empty(rg_val)) {
        return std::ranges::range_value_t<Range>{};
    }
    auto pos = std::ranges::begin(rg_val);
    auto max_val = *pos;
    while (++pos != std::ranges::end(rg_val)) {
        if (*pos > max_val) max_val = *pos;
    }
    return max_val;
}

void demo_range_utils() {
    std::cout << "--- 6.1.7 范围实用工具 ---\n";
    std::vector<int> coll{42, 8, 0, 15, 7};
    int arr[] = {3, 9, 1, 7, 5};

    std::cout << std::format("  maxValue(vector): {}\n", maxValue(coll));
    std::cout << std::format("  maxValue(array):  {}\n", maxValue(arr));
    auto greaterThan10 = [](int v) { return v > 10; };
    std::cout << std::format("  maxValue(filter): {}\n",
        maxValue(coll | std::views::filter(greaterThan10)));

    // 范围工具函数
    std::cout << std::format("  empty: {}, size: {}\n", std::ranges::empty(coll), std::ranges::size(coll));
    std::cout << std::format("  distance: {}, front: {}, back: {}\n",
        std::ranges::distance(coll), coll.front(), coll.back());
    std::cout << "\n";
}

// 6.2 借用迭代器和范围
void demo_borrowed() {
    std::cout << "--- 6.2 借用迭代器和范围 ---\n";

    // 左值: 安全，借用范围
    std::vector<int> coll{0, 8, 15};
    auto pos0 = std::ranges::find(coll, 8);
    std::cout << std::format("  左值find(8): {} (安全)\n", *pos0);

    // 临时对象: 返回 dangling（编译时错误）
    // auto pos1 = std::ranges::find(std::vector<int>{0, 8, 15}, 8);
    // *pos1;  // 编译错误: dangling

    // 安全方式: 绑定到引用
    auto get_data = []() { return std::vector<int>{0, 8, 15, 42, 7}; };
    const auto& data = get_data();
    auto pos2 = std::ranges::find(data, 42);
    if (pos2 != data.end()) {
        std::cout << std::format("  安全方式find(42): {}\n", *pos2);
    }

    // 借用范围: iota_view, span, string_view, subrange 等
    auto pos3 = std::ranges::find(std::views::iota(1, 100), 42);
    std::cout << std::format("  iota find(42): {} (借用范围)\n", *pos3);

    // borrowed_range 检查
    std::cout << std::format("  vector& borrowed: {}\n", std::ranges::borrowed_range<std::vector<int>&>);
    std::cout << std::format("  vector  borrowed: {}\n", std::ranges::borrowed_range<std::vector<int>>);
    std::cout << std::format("  span     borrowed: {}\n", std::ranges::borrowed_range<std::span<int>>);
    std::cout << std::format("  iota     borrowed: {}\n",
        std::ranges::borrowed_range<decltype(std::views::iota(1))>);
    std::cout << "\n";
}

// 6.3 使用视图
void demo_views_detail() {
    std::cout << "--- 6.3 视图详解 ---\n";
    std::vector<int> coll{8, 15, 7, 0, 9};

    // 视图的类型
    auto v1 = std::views::take(coll, 4);                     // take_view<ref_view<vector>>
    auto v2 = std::views::take(std::move(coll), 4);           // take_view<owning_view<vector>>

    std::cout << std::format("  v1 type: {}\n", typeid(v1).name());
    std::cout << std::format("  v2 type: {}\n", typeid(v2).name());

    // 源视图
    std::cout << "  [源视图]:\n";
    std::cout << "    iota(1,6):   "; print(std::views::iota(1, 6));
    std::cout << "    single(42):  "; print(std::views::single(42));
    std::cout << "    empty<int>:  "; print(std::views::empty<int>);

    // 适配视图
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::cout << "  [适配视图]:\n";
    std::cout << "    take(4):       "; print(v | std::views::take(4));
    std::cout << "    drop(4):       "; print(v | std::views::drop(4));
    std::cout << "    filter(even):  "; print(v | std::views::filter([](int x) { return x % 2 == 0; }));
    std::cout << "    transform(x3): "; print(v | std::views::transform([](int x) { return x * 3; }));
    std::cout << "    reverse:       "; print(v | std::views::reverse);
    std::cout << "    take_while<5:  "; print(v | std::views::take_while([](int x) { return x < 5; }));
    std::cout << "    drop_while<5:  "; print(v | std::views::drop_while([](int x) { return x < 5; }));

    // elements/keys/values
    std::map<std::string, int> m{{"a", 1}, {"b", 2}, {"c", 3}};
    std::cout << "    keys:           ";
    for (const auto& k : m | std::views::keys) std::cout << k << " ";
    std::cout << "\n    values:         ";
    for (const auto& val : m | std::views::values) std::cout << val << " ";
    std::cout << "\n";

    // join: 展平嵌套范围
    std::vector<std::vector<int>> nested{{1, 2}, {3, 4, 5}, {6}};
    std::cout << "    join:           ";
    print(nested | std::views::join);

    // split
    std::string csv = "hello,world,cpp,20";
    std::cout << "    split(','):     ";
    for (auto word : csv | std::views::split(',')) {
        std::cout << std::string(word.begin(), word.end()) << " | ";
    }
    std::cout << "\n";

    // common: 使 begin/end 类型相同
    auto tv = v | std::views::take(5);
    std::cout << std::format("    take common: {}\n",
        std::ranges::common_range<decltype(tv)>);
    auto cv = v | std::views::take(5) | std::views::common;
    std::cout << std::format("    take|common:  {}\n",
        std::ranges::common_range<decltype(cv)>);
    std::cout << "\n";
}

// 6.3.2 延迟求值 (Lazy Evaluation)
void demo_lazy_evaluation() {
    std::cout << "--- 6.3.2 延迟求值 ---\n";
    std::vector<int> coll{8, 15, 7, 0, 9};

    int filter_count = 0;
    int trans_count = 0;

    auto vColl = coll
        | std::views::filter([&](int i) {
            ++filter_count;
            return i % 3 == 0;
        })
        | std::views::transform([&](int i) {
            ++trans_count;
            return -i;
        });

    // 定义视图时不会调用任何 filter/transform
    std::cout << std::format("  定义后: filter调用了{}次, transform调用了{}次\n",
        filter_count, trans_count);

    // 使用时才调用（延迟求值）
    std::cout << "  遍历结果: ";
    for (int val : vColl) std::cout << val << " ";
    std::cout << "\n";
    std::cout << std::format("  遍历后: filter调用了{}次, transform调用了{}次\n",
        filter_count, trans_count);

    // find 只处理到找到为止
    filter_count = trans_count = 0;
    std::vector<int> coll2{8, 15, 7, 0, 9};
    auto v2 = coll2 | std::views::filter([&](int i) { ++filter_count; return i % 3 == 0; })
                    | std::views::transform([&](int i) { ++trans_count; return -i; });
    auto pos = std::ranges::find(v2, 0);
    std::cout << std::format("  find(0)后: filter={}次, transform={}次 (只处理到0为止)\n",
        filter_count, trans_count);
    std::cout << "\n";
}

// 6.3.3 视图中的缓存
void demo_caching() {
    std::cout << "--- 6.3.3 视图缓存 ---\n";
    std::vector<int> coll{8, 15, 7, 0, 9};
    int filter_count = 0;

    auto vColl = coll | std::views::filter([&](int i) {
        ++filter_count;
        return i % 3 == 0;
    });

    // 第一次遍历: 需要查找第一个元素
    std::cout << "  第一次遍历: ";
    for (int val : vColl) std::cout << val << " ";
    std::cout << std::format("(filter调用了{}次)\n", filter_count);

    // 第二次遍历: begin() 被缓存
    filter_count = 0;
    std::cout << "  第二次遍历: ";
    for (int val : vColl) std::cout << val << " ";
    std::cout << std::format("(filter调用了{}次, begin被缓存)\n", filter_count);

    // 建议: 即时创建即时使用
    std::cout << "  建议: 即时创建视图并立即使用\n";
    std::cout << "\n";
}

// 6.5 视图与 const
void demo_views_const() {
    std::cout << "--- 6.5 视图与const ---\n";

    // 万能引用版本可以接受所有视图
    auto print_all = [](auto&& rg_val) {
        for (const auto& elem : rg_val) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    };

    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::list<int> lst{1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto is_even = [](int v) { return v % 2 == 0; };

    // 这些都能工作（万能引用版本）
    std::cout << "  vec|take(3):      "; print_all(vec | std::views::take(3));
    std::cout << "  vec|drop(3):      "; print_all(vec | std::views::drop(3));
    std::cout << "  lst|take(3):      "; print_all(lst | std::views::take(3));
    std::cout << "  lst|drop(3):      "; print_all(lst | std::views::drop(3));
    std::cout << "  vec|filter(even): "; print_all(vec | std::views::filter(is_even));

    // 注意: const 引用版本对某些视图不能工作
    // void print_const(const auto& rg_val) { ... }
    // print_const(lst | std::views::drop(3));   // 编译错误
    // print_const(vec | std::views::filter(is_even));  // 编译错误

    // 视图不传递 const 到元素（浅常量性）
    std::array<int, 5> arr{1, 2, 3, 4, 5};
    auto tv = std::views::take(arr, 3);
    // tv 是非 const，其引用的元素也不是 const
    // 这意味着可以修改：
    for (auto& elem : std::views::take(arr, 3)) {
        elem *= 10;  // 修改原始元素
    }
    std::cout << "  修改后的arr:      "; print(arr);
    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第6章：范围与视图 ===\n\n";

    demo_range_algorithms();
    demo_views_pipeline();
    demo_sentinels();
    demo_counted_range();
    demo_projections();
    demo_range_utils();
    demo_borrowed();
    demo_views_detail();
    demo_lazy_evaluation();
    demo_caching();
    demo_views_const();

    return 0;
}
