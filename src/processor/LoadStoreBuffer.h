#ifndef RISCV_SIMULATOR_LOADSTOREBUFFER_H
#define RISCV_SIMULATOR_LOADSTOREBUFFER_H
#include"ReservationStation.h"
#include"Memory.hpp"
const int LSBSIZE = 32;
enum nodeType{load, store};
class lsbNode: protected rsNode{
    friend class LoadStoreBuffer;
private:
   // Vj, Vk for offset addr(load)
   // Vj for addr, Vk for addr(store)
   int time = 0;
   u32 addr = 0;
   nodeType los;
};

class timeCnt{
    friend class LoadStoreBuffer;
    friend class CPU;
private:
    int loadIndex = 0;
    int storeIndex = 0;
    int loadTime = 0;
    int storeTime = 0;
public:
    void zero();
};

class LoadStoreBuffer{
    friend class CPU;
private:
    lsbNode lsb[LSBSIZE];
    timeCnt cur;
public:
    LoadStoreBuffer() = default;
    bool full();
    void flush();
    void Issue(ReorderBuffer &rob, Register &r, Decode &decode, int t);
    void Execute(ReorderBuffer &rob);
    void Load(Memory &mem);
    void Commit(robNode &rn);
    void Store(Memory &mem, ReorderBuffer &rob);
    bool Write(CDB &cdb);
    void fetchData(CDB &cdb);
};

void timeCnt::zero() {
    loadIndex = 0;
    storeIndex = 0;
    loadTime = 0;
    storeTime = 0;
}
bool LoadStoreBuffer::full(){
    bool isFull = true;
    for (auto &it :lsb) {
        if (!it.busy) {
            isFull = false;
            break;
        }
    }
    return isFull;
}
void LoadStoreBuffer::flush() {
    cur.zero();
    for (int i = 0; i < LSBSIZE; i++) {
        lsb[i] = lsbNode();
    }
}

void LoadStoreBuffer::Issue(ReorderBuffer &rob, Register &r, Decode &decode, int t) {
    int index = 0;
    while (lsb[index].busy) index++;
    lsb[index].label = rob.tag;
    lsb[index].orderType = decode.orderType;
    lsb[index].status = issue;
    lsb[index].time = t;
    if (decode.type == 'L') {
        lsb[index].los = load;
        int label1 = r.Reg[decode.rs1].label;
        if (label1) {
            if (rob.robBuffer[label1].ready) lsb[index].V1 = rob.robBuffer[label1].res;
            else lsb[index].Q1 = label1;
        } else lsb[index].V1 = rob.robBuffer[decode.rs1].res;
        lsb[index].Q2 = 0;
        lsb[index].V2 = decode.imm;
    } else {
        lsb[index].los = store;
        lsb[index].V1 = decode.imm;
        int label1 = r.Reg[decode.rs1].label;
        if (label1) {
            if (rob.robBuffer[label1].ready) lsb[index].V1 += rob.robBuffer[label1].res;
            else lsb[index].Q1 = label1;
        } else lsb[index].V1 += rob.robBuffer[decode.rs1].res;
        int label2 = r.Reg[decode.rs2].label;
        if (label2) {
            if (rob.robBuffer[label2].ready) lsb[index].V2 = rob.robBuffer[label2].res;
            else lsb[index].Q2 = label2;
        } else lsb[index].V2 = rob.robBuffer[decode.rs2].res;
    }
}

void LoadStoreBuffer::Execute(ReorderBuffer &rob) {
    for (int i = 0; i < LSBSIZE; i++) {
        if (lsb[i].status == issue && !lsb[i].Q1 && !lsb[i].Q2) {
            if (lsb[i].los == load) {
                lsb[i].addr = lsb[i].V1 + lsb[i].V2;
                lsb[i].status = execute;
            } else {
                lsb[i].addr = lsb[i].V1;
                lsb[i].res = lsb[i].V2;
                rob.robBuffer[lsb[i].label].dest = lsb[i].V1;
                lsb[i].status = execute;
            }
            break;
        }
    }
}

void LoadStoreBuffer::Load(Memory &mem) {
    bool update = false;
    if (cur.loadTime) {
        cur.loadTime--;
        if (!cur.loadTime) {
            lsb[cur.loadIndex].status = write;
            if (lsb[cur.loadIndex].orderType == LB) lsb[cur.loadIndex].res = Decode::extension((u32)mem.getAddr<u8, 8>(lsb[cur.loadIndex].addr), 8);
            else if (lsb[cur.loadIndex].orderType == LH) lsb[cur.loadIndex].res = Decode::extension((u32)mem.getAddr<u16, 16>(lsb[cur.loadIndex].addr), 16);
            else if (lsb[cur.loadIndex].orderType == LW) lsb[cur.loadIndex].res = Decode::extension((u32)mem.getAddr<u32, 32>(lsb[cur.loadIndex].addr),32);
            else if (lsb[cur.loadIndex].orderType == LBU) lsb[cur.loadIndex].res = (u32)mem.getAddr<u8, 8>(lsb[cur.loadIndex].addr);
            else if (lsb[cur.loadIndex].orderType == LHU) lsb[cur.loadIndex].res = (u32)mem.getAddr<u16, 16>(lsb[cur.loadIndex].addr);
            cur.loadIndex = 0;
            update = true;
        } else {
            // check storeQueue
            for (auto &it: lsb) {
                if (it.los == store && it.busy) {
                    if (it.status == execute || it.status == write) {
                        if (it.addr == lsb[cur.loadIndex].addr && it.time > lsb[cur.loadIndex].time) {
                            lsb[cur.loadIndex].res = it.res;
                            lsb[cur.loadIndex].status = write;
                            cur.loadIndex = 0;
                            cur.loadTime = 0;
                            update = true;
                        }
                    }
                }
            }
        }
    } else update = true;
    if (update) {
        for (int i = 0; i < LSBSIZE; i++) {
            if (lsb[i].los == load && lsb[i].status == execute) {
                cur.loadIndex = i;
                cur.loadTime = 3;
                break;
            }
        }
    }
}

bool LoadStoreBuffer::Write(CDB &cdb) {
    for (auto &it: lsb) {
        if (it.status == write && it.los == load) {
            cdb.broadcast(it.label, it.res);
            it.busy = false;
            return true;
        }
        if (it.status == execute && it.los == store) {
            cdb.broadcast(it.label, it.res);
            it.status = write;
            return true;
        }
    }
    return false;
}

// no commit until finished
void LoadStoreBuffer::Commit(robNode &rn) {
    for (int i = 0; i < LSBSIZE; i++) {
        if (rn.label == lsb[i].label && lsb[i].los == store) {
            lsb[i].status = commit;
            cur.storeIndex = i;
            cur.storeTime = 3;
        }
    }
}

void LoadStoreBuffer::Store(Memory &mem, ReorderBuffer &rob) {
    if (cur.storeTime) {
        cur.storeTime--;
        if (!cur.storeTime) {
            lsbNode curNode = lsb[cur.storeIndex];
            if (curNode.orderType == SB) mem.writeAddr<u8, 8>(curNode.addr, curNode.res & 11111111);
            else if (curNode.orderType == SH) mem.writeAddr<u16, 16>(curNode.addr, curNode.res & 0xffff);
            else if (curNode.orderType == SW) mem.writeAddr<u32, 32>(curNode.addr, curNode.res);
            lsb[cur.storeIndex].busy = false;
            cur.storeIndex = 0;
            rob.pop();
        }
    }
}


void LoadStoreBuffer::fetchData(CDB &cdb) {
    if (!cdb.bus.busy) return;
    for (auto &it: lsb) {
        if (it.status == issue) {
            if (it.Q1 == cdb.bus.label) {
                if (it.los == load) {
                    it.V1 = cdb.bus.val;
                    it.Q1 = 0;
                } else {
                    it.V1 += cdb.bus.val;
                    it.Q1 = 0;
                }
            } else if (it.Q2 == cdb.bus.label) {
                it.V2 = cdb.bus.val;
                it.Q2 = 0;
            }
        }
    }
}
#endif //RISCV_SIMULATOR_LOADSTOREBUFFER_H
