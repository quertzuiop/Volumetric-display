#include <chrono>

using namespace std;

int main() {
    auto start = chrono::steady_clock::now();
    auto a = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start).count();
    for (int i = 0; i < 48000; i++) {
        a = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start).count();
    }
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    printf("Total time for 10000 calls: %lld microseconds\n", duration);
}