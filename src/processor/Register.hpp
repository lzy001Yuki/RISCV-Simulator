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
    void print() {
        std::cout<<"Register Status-----------------\n";
        for (int i = 0; i < REGSIZE / 2; i++) {
            std::cout<<"reg"<<i*2<<"\'s val:\t"<<Reg[i*2].val;
            //<<"\tlabel:\t"<<Reg[i*2].label<<'\t';
            std::cout<<"reg"<<i*2+1<<"\'s val:\t"<<Reg[i*2+1].val;
            //<<"\tlabel:\t"<<Reg[i*2+1].label<<'\n';
        }
    }
};
#endif //RISCV_SIMULATOR_REGISTER_HPP
