#ifndef RISCV_SIMULATOR_LOADSTOREBUFFER_H
#define RISCV_SIMULATOR_LOADSTOREBUFFER_H
#include"ReservationStation.h"
#include"Memory.hpp"
const int LSBSIZE = 3;
enum nodeType{load, store};
class lsbNode: protected rsNode{
    friend class LoadStoreBuffer;
    friend class CPU;
private:
   // Vj, Vk for offset addr(load)
   // Vj for addr, Vk for addr(store)
   int time = 0;
   u32 addr = 0;
   nodeType los;
public:
    friend std::ostream &operator<<(std::ostream &os, const lsbNode &tmp);
};
std::ostream& operator<<(std::ostream& os, const lsbNode &tmp) {
    if (tmp.los == load) os<<"load\t";
    else os<<"store\t";
    os << "label\t"<<tmp.label<<'\t'<<
       "V1\t"<<tmp.V1<<"\tV2\t"<<tmp.V2<<"\tQ1\t"<<tmp.Q1<<"\tQ2\t"<<tmp.Q2<<
       "\tres\t"<<tmp.res<<'\t'<<"status\t"<<tmp.status<<"\ttime\t"<<tmp.time<<"\taddr\t"<<tmp.addr<<'\n';
    return os;
}

class timeCnt{
    friend class LoadStoreBuffer;
    friend class CPU;
private:
    int loadIndex = -1;
    int storeIndex = -1;
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
    bool Issue(ReorderBuffer &rob, Register &r, Decode &decode, int t);
    void Execute(ReorderBuffer &rob);
    void Load(Memory &mem);
    void Commit(robNode &rn);
    void Store(Memory &mem, ReorderBuffer &rob);
    bool Write(CDB &cdb);
    void fetchData(CDB &cdb);
    void print();
};

void timeCnt::zero() {
    loadIndex = -1;
    storeIndex = -1;
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

bool LoadStoreBuffer::Issue(ReorderBuffer &rob, Register &r, Decode &decode, int t) {
    int index = 0;
    while (lsb[index].busy) index++;
    if (index >= 8) return false;
    lsb[index].label = rob.tag;
    if (rob.tag == 32) {
        int y = 2;
    }
    lsb[index].orderType = decode.orderType;
    lsb[index].status = issue;
    lsb[index].busy = true;
    lsb[index].time = t;
    if (decode.type == 'L') {
        lsb[index].los = load;
        int label1 = r.Reg[decode.rs1].label;
        if (label1) {
            if (rob.robBuffer[label1].ready) lsb[index].V1 = rob.robBuffer[label1].res;
            else lsb[index].Q1 = label1;
        } else lsb[index].V1 = r.Reg[decode.rs1].val;
        lsb[index].Q2 = 0;
        lsb[index].V2 = decode.imm;
    } else {
        lsb[index].los = store;
        lsb[index].V1 = decode.imm;
        int label1 = r.Reg[decode.rs1].label;
        if (label1) {
            if (rob.robBuffer[label1].ready) lsb[index].V1 += rob.robBuffer[label1].res;
            else lsb[index].Q1 = label1;
        } else lsb[index].V1 += r.Reg[decode.rs1].val;
        int label2 = r.Reg[decode.rs2].label;
        if (label2) {
            if (rob.robBuffer[label2].ready) lsb[index].V2 = rob.robBuffer[label2].res;
            else lsb[index].Q2 = label2;
        } else lsb[index].V2 = r.Reg[decode.rs2].val;
    }
    return true;
}

void LoadStoreBuffer::Execute(ReorderBuffer &rob) {
    for (int i = 0; i < LSBSIZE; i++) {
        if (lsb[i].status == issue && !lsb[i].Q1 && !lsb[i].Q2 &&lsb[i].busy) {
            if (lsb[i].los == load) {
                lsb[i].addr = lsb[i].V1 + lsb[i].V2;
                lsb[i].status = execute;
                //std::cout<<"ExecLoad\t"<<lsb[i];
            } else {
                lsb[i].addr = lsb[i].V1;
                lsb[i].res = lsb[i].V2;
                rob.robBuffer[lsb[i].label].dest = lsb[i].V1;
                lsb[i].status = execute;
                //std::cout<<"ExecStore\t"<<lsb[i];
            }
            break;
        }
    }
}

void LoadStoreBuffer::Load(Memory &mem) {
    bool update = false;
    if (cur.loadTime) {
        if (lsb[cur.loadIndex].label == 1) {
            int y = 2;
        }
        cur.loadTime--;
        // should be the latest lsb.store
        int maxTime = 0;
        u32 ans = 0;
        for (auto &it: lsb) {
            if (it.los == store && it.busy) {
                if (it.status == execute || it.status == write || it.status == commit) {
                    if (it.addr == lsb[cur.loadIndex].addr && it.time <= lsb[cur.loadIndex].time) {
                        if (it.time > maxTime) {
                            maxTime = it.time;
                            ans = it.res;
                        }
                    }
                } else if (it.status == issue && !it.Q1 && !it.Q2) {
                    if (it.V1 == lsb[cur.loadIndex].addr && it.time <= lsb[cur.loadIndex].time) {
                        if (it.time > maxTime) {
                            maxTime = it.time;
                            ans = it.V2;
                        }
                    }
                }
            }
        }
        if (maxTime) {
            lsb[cur.loadIndex].res = ans;
            lsb[cur.loadIndex].status = write;
            if (lsb[cur.loadIndex].res == 45 && lsb[cur.loadIndex].orderType == LW) {
                int y = 2;
            }
            //std::cout << "Load through store\t" << lsb[cur.loadIndex];
            cur.loadIndex = -1;
            cur.loadTime = 0;
            update = true;
        }
        if (!update) {
            if (!cur.loadTime && cur.loadIndex != -1) {
                lsb[cur.loadIndex].status = write;
                //std::cout << "load\t" << lsb[cur.loadIndex];
                if (lsb[cur.loadIndex].orderType == LB)
                    lsb[cur.loadIndex].res = Decode::extension((u32) mem.getAddr<u8, 8>(lsb[cur.loadIndex].addr), 8);
                else if (lsb[cur.loadIndex].orderType == LH)
                    lsb[cur.loadIndex].res = Decode::extension((u32) mem.getAddr<u16, 16>(lsb[cur.loadIndex].addr), 16);
                else if (lsb[cur.loadIndex].orderType == LW) {
                    lsb[cur.loadIndex].res = Decode::extension((u32) mem.getAddr<u32, 32>(lsb[cur.loadIndex].addr), 32);
                } else if (lsb[cur.loadIndex].orderType == LBU)
                    lsb[cur.loadIndex].res = (u32) mem.getAddr<u8, 8>(lsb[cur.loadIndex].addr);
                else if (lsb[cur.loadIndex].orderType == LHU)
                    lsb[cur.loadIndex].res = (u32) mem.getAddr<u16, 16>(lsb[cur.loadIndex].addr);
                if (lsb[cur.loadIndex].res == 45 && lsb[cur.loadIndex].orderType == LW) {
                    int y = 2;
                }
                cur.loadIndex = -1;
                update = true;
            }
        }
    } else update = true;

    if (update) {
        for (int i = 0; i < LSBSIZE; i++) {
            if (lsb[i].los == load && lsb[i].status == execute) {
                //std::cout<<"newLoad\t"<<lsb[i];
                cur.loadIndex = i;
                cur.loadTime = 3;
                break;
            }
        }
    }
}

bool LoadStoreBuffer::Write(CDB &cdb) {
    for (auto &it: lsb) {
        if (it.status == write && it.los == load && it.busy) {
            //std::cout<<"writeLoad\t"<<it;
            cdb.broadcast(it.label, it.res);
            it.busy = false;
            return true;
        }
        if (it.status == execute && it.los == store) {
            //std::cout<<"writeStore\t"<<it;
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
        if (rn.label == lsb[i].label && lsb[i].los == store && lsb[i].busy) {
            lsb[i].status = commit;
            cur.storeIndex = i;
            cur.storeTime = 3;
            break;
        }
    }
}

void LoadStoreBuffer::Store(Memory &mem, ReorderBuffer &rob) {
    if (cur.storeTime > 0) {
        cur.storeTime--;
        if (!cur.storeTime) {
            lsbNode curNode = lsb[cur.storeIndex];
            if (curNode.orderType == SB) mem.writeAddr<u8, 8>(curNode.addr, curNode.res & 11111111);
            else if (curNode.orderType == SH) mem.writeAddr<u16, 16>(curNode.addr, curNode.res & 0xffff);
            else if (curNode.orderType == SW) {
                mem.writeAddr<u32, 32>(curNode.addr, curNode.res);
            }
            lsb[cur.storeIndex].busy = false;
            cur.storeIndex = -1;
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

void LoadStoreBuffer::print() {
    std::cout<<"LoadStoreBuffer Status --------------------------\n";
    std::cout<<"loadIndex\t"<<cur.loadIndex<<"\tloadTime\t"<<cur.loadTime<<"\tstoreIndex\t"
    <<cur.storeIndex<<"\tstoreTime\t"<<cur.storeTime<<'\n';
    for (int i = 0; i < LSBSIZE; i++) {
        if (lsb[i].busy) std::cout<<lsb[i];
    }
}
#endif //RISCV_SIMULATOR_LOADSTOREBUFFER_H
