#ifndef RISCV_SIMULATOR_COMMONDATABUS_HPP
#define RISCV_SIMULATOR_COMMONDATABUS_HPP
#include<iostream>
#include<exception>
#include<utility>
#include"../../src/utils/Type.hpp"
const int CDBSIZE = 4;
class CDB{
private:
    struct busNode{
        bool busy = false;
        int label = 0; // rob
        u32 val = 0;
    };
    //busNode bus[CDBSIZE];
public:
    /*
    void addBus(int label, u32 &val) {
        int index = 0;
        while (bus[index].busy) index++;
        if (index == CDBSIZE) throw std::exception();
        bus[index] = {true, label, val};
    }

    std::pair<bool, u32> getVal(int label) const {
        for (int i = 0; i < CDBSIZE; i++) {
            if (bus[i].label == label && bus[i].busy) return {true, bus[i].val};
        }
        return {false, -1};
    }

    void clearBus() {
        for (int i = 0; i < CDBSIZE; i++) bus[i].busy = false;
    }*/
    busNode bus;
    void broadcast(int label, u32 &val) {
        bus = (busNode){true, label, val};
        if (label == 30) {
            int y = 2;
        }
    }

    void flushCDB() {
        bus = (busNode){false, 0, 0};
    }

};
#endif //RISCV_SIMULATOR_COMMONDATABUS_HPP
