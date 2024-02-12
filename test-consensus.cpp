#include <cassert>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

template <typename T>
static inline int CAS(T *ptr, T *oldValPtr, T *newValPtr) {
    int ret;
    int res;
    T oldVal = *oldValPtr;
    T newVal = *newValPtr;
    __asm__ __volatile__("1:\n"
                         "ldaxr %w0, [%2]\n"
                         "mov %w1, %w0\n"
                         "cmp %w0, %w3\n"
                         "b.ne 2f\n"
                         "mov %w0, %w4\n"
                         "stlxr %w1, %w0, [%2]\n"
                         "cbnz %w1, 1b\n"
                         "2:\n"
                         : "=&r"(ret), "=&r"(res), "+r"(ptr)
                         : "r"(oldVal), "r"(newVal)
                         : "cc", "memory");
    return ret;
}

int main() {

    int num = 10;
    int original = 0;
    std::vector<int> final(num);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(50, 10000);

    auto work = [&](int id, int proposed) {
        auto sleep = 100;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        int chosen = 0;
        int old = 0;
        if (CAS(&original, &old, &proposed) == 0) {
            chosen = proposed;
        } else {
            chosen = original;
        }
        assert(chosen != 0);
        final[id] = chosen;
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < num; i++) {
        int proposed = distrib(gen);
        std::cout << i << ' ' << proposed << '\n';
        threads.emplace_back(work, i, proposed);
    }

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    for (int i = 0; i < num; i++) {
        std::cout << "Thread " << i << " chose value = " << final[i];
        std::cout << std::endl;
    }

    return 0;
}