#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <stop_token>
#include <mutex>
#include <condition_variable>
#include <future>
#include <syncstream>
#include <chrono>
#include <queue>
#include <atomic>

using namespace std::literals;

// 辅助: 线程安全输出
auto syncOut(std::ostream& strm = std::cout) {
    return std::osyncstream{strm};
}

// ============================================================
// 第12章: std::jthread和停止令牌
// 参考: C++20 Complete Guide Chapter 12
// ============================================================

// --- 12.1.1 std::thread的问题 ---

// 演示: thread析构导致terminate (注释掉的代码,仅说明)
void demo_thread_problem() {
    std::cout << "--- 12.1.1 std::thread的问题 ---\n";

    // std::thread的问题: 析构时如果不join/detach会调用terminate
    // 错误示例(注释掉,否则会崩溃):
    // {
    //     std::thread t{[] { std::this_thread::sleep_for(100ms); }};
    //     // 离开作用域, 没有join/detach -> std::terminate()
    // }

    // 正确但繁琐的做法:
    {
        std::thread t{[] {
            syncOut() << "  thread running\n";
        }};
        try {
            // ... 可能抛异常的代码 ...
        } catch (...) {
            t.join();  // 异常时也要join
            throw;
        }
        t.join();
    }

    std::cout << "  std::thread需要手动join/detach,异常时很繁琐\n";
    std::cout << "\n";
}

// --- 12.1.2 使用std::jthread ---
void demo_jthread_basic() {
    std::cout << "--- 12.1.2 使用std::jthread ---\n";

    // jthread: RAII类型, 析构时自动join
    {
        std::jthread t{[] {
            syncOut() << "  jthread running (auto-join)\n";
            std::this_thread::sleep_for(10ms);
        }};
        // 离开作用域自动调用join, 不会terminate
    }
    syncOut() << "  jthread析构后自动join完成\n";

    // 多个jthread也安全
    {
        std::jthread t1{[] { std::this_thread::sleep_for(10ms); }};
        std::jthread t2{[] { std::this_thread::sleep_for(5ms); }};
        // 两个线程都会自动join
    }
    syncOut() << "  多个jthread析构安全完成\n";

    // jthread API与thread兼容
    std::jthread t{[] {
        syncOut() << "  jthread id: " << std::this_thread::get_id() << "\n";
    }};
    syncOut() << "  hardware_concurrency: " << std::jthread::hardware_concurrency() << "\n";
    syncOut() << "  joinable: " << t.joinable() << "\n";
    t.join();

    std::cout << "\n";
}

// --- 12.1.3 停止令牌和停止回调 ---
void demo_stop_token() {
    std::cout << "--- 12.1.3 停止令牌和停止回调 ---\n";

    // 1. 不带stop_token的线程: 停止请求被忽略
    {
        std::jthread t{[] {
            for (int i = 0; i < 5; ++i) {
                std::this_thread::sleep_for(10ms);
            }
            syncOut() << "  no-stop-token thread done\n";
        }};
        t.request_stop();  // 线程不会响应
        // 析构时join会等待线程完成
    }

    // 2. 带stop_token的线程: 可以响应停止请求
    {
        std::jthread t{[](std::stop_token st) {
            int count = 0;
            while (!st.stop_requested()) {
                std::this_thread::sleep_for(10ms);
                ++count;
            }
            syncOut() << "  stop-token thread stopped after " << count << " iterations\n";
        }};
        std::this_thread::sleep_for(55ms);
        t.request_stop();  // 线程会响应停止
    }

    // 3. 使用stop_callback响应停止
    {
        std::jthread t{[](std::stop_token st) {
            // 注册停止回调
            std::stop_callback cb{st, [] {
                syncOut() << "  stop callback triggered!\n";
            }};
            // 模拟工作
            while (!st.stop_requested()) {
                std::this_thread::sleep_for(10ms);
            }
            syncOut() << "  thread exiting\n";
            // cb析构时自动注销回调
        }};
        std::this_thread::sleep_for(35ms);
        t.request_stop();
    }

    std::cout << "\n";
}

// --- 12.1.4 停止令牌和条件变量 ---
void demo_stop_token_cv() {
    std::cout << "--- 12.1.4 停止令牌和条件变量 ---\n";

    std::queue<std::string> messages;
    std::mutex messagesMx;
    std::condition_variable_any messagesCV;

    // 消费者线程: 等待消息或停止信号
    std::jthread consumer{[&](std::stop_token st) {
        while (!st.stop_requested()) {
            std::string msg;
            {
                std::unique_lock lock(messagesMx);
                // wait支持stop_token: 停止请求会中断等待
                if (!messagesCV.wait(lock, st, [&] {
                    return !messages.empty();
                })) {
                    syncOut() << "  consumer: stop requested while waiting\n";
                    return;  // 停止请求中断了wait
                }
                msg = messages.front();
                messages.pop();
            }
            syncOut() << "  consumer got: " << msg << "\n";
        }
    }};

    // 生产者: 发送消息
    {
        std::scoped_lock lg{messagesMx};
        messages.push("Hello");
        messagesCV.notify_one();
    }
    std::this_thread::sleep_for(50ms);

    {
        std::scoped_lock lg{messagesMx};
        messages.push("World");
        messagesCV.notify_one();
    }
    std::this_thread::sleep_for(50ms);

    // 析构时自动request_stop + join
    // 停止信号会中断condition_variable的wait
    syncOut() << "  jthread析构, 发出停止信号...\n";

    std::cout << "\n";
}

// --- 12.2 停止源和停止令牌 ---
void demo_stop_source_token() {
    std::cout << "--- 12.2 停止源和停止令牌 ---\n";

    // 独立于jthread使用停止机制
    std::stop_source ssrc;                  // 创建共享停止状态
    std::stop_token stok{ssrc.get_token()}; // 获取令牌

    syncOut() << "  stop_possible: " << stok.stop_possible() << "\n";
    syncOut() << "  stop_requested: " << stok.stop_requested() << "\n";

    // 在另一个线程中使用令牌
    auto fut = std::async(std::launch::async, [stok] {
        int count = 0;
        while (!stok.stop_requested()) {
            std::this_thread::sleep_for(10ms);
            ++count;
        }
        syncOut() << "  async task stopped after " << count << " iterations\n";
    });

    std::this_thread::sleep_for(55ms);
    ssrc.request_stop();  // 请求停止
    fut.wait();

    // 停止是不可撤回的
    syncOut() << "  stop_requested after: " << stok.stop_requested() << "\n";

    // nostopstate: 不关联停止状态的stop_source
    std::stop_source emptySSrc{std::nostopstate};
    syncOut() << "  nostopstate stop_possible: "
        << emptySSrc.get_token().stop_possible() << "\n";

    std::cout << "\n";
}

// --- 12.2.1 停止源和停止令牌详解 ---
void demo_stop_api_detail() {
    std::cout << "--- 12.2.1 停止源/令牌API详解 ---\n";

    std::stop_source s1;
    std::stop_source s2{s1};           // 复制: 共享停止状态
    std::stop_source s3{std::move(s1)}; // 移动: s1不再有状态

    syncOut() << "  s1 stop_possible: " << s1.stop_possible() << "\n";  // false (被移走)
    syncOut() << "  s2 stop_possible: " << s2.stop_possible() << "\n";  // true
    syncOut() << "  s3 stop_possible: " << s3.stop_possible() << "\n";  // true

    // s2和s3共享状态
    s2.request_stop();
    syncOut() << "  s3 stop_requested: " << s3.stop_requested() << "\n"; // true

    // 赋值
    s1 = std::stop_source{};  // 给s1分配新的停止状态
    syncOut() << "  s1 reassigned stop_possible: " << s1.stop_possible() << "\n";

    // stop_token
    std::stop_token t1 = s1.get_token();
    std::stop_token t2{t1};          // 复制: 共享状态
    std::stop_token t3{std::move(t1)}; // 移动

    syncOut() << "  t2 == t3: " << (t2 == t3) << "\n";
    syncOut() << "  t2 stop_possible: " << t2.stop_possible() << "\n";

    // swap
    std::stop_source sa, sb;
    sa.request_stop();
    syncOut() << "  before swap: sa=" << sa.stop_requested()
        << " sb=" << sb.stop_requested() << "\n";
    sa.swap(sb);
    syncOut() << "  after swap:  sa=" << sa.stop_requested()
        << " sb=" << sb.stop_requested() << "\n";

    std::cout << "\n";
}

// --- 12.2.2 使用停止回调 ---
void demo_stop_callback() {
    std::cout << "--- 12.2.2 使用停止回调 ---\n";

    std::stop_source ssrc;
    std::stop_token stok{ssrc.get_token()};

    // 在主线程注册回调
    std::stop_callback cb1{stok, [] {
        syncOut() << "  callback1: main thread stop requested\n";
    }};

    // 在异步任务中注册回调
    auto fut = std::async(std::launch::async, [stok] {
        auto id = std::this_thread::get_id();

        std::stop_callback cb2{stok, [id] {
            syncOut() << "  callback2: in async task "
                << (id == std::this_thread::get_id() ? "(task thread)" : "(caller thread)")
                << "\n";
        }};
        std::this_thread::sleep_for(50ms);

        std::stop_callback cb3{stok, [id] {
            syncOut() << "  callback3: in async task "
                << (id == std::this_thread::get_id() ? "(task thread)" : "(caller thread)")
                << "\n";
        }};
        std::this_thread::sleep_for(50ms);
    });

    std::this_thread::sleep_for(80ms);
    syncOut() << "  requesting stop...\n";
    ssrc.request_stop();  // 触发已注册的回调

    fut.wait();

    // request_stop只能成功一次(不可撤回)
    bool again = ssrc.request_stop();
    syncOut() << "  second request_stop: " << again << " (already stopped)\n";

    std::cout << "\n";
}

// --- 12.3 std::jthread详解 ---
void demo_jthread_detail() {
    std::cout << "--- 12.3 std::jthread详解 ---\n";

    // jthread创建和基本操作
    std::jthread t{[](std::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(10ms);
        }
        syncOut() << "  jthread received stop\n";
    }};

    // 获取停止源和令牌
    auto ssrc = t.get_stop_source();
    auto stok = t.get_stop_token();

    syncOut() << "  joinable: " << t.joinable() << "\n";
    syncOut() << "  get_id: " << t.get_id() << "\n";
    syncOut() << "  stop_possible: " << stok.stop_possible() << "\n";

    // 通过stop_source请求停止
    ssrc.request_stop();
    t.join();
    syncOut() << "  after join\n";

    // 移动赋值: 先停止旧线程再转移
    std::jthread t2;
    t2 = std::jthread{[] {
        syncOut() << "  moved thread running\n";
    }};
    t2.join();

    std::cout << "\n";
}

// --- 12.3.1 jthread集合和共享停止令牌 ---
void demo_jthread_collection() {
    std::cout << "--- 12.3.1 jthread集合 ---\n";

    // 启动多个jthread
    {
        std::vector<std::jthread> threads;
        std::atomic<int> counter{0};

        for (int i = 0; i < 5; ++i) {
            threads.emplace_back([&counter](std::stop_token st, int id) {
                while (!st.stop_requested()) {
                    ++counter;
                    std::this_thread::sleep_for(10ms);
                }
                syncOut() << "  thread " << id << " stopped\n";
            }, i);
        }

        std::this_thread::sleep_for(55ms);

        // 更好的停止方式: 先请求所有线程停止, 再析构(自动join)
        for (auto& t : threads) {
            t.request_stop();
        }
        // 析构时join, 此时线程应该已经快结束了
    }

    // 使用共享停止令牌控制所有线程
    {
        std::stop_source sharedSource;
        std::stop_token sharedToken{sharedSource.get_token()};
        std::vector<std::jthread> threads;

        for (int i = 0; i < 3; ++i) {
            // 将共享令牌作为参数传递给线程
            threads.emplace_back([i](std::stop_token st) {
                while (!st.stop_requested()) {
                    std::this_thread::sleep_for(10ms);
                }
                syncOut() << "  shared-token thread " << i << " stopped\n";
            }, sharedToken);  // 传入共享令牌(但jthread会用自己的)
        }

        std::this_thread::sleep_for(35ms);

        // 用共享stop_source同时停止所有线程
        sharedSource.request_stop();
    }

    std::cout << "\n";
}

// --- 综合示例: 带超时和取消的任务 ---
void demo_practical_example() {
    std::cout << "--- 综合示例: 带取消的任务处理 ---\n";

    std::atomic<int> processed{0};
    std::atomic<bool> force_stop{false};

    std::jthread worker{[&](std::stop_token st) {
        // 注册清理回调
        std::stop_callback cleanup{st, [&] {
            syncOut() << "  cleanup: processed " << processed.load() << " items\n";
            force_stop = true;
        }};

        // 模拟工作
        for (int i = 1; i <= 100 && !st.stop_requested() && !force_stop; ++i) {
            std::this_thread::sleep_for(10ms);
            ++processed;
        }

        if (st.stop_requested()) {
            syncOut() << "  worker: cancelled gracefully\n";
        } else {
            syncOut() << "  worker: completed all work\n";
        }
    }};

    // 主线程决定何时取消
    std::this_thread::sleep_for(120ms);
    syncOut() << "  main: requesting stop...\n";
    worker.request_stop();
    // 析构时自动join

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第12章: std::jthread和停止令牌 ===\n\n";

    demo_thread_problem();
    demo_jthread_basic();
    demo_stop_token();
    demo_stop_token_cv();
    demo_stop_source_token();
    demo_stop_api_detail();
    demo_stop_callback();
    demo_jthread_detail();
    demo_jthread_collection();
    demo_practical_example();

    return 0;
}
