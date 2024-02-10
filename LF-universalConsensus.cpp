#include <cassert>
#include <chrono>
#include <iostream>
#include <queue>
#include <random>
#include <stack>
#include <string>
#include <thread>
#include <vector>

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

struct Node {
    Invocation invoc;
    Node *next;
    int seq;
    std::atomic<Node *> a;

    Node() : next(nullptr), seq(0), a(nullptr) {}
    Node(Invocation invoc) : Node() { this->invoc = std::move(invoc); }

    Node *consensus(Node *candidate) {
        if (next != nullptr) {
            return next;
        }
        Node *old = nullptr;
        Node *chosen = nullptr;

        if (a.compare_exchange_strong(old, candidate)) {
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
    std::uniform_int_distribution<> distrib(1, 100);

    std::cerr << "\033[91m"
              << "[LOCK-FREE UNIVERSAL CONSENSUS]"
              << "\033[0m" << std::endl;

    std::vector<std::thread> threads;
    LFUniversal *foo = new LFUniversal(n);

    auto work = [&](Invocation invoc, int id) {
        auto d = distrib(gen);
        std::this_thread::sleep_for(std::chrono::milliseconds(d));
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