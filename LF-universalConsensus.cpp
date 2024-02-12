#include <cassert>
#include <chrono>
#include <iostream>
#include <queue>
#include <random>
#include <stack>
#include <string>
#include <thread>
#include <vector>

#define CHECK 0

enum SynchronisedOutput { IO_Lock, IO_Unlock };

inline std::ostream &operator<<(std::ostream &os, SynchronisedOutput so) {
    static std::mutex mutex;

    if (IO_Lock == so)
        mutex.lock();
    else if (IO_Unlock == so)
        mutex.unlock();
    return os;
}

#define sync_os(Os) (Os) << IO_Lock
#define sync_cout sync_os(std::cerr)
#define sync_endl '\n' << IO_Unlock

struct Invocation {
    std::string op;
    int val;
    int whoInvoked;
};

struct SeqStack {
    std::stack<int> st;
    bool dbg = false;
    void apply(Invocation invoc) {
        std::string op = invoc.op;
        int t = invoc.whoInvoked;
        if (op == "PUSH") {
            st.push(invoc.val);
            if (dbg) {
                std::cout << "Thread " << t << " called " << invoc.op
                          << " on value " << invoc.val << "\n";
            }
            return;
        }
        assert(op == "POP");
        if (op == "POP") {
            bool flag = true;
            int z = -1;
            if (!st.empty()) {
                z = st.top();
                st.pop();
                flag = false;
            }
            if (dbg) {
                if (flag)
                    std::cout << "Thread " << t << " called " << invoc.op
                              << " and got "
                              << "EMPTY"
                              << "\n";
                else
                    std::cout << "Thread " << t << " called " << invoc.op
                              << " and got " << z << "\n";
            }
        }
    }
};

struct SeqQueue {
    std::queue<int> q;
    bool dbg = false;
    void apply(Invocation invoc) {
        std::string op = invoc.op;
        int t = invoc.whoInvoked;
        if (op == "PUSH") {
            q.push(invoc.val);
            if (dbg) {
                std::cout << "Thread " << t << " called " << invoc.op
                          << " on value " << invoc.val << "\n";
            }
            return;
        }
        assert(op == "POP");
        if (op == "POP") {
            bool flag = true;
            int z = -1;
            if (!q.empty()) {
                z = q.front();
                q.pop();
                flag = false;
            }
            if (dbg) {
                if (flag)
                    std::cout << "Thread " << t << " called " << invoc.op
                              << " and got "
                              << "EMPTY"
                              << "\n";
                else
                    std::cout << "Thread " << t << " called " << invoc.op
                              << " and got " << z << "\n";
            }
        }
    }
};

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

struct Node {
    Invocation invoc;
    Node *next;
    int seq;
    Node *a;

    Node() : next(nullptr), seq(0), a(this) {}
    Node(Invocation invoc) : Node() { this->invoc = std::move(invoc); }

    Node *consensus(Node *candidate) {
        if (next != nullptr) {
            return next;
        }
        Node *old = this;
        Node *chosen = nullptr;

        if (CAS(&a, &old, &candidate) == 0) {
            chosen = candidate;
        } else {
            chosen = a;
        }
        return chosen;
    }

    static Node *max(const std::vector<Node *> &array) {
        Node *max = array[0];
        for (size_t i = 1; i < array.size(); i++) {
            if ((max->seq) < (array[i]->seq))
                max = array[i];
        }
        return max;
    }
};

struct LFUniversal {
    int numThreads;
    std::vector<Node *> head;
    Node *tail;

    LFUniversal(int numThreads) : numThreads(numThreads), tail(new Node) {
        tail->seq = 1;
        head.resize(numThreads, tail);
    }

    void apply(Invocation invoc, int id) {
        Node *prefer = new Node(invoc);

        while (prefer->seq == 0) {
            Node *before = Node::max(head);

            if (CHECK == 1) {
                sync_cout << prefer->invoc.whoInvoked
                          << " is trying to be seq. number " << before->seq + 1
                          << sync_endl;
            }

            Node *after = before->consensus(prefer);
            before->next = after;
            after->seq = before->seq + 1;
            head[id] = after;
        }
    }
};

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cout << "ENTER CORRECT ARGUMENTS\n";
        exit(0);
    }
    int n = atoi(argv[1]);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(-100, 101);

    std::cerr << "\033[91m"
              << "[LOCK-FREE UNIVERSAL CONSENSUS]"
              << "\033[0m" << std::endl;

    std::vector<std::thread> threads;
    LFUniversal *foo = new LFUniversal(n);

    auto work = [&](Invocation invoc, int id) {
        auto sleep = 50;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        foo->apply(invoc, id);
    };

    for (int i = 0; i < n; i++) {
        Invocation invoc;
        invoc.whoInvoked = i;
        auto a = distrib(gen);
        invoc.op = "POP";
        if (a % 2) {
            invoc.op = "PUSH";
            invoc.val = distrib(gen);
        }
        threads.emplace_back(work, invoc, i);
    }

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::cout << "\033[92m"
              << "[FINAL QUEUE LOG]"
              << "\033[0m" << std::endl;

    SeqQueue *myobj1 = new SeqQueue;
    myobj1->dbg = true;
    Node *current1 = foo->tail->next;
    while (current1 != NULL) {
        myobj1->apply(current1->invoc);
        current1 = current1->next;
    }

    std::cout << "\033[92m"
              << "[FINAL STACK LOG]"
              << "\033[0m" << std::endl;

    SeqStack *myobj2 = new SeqStack;
    myobj2->dbg = true;
    Node *current2 = foo->tail->next;
    while (current2 != NULL) {
        myobj2->apply(current2->invoc);
        current2 = current2->next;
    }

    return 0;
}