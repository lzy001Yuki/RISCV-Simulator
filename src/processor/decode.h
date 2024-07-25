#ifndef RISCV_SIMULATOR_DECODE_H
#define RISCV_SIMULATOR_DECODE_H
#include<iostream>
#include "../utils/Type.hpp"

class Decode{
    friend class ALU;
    friend class ReservationStation;
    friend class LoadStoreBuffer;
    friend class CPU;
    friend class ReorderBuffer;
protected:
    char type;
    u32 code; // std::cin>>std::hex>>code
    Type orderType;
    u32 imm = 0;
    u8 rd = 0, rs1 = 0, rs2 = 0, funct3, funct7;

    template<class T> T get(int l, int r);
    static u32 extension(u32 data, int len);
public:
    Decode() = default;
    explicit Decode(u32 &input) : code(input){}
    void decode();
};

template<class T>
T Decode::get(int l, int r) {
    T ans = code & ((1<<(r + 1)) - 1);
    return ans >> l;
}

u32 Decode::extension(u32 data, int len) {
    if (len == 32) return data;
    if ((data>>(len - 1) & 1) == 0) return data;
    else {
        u32 tmp = 1;
        for (int i = 0; i < data - len; i++) {
            tmp = (tmp<<1) + 1;
        }
        tmp = tmp<<len;
        return data | tmp;
    }
}

void Decode::decode() {
    u8 op = get<u8>(0, 6);
    switch(op) {
        case 0x13:
            // I-type
            type = 'I';
            funct3 = get<u8>(12, 14);
            rd = get<u8>(7, 11);
            rs1 = get<u8>(15, 19);
            switch(funct3) {
                case 0:
                    orderType = ADDI;
                    imm = extension(get<u32> (20, 31), 12);
                    break;
                case 1:
                    orderType = SLLI;
                    imm = get<u32>(20, 31);
                case 2:
                    orderType = SLTI;
                    imm = extension(get<u32> (20, 31), 12);
                    break;
                case 3:
                    orderType = SLTUI;
                    imm = extension(get<u32>(20, 31), 12);
                    break;
                case 4:
                    orderType = XORI;
                    imm = extension(get<u32>(20, 31), 12);
                    break;
                case 5:
                    imm = get<u32>(20, 31);
                    if ((imm & 10) >> 1) {
                        orderType = SRAI;
                        imm = imm << 2 >> 2;
                    } else orderType = SRLI;
                    break;
                case 6:
                    orderType = ORI;
                    imm = extension(get<u32>(20, 31), 12);;
                    break;
                case 7:
                    orderType = ANDI;
                    imm = extension(get<u32>(20, 31), 12);
                    break;
            }
            break;
        case 0x17:
            type = 'U';
            orderType = AUIPC;
            rd = get<u8>(7, 11);
            imm = extension(get<u32>(12, 31), 20) << 12;
            break;
        case 0x37:
            // U-type
            type = 'U';
            orderType = LUI;
            rd = get<u8>(7, 11);
            imm = extension(get<u32>(12, 31), 20) << 12;
            break;
        case 0x23:
            // S-type
            type = 'S';
            funct3 = get<u8>(12, 14);
            rs1 = get<u8>(15, 19);
            rs2 = get<u8>(20, 24);
            imm = extension((get<u32>(7, 11) + get<u32>(25, 31)) << 5, 12);
            switch (funct3) {
                case 0:
                    orderType = SB;
                    break;
                case 1:
                    orderType = SH;
                    break;
                case 2:
                    orderType = SW;
                    break;
            }
            break;
        case 0x6f:
            type = 'J';
            orderType = JAL;
            rd = get<u8>(7, 11);
            rs1 = get<u8>(15, 19);
            imm = extension((get<u32>(31, 31)<<20) + (get<u32>(12, 19)<<12) + (get<u32>(20, 20)<<11) +
                            (get<u32>(21, 30)<<1), 21);
            break;
        case 0x67:
            type ='I';
            orderType = JALR;
            rd = get<u8>(7, 11);
            rs1 = get<u8>(15, 19);
            imm = extension(get<u32>(20, 31)<<1, 13);
            break;
        case 0x03:
            type = 'I';
            funct3 = get<u8>(12, 14);
            rs1 = get<u8>(15, 19);
            rs2 = get<u8>(20, 24);
            switch(funct3) {
                case 0:
                    orderType = LB;
                    break;
                case 1:
                    orderType = LH;
                    break;
                case 2:
                    orderType = LW;
                    break;
                case 4:
                    orderType = LBU;
                    break;
                case 5:
                    orderType = LHU;
                    break;
            }
            break;
        case 0x33:
            type = 'R';
            funct3 = get<u8>(12, 14);
            funct7 = get<u8>(25, 31);
            rd = get<u8>(7, 11);
            rs1 = get<u8>(15, 19);
            rs2 = get<u8>(20, 24);
            switch(funct3) {
                case 0:
                    if ((funct7 >> 5) & 1) orderType = SUB;
                    else orderType = ADD;
                    break;
                case 1:
                    orderType = SLL;
                    break;
                case 2:
                    orderType = SLT;
                    break;
                case 3:
                    orderType = SLTU;
                    break;
                case 4:
                    orderType = XOR;
                    break;
                case 5:
                    if ((funct7 >> 5) & 1) orderType = SRA;
                    else orderType = SRL;
                    break;
                case 6:
                    orderType = OR;
                    break;
                case 7:
                    orderType = AND;
                    break;
            }
            break;
        case 0x63:
            type = 'B';
            funct3 = get<u8>(12, 14);
            rs1 = get<u8>(15, 19);
            rs2 = get<u8>(20, 24);
            imm = extension((get<u32>(8, 11) << 1) + (get<u32>(7, 7)<<11) + (get<u32>(25, 30) << 5) + (get<u32>(31, 31) << 12), 13);
            switch(funct3) {
                case 0:
                    orderType = BEQ;
                    break;
                case 1:
                    orderType = BNE;
                    break;
                case 4:
                    orderType = BLT;
                    break;
                case 5:
                    orderType = BGE;
                    break;
                case 6:
                    orderType = BLTU;
                    break;
                case 7:
                    orderType = BGEU;
                    break;
            }
            break;
        default:
            break;
    }
}
#endif //RISCV_SIMULATOR_DECODE_H
