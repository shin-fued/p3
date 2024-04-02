#include <stdio.h>
#include "shell.h"
#define GET_BITS(start, len, input) ((uint32_t) (((input) >> (start)) & ((1 << (len)) - 1)))
#define USIGN(in) (uint32_t) in
#define SIGN(in) (int32_t) in
#define GET_RS(in) GET_BITS(21, 5, in)
#define GET_RT(in) GET_BITS(16, 5, in)
#define GET_IM(in) GET_BITS(0, 16, in)
#define REGS_C(in) CURRENT_STATE.REGS[in] //check if current or next
#define REGS_N(in) NEXT_STATE.REGS[in]
#define addi(in) NEXT_STATE.REGS[GET_RT(in)] = REGS_C(GET_RS(in)) + SIGN((GET_BITS(15, 1, in) ? 0xffff << 16 : 0) | GET_IM(in))
#define addiu(in) NEXT_STATE.REGS[in] = REGS_C(GET_RT(in)) + USIGN(GET_IM(in))
#define PC_28to31 GET_BITS(28, 4, CURRENT_STATE.PC)

typedef enum{
    Tr = 1,
    Fa = 0
} bool;


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
    if((SIGN(REGS_C(GET_RS(in))) < SIGN(GET_BITS(15, 1, in)? 0xffff  << 16: 0x0 | GET_IM(in))) && s){
        NEXT_STATE.REGS[GET_RT(in)] = USIGN(1);
    }
    if((REGS_C(GET_RS(in)) < USIGN(GET_IM(in) | (16 >> (GET_BITS(15,1,in) << 31)))) && !s){
        NEXT_STATE.REGS[GET_RT(in)] = USIGN(1);
    }
    else{
        NEXT_STATE.REGS[GET_RT(in)] = 0;
    }
}

void LB(uint32_t in){
    uint32_t addr = REGS_C(GET_RS(in)) + (GET_IM(in) | (16 >> (GET_BITS(15,1,in) << 31)));
    NEXT_STATE.REGS[GET_RT(in)] = (24 >> (GET_BITS(7, 1, mem_read_32(addr)) << 31)) | GET_BITS(0, 7, mem_read_32(addr));
    norm();
}

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    uint32_t a = mem_read_32(CURRENT_STATE.PC);
    uint32_t b = GET_BITS(26, 6, a);
    switch (b)
    {
        case (uint32_t)0x2:{ //J
            uint32_t temp = USIGN(GET_BITS(0, 26, a) << 2 );
            jumpu(temp);
            break;
        }
        case (uint32_t)0x3:{ //JAL
            uint32_t temp = USIGN(GET_BITS(0, 26, a) << 2 );
            REGS_C(31) = CURRENT_STATE.PC + 4;
            jumpu(temp);
            break;
        }
        case (uint32_t)0x4:{ //BEQ
            int32_t target = (GET_BITS(15, 1, a) ? 0x3fff << 14: 0x0) | (GET_IM(a) << 2);
            if(CURRENT_STATE.REGS[GET_RS(a)] == CURRENT_STATE.REGS[GET_RT(a)]){ //check sign extension
                jumpi(target);
            }
            else norm();
            break;
        }
        case (uint32_t)0x5:{ //BNE, sign extend this
            int32_t target = (GET_BITS(15, 1, a) ? 0x3fff << 14: 0x0) | (GET_IM(a) << 2);
            if(REGS_C(GET_RS(a)) != REGS_C(GET_RT(a))){
                jumpi(target);
            }
            else norm();
            break;
        }
        case (uint32_t)0x6:{ //BLEZ
            int32_t target = (GET_BITS(15, 1, a) ? 0x3fff << 14: 0x0) | (GET_IM(a) << 2);
            if( REGS_C(a) == 0 || 1 == GET_BITS(31, 1, REGS_C(GET_RS(a)))) {
                jumpi(target);
            }
            else norm();
            break;
        }
        case (uint32_t)0x7:{ //BGTZ
            int32_t target = (GET_BITS(15, 1, GET_IM(a)) ? 0x3fff << 14: 0x0) | (GET_IM(a) << 2);
            if(REGS_C(GET_RS(a)) != 0 && GET_BITS(31, 1, REGS_C(GET_RS(a))) == 0){
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
            NEXT_STATE.REGS[GET_RT(a)] = REGS_C(GET_RS(a)) & USIGN(GET_IM(a));
            norm();
            break;
        }
        case (uint32_t)0xd:{ //ori
            NEXT_STATE.REGS[GET_RT(a)] = REGS_C(GET_RS(a)) | USIGN(GET_IM(a));
            norm();
            break;
        }
        case (uint32_t)0xe:{ //xori
            NEXT_STATE.REGS[GET_RT(a)] = REGS_C(GET_RS(a)) ^ USIGN(GET_IM(a));
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
        default:
            break;
    }
}