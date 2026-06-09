# spaceship-operator

C++20 完整特性学习项目，基于《C++20 Complete Guide》全书 24 章实现。

## 参考资料

- [C++20 Complete Guide 中文版](https://cppguide.cn/pages/cpp20completeguides/)
- [cppstd20.com](https://www.cppstd20.com/)

## 章节

| # | 章节 | 目录 |
|---|------|------|
| 01 | 比较与 `<=>` 运算符 | `src/01_comparison` |
| 02 | 函数参数的占位符类型 | `src/02_placeholder_types` |
| 03 | 概念、需求与约束 | `src/03_concepts` |
| 04 | 概念、需求与约束详解 | `src/04_concepts_detail` |
| 05 | 标准概念详解 | `src/05_standard_concepts` |
| 06 | 范围与视图 | `src/06_ranges_views` |
| 07 | 范围与视图实用工具 | `src/07_range_utilities` |
| 08 | 视图类型详解 | `src/08_view_types` |
| 09 | Span | `src/09_spans` |
| 10 | 格式化输出 | `src/10_format` |
| 11 | `<chrono>` 日期与时区 | `src/11_chrono` |
| 12 | `std::jthread` 与停止令牌 | `src/12_jthread` |
| 13 | 并发特性 | `src/13_concurrency` |
| 14 | 协程 | `src/14_coroutines` |
| 15 | 协程详解 | `src/15_coroutine_detail` |
| 16 | 模块 | `src/16_modules` |
| 17 | Lambda 扩展 | `src/17_lambda_extensions` |
| 18 | 编译期计算 | `src/18_compile_time` |
| 19 | 非类型模板参数扩展 | `src/19_nttp_extensions` |
| 20 | 新的类型特性 | `src/20_type_traits` |
| 21 | 核心语言小改进 | `src/21_core_improvements` |
| 22 | 泛型编程改进 | `src/22_generic_improvements` |
| 23 | 标准库改进 | `src/23_stdlib_improvements` |
| 24 | 已弃用与移除的特性 | `src/24_deprecated_removed` |

## Build

```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
```

## Requirements

- C++20 编译器 (MSVC 14.44+ / GCC 10+ / Clang 10+)
- CMake 3.20+

## Notes

- 部分特性（Lambda NTTP、Pack init-capture）因 MSVC ICE 问题以注释文档形式呈现
- ch16 模块为占位文件（模块系统需编译器进一步完善支持）
