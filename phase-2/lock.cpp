// Build: c++ -std=c++17 -pthread lock.cpp -o lock
// Run:   ./lock

#include <iostream>
#include <mutex>
#include <thread>

int main() {
    int counter = 0;
    std::mutex mtx;

    auto increment = [&]() {
        std::lock_guard<std::mutex> guard(mtx); // Lock here.
        ++counter;
    }; // Automatically unlock when guard leaves scope.

    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();

    std::cout << counter << '\n'; // Always prints 2.
}
