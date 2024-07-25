#ifndef RISCV_SIMULATOR_MEMORY_HPP
#define RISCV_SIMULATOR_MEMORY_HPP
#include<iostream>
#include"../utils/Type.hpp"
const int Size = 2e6;
class Memory{
private:
    u8 mem[Size] = {0};
public:
    // byte u8
    // halfWord  u16
    // Word u32
    template<class T, int len>
    void writeAddr(u32& addr, T input) {
        for (int i = 0; i < len / 8; i++) {
            mem[addr + i] = input & 11111111;
            input = input >> 8;
        }
    }

    template <class T, int len>
    T getAddr(u32& addr) {
        // reverse
        T ans;
        for (int i = 0; i < len / 8; i++) {
            ans += mem[addr + i] << (8 * i);
        }
        return ans;
    }
};
#endif //RISCV_SIMULATOR_MEMORY_HPP
