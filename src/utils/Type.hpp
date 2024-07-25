#ifndef RISCV_SIMULATOR_TYPE_HPP
#define RISCV_SIMULATOR_TYPE_HPP
#include<iostream>
typedef u_int32_t  u32;
typedef u_int8_t u8;
typedef u_int16_t u16;
enum Type{
    LUI, AUIPC, // U
    JAL, // J
    JALR, // I
    BEQ, BNE, BLT, BGE, BLTU, BGEU, // B
    LB, LW, LH, LHU, LBU, // I
    SB, SW, SH, // S
    ADDI, SLTI, ORI, XORI, ANDI, SLLI, SRLI, SRAI, SLTUI, // I
    ADD, SUB, SLL, SLT, SLTU, XOR, OR, AND, SRL, SRA // R
};

enum Status{
   issue, execute, write, commit
};
// issue(rob/rs/cdb/reg) (from instruction queue)
// --> execute(alu) (2steps-- 1>receive data(when other instruction is written) 2>calculate )
// --> write(rob/rs/reg) (queue front)
// -->commit(rob)
#endif //RISCV_SIMULATOR_TYPE_HPP
