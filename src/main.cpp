#include"processor/cpu.hpp"

int main() {
    //freopen("../testcases/basicopt1.data", "r", stdin);
    //freopen("../testcases/pi.data", "r", stdin);
    //freopen("../sample/sample.data", "r", stdin);
    //freopen("decode.txt", "w", stdout);
    //freopen("d.txt", "w", stdout);
    CPU cpu;
    cpu.read();
    cpu.work();
    return 0;
}
