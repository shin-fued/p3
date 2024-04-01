#include <stdio.h>
#include "shell.h"
#define GET_BITS(start, len, input) ((uint32_t) (((input) >> (start)) & ((1 << (len)) - 1)))
#define SIGN(in) (int32_t) in
#define USIGN(in) (uint32_t) in
#define GET_RS(in) USIGN(GET_BITS(21, 5, in))
#define GET_RT(in) USIGN(GET_BITS(16, 5, in))
#define GET_IM(in) USIGN(GET_BITS(0, 16, in))
#define REGS_C(in) CURRENT_STATE.REGS[in] //check if current or next
#define REGS_N(in) NEXT_STATE.REGS[in]
#define addi(in) NEXT_STATE.REGS[in] = SIGN(REGS_C(rs)) + SIGN(in)
#define addiu(in) NEXT_STATE.REGS[in] = USIGN(REGS_C(rs)) + USIGN(in)
#define PC_28to31 GET_BITS(28, 4, CURRENT_STATE.PC)


void norm(){
    NEXT_STATE.PC=CURRENT_STATE.PC+4;
}

void jumpu(uint32_t target){
    NEXT_STATE.PC=CURRENT_STATE.PC+target;
}

void jumpi(int32_t target){
    NEXT_STATE.PC= CURRENT_STATE.PC+target;
}

void stli(uint32_t in){
    if(SIGN(GET_RS(in)) < SIGN(GET_IM(in))){
        NEXT_STATE.REGS[GET_RT(in)] = 1;
    }
    else{
        NEXT_STATE.REGS[GET_RT(in)] = 0;
    }
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
            uint32_t temp = (GET_BITS(0, 26, a) << 2 );
            jumpu(temp);
            break;
        }
        case (uint32_t)0x3:{ //JAL
            uint32_t temp = (GET_BITS(0, 26, a) << 2 );
            REGS_C(31) = CURRENT_STATE.PC + 4;
            jumpu(temp);
            break;
        }
        case (uint32_t)0x4:{ //BEQ
            int32_t target = SIGN(GET_BITS(0, 16, a) << 2); //sign extend this
            if(CURRENT_STATE.REGS[GET_RS(a)] == CURRENT_STATE.REGS[GET_RT(a)]){
                jumpi(target);
            }
            break;
        }
        case (uint32_t)0x5:{ //BNE, sign extend this
            int32_t target = SIGN(GET_BITS(0, 16, a) << 2);
            if(CURRENT_STATE.REGS[GET_RS(a)] != CURRENT_STATE.REGS[GET_RT(a)]){
                jumpi(target);
            }
            break;
        }
        case (uint32_t)0x6:{ //BLEZ
            int32_t target = SIGN(GET_BITS(0, 16, a) << 2);
            uint32_t rs = GET_RS(a);
            if( REGS_C(rs) == 0 || 1 == GET_BITS(31, 1, REGS_C(rs))) {
                jumpi(target);
            }
            break;
        }
        case (uint32_t)0x7:{ //BGTZ
            int32_t target = SIGN(GET_BITS(0, 16, a) << 2);
            uint32_t rs = GET_RS(a);
            if(REGS_C(rs) != 0 && GET_BITS(31, 1, REGS_C(rs)) == 0){
                jumpi(target);
            }
            break;
        }
        case (uint32_t)0x8:{ //ADDI
            int32_t rs = GET_RS(a);
            int32_t rt = GET_RT(a);
            int32_t im = GET_IM(a);
            addi(im);
            printf("rt: %x\n", rt);
            norm();
            break;
        }
        case (uint32_t)0x9:{ //ADDIU
            uint32_t rs = GET_RS(a);
            uint32_t rt = GET_RT(a);
            uint32_t im = GET_IM(a);
            printf("rt: %x\n", rt);
            addiu(im);
            norm();
            break;
        }
        case (uint32_t)0xa:{ //stli
            stli(a);
        }
        default:
            break;
    }
}