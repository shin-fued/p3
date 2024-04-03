#include <stdio.h>
#include "shell.h"
#define GET_BITS(start, len, input) ((uint32_t) (((input) >> (start)) & ((1 << (len)) - 1)))
#define USIGN(in) (uint32_t) in
#define SIGN(in) (int32_t) in
#define SIGN64(in) (int64_t) in
#define GET_RS(in) USIGN(GET_BITS(21, 5, in))
#define GET_RT(in) USIGN(GET_BITS(16, 5, in))
#define GET_IM(in) USIGN(GET_BITS(0, 16, in))
#define GET_RD(in) USIGN(GET_BITS(11, 5, in))
#define GET_SP_OP(in) USIGN(GET_BITS(0, 6, in))
#define PC_28to31 GET_BITS(28, 4, CURRENT_STATE.PC)

typedef enum{
    Tr = 1,
    Fa = 0
} bool;

void addi(uint32_t in){
    uint32_t rt = GET_RT(in);
    uint32_t rs = GET_RS(in);
    NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + SIGN((GET_IM(in) | ((GET_BITS(14,1,in) << 31) >> 16)) | GET_IM(in));
}

void addiu(uint32_t in){
    NEXT_STATE.REGS[GET_RT(in)] = CURRENT_STATE.REGS[GET_RS(in)] + USIGN((GET_IM(in) | ((GET_BITS(14,1,in) << 31) >> 16)) | GET_IM(in));
}


void norm(){
    NEXT_STATE.PC=CURRENT_STATE.PC+4;
}

void jumpu(uint32_t target){
    NEXT_STATE.PC=CURRENT_STATE.PC + target;
}

void jumpi(int32_t target){
    NEXT_STATE.PC= CURRENT_STATE.PC + target;
}

void stli(uint32_t in, bool s){
    if((SIGN(CURRENT_STATE.REGS[GET_RS(in)]) < SIGN(GET_BITS(15, 1, in)? 0xffff  << 16: 0x0 | GET_IM(in))) && s){
        NEXT_STATE.REGS[GET_RT(in)] = USIGN(1);
    }
    if((CURRENT_STATE.REGS[GET_RS(in)] < USIGN(GET_IM(in) | ((GET_BITS(15,1,in) << 31)>> 16))) && !s){
        NEXT_STATE.REGS[GET_RT(in)] = USIGN(1);
    }
    else{
        NEXT_STATE.REGS[GET_RT(in)] = 0;
    }
}

void LB(uint32_t in){
    uint32_t addr = CURRENT_STATE.REGS[GET_RS(in)] + (GET_IM(in) | ((GET_BITS(15,1,in) << 31)>>16));
    NEXT_STATE.REGS[GET_RT(in)] = ((GET_BITS(7, 1, mem_read_32(addr)) << 31)>>24) | GET_BITS(0, 8, mem_read_32(addr));
    norm();
}

void LH(uint32_t in){
    uint32_t addr = CURRENT_STATE.REGS[GET_RS(in)] + (GET_IM(in) | ((GET_BITS(15,1,in) << 31) >>16));
    NEXT_STATE.REGS[GET_RT(in)] = ((GET_BITS(15, 1, mem_read_32(addr)) << 31) >>16) | GET_BITS(0, 16, mem_read_32(addr));
    norm();
}

void LW(uint32_t in){ //not working properly
    uint32_t addr = CURRENT_STATE.REGS[GET_RS(in)] + (GET_IM(in) | ((GET_BITS(15,1,in) << 31) >>16));
    NEXT_STATE.REGS[GET_RT(in)] = mem_read_32(addr);
    norm();
}

void LBU(uint32_t in){
    uint32_t addr = CURRENT_STATE.REGS[GET_RS(in)] + (GET_IM(in) | ((GET_BITS(7,1,in) << 31) >>16));
    NEXT_STATE.REGS[GET_RT(in)] = USIGN(0x0 | GET_BITS(0, 8, mem_read_32(addr)));
    norm();
}

void LHU(uint32_t in){
    uint32_t addr = CURRENT_STATE.REGS[GET_RS(in)] + (GET_IM(in) | ((GET_BITS(15,1,in) << 31) >>16));
    NEXT_STATE.REGS[GET_RT(in)] = USIGN(0x0 | GET_BITS(0, 16, mem_read_32(addr)));
    norm();
}

void SB(uint32_t in){
    uint32_t addr = CURRENT_STATE.REGS[GET_RS(in)] + (GET_IM(in) | ((GET_BITS(15,1,in) << 31) >> 16));
    uint32_t data = GET_BITS(0, 8, CURRENT_STATE.REGS[GET_RT(in)]);
    mem_write_32(addr, data);
    norm();
}

void SH(uint32_t in){
    uint32_t addr = CURRENT_STATE.REGS[GET_RS(in)] + (GET_IM(in) | ((GET_BITS(15,1,in) << 31) >>16));
    uint32_t data = GET_BITS(0, 16, CURRENT_STATE.REGS[GET_RT(in)]);
    mem_write_32(addr, data);
    norm();
}

void SW(uint32_t in){
    uint32_t addr = CURRENT_STATE.REGS[GET_RS(in)] + (GET_IM(in) | ((GET_BITS(15,1,in) << 31) >> 16));
    uint32_t data = CURRENT_STATE.REGS[GET_RT(in)];
    mem_write_32(addr, data);
    norm();
}

void BLTZ(uint32_t in){
    int32_t target = SIGN((GET_IM(in) << 2) | ((GET_BITS(15, 1 ,GET_IM(in)) << 31) >> 14));
    if (GET_BITS(31, 1, CURRENT_STATE.REGS[GET_RS(in)]) == 0x1)
    {
        NEXT_STATE.PC = CURRENT_STATE.PC + target;
    }
    else NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void BGEZ(uint32_t in){
    int32_t target = SIGN((GET_IM(in) << 2) | ((GET_BITS(15, 1, GET_IM(in)) << 31) >> 14));
    if (GET_BITS(31, 1, CURRENT_STATE.REGS[GET_RS(in)]) == 0x0)
    {
        NEXT_STATE.PC = CURRENT_STATE.PC + target;
    }
    else NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void BLTZAL(uint32_t in){
    int32_t target = SIGN((GET_IM(in) << 2) | ((GET_BITS(15, 1 ,GET_IM(in)) << 31) >> 14));
    CURRENT_STATE.REGS[31] =  CURRENT_STATE.PC + 4;
    if (GET_BITS(31, 1, CURRENT_STATE.REGS[GET_RS(in)]) == 0x1)
    {
        NEXT_STATE.PC = CURRENT_STATE.PC + target;
    }
    else NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void BGEZAL(uint32_t in){
    int32_t target = SIGN((GET_IM(in) << 2) | ((GET_BITS(15, 1 ,GET_IM(in)) << 31) >> 14));
    CURRENT_STATE.REGS[31] =  CURRENT_STATE.PC + 4;
    if (GET_BITS(31, 1, CURRENT_STATE.REGS[GET_RS(in)]) == 0x0)
    {
        NEXT_STATE.PC = CURRENT_STATE.PC + target;
    }
    else NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void SLL(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RT(in)] << GET_BITS(6, 5, in);
    norm();
}

void SRL(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RT(in)] >> GET_BITS(6, 5, in);
    norm();
}

void SRA(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = (CURRENT_STATE.REGS[GET_RT(in)] >> GET_BITS(6, 5, in)) | ((GET_BITS(31, 1, CURRENT_STATE.REGS[GET_RT(in)]) << 31) >> GET_BITS(6, 5, in));
    norm();
}

void SLLV(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RT(in)] << GET_BITS(0, 5, CURRENT_STATE.REGS[GET_RS(in)]);
    norm();
}

void SRLV(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RT(in)] >> GET_BITS(0, 5, CURRENT_STATE.REGS[GET_RS(in)]);
    norm();
}

void SRAV(uint32_t in){
    uint32_t s= GET_BITS(0, 5, CURRENT_STATE.REGS[GET_RS(in)]);
    NEXT_STATE.REGS[GET_RD(in)] = (CURRENT_STATE.REGS[GET_RT(in)] >> s) | ((GET_BITS(31, 1, CURRENT_STATE.REGS[GET_RT(in)]))<<31)>>s;
    norm();
}

void JR(uint32_t in){
    NEXT_STATE.PC=CURRENT_STATE.PC+CURRENT_STATE.REGS[GET_RS(in)];
}

void JALR(uint32_t in){ //current or next state
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.PC + 4;
    NEXT_STATE.PC=CURRENT_STATE.PC+CURRENT_STATE.REGS[GET_RS(in)];
}

void ADD(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = SIGN(CURRENT_STATE.REGS[GET_RS(in)]) + SIGN(CURRENT_STATE.REGS[GET_RT(in)]);
    norm();
}

void ADDU(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RS(in)] + CURRENT_STATE.REGS[GET_RT(in)];
    norm();
}

void SUB(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = SIGN(CURRENT_STATE.REGS[GET_RS(in)]) - SIGN(CURRENT_STATE.REGS[GET_RT(in)]);
    norm();
}

void SUBU(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RS(in)] - CURRENT_STATE.REGS[GET_RT(in)];
    norm();
}

void AND(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RS(in)] & CURRENT_STATE.REGS[GET_RT(in)];
    norm();
}

void OR(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RS(in)] | CURRENT_STATE.REGS[GET_RT(in)];
    norm();
}

void XOR(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.REGS[GET_RS(in)] ^ CURRENT_STATE.REGS[GET_RT(in)];
    norm();
}

void NOR(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = ~(CURRENT_STATE.REGS[GET_RS(in)] | CURRENT_STATE.REGS[GET_RT(in)]);
    norm();
}

void SLT(uint32_t in){
    if(SIGN(CURRENT_STATE.REGS[GET_RS(in)]) < SIGN(CURRENT_STATE.REGS[GET_RT(in)])){
        NEXT_STATE.REGS[GET_RD(in)] = 0x1;
    }
    else NEXT_STATE.REGS[GET_RD(in)] = 0x0;
    norm();
}

void SLTU(uint32_t in){
    if(CURRENT_STATE.REGS[GET_RS(in)] < CURRENT_STATE.REGS[GET_RT(in)]){
        NEXT_STATE.REGS[GET_RD(in)] = 0x1;
    }
    else NEXT_STATE.REGS[GET_RD(in)] = 0x0;
    norm();
}

void MULT(uint32_t in){
    int64_t t = SIGN64(SIGN(CURRENT_STATE.REGS[GET_RS(in)]))*SIGN64(SIGN(CURRENT_STATE.REGS[GET_RT(in)]));
    NEXT_STATE.LO = t & 0xffff;
    NEXT_STATE.HI = t & 0xffff0000;
    norm();
}

void MULTU(uint32_t in){
    uint64_t t = (uint64_t)CURRENT_STATE.REGS[GET_RS(in)]*(uint64_t)CURRENT_STATE.REGS[GET_RT(in)];
    NEXT_STATE.LO = t & 0xffff;
    NEXT_STATE.HI = t & 0xffff0000;
    norm();
}

void MFHI(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.HI;
    norm();
}

void MFLO(uint32_t in){
    NEXT_STATE.REGS[GET_RD(in)] = CURRENT_STATE.LO;
    norm();
}

void MTLO(uint32_t in){
    NEXT_STATE.LO = CURRENT_STATE.REGS[GET_RS(in)];
    norm();
}

void MTHI(uint32_t in){
    NEXT_STATE.HI = CURRENT_STATE.REGS[GET_RS(in)];
    norm();
}

void DIV(uint32_t in){
    NEXT_STATE.LO = SIGN(CURRENT_STATE.REGS[GET_RS(in)]) / SIGN(CURRENT_STATE.REGS[GET_RT(in)]);
    NEXT_STATE.HI = SIGN(CURRENT_STATE.REGS[GET_RS(in)]) % SIGN(CURRENT_STATE.REGS[GET_RT(in)]);
    norm();
}

void DIVU(uint32_t in){
    NEXT_STATE.LO = (CURRENT_STATE.REGS[GET_RS(in)]) / (CURRENT_STATE.REGS[GET_RT(in)]);
    NEXT_STATE.HI = (CURRENT_STATE.REGS[GET_RS(in)]) % (CURRENT_STATE.REGS[GET_RT(in)]);
    norm();
}

void SYSCALL(uint32_t in) {
    if (CURRENT_STATE.REGS[2] == 0xA) {
        RUN_BIT = 0;
    } else {
        norm();
    }
}

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    uint32_t a = mem_read_32(CURRENT_STATE.PC);
    uint32_t b = GET_BITS(26, 6, a);
    if(b == 0x0){
        uint32_t c = GET_SP_OP(a);
        switch (c)
        {
        case (uint32_t)0x0:
            SLL(a);
            break;
        case (uint32_t)0x2:
            SRL(a);
            break;
        case (uint32_t)0x3:
            SRA(a);
            break;
        case (uint32_t)0x4:
            SLLV(a);
            break;
        case (uint32_t)0x6:
            SRLV(a);
            break;
        case (uint32_t)0x8:
            JR(a);
            break;
        case (uint32_t)0x9:
            JALR(a);
            break;
        case (uint32_t)0xc:
            SYSCALL(a);
            break;
        case (uint32_t)0x10:
            MFHI(a);
            break;
        case (uint32_t)0x11:
            MTHI(a);
            break;
        case (uint32_t)0x12:
            MFLO(a);
            break;
        case (uint32_t)0x13:
            MTLO(a);
            break;
        case (uint32_t)0x18:
            MULT(a);
            break;
        case (uint32_t)0x19:
            MULTU(a);
            break;
        case (uint32_t)0x1a:
            DIV(a);
            break;
        case (uint32_t)0x1b:
            DIVU(a);
            break;
        case (uint32_t)0x20:
            ADD(a);
            break;
        case (uint32_t)0x21:
            ADDU(a);
            break;
        case (uint32_t)0x22:
            SUB(a);
            break;
        case (uint32_t)0x23:
            SUBU(a);
            break;
        case (uint32_t)0x24:
            AND(a);
            break;
        case (uint32_t)0x25:
            OR(a);
            break;
        case (uint32_t)0x26:
            XOR(a);
            break;
        case (uint32_t)0x27:
            NOR(a);
            break;
        case (uint32_t)0x2a:
            SLT(a);
            break;
        case (uint32_t)0x2b:
            SLTU(a);
            break;
        default:
            break;
        }
    }
    else if(b == 0x1){
        uint32_t c = GET_RS(a);
        switch (c)
        {
            case(uint32_t)0x0:{
                BLTZ(a);
                break;
            }
            case(uint32_t)0x1:{
                BGEZ(a);
                break;
            }
            case(uint32_t)0x10:{
                BLTZAL(a);
                break;
            }
            case(uint32_t)0x11:{
                BGEZAL(a);
                break;
            }
            default:
                break;
        }
    }
    switch (b)
    {
        case (uint32_t)0x2:{ //J
            uint32_t temp = USIGN(GET_BITS(0, 26, a) << 2 );
            jumpu(temp);
            break;
        }
        case (uint32_t)0x3:{ //JAL
            uint32_t temp = USIGN(GET_BITS(0, 26, a) << 2 );
            NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
            jumpu(temp);
            break;
        }
        case (uint32_t)0x4:{ //BEQ
            int32_t target = SIGN((GET_BITS(15, 1, a) ? 0x3fff << 14: 0x0) | (GET_IM(a) << 2));
            if(CURRENT_STATE.REGS[GET_RS(a)] == CURRENT_STATE.REGS[GET_RT(a)]){ //check sign extension
                jumpi(target);
            }
            else norm();
            break;
        }
        case (uint32_t)0x5:{ //BNE, sign extend this
            int32_t target = SIGN((GET_BITS(15, 1, a) ? 0x3fff << 14: 0x0) | (GET_IM(a) << 2));
            if(CURRENT_STATE.REGS[GET_RS(a)] != CURRENT_STATE.REGS[GET_RT(a)]){
                jumpi(target);
            }
            else norm();
            break;
        }
        case (uint32_t)0x6:{ //BLEZ
            int32_t target = SIGN((GET_BITS(15, 1, a) ? 0x3fff << 14: 0x0) | (GET_IM(a) << 2));
            if( CURRENT_STATE.REGS[GET_RS(a)] == 0 || 1 == GET_BITS(31, 1, CURRENT_STATE.REGS[GET_RS(a)])) {
                jumpi(target);
            }
            else norm();
            break;
        }
        case (uint32_t)0x7:{ //BGTZ
            int32_t target = SIGN((GET_BITS(15, 1, GET_IM(a)) ? 0x3fff << 14: 0x0) | (GET_IM(a) << 2));
            if(CURRENT_STATE.REGS[GET_RS(a)] != 0 && GET_BITS(31, 1, CURRENT_STATE.REGS[GET_RS(a)]) == 0){
                jumpi(target);
            }
            else norm();
            break;
        }
        case (uint32_t)0x8:{ //ADDI
            addi(a);
            norm();
            break;
        }
        case (uint32_t)0x9:{ //ADDIU
            addiu(a);
            //printf("rt: %x\n", rt);
            norm();
            break;
        }
        case (uint32_t)0xa:{ //stli
            stli(a, Tr);
            norm();
            break;
        }
        case (uint32_t)0xb:{ //stliu
            stli(a, Fa);
            norm();
            break;
        }
        case (uint32_t)0xc:{ //andi
            NEXT_STATE.REGS[GET_RT(a)] = CURRENT_STATE.REGS[GET_RS(a)] & USIGN(GET_IM(a));
            norm();
            break;
        }
        case (uint32_t)0xd:{ //ori
            NEXT_STATE.REGS[GET_RT(a)] = CURRENT_STATE.REGS[GET_RS(a)] | USIGN(GET_IM(a));
            norm();
            break;
        }
        case (uint32_t)0xe:{ //xori
            NEXT_STATE.REGS[GET_RT(a)] = CURRENT_STATE.REGS[GET_RS(a)] ^ USIGN(GET_IM(a));
            norm();
            break;
        }
        case (uint32_t)0xf:{ //lui
            NEXT_STATE.REGS[GET_RT(a)] = USIGN(GET_IM(a) << 16);
            norm();
            break;
        }
        case(uint32_t)0x20:{
            LB(a);
        }
        case(uint32_t)0x21:{
            LH(a);
        }
        case(uint32_t)0x22:{
            LW(a);
        }
        case(uint32_t)0x24:{
            LBU(a);
        }
        case(uint32_t)0x25:{
            LHU(a);
        }
        case(uint32_t)0x28:{
            SB(a);
        }
        case(uint32_t)0x29:{
            SH(a);
        }
        case(uint32_t)0x2b:{
            SW(a);
        }
        default:
            break;
    }
}