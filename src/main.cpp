#include"processor/cpu.hpp"
#include <cstdio>

int main() {
    //freopen("../testcases/tak.data", "r", stdin);
    //freopen("../testcases/naive.data", "r", stdin);
    //freopen("../sample/sample.data", "r", stdin);
    //freopen("decode.txt", "w", stdout);
    //freopen("decode.txt", "w", stdout);
    CPU cpu;
    cpu.read();
    cpu.work();
    return 0;
}
