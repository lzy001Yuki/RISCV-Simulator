#ifndef RISCV_SIMULATOR_REORDER_H
#define RISCV_SIMULATOR_REORDER_H
#include"decode.h"
#include"CommonDataBus.hpp"
#include"opQueue.h"
#include"Register.hpp"

/*
 * advantage of ROB:
 * convenient to flush;
 * because it ensures that the value of a register isn't changed
 * at the time of one executed instruction
 */

class robNode {
    friend class CPU;
    friend class ReorderBuffer;
    friend class ReservationStation;
    friend class LoadStoreBuffer;
private:
    bool ready = false;
    int label = 0;
    bool jump = false;
    u32 nowPC = 0;
    u32 dest = 0; // PC or rd
    u32 res = 0; // imm or result
    Decode decode;
    // need type/orderType/code/rd from class Decode
public:
    robNode() = default;
    explicit robNode(Decode &de, u32 &now, u32 &des, bool j) :decode(de), nowPC(now), dest(des), jump(j){}
    friend std::ostream& operator<<(std::ostream &os, const robNode &tmp);
};
std::ostream &operator<<(std::ostream &os, const robNode &tmp) {
    os<<tmp.label<<'\t'<<tmp.decode;
    os<<"nowPC\t"<<tmp.nowPC<<"\tdest\t"<<tmp.dest<<"\tres\t"<<tmp.res<<'\n';
    return os;
}

class ReorderBuffer{
public:
    int tag = 1;
    Queue<robNode, 32> robBuffer;
    bool empty();
    bool full();
    void flushRob();
    void addRob(robNode &rob, Register &r);
    bool Issue(Decode &input, u32 &nowPC, u32 &dest, bool pred, Register &r);
    void fetchData(CDB &cdb);
    void commit(Register &r, CDB &cdb, robNode &tmp);
    void pop();
    robNode front();
    void print();
};

bool ReorderBuffer::empty() {return robBuffer.empty();}
bool ReorderBuffer::full() {return robBuffer.full();}
void ReorderBuffer::addRob(robNode &rob, Register &r) {
    rob.label = tag;
    r.Reg[rob.decode.rd].label = tag;
    tag++;
    robBuffer.push(rob);
}
void ReorderBuffer::flushRob() {
    tag = 1;
    robBuffer.clear();
}
bool ReorderBuffer::Issue(Decode &input, u32 &nowPC, u32 &dest, bool pred, Register &r) {
    if (robBuffer.full()) return false;
    robNode newNode(input, nowPC, dest, pred);
    // dest is not necessarily the destination register
    // can try r.Reg[newNode.dest].label
    if (newNode.decode.rd) r.Reg[newNode.decode.rd].label = tag;
    newNode.label = tag; // rename the register
    tag++;
    robBuffer.push(newNode);
    return true;
}

void ReorderBuffer::fetchData(CDB &cdb) {
    if (!cdb.bus.busy) return;
    robBuffer[cdb.bus.label].ready = true;
    robBuffer[cdb.bus.label].res = cdb.bus.val;
}


void ReorderBuffer::commit(Register &r, CDB &cdb, robNode &tmp) {
    if (tmp.dest) {
        r.Reg[tmp.dest].val = tmp.res;
        cdb.broadcast(tmp.label, tmp.res);
        // eliminate dependency
        if (tmp.label == r.Reg[tmp.dest].label) r.Reg[tmp.dest].label = 0;
    }
}

void ReorderBuffer::pop() {
    robBuffer.pop();
}
robNode ReorderBuffer::front() {return robBuffer.front();}
void ReorderBuffer::print() {
    std::cout<<"ReorderBuffer Status ----------------------------\n";
    robBuffer.print();
}
#endif //RISCV_SIMULATOR_REORDER_H
