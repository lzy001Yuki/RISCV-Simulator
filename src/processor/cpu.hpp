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
        if (code ==  4217368687) {
            int y = 2;
        }
        Decode decoder(code);
        decoder.decode();
        //std::cout<<"fetch\t"<<decoder<<'\n';
        if (decoder.orderType == BLTU) {
            int y = 2;
        }
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
            if (decoder.type != 'J' || decoder.rd != 0) opq.enQueue(PC, decoder, false);
            if (decoder.orderType == JALR) {
                opq.stall = true;
                return;
            }
            if (decoder.type == 'J') PC += decoder.imm;
            else PC += 4;
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
        //std::cout<<"issue\n";
        //if (opq.stall) return;
        if (rob.full()) return;
        if (opq.empty()) {
            fetch();
            return;
        }
        opNode curInstr = opq.front();
        //std::cout<<"issue\t"<<curInstr.decode<<'\n';
        fetch();
        if (curInstr.decode.orderType == BLTU) {
            int y = 2;
        }

        if (curInstr.decode.code == 19998243) {
            int y = 2;
        }
        // rob/rs/cdb/reg
        if (curInstr.decode.type == 'I' || curInstr.decode.type == 'R') {
            // to rob & rs
            if (!rs.Issue(rob, reg, curInstr.decode, curInstr.pc)) return;
            u32 dest = (u32) curInstr.decode.rd;
            rob.Issue(curInstr.decode, curInstr.pc, dest, false, reg);
        } else if (curInstr.decode.type == 'U') {
            // no need for execute & issue
            u32 dest = (u32) curInstr.decode.rd;
            if (!rs.Issue(rob, reg, curInstr.decode, curInstr.pc)) return;
            rob.Issue(curInstr.decode, curInstr.pc, dest, false, reg);
        } else if (curInstr.decode.type == 'B') {
            if (rs.full()) return;
            u32 dest = curInstr.decode.imm + curInstr.pc;
            if (!rs.Issue(rob, reg, curInstr.decode, curInstr.pc)) return;
            rob.Issue(curInstr.decode, curInstr.pc, dest, curInstr.jump, reg);
        } else if (curInstr.decode.type == 'S' || curInstr.decode.type == 'L') {
            if (lsb.full()) return;
            // no need for dest, because no rd
            if (!lsb.Issue(rob, reg, curInstr.decode, clk)) return;
            u32 dest = 0;
            if (curInstr.decode.type == 'L') dest = curInstr.decode.rd;
            rob.Issue(curInstr.decode, curInstr.pc, dest, false, reg);
        } else if (curInstr.decode.type == 'J') {
            // JALR belongs to 'I' type
            // J type -->JAL
            u32 dest = (u32) curInstr.decode.rd;
            if (!rs.Issue(rob, reg, curInstr.decode, curInstr.pc)) return;
            rob.Issue(curInstr.decode, curInstr.pc, dest, true, reg);
        }
        opq.deQueue();
    }
    void execute() {
        //std::cout<<"execute\n";
        rsNode ans = rs.Calc();
        if (rob.robBuffer[ans.label].decode.code == 3907492579) {
            int y = 2;
        }
        if (ans.busy) {
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
        //std::cout<<"writeResult\n";
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
        //std::cout<<"commit\n";
        lsb.Store(mem, rob);
        if (lsb.cur.storeTime) return;
        if (rob.empty()) return;
        robNode comNode = rob.front();
        if (!comNode.ready) {
            return;
        }
        com++;
        if (comNode.label == 1 && comNode.res == 45 && comNode.decode.orderType == LW) {
            int y = 2;
        }
        if (com == 2000000) {
            int y = 0;
            //exit(0);
        }
        if (comNode.decode.code == 0x0ff00513) {
            //std::cout<<clk<<'\n';
            std::cout<<std::dec<<(reg.Reg[10].val & 255)<<'\n';
            exit(0);
        }
        if (comNode.decode.type != 'S' && comNode.decode.type != 'B') {
            rob.commit(reg, cdb);
            rs.fetchData(cdb);
            lsb.fetchData(cdb);
        } else if (comNode.decode.type == 'S') {
            lsb.Commit(comNode);
            rob.pop();
        } else if (comNode.decode.type == 'B') {
            if (comNode.res == comNode.jump) {
                pre.update(comNode.nowPC, true);
                rob.pop();
            } else {
                //std::cout<<"false prediction\n";
                if (comNode.res) PC = comNode.dest;
                else PC = comNode.nowPC + 4;
                pre.update(comNode.nowPC, false);
                opq.flush();
                rs.flush();
                lsb.flush();
                rob.flushRob();
                cdb.flushCDB();
                reg.flushReg();
                fetch();
                //exit(0);
            }
        }
        //std::cout<<"commit\t"<<comNode;
        //reg.print();
    }
public:
    u32 PC = 0;
    int clk = 0;
    int com = 0;
    CPU() = default;
    void read() {
        std::string str;
        u32 pc = 0;
        while (getline(std::cin, str)) {
            if (str[0] != '@' && !(str[0] >= '0' && str[0] <= '9') && !(str[0] >= 'A' && str[0] <= 'F') && str.size() != 0) break;
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
                    if (str.size() < 3) str = str.substr(2);
                    else str = str.substr(3);
                    pc++;
                }
            }
        }

    }
    void work() {
        /*void (CPU::*func[4]) () = {&CPU::issue, &CPU::execute, &CPU::writeResult, &CPU::commit};
        while (true) {
            clk++;
            //std::cout<<clk<<" ";
            //std::cout<<"PC:"<<PC<<"--------------------\n";
            std::shuffle(func, func + 4, std::mt19937(std::random_device()()));
            //fetch();
            //std::cout<<PC<<'\n';
            //std::cout<<"process----------------\n"
            (this->*func[0]) ();
            (this->*func[1]) ();
            (this->*func[2]) ();
            (this->*func[3]) ();
            if (clk == 110576788) {
                exit(0);
            }
            //print1();
            //print2();
            //std::cout<<"newPC\t"<<PC<<'\n';
            //std::cout<<'\n';
            //rs.print();
            //rob.print();
            //lsb.print();
            //reg.print();
        }*/
        std::random_device gen;
        std::mt19937 rd(gen());
        int flag[4] = {1, 2, 3, 4};
        while (true) {
            ++clk;
            std::shuffle(flag, flag + 4, rd);
            for (int i = 0; i < 4; i++) {
                if (flag[i] == 1) issue();
                if (flag[i] == 2) execute();
                if (flag[i] == 3) writeResult();
                if (flag[i] == 4) commit();
            }
        }
    }

    void print1() {
        void (CPU::*func[4]) () = {&CPU::issue, &CPU::execute, &CPU::writeResult, &CPU::commit};
        (this->*func[0]) ();
        (this->*func[1]) ();
        (this->*func[2]) ();
        (this->*func[3]) ();
    }

    void print2() {
        void (CPU::*func[4]) () = {&CPU::issue, &CPU::execute, &CPU::writeResult, &CPU::commit};
        for (int i = 0; i < 4; i++) {
            std::string funct;
            std::getline(std::cin, funct);
            if (funct == "issue")  (this->*func[0]) ();
            if (funct == "execute")  (this->*func[1]) ();
            if (funct == "writeResult")  (this->*func[2]) ();
            if (funct == "commit")  (this->*func[3]) ();
        }
    }
};
#endif //RISCV_SIMULATOR_CPU_HPP
