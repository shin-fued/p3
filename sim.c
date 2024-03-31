#include <stdio.h>
#include "shell.h"
#define GET_BITS(start, len, input) ((uint32_t) (((input) >> (start)) & ((1 << (len)) - 1)))
#define SIGN(in) (int32_t) in
#define USIGN(in) (uint32_t) in
#define GET_RS(in) USIGN(GET_BITS(21, 5, a))
#define GET_RT(in) USIGN((uint32_t) GET_BITS(16, 5, a)) 
#define GET_IM(in) USIGN(GET_BITS(0, 16, a))
#define REGS(in) CURRENT_STATE.REGS[in]
#define add(in) REGS(rt) = REGS(rs) + in

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    uint32_t a = mem_read_32(CURRENT_STATE.PC);
    uint32_t b = GET_BITS(26, 6, a);
    printf("b: %x\n", b);
    switch (b)
    {
    case (uint32_t)0x2:{ //J
        uint32_t temp=(GET_BITS(0, 26, a) << 2 )|| GET_BITS(28, 3, a); //how to delay instruction and check if this is correct
        jump(temp);
        break;
    }
    case (uint32_t)0x3:{ //JAL
        uint32_t temp=(GET_BITS(0, 26, a) << 2 ) || GET_BITS(28, 3, a); //how to delay and check if this is correct
        REGS(31) = CURRENT_STATE.PC + 8
        jump(temp);
        break;
    }
    case (uint32_t)0x4:{ //BEQ
        uint32_t target = GET_BITS(0, 14, GET_BITS(0, 15, a)) || (GET_BITS(0, 16, a) << 2); //sign extend this
        if(CURRENT_STATE.REGS[GET_RS(a)] == CURRENT_STATE.REGS[GET_RT(a)]){
            jump(target ,0)
        }
        break;
    }
    case (uint32_t)0x5:{ //BNE, sign extend this
        uint32_t target = SIGN(GET_BITS(0, 15, a)) || (GET_BITS(0, 16, a) << 2);
        if(CURRENT_STATE.REGS[GET_RS(a)] != CURRENT_STATE.REGS[GET_RT(a)]){
            jump(target ,0)
        }
        break;
    }
    case (uint32_t)0x6:{ //BLTEZ, sign extend this
        uint32_t target = SIGN(GET_BITS(0, 15, a)) || (GET_BITS(0, 16, a) << 2);
        if(CURRENT_STATE.REGS[GET_RS(a)] != CURRENT_STATE.REGS[GET_RT(a)]){
            jump(target ,0)
        }
        break;
    case (uint32_t)0x8:{ //ADDI
        int32_t rs = GET_RS(a)
        int32_t rt = GET_RT(a)
        int32_t im = GET_IM(a)
        add(im);
        printf("rt: %x\n", CURRENT_STATE.REGS[rt]);
        norm();
        break;
    }
    case (uint32_t)0x9:{ //ADDIU
        uint32_t rs = GET_RS(a);
        uint32_t rt = GET_RT(a);
        uint32_t im = GET_IM(a);
        add(im);
        norm();
        break;
    }
    default:
        break;
    }
}

void norm(){
    NEXT_STATE.PC=CURRENT_STATE.PC+4;
}

void jump(uint32_t target , int j){
    if(j){CURRENT_STATE.PC=CURRENT_STATE.PC + 4;}
    NEXT_STATE.PC=CURRENT_STATE.PC+target;
}