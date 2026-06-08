// ============================================================
// 第14章: 协程 (Coroutines)
// 参考: C++20 Complete Guide Chapter 14
// ============================================================
#ifdef _WIN32
#include <windows.h>
#endif
#include <coroutine>
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <cassert>
#include <ranges>

// ============================================================
// 14.2 基本协程接口: CoroTask
// ============================================================

// 简单任务协程接口 - 提供resume()恢复协程
class [[nodiscard]] CoroTask {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
private:
    CoroHdl hdl;
public:
    struct promise_type {
        auto get_return_object() {
            return CoroTask{CoroHdl::from_promise(*this)};
        }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    CoroTask(auto h) : hdl{h} {}
    ~CoroTask() { if (hdl) hdl.destroy(); }

    // 禁止复制, 允许移动
    CoroTask(const CoroTask&) = delete;
    CoroTask& operator=(const CoroTask&) = delete;
    CoroTask(CoroTask&& c) noexcept : hdl{std::exchange(c.hdl, nullptr)} {}
    CoroTask& operator=(CoroTask&&) = delete;

    // 恢复协程, 返回是否还有后续
    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
};

// ============================================================
// 14.2.1 第一个协程
// ============================================================

CoroTask coro(int max) {
    std::cout << "                 CORO " << max << " start\n";
    for (int val = 1; val <= max; ++val) {
        std::cout << "                   CORO   " << val << "/" << max << "\n";
        co_await std::suspend_always{};  // 暂停点
    }
    std::cout << "                 CORO " << max << " end\n";
}

void demo_first_coro() {
    std::cout << "--- 14.2.1 第一个协程 ---\n";
    auto coroTask = coro(3);
    std::cout << "coro() started\n";
    while (coroTask.resume()) {
        std::cout << "coro() suspended\n";
    }
    std::cout << "coro() done\n\n";
}

// ============================================================
// 14.2.2 多个协程
// ============================================================

void demo_multiple_coro() {
    std::cout << "--- 14.2.2 多个协程 ---\n";
    auto coroTask1 = coro(3);
    auto coroTask2 = coro(5);
    std::cout << "coro(3) and coro(5) started\n";

    coroTask2.resume();  // 恢复一次第二个协程

    while (coroTask1.resume()) {
        std::cout << "coro() suspended\n";
    }
    std::cout << "coro() done\n";

    coroTask2.resume();  // 再恢复一次第二个协程
    std::cout << "\n";
}

// ============================================================
// 14.3.1 产生值的协程: CoroGen (co_yield)
// ============================================================

class [[nodiscard]] CoroGen {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
private:
    CoroHdl hdl;
public:
    struct promise_type {
        int coroValue = 0;  // co_yield产生的最后一个值

        auto yield_value(int val) {   // co_yield的反应
            coroValue = val;
            return std::suspend_always{};
        }

        auto get_return_object() { return CoroHdl::from_promise(*this); }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    CoroGen(auto h) : hdl{h} {}
    ~CoroGen() { if (hdl) hdl.destroy(); }
    CoroGen(const CoroGen&) = delete;
    CoroGen& operator=(const CoroGen&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
    int getValue() const { return hdl.promise().coroValue; }
};

CoroGen coroGen(int max) {
    std::cout << "                 CORO " << max << " start\n";
    for (int val = 1; val <= max; ++val) {
        std::cout << "                   CORO   " << val << "/" << max << "\n";
        co_yield val;  // 产生值并暂停
    }
    std::cout << "                 CORO " << max << " end\n";
}

void demo_co_yield() {
    std::cout << "--- 14.3.1 co_yield ---\n";
    auto gen = coroGen(3);
    std::cout << "coroGen() started\n";
    while (gen.resume()) {
        auto val = gen.getValue();
        std::cout << "coro() suspended with " << val << "\n";
    }
    std::cout << "coro() done\n\n";
}

// ============================================================
// 14.3.1 迭代协程产生的值: Generator<T>
// ============================================================

template<typename T>
class [[nodiscard]] Generator {
public:
    struct promise_type {
        T coroValue{};
        auto yield_value(T val) {
            coroValue = val;
            return std::suspend_always{};
        }
        auto get_return_object() {
            return std::coroutine_handle<promise_type>::from_promise(*this);
        }
        auto initial_suspend() { return std::suspend_always{}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };
private:
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;
public:
    Generator(auto h) : hdl{h} {}
    ~Generator() { if (hdl) hdl.destroy(); }
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    // 迭代器接口
    struct iterator {
        CoroHdl hdl;
        iterator(auto p) : hdl{p} {}
        void getNext() {
            if (hdl) { hdl.resume(); if (hdl.done()) hdl = nullptr; }
        }
        T operator*() const { assert(hdl != nullptr); return hdl.promise().coroValue; }
        iterator& operator++() { getNext(); return *this; }
        bool operator==(const iterator&) const = default;
    };

    iterator begin() const {
        if (!hdl || hdl.done()) return iterator{nullptr};
        iterator itor{hdl};
        itor.getNext();
        return itor;
    }
    iterator end() const { return iterator{nullptr}; }
};

Generator<int> iotaGen(int max) {
    for (int i = 1; i <= max; ++i) {
        co_yield i;
    }
}

void demo_generator() {
    std::cout << "--- 14.3.1 Generator迭代 ---\n";
    auto gen = iotaGen(5);
    std::cout << "  values: ";
    for (const auto& val : gen) {
        std::cout << val << " ";
    }
    std::cout << "\n\n";
}

// ============================================================
// 14.3.2 返回值的协程: ResultTask (co_return)
// ============================================================

template<typename T>
class [[nodiscard]] ResultTask {
public:
    struct promise_type {
        T result{};
        void return_value(const auto& val) { result = val; }

        auto get_return_object() {
            return std::coroutine_handle<promise_type>::from_promise(*this);
        }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };
private:
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;
public:
    ResultTask(auto h) : hdl{h} {}
    ~ResultTask() { if (hdl) hdl.destroy(); }
    ResultTask(const ResultTask&) = delete;
    ResultTask& operator=(const ResultTask&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
    T getResult() const { return hdl.promise().result; }
};

ResultTask<double> average(auto coll) {
    double sum = 0;
    for (const auto& elem : coll) {
        std::cout << "    process " << elem << "\n";
        sum += elem;
        co_await std::suspend_always{};
    }
    co_return sum / std::ranges::ssize(coll);
}

void demo_co_return() {
    std::cout << "--- 14.3.2 co_return ---\n";
    std::vector values{0, 8, 15, 47, 11, 42};
    auto task = average(std::views::all(values));
    std::cout << "resume()\n";
    while (task.resume()) {
        std::cout << "resume() again\n";
    }
    std::cout << "result: " << task.getResult() << "\n\n";
}

// ============================================================
// 14.4.1 自定义等待器 (Awaiter)
// ============================================================

class TracingAwaiter {
    inline static int maxId = 0;
    int id;
public:
    static int& getMaxId() { return maxId; }
    TracingAwaiter() : id{++maxId} {
        std::cout << "                  AWAITER " << id << ": ==> constructor\n";
    }
    ~TracingAwaiter() {
        std::cout << "                  AWAITER " << id << ": <== destructor\n";
    }
    TracingAwaiter(const TracingAwaiter&) = delete;

    bool await_ready() const noexcept {
        std::cout << "                  AWAITER " << id << ":        await_ready() => false\n";
        return false;
    }
    void await_suspend(auto) const noexcept {
        std::cout << "                  AWAITER " << id << ":        await_suspend() => accept\n";
        // void返回: 接受暂停
    }
    void await_resume() const noexcept {
        std::cout << "                  AWAITER " << id << ":        await_resume()\n";
    }
};

CoroTask coroTrace(int max) {
    std::cout << "    START coro(" << max << ")\n";
    for (int i = 1; i <= max; ++i) {
        std::cout << "    CORO:  " << i << "/" << max << "\n";
        co_await TracingAwaiter{};
        std::cout << "    CONTINUE coro(" << max << ")\n";
    }
    std::cout << "    END coro(" << max << ")\n";
}

void demo_awaiter() {
    std::cout << "--- 14.4.1 自定义等待器 ---\n";
    TracingAwaiter::getMaxId() = 0;  // 重置ID
    auto coTask = coroTrace(2);
    std::cout << "started\n";
    while (coTask.resume()) {
        std::cout << "  suspended\n";
    }
    std::cout << "done\n\n";
}

// ============================================================
// 14.4.3 恢复子协程: CoroTaskSub
// ============================================================

class [[nodiscard]] CoroTaskSub {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
private:
    CoroHdl hdl;
public:
    struct promise_type {
        CoroHdl subHdl = nullptr;  // 子协程句柄

        auto get_return_object() { return CoroTaskSub{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    CoroTaskSub(auto h) : hdl{h} {}
    ~CoroTaskSub() {
        if (hdl) {
            // 销毁子协程（如果有）
            if (hdl.promise().subHdl) {
                hdl.promise().subHdl.destroy();
            }
            hdl.destroy();
        }
    }
    CoroTaskSub(const CoroTaskSub&) = delete;
    CoroTaskSub& operator=(const CoroTaskSub&) = delete;

    // 作为awaitable: co_await subCoro()
    bool await_ready() { return false; }
    void await_suspend(auto awaitHdl) {
        awaitHdl.promise().subHdl = hdl;  // 注册子协程
        hdl = nullptr;  // 转移所有权，防止临时对象析构销毁子协程
    }
    void await_resume() {}

    // resume: 委托给最深层的子协程
    bool resume() const {
        if (!hdl || hdl.done()) return false;
        CoroHdl inner = hdl;
        while (inner.promise().subHdl && !inner.promise().subHdl.done()) {
            inner = inner.promise().subHdl;
        }
        inner.resume();
        return !hdl.done();
    }
};

CoroTaskSub subCoro() {
    std::cout << "       coro(): PART1\n";
    co_await std::suspend_always{};
    std::cout << "       coro(): PART2\n";
}

CoroTaskSub callCoro() {
    std::cout << "    callCoro(): CALL coro()\n";
    co_await subCoro();  // 委托给子协程
    std::cout << "    callCoro(): coro() done\n";
    co_await std::suspend_always{};
    std::cout << "    callCoro(): END\n";
}

void demo_sub_coro() {
    std::cout << "--- 14.4.3 恢复子协程 ---\n";
    auto coroTask = callCoro();
    std::cout << "MAIN: callCoro() initialized\n";
    while (coroTask.resume()) {
        std::cout << "MAIN: callCoro() suspended\n";
    }
    std::cout << "MAIN: callCoro() done\n\n";
}

// ============================================================
// 14.4.4 从挂起状态向协程传递值 (BackAwaiter)
// ============================================================

template<typename Hdl>
class BackAwaiter {
    Hdl hdl = nullptr;
public:
    BackAwaiter() = default;
    bool await_ready() const noexcept { return false; }
    void await_suspend(Hdl h) noexcept { hdl = h; }
    auto await_resume() const noexcept { return hdl.promise().backValue; }
};

class [[nodiscard]] CoroGenBack {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
private:
    CoroHdl hdl;
public:
    struct promise_type {
        int coroValue = 0;
        std::string backValue;

        auto yield_value(int val) {
            coroValue = val;
            backValue.clear();
            return BackAwaiter<CoroHdl>{};
        }
        auto get_return_object() { return CoroHdl::from_promise(*this); }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    CoroGenBack(auto h) : hdl{h} {}
    ~CoroGenBack() { if (hdl) hdl.destroy(); }
    CoroGenBack(const CoroGenBack&) = delete;
    CoroGenBack& operator=(const CoroGenBack&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
    int getValue() const { return hdl.promise().coroValue; }
    void setBackValue(const auto& val) { hdl.promise().backValue = val; }
};

CoroGenBack coroBack(int max) {
    std::cout << "                 CORO " << max << " start\n";
    for (int val = 1; val <= max; ++val) {
        std::cout << "                   CORO  " << val << "/" << max << "\n";
        auto back = co_yield val;  // 产生值并等待响应
        std::cout << "                  CORO => " << back << "\n";
    }
    std::cout << "                 CORO " << max << " end\n";
}

void demo_back_value() {
    std::cout << "--- 14.4.4 向协程传回值 ---\n";
    auto gen = coroBack(3);
    std::cout << "coro() started\n\n";
    std::cout << "resume coro()\n";
    while (gen.resume()) {
        auto val = gen.getValue();
        std::cout << "coro() suspended with " << val << "\n";
        std::string back = (val % 2 != 0) ? "OK" : "ERR";
        std::cout << "resume coro() with back: " << back << "\n";
        gen.setBackValue(back);
    }
    std::cout << "coro() done\n\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第14章: 协程 (Coroutines) ===\n\n";

    demo_first_coro();
    demo_multiple_coro();
    demo_co_yield();
    demo_generator();
    demo_co_return();
    demo_awaiter();
    demo_sub_coro();
    demo_back_value();

    std::cout << std::flush;
    return 0;
}
