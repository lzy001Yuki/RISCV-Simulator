#include"processor/cpu.hpp"

int main() {
    //freopen("../testcases/hanoi.data", "r", stdin);
    //freopen("../testcases/pi.data", "r", stdin);
    //freopen("../sample/sample.data", "r", stdin);
    //freopen("decode.txt", "w", stdout);
    //freopen("d.txt", "w", stdout);
    //freopen("order.txt", "w", stdout);
    CPU cpu;
    cpu.read();
    cpu.work();
    return 0;
}
