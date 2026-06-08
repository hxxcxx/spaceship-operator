#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <queue>
#include <thread>
#include <stop_token>
#include <mutex>
#include <condition_variable>
#include <latch>
#include <barrier>
#include <semaphore>
#include <atomic>
#include <syncstream>
#include <chrono>
#include <cmath>
#include <random>
#include <algorithm>
#include <memory>
#include <fstream>

using namespace std::literals;

auto syncOut(std::ostream& strm = std::cout) {
    return std::osyncstream{strm};
}

// ============================================================
// 第13章: 并发特性
// 参考: C++20 Complete Guide Chapter 13
// ============================================================

// --- 13.1.1 闩锁 (std::latch) ---
void demo_latch() {
    std::cout << "--- 13.1.1 闩锁 (std::latch) ---\n";

    // 场景1: 等待所有任务完成
    {
        std::array tags{'.', '?', '8', '+', '-'};
        std::latch allDone{static_cast<std::ptrdiff_t>(tags.size())};

        std::jthread t1{[&](std::stop_token) {
            for (unsigned i = 0; i < tags.size(); i += 2) {
                std::this_thread::sleep_for(10ms);
                syncOut() << "  task[" << tags[i] << "] done\n";
                allDone.count_down();
            }
        }};
        std::jthread t2{[&](std::stop_token) {
            for (unsigned i = 1; i < tags.size(); i += 2) {
                std::this_thread::sleep_for(15ms);
                syncOut() << "  task[" << tags[i] << "] done\n";
                allDone.count_down();
            }
        }};

        syncOut() << "  waiting for all tasks...\n";
        allDone.wait();
        syncOut() << "  all tasks done (threads may still running)\n";
    }

    // 场景2: 让所有线程一起开始
    {
        constexpr int numThreads = 5;
        std::latch allReady{numThreads};

        std::vector<std::jthread> threads;
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([i, &allReady] {
                std::this_thread::sleep_for(std::chrono::milliseconds(20 * i));
                syncOut() << "  thread " << i << " ready\n";
                allReady.arrive_and_wait();  // 所有线程都到这里才继续
                syncOut() << "  thread " << i << " started\n";
            });
        }
    }

    // latch API
    syncOut() << "  latch::max(): " << std::latch::max() << "\n";

    std::cout << "\n";
}

// --- 13.1.2 屏障 (std::barrier) ---
void demo_barrier() {
    std::cout << "--- 13.1.2 屏障 (std::barrier) ---\n";

    // 多线程反复计算平方根, 每轮结束时打印所有值
    std::vector values{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};

    auto printValues = [&values]() noexcept {
        std::string line;
        for (auto val : values) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%-7.5g ", val);
            line += buf;
        }
        syncOut() << "  " << line << "\n";
    };

    printValues();

    // barrier: 计数器初始值 + 回调(必须noexcept)
    std::barrier allDone{static_cast<std::ptrdiff_t>(values.size()), printValues};

    std::vector<std::jthread> threads;
    for (std::size_t idx = 0; idx < values.size(); ++idx) {
        threads.emplace_back([idx, &values, &allDone] {
            for (int i = 0; i < 5; ++i) {
                values[idx] = std::sqrt(values[idx]);
                allDone.arrive_and_wait();
            }
        });
    }

    // 等待所有线程自然完成(5轮计算)
    threads.clear();

    std::cout << "\n";
}

// --- 13.2.1 计数信号量 ---
void demo_counting_semaphore() {
    std::cout << "--- 13.2.1 计数信号量 ---\n";

    // 用信号量限制并发线程数
    constexpr int numThreads = 5;
    std::counting_semaphore<numThreads> enabled{0};
    std::atomic<int> activeCount{0};
    std::atomic<int> maxActive{0};

    std::vector<std::jthread> pool;
    for (int idx = 0; idx < numThreads; ++idx) {
        pool.emplace_back([&](std::stop_token st) {
            while (!st.stop_requested()) {
                if (enabled.try_acquire_for(100ms)) {
                    int cur = ++activeCount;
                    int m = maxActive.load();
                    while (cur > m) {
                        maxActive.compare_exchange_weak(m, cur);
                    }
                    syncOut() << "  thread active, count=" << cur << "\n";
                    std::this_thread::sleep_for(50ms);
                    --activeCount;
                    enabled.release();
                }
            }
        });
    }

    // 启用2个并发
    syncOut() << "  enabling 2 concurrent threads\n";
    enabled.release(2);
    std::this_thread::sleep_for(200ms);

    // 再启用2个
    syncOut() << "  enabling 2 more\n";
    enabled.release(2);
    std::this_thread::sleep_for(200ms);

    syncOut() << "  max active: " << maxActive.load() << "\n";
    syncOut() << "  semaphore::max(): " << decltype(enabled)::max() << "\n";

    std::cout << "\n";
}

// --- 13.2.2 二元信号量 ---
void demo_binary_semaphore() {
    std::cout << "--- 13.2.2 二元信号量 ---\n";

    int sharedData{0};
    std::binary_semaphore dataReady{0};
    std::binary_semaphore dataDone{0};

    std::jthread process{[&](std::stop_token st) {
        while (!st.stop_requested()) {
            if (dataReady.try_acquire_for(200ms)) {
                int data = sharedData;
                syncOut() << "  process: read " << data << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(30 * data));
                syncOut() << "  process: done\n";
                dataDone.release();
            }
        }
    }};

    for (int i = 0; i < 5; ++i) {
        syncOut() << "  main: store " << i << "\n";
        sharedData = i;
        dataReady.release();
        dataDone.acquire();
        syncOut() << "  main: processing done\n";
    }

    std::cout << "\n";
}

// --- 13.3.1 原子引用 (std::atomic_ref) ---
void demo_atomic_ref() {
    std::cout << "--- 13.3.1 原子引用 (std::atomic_ref) ---\n";

    // 普通数组, 非原子初始化
    std::array<int, 100> values;
    std::fill_n(values.begin(), values.size(), 1000);

    std::stop_source allStopSource;
    std::stop_token allStopToken{allStopSource.get_token()};

    std::vector<std::jthread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&values](std::stop_token st) {
            std::mt19937 eng{std::random_device{}()};
            std::uniform_int_distribution distr{0, int(values.size() - 1)};
            while (!st.stop_requested()) {
                int idx = distr(eng);
                std::atomic_ref val{values[idx]};  // 临时原子访问
                --val;
            }
        }, allStopToken);
    }

    std::this_thread::sleep_for(100ms);
    allStopSource.request_stop();

    // 统计结果
    int zeroCount = 0;
    for (auto v : values) {
        if (v <= 0) ++zeroCount;
    }
    syncOut() << "  values <= 0: " << zeroCount << "/" << values.size() << "\n";

    // atomic_ref 特性
    int x = 42;
    std::atomic_ref<int> aref{x};
    syncOut() << "  atomic_ref value: " << aref.load() << "\n";
    aref.store(100);
    syncOut() << "  after store(100): " << x << "\n";
    syncOut() << "  is_lock_free: " << aref.is_lock_free() << "\n";
    syncOut() << "  is_always_lock_free: "
        << std::atomic_ref<int>::is_always_lock_free << "\n";

    // required_alignment
    syncOut() << "  required_alignment: "
        << std::atomic_ref<int>::required_alignment << "\n";

    // const atomic_ref 仍可修改被引用对象
    const std::atomic_ref<int> caref{x};
    caref = 200;  // 合法
    syncOut() << "  const atomic_ref assign: " << x << "\n";

    std::cout << "\n";
}

// --- 13.3.2 原子共享指针 ---
void demo_atomic_shared_ptr() {
    std::cout << "--- 13.3.2 原子共享指针 ---\n";

    // 无锁链表头节点
    struct Node {
        std::string val;
        std::shared_ptr<Node> next;
    };

    std::atomic<std::shared_ptr<Node>> head;

    auto insert = [&](const std::string& v) {
        auto p = std::make_shared<Node>();
        p->val = v;
        p->next = head.load();
        while (!head.compare_exchange_weak(p->next, p))
            ;
    };

    // 多线程并发插入
    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&](std::stop_token) {
                for (auto s : {"hi ", "hey ", "ho ", "last "}) {
                    insert(std::to_string(i) + s);
                    std::this_thread::sleep_for(1ms);
                }
            });
        }
    }

    // 打印链表(前10个)
    syncOut() << "  list head -> ";
    int count = 0;
    for (auto p = head.load(); p && count < 10; p = p->next, ++count) {
        syncOut() << p->val << " -> ";
    }
    syncOut() << "...\n";

    // 原子弱指针
    {
        std::atomic<std::weak_ptr<int>> pShared;
        std::atomic<bool> done{false};

        std::jthread updates{[&] {
            for (int i = 0; i < 5; ++i) {
                auto sp = std::make_shared<int>(i);
                pShared.store(sp);
                std::this_thread::sleep_for(50ms);
            }
            done.store(true);
        }};

        int readCount = 0;
        while (!done.load()) {
            if (auto sp = pShared.load().lock()) {
                syncOut() << "  weak_ptr read: " << *sp << "\n";
                ++readCount;
            }
            std::this_thread::sleep_for(30ms);
        }
        syncOut() << "  total reads: " << readCount << "\n";
    }

    std::cout << "\n";
}

// --- 13.3.3 原子浮点型 ---
void demo_atomic_float() {
    std::cout << "--- 13.3.3 原子浮点型 ---\n";

    // C++20: atomic<float/double> 支持 fetch_add/fetch_sub
    std::atomic<double> ad{0.0};
    ad += 10.5;
    ad += 20.3;
    syncOut() << "  atomic<double>: " << ad.load() << "\n";

    auto old = ad.fetch_add(5.0);
    syncOut() << "  fetch_add(5.0): old=" << old << " new=" << ad.load() << "\n";

    old = ad.fetch_sub(3.0);
    syncOut() << "  fetch_sub(3.0): old=" << old << " new=" << ad.load() << "\n";

    std::cout << "\n";
}

// --- 13.3.4 原子wait/notify ---
void demo_atomic_wait_notify() {
    std::cout << "--- 13.3.4 原子wait/notify ---\n";

    // 基本用法: wait直到值变化
    std::atomic<int> aVal{0};

    std::jthread tRead{[&](std::stop_token) {
        int lastX = aVal.load();
        while (lastX >= 0) {
            aVal.wait(lastX);  // 阻塞直到值不再是lastX
            lastX = aVal.load();
            if (lastX >= 0) {
                syncOut() << "  reader: x changed to " << lastX << "\n";
            }
        }
        syncOut() << "  reader done\n";
    }};

    std::jthread tWrite{[&](std::stop_token) {
        for (int newVal : {17, 34, 3, 42, -1}) {
            std::this_thread::sleep_for(50ms);
            aVal = newVal;
            aVal.notify_all();
        }
    }};

    std::cout << "\n";
}

// --- 13.3.5 atomic_flag扩展 ---
void demo_atomic_flag() {
    std::cout << "--- 13.3.5 atomic_flag扩展 ---\n";

    std::atomic_flag flag{};

    syncOut() << "  initial test: " << flag.test() << "\n";
    flag.test_and_set();
    syncOut() << "  after set: " << flag.test() << "\n";
    flag.clear();
    syncOut() << "  after clear: " << flag.test() << "\n";

    // 自旋锁示例
    std::atomic_flag spinLock{};
    int counter{0};

    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < 5; ++i) {
            threads.emplace_back([&](std::stop_token) {
                for (int j = 0; j < 1000; ++j) {
                    while (spinLock.test_and_set(std::memory_order_acquire))
                        ;
                    ++counter;
                    spinLock.clear(std::memory_order_release);
                }
            });
        }
    }

    syncOut() << "  spinlock counter: " << counter << " (expected 5000)\n";

    std::cout << "\n";
}

// --- 13.4 同步输出流 ---
void demo_syncstream() {
    std::cout << "--- 13.4 同步输出流 ---\n";

    // 13.4.1 无同步的问题(注释演示)
    syncOut() << "  无同步时多线程输出会交错\n";

    // 13.4.2 使用osyncstream
    auto squareRoot = [](int num) {
        for (int i = 0; i < num; ++i) {
            std::osyncstream{std::cout} << "  sqrt(" << i
                << ") = " << std::sqrt(i) << "\n";
        }
    };

    {
        std::jthread t1{[&] { squareRoot(4); }};
        std::jthread t2{[&] { squareRoot(4); }};
    }

    // flush_emit操纵符
    syncOut() << "  flush_emit: 可以在析构前刷新\n";
    {
        std::osyncstream syncStrm{std::cout};
        syncStrm << "  这行在析构时输出\n";
        syncStrm << "  这行也是" << std::flush_emit;  // 立即输出
        syncStrm << "  这行等析构\n";
    }

    // 13.4.3 对文件使用同步输出流
    {
        std::ofstream fs{"concurrency_out.txt"};
        if (fs) {
            auto fileWriter = [&fs](int id, int num) {
                for (int i = 0; i < num; ++i) {
                    std::osyncstream syncStrm{fs};
                    syncStrm << "thread " << id << ": sqrt("
                        << i << ") = " << std::sqrt(i) << "\n" << std::flush_emit;
                }
            };

            std::jthread t1{fileWriter, 1, 3};
            std::jthread t2{fileWriter, 2, 3};
            syncOut() << "  文件输出已写入 concurrency_out.txt\n";
        }
    }

    std::cout << "\n";
}

// ============================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== 第13章: 并发特性 ===\n\n";

    demo_latch();
    demo_barrier();
    demo_counting_semaphore();
    demo_binary_semaphore();
    demo_atomic_ref();
    demo_atomic_shared_ptr();
    demo_atomic_float();
    demo_atomic_wait_notify();
    demo_atomic_flag();
    demo_syncstream();

    return 0;
}
