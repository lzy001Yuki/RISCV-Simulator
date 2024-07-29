#ifndef RISCV_SIMULATOR_COMMONDATABUS_HPP
#define RISCV_SIMULATOR_COMMONDATABUS_HPP
#include<iostream>
#include<exception>
#include<utility>
#include"../../src/utils/Type.hpp"
class CDB{
private:
    struct busNode{
        bool busy = false;
        int label = 0; // rob
        u32 val = 0;
    };
public:
    busNode bus;
    void broadcast(int label, u32 &val) {
        bus = (busNode){true, label, val};
    }

    void flushCDB() {
        bus = (busNode){false, 0, 0};
    }

};
#endif //RISCV_SIMULATOR_COMMONDATABUS_HPP
