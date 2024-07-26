#include"processor/cpu.hpp"
#include <cstdio>

int main() {
    freopen("../testcases/sample.data", "r", stdin);
    //freopen("decode.txt", "w", stdout);
    freopen("process.txt", "w", stdout);
    CPU cpu;
    cpu.read();
    cpu.work();
    return 0;
}
