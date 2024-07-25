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
        if (code == 0x0ff00513) {
            opq.end = true;
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
            if (decoder.type == 'J') PC += decoder.imm;
            else PC += 4;
            if (decoder.orderType == JALR) opq.stall = true;
        }
    }
    // check if opq is empty
    // issue & decode in the same clockTime
    // goal for issue --> get data from (physical register)
    /*
     * if the instruction has rs, then must be issued
     * else only need enQueue, save rd
     * */
    void issue() {
        if (rob.full()) return;
        if (opq.stall) return;
        opNode curInstr = opq.front();
        opq.deQueue();
        // fetch();
        // rob/rs/cdb/reg
        if (curInstr.decode.type == 'I' || curInstr.decode.type == 'R' || curInstr.decode.type == 'L') {
            // to rob & rs
            if (curInstr.decode.type == 'L' && lsb.full()) return;
            rs.Issue(rob, reg, curInstr.decode);
            u32 dest = (u32) curInstr.decode.rd;
            rob.Issue(curInstr.decode, curInstr.pc, dest, false, reg);
        } else if (curInstr.decode.type == 'U') {
            // no need for execute & issue
            u32 dest = (u32) curInstr.decode.rd;
            robNode newRob(curInstr.decode, curInstr.pc, dest, false);
            if (curInstr.decode.orderType == LUI) newRob.res = curInstr.decode.imm;
            if (curInstr.decode.orderType == AUIPC) newRob.res = curInstr.decode.imm + curInstr.pc;
            rob.addRob(newRob, reg);
        } else if (curInstr.decode.type == 'B') {
            if (rs.full()) return;
            u32 dest = curInstr.decode.imm + curInstr.pc;
            rs.Issue(rob, reg, curInstr.decode);
            rob.Issue(curInstr.decode, curInstr.pc, dest, curInstr.jump, reg);
        } else if (curInstr.decode.type == 'S') {
            if (lsb.full()) return;
            // no need for dest, because no rd
            lsb.Issue(rob, reg, curInstr.decode, clk);
            u32 dest = 0;
            rob.Issue(curInstr.decode, curInstr.pc, dest, false, reg);
        } else if (curInstr.decode.type == 'J') {
            // JALR belongs to 'I' type
            // J type -->JAL
            u32 dest = (u32) curInstr.decode.rd;
            robNode newRob(curInstr.decode, curInstr.pc, dest, true);
            newRob.res = curInstr.pc + 4;
            rob.addRob(newRob, reg);
        }
    }
    void execute() {
        rsNode ans = rs.Calc();
        if (ans.orderType == JALR) {
            u32 copy = PC;
            PC = ans.res;
            ans.res = copy;
        }
        lsb.Execute(rob);
        if (lsb.cur.loadTime) lsb.Load(mem);
    }
    void writeResult() {
        if (opq.stall) return;
        rs.Write(cdb, rob);
        lsb.Write(cdb);
        rob.fetchData(cdb);
        rs.fetchData(cdb);
        lsb.fetchData(cdb);
        cdb.flushCDB();
    }
    void commit() {
        if (opq.stall) return;
        lsb.Store(mem, rob);
        if (lsb.cur.storeTime) return;
        if (rob.empty()) return;
        robNode comNode = rob.front();
        if (!comNode.ready) return;
        if (comNode.decode.code == 0x0ff00513) {
            std::cout<<std::dec<<(reg.Reg[10].val & 0xff)<<'\n';
            exit(0);
        }
        if (comNode.decode.type != 'S' && comNode.decode.type != 'B') {
            rob.commit(reg);
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
        while (std::cin) {
            if (str[0] == '@') {
                str.substr(1);
                std::stringstream st(str);
                st>>std::hex>>pc;
            } else {
                std::stringstream st(str);
                u8 code;
                st>>std::hex>>code;
                mem.writeAddr<u8, 8>(pc, code);
                pc++;
            }
        }
    }
    void work() {
        void (CPU::*func[4]) () = {&CPU::issue, &CPU::execute, &CPU::writeResult, &CPU::commit};
        while (true) {
            std::shuffle(func, func + 4, std::mt19937(std::random_device()()));
            (this->*func[0]) ();
            (this->*func[1]) ();
            (this->*func[2]) ();
            (this->*func[3]) ();
        }
    }

};
#endif //RISCV_SIMULATOR_CPU_HPP
