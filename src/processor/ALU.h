#ifndef RISCV_SIMULATOR_ALU_H
#define RISCV_SIMULATOR_ALU_H
#include"decode.h"
class ALU{
public:
    static u32 Calc(Type &op, u32 &in1, u32 &in2) {
        if (op == LUI || op == AUIPC || op == JAL) return in1 + in2;
        if (op == ADD || op == ADDI) return in1 + in2;
        if (op == LW || op == LB ||  op == LH || op == LBU || op == LHU) return in1 + in2;
        if (op == SW || op == SB || op == SH) return in1 + in2;
        if (op == SUB) return in1 - in2;
        if (op == BEQ) return in1 == in2;
        if (op == BNE) return in1 != in2;
        if (op == BLTU || op == SLTUI || op == SLTU) return in1 < in2;
        if (op == BGEU) return in1 >= in2;
        if (op == BLT || op == SLTI || op == SLT) return static_cast<int32_t> (in1) < static_cast<int32_t>(in2);
        if (op == BGE) return static_cast<int32_t>(in1) >= static_cast<int32_t>(in2);
        if (op == XORI || op == XOR) return in1 ^ in2;
        if (op == ORI || op == OR) return in1 | in2;
        if (op == ANDI || op == AND) return in1 & in2;
        if (op == JALR) return (in1 + in2) & 0xfffffffe;
        if (op == SRL || op == SRLI) return in1 >> in2;
        if (op == SLL || op == SLLI) return in1 << in2;
        if (op == SRA || op == SRAI) return static_cast<int32_t>(in1) >> in2;
        return 0;
    }
};
#endif //RISCV_SIMULATOR_ALU_H
