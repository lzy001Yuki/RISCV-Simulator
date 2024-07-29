#ifndef RISCV_SIMULATOR_RESERVATIONSTATION_H
#define RISCV_SIMULATOR_RESERVATIONSTATION_H
#include"Reorder.h"
#include"ALU.h"
/*
 * There are three stages to the Tomasulo algorithm: "Issue", "Execute", "Write Result".
 * In an extension to the algorithm, there is an additional "Commit" stage.
 * During the Commit stage, instruction results are stored in a register or memory.
 * The "Write Result" stage is modified to place results in the re-order buffer.
 * Each instruction is tagged in the reservation station with its index in the ROB for this purpose.
*/
class rsNode{
    friend class CPU;
    friend class ReservationStation;
protected:
    Status status;
    Type orderType;
    int label = 0;
    u32 res = 0;
    u32 Q1 = 0, Q2 = 0, V1 = 0, V2 = 0;
    bool busy = false;
    // Q for robTag, V for value
public:
    rsNode() = default;
    friend std::ostream& operator << (std::ostream &os, const rsNode &tmp);
};
std::ostream &operator<<(std::ostream &os, const rsNode &tmp) {
    os <<"label\t"<<tmp.label<<'\t'<<
    "V1\t"<<tmp.V1<<"\tV2\t"<<tmp.V2<<"\tQ1\t"<<tmp.Q1<<"\tQ2\t"<<tmp.Q2<<
    "\tres\t"<<tmp.res<<'\t'<<"status\t"<<tmp.status<<'\n';
    return os;
}
const int RSSIZE = 8;
class ReservationStation{
    friend class CPU;
private:
    rsNode rs[RSSIZE];
public:
    ReservationStation() = default;
    bool full();
    void flush();
    rsNode Calc();
    bool Issue(ReorderBuffer &rob, Register &r, Decode &decode, u32 &nowPC);
    void Write(CDB &cdb, ReorderBuffer &rob);
    void fetchData(CDB &cdb);
    void print();;
};
bool ReservationStation::full() {
    bool isFull = true;
    for (auto &rss :rs) {
        if (!rss.busy) {
            isFull = false;
            break;
        }
    }
    return isFull;
}
void ReservationStation::flush() {
    for (auto& rss : rs) {
        rss = rsNode();
    }
}
bool ReservationStation::Issue(ReorderBuffer &rob, Register &r, Decode &decode, u32 &nowPC) {
    int index = 0;
    while (rs[index].busy) {
        index++;
    }
    if (index >= 8) return false;
    rs[index].label = rob.tag; // increment of tag
    rs[index].status = issue;
    rs[index].busy = true;
    rs[index].orderType = decode.orderType;
    if (decode.type == 'U') {
        rs[index].V1 = decode.imm;
        if (decode.orderType == LUI) rs[index].V2 = 0;
        else if (decode.orderType == AUIPC) rs[index].V2 = nowPC;
        return true;
    } else if (decode.type == 'J') {
        rs[index].V1 = 4;
        rs[index].V2 = nowPC;
        return true;
    }
    // I-Type rs+imm
    int label1 = r.Reg[decode.rs1].label;
    int label2 = r.Reg[decode.rs2].label;
    if (label1) {
        if (rob.robBuffer[label1].ready) rs[index].V1 = rob.robBuffer[label1].res;
        else rs[index].Q1 = label1;
    } else rs[index].V1 = r.Reg[decode.rs1].val;
    if (decode.type == 'I') {
        rs[index].V2 = decode.imm;
        rs[index].Q2 = 0;
    } else {
        if (label2) {
            if (rob.robBuffer[label2].ready) rs[index].V2 = rob.robBuffer[label2].res;
            else rs[index].Q2 = label2;
        } else rs[index].V2 = r.Reg[decode.rs2].val;
    }
    return true;
}

rsNode ReservationStation::Calc() {
    for (int i = 0; i < RSSIZE; i++) {
        if (rs[i].status == issue && !rs[i].Q1 && !rs[i].Q2 && rs[i].busy) {
            rs[i].status = execute;
            rs[i].res = ALU::Calc(rs[i].orderType, rs[i].V1, rs[i].V2);
            //std::cout<<"ExecRs\t"<<rs[i];
            return rs[i];
        }
    }
    return {};
}

// write and broadcast are at the same time
void ReservationStation::Write(CDB &cdb, ReorderBuffer &rob) {
    for (int i = 0; i < RSSIZE; i++) {
        if (rs[i].status == execute) {
            //std::cout<<"writeRs\t"<<rs[i];
            rs[i].status = write;
            cdb.broadcast(rs[i].label, rs[i].res);
            rob.robBuffer[rs[i].label].ready = true;
            rob.robBuffer[rs[i].label].res = rs[i].res;
            rs[i].busy = false;
            rs[i].res = 0;
            rs[i] = rsNode();
            return;
        }
    }
}

void ReservationStation::fetchData(CDB &cdb) {
    if (!cdb.bus.busy) return;
    for (int i = 0; i < RSSIZE; i++) {
        if (rs[i].busy) {
            if (rs[i].status == issue) {
                if (rs[i].Q1 == cdb.bus.label) {
                    rs[i].V1 = cdb.bus.val;
                    rs[i].Q1 = 0;
                } else if (rs[i].Q2 == cdb.bus.label) {
                    rs[i].V2 = cdb.bus.val;
                    rs[i].Q2 = 0;
                }
            }
        }
    }
}

void ReservationStation::print() {
    std::cout<<"ReservationStation Status-----------------------\n";
    for (int i = 0; i < RSSIZE; i++) {
        if (rs[i].busy) std::cout<<rs[i];
    }
}
#endif //RISCV_SIMULATOR_RESERVATIONSTATION_H
