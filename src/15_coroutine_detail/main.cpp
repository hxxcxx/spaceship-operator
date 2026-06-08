// ============================================================
// 第15章: 协程详解 (Coroutine Detail)
// 参考: C++20 Complete Guide Chapter 15
// ============================================================
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <coroutine>
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <cassert>
#include <utility>

// ============================================================
// 15.2 追踪协程生命周期
// ============================================================

class [[nodiscard]] TracingCoro {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;

    struct promise_type {
        promise_type() { std::cout << "          PROMISE: constructor\n"; }
        ~promise_type() { std::cout << "          PROMISE: destructor\n"; }

        auto get_return_object() {
            std::cout << "          PROMISE: get_return_object()\n";
            return TracingCoro{CoroHdl::from_promise(*this)};
        }
        auto initial_suspend() {
            std::cout << "          PROMISE: initial_suspend()\n";
            return std::suspend_always{};  // 延迟启动
        }
        void unhandled_exception() {
            std::cout << "          PROMISE: unhandled_exception()\n";
            std::terminate();
        }
        void return_void() {
            std::cout << "          PROMISE: return_void()\n";
        }
        auto final_suspend() noexcept {
            std::cout << "          PROMISE: final_suspend()\n";
            return std::suspend_always{};
        }
    };

    TracingCoro(auto h) : hdl{h} {
        std::cout << "               INTERFACE: construct\n";
    }
    ~TracingCoro() {
        std::cout << "               INTERFACE: destruct\n";
        if (hdl) hdl.destroy();
    }
    TracingCoro(const TracingCoro&) = delete;
    TracingCoro& operator=(const TracingCoro&) = delete;

    bool resume() const {
        std::cout << "               INTERFACE: resume()\n";
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
};

TracingCoro traceCoro(int max) {
    std::cout << "    START coro(" << max << ")\n";
    for (int i = 1; i <= max; ++i) {
        std::cout << "    CORO:  " << i << "/" << max << "\n";
        co_await std::suspend_always{};
        std::cout << "    CONTINUE coro(" << max << ")\n";
    }
    std::cout << "    END coro(" << max << ")\n";
}

void demo_tracing() {
    std::cout << "--- 15.2 追踪协程生命周期 ---\n";
    std::cout << "**** start coro()\n";
    auto coroTask = traceCoro(2);
    std::cout << "**** coro() started\n";
    std::cout << "\n**** resume coro() in loop\n";
    while (coroTask.resume()) {
        std::cout << "**** coro() suspended\n";
        std::cout << "\n**** resume coro() in loop\n";
    }
    std::cout << "\n**** coro() loop done\n\n";
}

// ============================================================
// 15.3.1 立即启动 (initial_suspend返回suspend_never)
// ============================================================

class [[nodiscard]] EagerCoro {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;

    struct promise_type {
        auto get_return_object() { return EagerCoro{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_never{}; }  // 立即启动!
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    EagerCoro(auto h) : hdl{h} {}
    ~EagerCoro() { if (hdl) hdl.destroy(); }
    EagerCoro(const EagerCoro&) = delete;
    EagerCoro& operator=(const EagerCoro&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
};

EagerCoro eagerCoro(int max) {
    std::cout << "    EAGER: start\n";
    for (int i = 1; i <= max; ++i) {
        std::cout << "    EAGER: " << i << "/" << max << "\n";
        co_await std::suspend_always{};
    }
    std::cout << "    EAGER: end\n";
}

void demo_eager_start() {
    std::cout << "--- 15.3.1 立即启动(eager) ---\n";
    std::cout << "before calling eagerCoro()\n";
    auto task = eagerCoro(2);
    // 协程在调用时就已经执行了第一条语句!
    std::cout << "after calling eagerCoro()\n";
    while (task.resume()) {
        std::cout << "  suspended\n";
    }
    std::cout << "done\n\n";
}

// ============================================================
// 15.5 协程中的异常处理
// ============================================================

class [[nodiscard]] ExceptCoro {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;

    struct promise_type {
        std::exception_ptr ePtr;

        auto get_return_object() { return ExceptCoro{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() {
            ePtr = std::current_exception();  // 存储异常
        }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    ExceptCoro(auto h) : hdl{h} {}
    ~ExceptCoro() { if (hdl) hdl.destroy(); }
    ExceptCoro(const ExceptCoro&) = delete;
    ExceptCoro& operator=(const ExceptCoro&) = delete;

    bool resume() {
        if (!hdl || hdl.done()) return false;
        hdl.promise().ePtr = nullptr;
        hdl.resume();
        if (hdl.promise().ePtr) {
            std::rethrow_exception(hdl.promise().ePtr);
        }
        return !hdl.done();
    }
};

ExceptCoro coroExcept(bool throwIt) {
    std::cout << "    coro: start\n";
    co_await std::suspend_always{};
    if (throwIt) {
        throw std::runtime_error("oops from coroutine!");
    }
    std::cout << "    coro: end\n";
}

void demo_exceptions() {
    std::cout << "--- 15.5 协程中的异常 ---\n";

    // 正常完成
    {
        auto task = coroExcept(false);
        std::cout << "  normal: ";
        while (task.resume()) { std::cout << "resumed "; }
        std::cout << "done\n";
    }

    // 抛出异常
    {
        auto task = coroExcept(true);
        try {
            task.resume();  // 第一次resume
            task.resume();  // 第二次resume -> 抛异常
        } catch (const std::exception& e) {
            std::cout << "  caught: " << e.what() << "\n";
        }
    }
    std::cout << "\n";
}

// ============================================================
// 15.7.3 对称转移 (Symmetric Transfer)
// ============================================================

// FinalAwaiter: 协程结束时恢复延续协程
struct FinalAwaiter {
    bool await_ready() noexcept { return false; }
    template<typename Hdl>
    std::coroutine_handle<> await_suspend(Hdl h) noexcept {
        if (h.promise().contHdl) {
            return h.promise().contHdl;     // 恢复延续协程
        }
        return std::noop_coroutine();       // 无操作
    }
    void await_resume() noexcept {}
};

// 支持延续的协程接口
class [[nodiscard]] ContinuationCoro {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;

    struct promise_type {
        std::coroutine_handle<> contHdl = nullptr;  // 延续协程

        auto get_return_object() { return ContinuationCoro{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return FinalAwaiter{}; }
    };

    ContinuationCoro(auto h) : hdl{h} {}
    ~ContinuationCoro() { if (hdl) hdl.destroy(); }
    ContinuationCoro(const ContinuationCoro&) = delete;
    ContinuationCoro& operator=(const ContinuationCoro&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }

    // 设置延续
    void setContinuation(std::coroutine_handle<> cont) {
        hdl.promise().contHdl = cont;
    }
};

ContinuationCoro stepCoro(std::string name, int steps) {
    for (int i = 1; i <= steps; ++i) {
        std::cout << "    " << name << ": step " << i << "/" << steps << "\n";
        co_await std::suspend_always{};
    }
    std::cout << "    " << name << ": done\n";
}

void demo_symmetric_transfer() {
    std::cout << "--- 15.7.3 对称转移 ---\n";
    auto coro1 = stepCoro("A", 2);
    auto coro2 = stepCoro("B", 3);

    // 设置: coro1完成后延续到coro2
    // coro1.setContinuation(coro1.hdl);  // 演示: noop_coroutine

    std::cout << "  resume coro1:\n";
    while (coro1.resume()) {
        std::cout << "  back to caller\n";
    }
    std::cout << "  coro1 finished\n";

    // 正常演示noop_coroutine
    std::cout << "  std::noop_coroutine: 不执行任何操作的协程句柄\n";
    auto noop = std::noop_coroutine();
    std::cout << "  noop.done() = " << noop.done() << " (总是false)\n\n";
}

// ============================================================
// 15.8.1 await_transform
// ============================================================

class [[nodiscard]] TransformCoro {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;

    struct promise_type {
        int lastValue = 0;

        auto get_return_object() { return TransformCoro{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }

        // await_transform: co_await 42 会调用此函数
        auto await_transform(int val) {
            lastValue = val;
            std::cout << "    await_transform(" << val << ")\n";
            return std::suspend_always{};
        }
    };

    TransformCoro(auto h) : hdl{h} {}
    ~TransformCoro() { if (hdl) hdl.destroy(); }
    TransformCoro(const TransformCoro&) = delete;
    TransformCoro& operator=(const TransformCoro&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
    int getLastValue() const { return hdl.promise().lastValue; }
};

TransformCoro transformDemo() {
    std::cout << "    start\n";
    co_await 42;       // 通过await_transform处理
    co_await 100;      // 通过await_transform处理
    std::cout << "    end\n";
}

void demo_await_transform() {
    std::cout << "--- 15.8.1 await_transform ---\n";
    auto task = transformDemo();
    while (task.resume()) {
        std::cout << "  lastValue: " << task.getLastValue() << "\n";
    }
    std::cout << "\n";
}

// ============================================================
// 15.8.2 operator co_await
// ============================================================

// Awaiter for operator co_await (移到外部,避免模板不能是本地类)
struct MyAwaiter {
    int val;
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<>) {}
    int await_resume() { return val; }
};

class MyAwaitable {
    int value;
public:
    explicit MyAwaitable(int v) : value{v} {}
    MyAwaiter operator co_await() { return MyAwaiter{value}; }
};

class [[nodiscard]] CoAwaitOpCoro {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;

    struct promise_type {
        auto get_return_object() { return CoAwaitOpCoro{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    CoAwaitOpCoro(auto h) : hdl{h} {}
    ~CoAwaitOpCoro() { if (hdl) hdl.destroy(); }
    CoAwaitOpCoro(const CoAwaitOpCoro&) = delete;
    CoAwaitOpCoro& operator=(const CoAwaitOpCoro&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
};

CoAwaitOpCoro coAwaitOpDemo() {
    std::cout << "    start\n";
    auto v1 = co_await MyAwaitable{10};
    std::cout << "    got: " << v1 << "\n";
    auto v2 = co_await MyAwaitable{20};
    std::cout << "    got: " << v2 << "\n";
    std::cout << "    end\n";
}

void demo_operator_co_await() {
    std::cout << "--- 15.8.2 operator co_await ---\n";
    auto task = coAwaitOpDemo();
    while (task.resume()) {
        std::cout << "  suspended\n";
    }
    std::cout << "\n";
}

// ============================================================
// 15.1.1 协程Lambda表达式
// ============================================================

void demo_coro_lambda() {
    std::cout << "--- 15.1.1 协程Lambda ---\n";

    // 协程lambda: 必须指定返回类型, 不应捕获外部变量
    auto makeCoro = [](int max) -> TracingCoro {
        std::cout << "    LAMBDA coro(" << max << ")\n";
        for (int i = 1; i <= max; ++i) {
            std::cout << "    LAMBDA: " << i << "/" << max << "\n";
            co_await std::suspend_always{};
        }
        std::cout << "    LAMBDA end\n";
    };

    // 注意: 不能在一条语句中创建并调用lambda协程
    // auto task = makeCoro(3);  // OK: lambda存活
    // auto task = [](int m)->TracingCoro{...}(3); // 危险: lambda临时对象被销毁

    auto task = makeCoro(2);
    std::cout << "  lambda coro started\n";
    while (task.resume()) {
        std::cout << "  suspended\n";
    }
    std::cout << "  done\n\n";
}

// 需要前向声明TracingCoro::CoroHdl给lambda使用
// (实际上面demo_coro_lambda中的第一个lambda不工作,只是说明)

// ============================================================
// 15.4 协程句柄详解
// ============================================================

class [[nodiscard]] HandleDemoCoro {
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;  // public以便演示

    struct promise_type {
        auto get_return_object() { return HandleDemoCoro{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    HandleDemoCoro(auto h) : hdl{h} {}
    ~HandleDemoCoro() { if (hdl) hdl.destroy(); }
    HandleDemoCoro(const HandleDemoCoro&) = delete;
    HandleDemoCoro& operator=(const HandleDemoCoro&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
};

HandleDemoCoro handleDemoCoro() {
    std::cout << "    step\n";
    co_await std::suspend_always{};
    std::cout << "    done\n";
}

void demo_coro_handle() {
    std::cout << "--- 15.4 协程句柄详解 ---\n";

    auto task = handleDemoCoro();

    // coroutine_handle基本操作
    std::cout << "  hdl valid: " << static_cast<bool>(task.hdl) << "\n";
    std::cout << "  hdl done(): " << task.hdl.done() << "\n";

    // address/from_address: 序列化句柄
    void* addr = task.hdl.address();
    std::cout << "  address: " << addr << "\n";

    // 从地址重建句柄(类型擦除)
    std::coroutine_handle<> genericHdl = std::coroutine_handle<>::from_address(addr);
    std::cout << "  from_address valid: " << static_cast<bool>(genericHdl) << "\n";

    // coroutine_handle<void>: 通用句柄,不能访问promise
    std::cout << "  generic done: " << genericHdl.done() << "\n";

    // 恢复
    task.resume();
    task.resume();
    std::cout << "\n";
}

// ============================================================
// 15.6.2 自定义内存分配
// ============================================================

class [[nodiscard]] PmrCoro {
public:
    // 静态内存池(简化版)
    inline static char buf[65536];
    inline static size_t bufUsed = 0;

    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;

    struct promise_type {
        auto get_return_object() { return PmrCoro{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }

        // 自定义operator new: 从静态缓冲区分配
        void* operator new(std::size_t sz) {
            std::cout << "    allocate " << sz << " bytes from static buffer\n";
            if (bufUsed + sz > sizeof(buf)) throw std::bad_alloc{};
            void* p = buf + bufUsed;
            bufUsed += sz;
            return p;
        }
        // 自定义operator delete
        void operator delete(void*, std::size_t) {
            // 简化: 不回收(单调缓冲区)
        }
    };

    PmrCoro(auto h) : hdl{h} {}
    ~PmrCoro() { if (hdl) hdl.destroy(); }
    PmrCoro(const PmrCoro&) = delete;
    PmrCoro& operator=(const PmrCoro&) = delete;

    bool resume() const {
        if (!hdl || hdl.done()) return false;
        hdl.resume();
        return !hdl.done();
    }
};

PmrCoro pmrCoro(int max) {
    for (int i = 1; i <= max; ++i) {
        co_await std::suspend_always{};
    }
}

void demo_custom_alloc() {
    std::cout << "--- 15.6.2 自定义内存分配 ---\n";
    PmrCoro::bufUsed = 0;
    auto t1 = pmrCoro(3);
    auto t2 = pmrCoro(5);
    std::cout << "  total allocated: " << PmrCoro::bufUsed << " bytes\n";
    std::cout << "  (避免了堆分配)\n\n";
}

// ============================================================
// 综合示例: 协程调度器(简化版)
// ============================================================

class CoroSched;
class [[nodiscard]] SchedTask {
    friend class CoroSched;
public:
    struct promise_type;
    using CoroHdl = std::coroutine_handle<promise_type>;
    CoroHdl hdl;

    struct promise_type {
        int priority = 0;
        auto get_return_object() { return SchedTask{CoroHdl::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto final_suspend() noexcept { return std::suspend_always{}; }

        // await_transform: co_await 优先级值
        auto await_transform(int prio) {
            priority = prio;
            return std::suspend_always{};
        }
    };

    SchedTask(auto h) : hdl{h} {}
    ~SchedTask() { if (hdl) hdl.destroy(); }
    SchedTask(SchedTask&& c) noexcept : hdl{std::exchange(c.hdl, nullptr)} {}
    SchedTask(const SchedTask&) = delete;
    SchedTask& operator=(const SchedTask&) = delete;
};

SchedTask schedCoro(std::string name, int max) {
    for (int i = 1; i <= max; ++i) {
        std::cout << "    " << name << ": step " << i << "/" << max << "\n";
        co_await i;  // 通过await_transform设置优先级并暂停
    }
}

void demo_scheduler() {
    std::cout << "--- 综合示例: 简化调度器 ---\n";
    // 简化: 轮流恢复
    auto t1 = schedCoro("A", 3);
    auto t2 = schedCoro("B", 2);

    std::cout << "  round-robin scheduling:\n";
    while (!t1.hdl.done() || !t2.hdl.done()) {
        if (!t1.hdl.done()) {
            t1.hdl.resume();
            if (!t1.hdl.done()) std::cout << "    [A prio=" << t1.hdl.promise().priority << "]\n";
        }
        if (!t2.hdl.done()) {
            t2.hdl.resume();
            if (!t2.hdl.done()) std::cout << "    [B prio=" << t2.hdl.promise().priority << "]\n";
        }
    }
    std::cout << "  all done\n\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第15章: 协程详解 ===\n\n";

    demo_tracing();
    demo_eager_start();
    demo_exceptions();
    demo_symmetric_transfer();
    demo_await_transform();
    demo_operator_co_await();
    demo_coro_lambda();
    demo_coro_handle();
    demo_custom_alloc();
    demo_scheduler();

    return 0;
}
