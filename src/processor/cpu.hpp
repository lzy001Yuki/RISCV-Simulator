#ifndef RISCV_SIMULATOR_CPU_HPP
#define RISCV_SIMULATOR_CPU_HPP
#include"Reorder.h"
#include"ReservationStation.h"
#include"LoadStoreBuffer.h"
#include"Memory.hpp"
#include"opQueue.h"
#include"Prediction.h"
#include"CommonDataBus.hpp"
#include<iomanip>
#include<fstream>
#include<random>
#include<algorithm>
class CPU{
private:
    ReservationStation rs;
    ReorderBuffer rob;
    LoadStoreBuffer lsb;
    Memory mem;
    opQueue opq;
    Predictor pre;
    Register reg;
    CDB cdb;


    // fetch -- issue
    void fetch() {
        if (opq.end || opq.full() || opq.stall) return;
        u32 code = mem.getAddr<u32, 32>(PC);
        Decode decoder(code);
        decoder.decode();
        u32 prePC = PC;
        if (code == 0x0ff00513) {
            opq.end = true;
            opq.enQueue(PC, decoder, false);
            return;
        }
        if (decoder.type == 'B') {
            if (pre.predict(PC)) {
                opq.enQueue(PC, decoder, true);
                PC += decoder.imm;
            } else {
                opq.enQueue(PC, decoder, false);
                PC += 4;
            }
        } else {
            opq.enQueue(PC, decoder, false);
            if (decoder.orderType == JALR) {
                opq.stall = true;
                return;
            }
            if (decoder.type == 'J') PC += decoder.imm;
            else PC += 4;
        }
        //std::cout<<prePC<<'\t'<<PC<<'\t';
        std::cout<<"fetch\t";
        decoder.print();
    }
    // check if opq is empty
    // issue & decode in the same clockTime
    // goal for issue --> get data from (physical register)
    /*
     * if the instruction has rs, then must be issued
     * else only need enQueue, save rd
     * */
    void issue() {
        //if (opq.stall) return;
        if (rob.full()) return;
        if (opq.empty()) {
            fetch();
            return;
        }
        opNode curInstr = opq.front();
        opq.deQueue();
        fetch();
        std::cout<<"issue\t";curInstr.decode.print();
        // rob/rs/cdb/reg
        if (curInstr.decode.type == 'I' || curInstr.decode.type == 'R') {
            // to rob & rs
            rs.Issue(rob, reg, curInstr.decode, curInstr.pc);
            u32 dest = (u32) curInstr.decode.rd;
            rob.Issue(curInstr.decode, curInstr.pc, dest, false, reg);
        } else if (curInstr.decode.type == 'U') {
            // no need for execute & issue
            u32 dest = (u32) curInstr.decode.rd;
            rs.Issue(rob, reg, curInstr.decode, curInstr.pc);
            rob.Issue(curInstr.decode, curInstr.pc, dest, false, reg);
        } else if (curInstr.decode.type == 'B') {
            if (rs.full()) return;
            u32 dest = curInstr.decode.imm + curInstr.pc;
            rs.Issue(rob, reg, curInstr.decode, curInstr.pc);
            rob.Issue(curInstr.decode, curInstr.pc, dest, curInstr.jump, reg);
        } else if (curInstr.decode.type == 'S' || curInstr.decode.type == 'L') {
            if (lsb.full()) return;
            // no need for dest, because no rd
            lsb.Issue(rob, reg, curInstr.decode, clk);
            u32 dest = 0;
            rob.Issue(curInstr.decode, curInstr.pc, dest, false, reg);
        } else if (curInstr.decode.type == 'J') {
            // JALR belongs to 'I' type
            // J type -->JAL
            u32 dest = (u32) curInstr.decode.rd;
            rs.Issue(rob, reg, curInstr.decode, curInstr.pc);
            rob.Issue(curInstr.decode, curInstr.pc, dest, true, reg);
        }
    }
    void execute() {
        rsNode ans = rs.Calc();
        if (ans.busy) {
            std::cout << "execute\t";
            rob.robBuffer[ans.label].decode.print();
            if (ans.orderType == JALR) {
                u32 copy = PC;
                PC = ans.res;
                ans.res = copy;
                opq.stall = false;
            }
        }
        lsb.Load(mem);
        lsb.Execute(rob);
    }
    void writeResult() {
        //if (opq.stall) return;
        rs.Write(cdb, rob);
        rob.fetchData(cdb);
        rs.fetchData(cdb);
        lsb.fetchData(cdb);
        lsb.Write(cdb);
        rob.fetchData(cdb);
        rs.fetchData(cdb);
        lsb.fetchData(cdb);
        cdb.flushCDB();
    }
    void commit() {
        //if (opq.stall) return;
        lsb.Store(mem, rob);
        if (lsb.cur.storeTime) return;
        if (rob.empty()) return;
        robNode comNode = rob.front();
        if (!comNode.ready) return;
        std::cout<<"commit\t";comNode.decode.print();
        if (comNode.decode.code == 0x0ff00513) {
            std::cout<<std::dec<<(reg.Reg[10].val & 0xff)<<'\n';
            exit(0);
        }
        if (comNode.decode.type != 'S' && comNode.decode.type != 'B') {
            rob.commit(reg, cdb);
            rs.fetchData(cdb);
            lsb.fetchData(cdb);
        } else if (comNode.decode.type == 'S') {
            lsb.Commit(comNode);
        } else if (comNode.decode.type == 'B') {
            if (comNode.res == comNode.jump) {
                pre.update(comNode.nowPC, true);
                rob.pop();
            } else {
                if (comNode.res) PC = comNode.dest;
                else PC = comNode.nowPC + 4;
                pre.update(comNode.nowPC, false);
                opq.flush();
                rs.flush();
                lsb.flush();
                rob.flushRob();
                reg.flushReg();
            }
        }
    }
public:
    u32 PC = 0;
    int clk = 0;
    CPU() = default;
    void read() {
        std::string str;
        u32 pc = 0;
        while (getline(std::cin, str)) {
            if (str[0] == '@') {
                std::string s = str.substr(1);
                u32 sum = 0;
                for (int i = 0; i < s.size(); i++) {
                    if (s[i] >= '0' && s[i] <= '9') sum = sum * 16 + s[i] - '0';
                    if (s[i] >= 'A' && s[i] <= 'F') sum = sum * 16 + 10 + s[i] - 'A';
                }
                pc = sum;
            } else {
                while (str.size()) {
                    std::string s = str.substr(0, 2);
                    u8 val = 0;
                    for (int i = 0; i < s.size(); i++) {
                        if (s[i] >= '0' && s[i] <= '9') val = val * 16 + s[i] - '0';
                        if (s[i] >= 'A' && s[i] <= 'F') val = val * 16 + 10 + s[i] - 'A';
                    }
                    mem.writeAddr<u8, 8>(pc, val);
                    str = str.substr(3);
                    pc++;
                }
            }
        }

    }
    void work() {
        void (CPU::*func[4]) () = {&CPU::issue, &CPU::execute, &CPU::writeResult, &CPU::commit};
        while (true) {
            clk++;
            if (clk == 24) {
                int y = 2;
            }
            if (clk == 23) {
                int y = 2;
            }
            std::cout<<clk<<"----------------------------------\n";
            std::cout<<"PC:"<<PC<<"--------------------\n";
            std::shuffle(func, func + 4, std::mt19937(std::random_device()()));
            (this->*func[0]) ();
            (this->*func[1]) ();
            (this->*func[2]) ();
            (this->*func[3]) ();
            if (clk == 100) exit(0);
            /*std::cout<<"register status--->\n";
            for (int i = 0; i < 32; i++) {
                if (reg.Reg[i].label) {
                    std::cout<<i<<'\t'<<reg.Reg[i].val<<'\n';
                }
            }*/
        }
    }

};
#endif //RISCV_SIMULATOR_CPU_HPP
