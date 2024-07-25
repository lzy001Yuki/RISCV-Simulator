#ifndef RISCV_SIMULATOR_OPQUEUE_H
#define RISCV_SIMULATOR_OPQUEUE_H
#include"../utils/Type.hpp"
#include"../utils/queue.hpp"
#include"decode.h"
class opNode{
public:
    u32 pc = 0;
    Decode decode;
    bool jump = false;
    opNode() = default;
    explicit opNode(u32 &p, Decode &d, bool j) : pc(p), decode(d), jump(j){}
};
const int OPSIZE = 64;
class opQueue{
public:
    bool stall = false;
    bool end = false;
    Queue<opNode, OPSIZE> opBuffer;

    opQueue() = default;
    void enQueue(u32 &pc, Decode &decode, bool flag);
    void deQueue();
    bool full();
    opNode front();
    void flush();
};

void opQueue::enQueue(u32 &pc, Decode &decode, bool flag) {
    opNode op(pc, decode, flag);
    opBuffer.push(op);
}

void opQueue::deQueue() {opBuffer.pop();}

bool opQueue::full() {return opBuffer.full();}

opNode opQueue::front() {return opBuffer.front();}

void opQueue::flush() {
    stall = false;
    end = false;
    opBuffer.clear();
}
#endif //RISCV_SIMULATOR_OPQUEUE_H
