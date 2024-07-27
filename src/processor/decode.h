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
    std::string t;
    u32 imm = 0;
    u8 rd = 0, rs1 = 0, rs2 = 0, funct3, funct7;

    template<class T> T get(int l, int r);
    static u32 extension(u32 data, int len);
public:
    Decode() = default;
    explicit Decode(u32 &input) : code(input){}
    void decode();
    friend std::ostream & operator<<(std::ostream &os, const Decode &tmp);
};

template<class T>
T Decode::get(int l, int r) {
    u32 ans = code;
    if (r < 31) ans = code & ((1<<(r + 1)) - 1);
    return ans >> l;
}

u32 Decode::extension(u32 data, int len) {
    if (len == 32) return data;
    u32 ans = 0;
    if (data >> (len - 1) & 1) ans = 0xffffffff>>len<<len;
    return ans | data;
}

void Decode::decode() {
    u8 op = get<u8>(0, 6);
    if (code == 33950819) {
        int y = 2;
    }
    switch(op) {
        case 0x17:
            type = 'U';
            orderType = AUIPC;
            t = "AUIPC";
            rd = get<u8>(7, 11);
            imm = extension(get<u32>(12, 31), 20) << 12;
            break;
        case 0x37:
            // U-type
            type = 'U';
            orderType = LUI;
            t = "LUI";
            rd = get<u8>(7, 11);
            imm = extension(get<u32>(12, 31), 20) << 12;
            break;
        case 0x23:
            // S-type
            type = 'S';
            funct3 = get<u8>(12, 14);
            rs1 = get<u8>(15, 19);
            rs2 = get<u8>(20, 24);
            imm = extension((get<u32>(7, 11)) + (get<u32>(25, 31) << 5), 12);
            switch (funct3) {
                case 0:
                    orderType = SB;
                    t = "SB";
                    break;
                case 1:
                    orderType = SH;
                    t = "SH";
                    break;
                case 2:
                    orderType = SW;
                    t = "SW";
                    break;
            }
            break;
        case 0x6f:
            type = 'J';
            orderType = JAL;
            t = "JAL";
            rd = get<u8>(7, 11);
            imm = extension((get<u32>(31, 31)<<20) + (get<u32>(12, 19)<<12) + (get<u32>(20, 20)<<11) +
                            (get<u32>(21, 30)<<1), 21);
            break;
        case 0x67:
            type ='I';
            orderType = JALR;
            t = "JALR";
            rd = get<u8>(7, 11);
            rs1 = get<u8>(15, 19);
            imm = extension(get<u32>(20, 31)<<1, 13);
            break;
        case 0x03:
            type = 'L';
            funct3 = get<u8>(12, 14);
            rd = get<u8>(7, 11);
            rs1 = get<u8>(15, 19);
            imm = extension(get<u32>(20, 31), 12);
            switch(funct3) {
                case 0:
                    orderType = LB;
                    t = "LB";
                    break;
                case 1:
                    orderType = LH;
                    t = "LH";
                    break;
                case 2:
                    orderType = LW;
                    t = "LW";
                    break;
                case 4:
                    orderType = LBU;
                    t = "LBU";
                    break;
                case 5:
                    orderType = LHU;
                    t = "LHU";
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
                    if ((funct7 >> 5) & 1) {
                        orderType = SUB;
                        t = "SUB";
                    }
                    else {
                        orderType = ADD;
                        t = "ADD";
                    }
                    break;
                case 1:
                    orderType = SLL;
                    t = "SLL";
                    break;
                case 2:
                    orderType = SLT;
                    t = "SLT";
                    break;
                case 3:
                    orderType = SLTU;
                    t = "SLTU";
                    break;
                case 4:
                    orderType = XOR;
                    t = "XOR";
                    break;
                case 5:
                    if ((funct7 >> 5) & 1) {
                        orderType = SRA;
                        t = "SRA";
                    }
                    else {
                        orderType = SRL;
                        t = "SRL";
                    }
                    break;
                case 6:
                    orderType = OR;
                    t = "OR";
                    break;
                case 7:
                    orderType = AND;
                    t = "AND";
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
                    t = "BEQ";
                    break;
                case 1:
                    orderType = BNE;
                    t = "BNE";
                    break;
                case 4:
                    orderType = BLT;
                    t = "BLT";
                    break;
                case 5:
                    orderType = BGE;
                    t = "BGE";
                    break;
                case 6:
                    orderType = BLTU;
                    t = "BLTU";
                    break;
                case 7:
                    orderType = BGEU;
                    t = "BGEU";
                    break;
            }
            break;
        case 0x13:
            // I-type
            type = 'I';
            funct3 = get<u8>(12, 14);
            rd = get<u8>(7, 11);
            rs1 = get<u8>(15, 19);
            switch(funct3) {
                case 0:
                    orderType = ADDI;
                    t = "ADDI";
                    imm = extension(get<u32> (20, 31), 12);
                    break;
                case 1:
                    orderType = SLLI;
                    t = "SLLI";
                    imm = get<u32>(20, 31);
                    break;
                case 2:
                    orderType = SLTI;
                    t = "SLTI";
                    imm = extension(get<u32> (20, 31), 12);
                    break;
                case 3:
                    orderType = SLTUI;
                    t = "SLTUI";
                    imm = extension(get<u32>(20, 31), 12);
                    break;
                case 4:
                    orderType = XORI;
                    t = "XORI";
                    imm = extension(get<u32>(20, 31), 12);
                    break;
                case 5:
                    imm = get<u32>(20, 31);
                    if ((imm & 10) >> 1) {
                        orderType = SRAI;
                        t = "SRAI";
                        imm = imm << 2 >> 2;
                    } else {
                        orderType = SRLI;
                        t = "SRLI";
                    }
                    break;
                case 6:
                    orderType = ORI;
                    t = "ORI";
                    imm = extension(get<u32>(20, 31), 12);;
                    break;
                case 7:
                    orderType = ANDI;
                    t = "ANDI";
                    imm = extension(get<u32>(20, 31), 12);
                    break;
            }
            break;
        default:
            break;
    }
}

std::ostream& operator<< (std::ostream& os, const Decode &tmp) {
    //std::cout<<tmp.t<<'\t'<<"rd\t"<<(int)tmp.rd<<"\trs1\t"<<(int)tmp.rs1<<"\trs2\t"<<(int)tmp.rs2<<"\timm\t"<<tmp.imm<<'\t';
    std::cout<<tmp.t<<'\t'<<tmp.code<<'\t'<<(int)tmp.rs1<<'\t'<<(int)tmp.rs2<<'\t'<<(int)tmp.rd<<'\t'<<tmp.imm<<'\n';
}
#endif //RISCV_SIMULATOR_DECODE_H
