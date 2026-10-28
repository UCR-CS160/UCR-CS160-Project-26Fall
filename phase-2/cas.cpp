// Build: c++ -std=c++20 -pthread cas.cpp -o cas
// Run:   ./cas

#include <atomic>
#include <iostream>
#include <thread>

int main() {
    int value = 0;
    bool success[2] = {};
    int expected_after[2] = {};

    auto try_update = [&](int tid) {
        std::atomic_ref<int> shared(value);
        int expected = 0;

        // If value equals expected, set it to 42 and return true.
        // Otherwise, copy value into expected and return false.
        success[tid] = shared.compare_exchange_strong(expected, 42);
        expected_after[tid] = expected;
    };

    std::thread t1(try_update, 0);
    std::thread t2(try_update, 1);
    t1.join();
    t2.join();

    // Read the results after both workers finish.
    for (int tid = 0; tid < 2; ++tid) {
        std::cout << "Thread " << tid
                  << ": success=" << std::boolalpha << success[tid]
                  << ", expected after CAS=" << expected_after[tid] << '\n';
    }
    std::cout << "Value: " << value << '\n'; // Always 42.
    std::cout << "Successful updates: " << success[0] + success[1]
              << '\n'; // Always 1.
}
