#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <stack>
#include <string>
#include <thread>
#include <vector>

std::mutex l;

// template<typename T>
struct Response {
    // return value
};

struct Invocation {
    // function pointer and arguments
    std::string op;
    int val;
    int whodid;
};

struct SeqObject {
    std::stack<int> st;
    int apply(Invocation invoc) {
        int t = invoc.whodid;
        if (invoc.op == "push") {
            st.push(invoc.val);
            l.lock();
            std::cout << "Thread " << t << " called " << invoc.op
                      << " on value " << invoc.val << "\n";
            l.unlock();
            return 0;
        }
        assert(invoc.op == "pop");
        if (invoc.op == "pop") {
            bool flag = true;
            int z = -1;
            if (!st.empty()) {
                z = st.top();
                st.pop();
                flag = false;
            }
            l.lock();
            if (flag)
                std::cout << "Thread " << t << " called " << invoc.op
                          << " and got "
                          << "EMPTY"
                          << "\n";
            else
                std::cout << "Thread " << t << " called " << invoc.op
                          << " and got " << z << "\n";
            l.unlock();
            return z;
        }

        assert(false);
        return 0;
    }
};

// we are running consensus on nodes of the linked list.
// The inputs to this consensus are references to the threads’ nodes, and the
// result is the unique winning node.

// [KIM] in case we have already ran
// consensus on a node, calling it again should give the cached result.

struct Node {
    Invocation invoc;
    Node *next;
    int seq;
    std::atomic<Node *> a;
    Node() {
        a = NULL;
        next = NULL;
    }
    Node(Invocation invoc) {
        a = NULL;
        next = NULL;
        this->invoc = invoc;
        seq = 0;
    }
    Node *consensus(Node *candidate) {
        if (next != NULL) {
            return next;
        }
        Node *old = NULL;
        Node *chosen = NULL;

        if (a.compare_exchange_strong(old, candidate)) {
            chosen = candidate;
        } else {
            chosen = a;
        }
        // next = chosen;

        return chosen;
    }
    static Node *max(std::vector<Node *> &array) {
        Node *max = array[0];
        for (int i = 1; i < (int)array.size(); i++) {
            if (max->seq < array[i]->seq)
                max = array[i];
        }
        return max;
    }
};

struct LFUniversal {
    int numThreads;
    std::vector<Node *> head; // vector of last node which each thread has seen.
    Node *tail;               // this is the initial node
    // ( tail -> -> -> -> ........ -> head ) : this is how our linked list of
    // function calls is growing
    LFUniversal(int numThreads) {
        this->numThreads = numThreads;
        tail = new Node; // setting up the base node
        tail->seq = 1;
        this->head.resize(numThreads);
        for (int i = 0; i < numThreads; i++) {
            head[i] = tail; // as of now each thread thinks that, 'tail' is the
                            // last node in linked list
        }
    }
    int apply(Invocation invoc, int id) {

        Node *prefer = new Node(invoc);

        while (prefer->seq == 0) {
            Node *before = Node::max(head);

            Node *after = before->consensus(prefer);

            before->next = after;
            after->seq = before->seq + 1;
            head[id] = after;
        }

        SeqObject *myobj = new SeqObject;
        Node *current = tail->next;
        while (current != prefer) {
            myobj->apply(current->invoc);
            current = current->next;
        }
        return myobj->apply(current->invoc);
    }
};

int main() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 100);

    std::cout << "[BEGIN]" << '\n';
    int n = 10;
    std::vector<std::jthread> threads;
    LFUniversal *foo = new LFUniversal(n);

    auto work = [&](Invocation invoc, int id) {
        auto d = distrib(gen);
        std::this_thread::sleep_for(std::chrono::milliseconds(d));
        foo->apply(invoc, id);
    };

    for (int i = 0; i < n; i++) {
        Invocation invoc;
        auto a = distrib(gen);
        invoc.op = "pop";
        if (a % 2)
            invoc.op = "push";
        invoc.val = distrib(gen);
        invoc.whodid = i;
        threads.emplace_back(work, invoc, i);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "\n\n {DEBUG MODE: FINAL STACK LOG} \n\n";

    SeqObject *myobj = new SeqObject;
    Node *current = foo->tail->next;
    while (current != NULL) {
        myobj->apply(current->invoc);
        current = current->next;
    }

    return 0;
}