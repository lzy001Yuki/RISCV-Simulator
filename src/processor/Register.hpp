#ifndef RISCV_SIMULATOR_REGISTER_HPP
#define RISCV_SIMULATOR_REGISTER_HPP
#include"../utils/Type.hpp"
const int REGSIZE = 32;
class Register{
private:
    struct reg{
        u32 val = 0;
        int label = 0;
    };
public:
    reg Reg[REGSIZE];
    void flushReg() {
        for (int i = 0; i < REGSIZE; i++) Reg[i].label = 0;
    }
};
#endif //RISCV_SIMULATOR_REGISTER_HPP
