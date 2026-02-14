//[[ INCLUDES ]]
#include <stdint.h>
#include <string.h>

//[[ MACROS ]]
#define VPXNULL 0
#define VPX_RPC 62
#define VPX_RSP 63
//== error codes ==

#define VPX_ERR_RREG 1
#define VPX_ERR_WREG 2

#define VPX_ERR_MEM_R8 3
#define VPX_ERR_MEM_R16 4
#define VPX_ERR_MEM_R32 5

#define VPX_ERR_MEM_W8 6
#define VPX_ERR_MEM_W16 7
#define VPX_ERR_MEM_W32 8

#define VPX_ERR_DIV_BY_ZERO 9 //Float version has _F at the end.
#define VPX_ERR_DIV_BY_ZERO_S 10
#define VPX_ERR_DIV_INT32_MAX_N1 11
#define VPX_ERR_CJMP_INVALID 12

#define VPX_ERR_RREG_64 13
#define VPX_ERR_WREG_64 14

#define VPX_ERR_INVALID_OPCODE 255

//this is essentially for formatting, if the system is little endian it does nothing
//otherwise it swaps
#ifdef VPX_BIG_ENDIAN

static inline uint32_t vpx2_32b_endian_fmt(uint32_t val){
    return __builtin_bswap32(val);
}
static inline uint32_t vpx2_16b_endian_fmt(uint16_t val){
    return __builtin_bswap16(val);
}
#else
//Likely optimized away to nothingness
static inline uint32_t vpx2_32b_endian_fmt(uint32_t val){
    return val;
}
static inline uint32_t vpx2_16b_endian_fmt(uint16_t val){
    return val;
}
#endif


#ifndef VPX_DEFINED

//[[ SYSTEM ]]
uint32_t vpx2_cpu_id = 0b1; //Gamma version
//[[ ERROR ]]
uint8_t vpx2_err_code = 0;
uint32_t vpx2_err_val = 0;
uint32_t vpx2_err_pc_state = 0;

uint8_t* vpx2_mem_ptr = VPXNULL;
uint32_t vpx2_mem_size = 0;

uint32_t vpx2_registers[64] = {
    0,
    //r62 = RPC
    //r63 = RSP

    //You can swap the register file for a 256 register one if you want!
    //I'll try make this as customizable as possible i guess
};

#else
extern uint32_t vpx2_cpu_id;

extern uint8_t vpx2_err_code;
extern uint32_t vpx2_err_val;
extern uint32_t vpx2_err_pc_state;

//[[ MEMORY VARIABLES ]]

extern uint8_t* vpx2_mem_ptr;
extern uint32_t vpx2_mem_size;
extern uint32_t vpx2_registers[64];




#endif


//[[ ERROR FUNCTIONS ]]
static inline void vpx2_log_err(uint8_t code, uint32_t value){
    vpx2_err_code = code;
    vpx2_err_val = value;
    vpx2_err_pc_state = vpx2_registers[VPX_RPC];
}



//[[ CPU REGISTER FUNCTIONS ]]

#ifdef VPX_SAFE
static inline uint32_t vpx2_rreg(uint8_t reg){
    //You may remove this check if you provide a different
    //register stack without using unsafe mode
    if(reg >= 64){
        //Log attempted register read
        vpx2_log_err(1, reg);
        return 0;
    }
    return vpx2_registers[reg];
}
static inline void vpx2_wreg(uint8_t reg, uint32_t val){
    if(reg >= 64){

        //Log attempted register write
        vpx2_log_err(2, reg);
        return;
    }
    vpx2_registers[reg] = val;
}

#else
static inline uint32_t vpx2_rreg(uint8_t reg){
    return vpx2_registers[reg];
}
static inline void vpx2_wreg(uint8_t reg, uint32_t val){
    vpx2_registers[reg] = val;
}


#endif

//[[ MEMORY FUNCTIONS ]]

#ifdef VPX_SAFE
static inline uint8_t vpx2_mem_r8(uint32_t adr){
    if(adr >= vpx2_mem_size){
        vpx2_log_err(3, adr); //log code and value
        return 0;
    }
    return vpx2_mem_ptr[adr];
}
static inline uint16_t vpx2_mem_r16(uint32_t adr){
    if(adr >= vpx2_mem_size - 2){
        vpx2_log_err(4, adr); //log code and value
        return 0;
    }
    uint16_t ds;
    memcpy(&ds, &vpx2_mem_ptr[adr], 2);
    return vpx2_16b_endian_fmt(ds);
}

static inline uint32_t vpx2_mem_r32(uint32_t adr){
    if(adr >= vpx2_mem_size - 4){
        vpx2_log_err(5, adr); //log code and value
        return 0;
    }
    uint32_t ds;
    memcpy(&ds, &vpx2_mem_ptr[adr], 4);
    return vpx2_32b_endian_fmt(ds);
}

static inline void vpx2_mem_w8(uint32_t adr, uint8_t val){
    if(adr >= vpx2_mem_size){
        vpx2_log_err(6, adr); //log code and value
        return;
    }
    vpx2_mem_ptr[adr] = val;
}
static inline void vpx2_mem_w16(uint32_t adr, uint16_t val){
    if(adr >= vpx2_mem_size - 2){
        vpx2_log_err(7, adr); //log code and value
        return;
    }
    uint16_t tmp = vpx2_16b_endian_fmt(val);
    memcpy(&vpx2_mem_ptr[adr], &tmp, 2);
}
static inline void vpx2_mem_w32(uint32_t adr, uint32_t val){
    if(adr >= vpx2_mem_size - 4){
        vpx2_log_err(8, adr); //log code and value
        return;
    }
    uint32_t tmp = vpx2_32b_endian_fmt(val);
    memcpy(&vpx2_mem_ptr[adr], &tmp, 4);
}

#else

static inline uint8_t vpx2_mem_r8(uint32_t adr){
    return vpx2_mem_ptr[adr];
}
static inline uint16_t vpx2_mem_r16(uint32_t adr){
    uint16_t ds;
    memcpy(&ds, &vpx2_mem_ptr[adr], 2);
    return vpx2_16b_endian_fmt(ds);
}

static inline uint32_t vpx2_mem_r32(uint32_t adr){
    uint32_t ds;
    memcpy(&ds, &vpx2_mem_ptr[adr], 4);
    return vpx2_32b_endian_fmt(ds);
}


static inline void vpx2_mem_w8(uint32_t adr, uint8_t val){
    vpx2_mem_ptr[adr] = val;
}
static inline void vpx2_mem_w16(uint32_t adr, uint16_t val){
    uint16_t tmp = vpx2_16b_endian_fmt(val);
    memcpy(&vpx2_mem_ptr[adr], &tmp, 2);
}
static inline void vpx2_mem_w32(uint32_t adr, uint32_t val){
    uint32_t tmp = vpx2_32b_endian_fmt(val);
    memcpy(&vpx2_mem_ptr[adr], &tmp, 4);
}






#endif

//[[ CONCIDERING THEY USE WRITE AND READ FUNCTIONS, IT IS NOT REQUIRED TO MAKE SEPERATE SAFE AND UNSAFE VARIANTS ]]
static inline void vpx2_mem_pu8(uint8_t val){
    //Push 8 bit
    uint32_t adr = vpx2_rreg(VPX_RSP);
    //Write then increment.
    vpx2_mem_w8(adr, val);
    vpx2_wreg(VPX_RSP, adr+1);
}
static inline void vpx2_mem_pu16(uint16_t val){
    //Push 16 bit
    uint32_t adr = vpx2_rreg(VPX_RSP);
    uint16_t tmp = vpx2_16b_endian_fmt(val);
    //Write then increment.
    vpx2_mem_w16(adr, tmp);
    vpx2_wreg(VPX_RSP, adr+2);
}
static inline void vpx2_mem_pu32(uint32_t val){
    //Push 32 bit
    uint32_t adr = vpx2_rreg(VPX_RSP);
    uint32_t tmp = vpx2_32b_endian_fmt(val);
    //Write then increment.
    vpx2_mem_w32(adr, tmp);
    vpx2_wreg(VPX_RSP, adr+4);
}


static inline uint8_t vpx2_mem_po8(){
    //pop 8 bit
    uint32_t adr = vpx2_rreg(VPX_RSP);
    //Decrement then read
    vpx2_wreg(VPX_RSP, adr-1);

    return vpx2_mem_r8(adr-1);
}
static inline uint16_t vpx2_mem_po16(){
    //pop 16 bit
    uint32_t adr = vpx2_rreg(VPX_RSP);
    //Decrement then read
    vpx2_wreg(VPX_RSP, adr-2);

    return vpx2_16b_endian_fmt(vpx2_mem_r16(adr-2));
}
static inline uint32_t vpx2_mem_po32(){
    //pop 32 bit
    uint32_t adr = vpx2_rreg(VPX_RSP);
    //Decrement then read
    vpx2_wreg(VPX_RSP, adr-4);

    return vpx2_32b_endian_fmt(vpx2_mem_r32(adr-4));
}

//[[ FETCH (basically pop but using RPC sorta) ]]

static inline uint8_t vpx2_mem_f8(){
    uint32_t adr = vpx2_rreg(VPX_RPC);
    uint8_t val = vpx2_mem_r8(adr);
    vpx2_wreg(VPX_RPC, adr+1);
    return val;
}
static inline uint16_t vpx2_mem_f16(){
    uint32_t adr = vpx2_rreg(VPX_RPC);
    uint16_t val = vpx2_mem_r16(adr);
    vpx2_wreg(VPX_RPC, adr+2);
    return val;
}
static inline uint32_t vpx2_mem_f32(){
    uint32_t adr = vpx2_rreg(VPX_RPC);
    uint32_t val = vpx2_mem_r32(adr);
    vpx2_wreg(VPX_RPC, adr+4);
    return val;
}




//[[ ISA SECTION ]]

//[[ ISA INSTRUCTIONS ]]

static inline void vpx2_isa_cpuid(){
    //===========================================
    //Gets the value of the CPU-ID and information about it.
    //===========================================
    //C syntax: registers[r1] = cpu_id;
    //Pseudocode: r1 <- cpu_id
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    vpx2_wreg(r1, vpx2_cpu_id);
    

}

static inline void vpx2_isa_mov(){
    //===========================================
    //Move value in r1 to r2.
    //===========================================
    //C syntax: registers[r1] = registers[r2];
    //Pseudocode: r1 <- r2
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t val = vpx2_rreg(r2);
    vpx2_wreg(r1, val);
    

}
static inline void vpx2_isa_movi(){
    //===========================================
    //Move immediate value to r1
    //===========================================
    //C syntax: registers[r1] = imm;
    //Pseudocode: r1 <- imm
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t imm = vpx2_mem_f32();
    vpx2_wreg(r1, imm);
}
static inline void vpx2_isa_inc(){
    //===========================================
    //Increment value of r1
    //===========================================
    //C syntax: registers[r1] = registers[r1]++;
    //Pseudocode: r1 <- r1 + 1
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint32_t val = vpx2_rreg(r1);
    vpx2_wreg(r1, val + 1);
}
static inline void vpx2_isa_dec(){
    //===========================================
    //Decrement value of r1
    //===========================================
    //C syntax: registers[r1] = registers[r1]--;
    //Pseudocode: r1 <- r1 - 1
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint32_t val = vpx2_rreg(r1);
    vpx2_wreg(r1, val - 1);
}

static inline void vpx2_isa_or(){
    //===========================================
    //Do an OR operation on r2 and r3, write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] | registers[r3];
    //Pseudocode: r1 <- r2 or r3
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    uint32_t val1 = val2 | val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_xor(){
    //===========================================
    //Do an XOR operation on r2 and r3, write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] ^ registers[r3];
    //Pseudocode: r1 <- r2 xor r3
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    uint32_t val1 = val2 ^ val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_and(){
    //===========================================
    //Do an AND operation on r2 and r3, write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] & registers[r3];
    //Pseudocode: r1 <- r2 and r3
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    uint32_t val1 = val2 & val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_not(){
    //===========================================
    //Do a NOT operation on r2 and write to r1
    //===========================================
    //C syntax: registers[r1] = ~registers[r2];
    //Pseudocode: r1 <- not r2
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = ~val2;
    vpx2_wreg(r1, val1);
}

static inline void vpx2_isa_ori(){
    //===========================================
    //Do an OR operation on r2 and immediate, write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] | imm;
    //Pseudocode: r1 <- r2 or imm
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = val2 | imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_xori(){
    //===========================================
    //Do an XOR operation on r2 and immediate, write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] ^ imm;
    //Pseudocode: r1 <- r2 xor imm
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = val2 ^ imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_andi(){
    //===========================================
    //Do an AND operation on r2 and immediate, write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] & imm;
    //Pseudocode: r1 <- r2 and imm
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = val2 & imm;
    vpx2_wreg(r1, val1);
}

static inline void vpx2_isa_sll(){
    //===========================================
    //Shift logical left of r2 by r3 and write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] << registers[r3];
    //Pseudocode: r1 <- r2 << r3
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    uint32_t val1 = val2 << val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_srl(){
    //===========================================
    //Shift logical right of r2 by r3 and write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] >> registers[r3];
    //Pseudocode: r1 <- r2 >> r3
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    uint32_t val1 = val2 >> val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_sra(){
    //===========================================
    //Shift arithmetic right of r2 by r3 and write to r1
    //WARNING: might not work always!
    //might add inline assembly version if this doesn't work. with preproccessor checks
    //===========================================
    //C syntax: registers[r1] = registers[r2] >> registers[r3];
    //Pseudocode: r1 <- r2 >> r3
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    int32_t val1 = (int32_t)val2 >> (int32_t)val3;
    vpx2_wreg(r1, (uint32_t)val1);
}

static inline void vpx2_isa_slli(){
    //===========================================
    //Immediate Shift logical left of r2 by imm and write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] << imm;
    //Pseudocode: r1 <- r2 << imm
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t imm = vpx2_mem_f8(); //imm is 8 bits because you can't shift by more anyway

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = val2 << imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_srli(){
    //===========================================
    //Immediate Shift logical right of r2 by imm and write to r1
    //===========================================
    //C syntax: registers[r1] = registers[r2] >> imm;
    //Pseudocode: r1 <- r2 >> imm
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t imm = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
 

    uint32_t val1 = val2 >> imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_srai(){
    //===========================================
    //Immediate Shift arithmetic right of r2 by imm and write to r1
    //WARNING: might not work always!
    //might add inline assembly version if this doesn't work. with preproccessor checks
    //===========================================
    //C syntax: registers[r1] = registers[r2] >> imm;
    //Pseudocode: r1 <- r2 >> imm
    //===========================================

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t imm = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);

    int32_t val1 = (int32_t)val2 >> (int32_t)imm;
    vpx2_wreg(r1, (uint32_t)val1);
}

static inline void vpx2_isa_add(){
    //===========================================
    //Add r2 and r3, write result to r1. (No carry)
    //===========================================
    //C syntax: registers[r1] = registers[r2] + registers[r3];
    //Pseudocode: r1 <- r2 + r3
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    uint32_t val1 = val2 + val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_sub(){
    //===========================================
    //Subtract r2 by r3, write result to r1.
    //===========================================
    //C syntax: registers[r1] = registers[r2] - registers[r3];
    //Pseudocode: r1 <- r2 - r3
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    uint32_t val1 = val2 - val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_mul(){
    //===========================================
    //Multiply r2 by r3, write result to r1.
    //===========================================
    //C syntax: registers[r1] = registers[r2] * registers[r3];
    //Pseudocode: r1 <- r2 * r3
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    uint32_t val1 = val2 * val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_udiv(){
    //===========================================
    //Divide r2 by r3, write result to r1. (Unsigned)
    //===========================================
    //C syntax: registers[r1] = registers[r2] / registers[r3];
    //Pseudocode: r1 <- r2 / r3
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);
    #ifdef VPX_SAFE
    if(val3 == 0){
        //Division by 0 error
        //Log aswell the register that contained it.
        vpx2_log_err(VPX_ERR_DIV_BY_ZERO, r3);
        return;
    }
    #endif

    uint32_t val1 = val2 / val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_sdiv(){
    //===========================================
    //Divide r2 by r3, write result to r1. (Signed)
    //===========================================
    //C syntax: registers[r1] = registers[r2] / registers[r3];
    //Pseudocode: r1 <- r2 / r3
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    #ifdef VPX_SAFE
    //Signed version is stricter.
    if((val3 == 0) || ((int32_t)val2 == UINT32_MAX && (int32_t)val2 == -1)){
        //Division by 0 error (But signed)
        //Log aswell the register that contained it.
        vpx2_log_err(VPX_ERR_DIV_BY_ZERO_S, r3);
        return;
    }
    if((int32_t)val2 == INT32_MIN && (int32_t)val3 == -1){
        vpx2_log_err(VPX_ERR_DIV_INT32_MAX_N1, r2); //Signed conversion error.
        return;
    }
    #endif


    int32_t val1 = (int32_t)val2 / (int32_t)val3;
    vpx2_wreg(r1, (uint32_t)val1);
}
static inline void vpx2_isa_urem(){
    //===========================================
    //Modulo/Remainder of r2 by r3, write result to r1. (Unsigned)
    //===========================================
    //C syntax: registers[r1] = registers[r2] % registers[r3];
    //Pseudocode: r1 <- r2 % r3
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);
    #ifdef VPX_SAFE
    if(val3 == 0){
        //Division by 0 error
        //Log aswell the register that contained it.
        vpx2_log_err(9, r3);
        return;
    }
    #endif

    uint32_t val1 = val2 % val3;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_srem(){
    //===========================================
    //Modulo/Remainder of r2 by r3, write result to r1. (Signed)
    //===========================================
    //C syntax: registers[r1] = registers[r2] % registers[r3];
    //Pseudocode: r1 <- r2 % r3
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t r3 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);
    uint32_t val3 = vpx2_rreg(r3);

    #ifdef VPX_SAFE
    //Signed version is stricter.
    if((val3 == 0)){
        //Division by 0 error (But signed)
        //Log aswell the register that contained it.
        vpx2_log_err(10, r3);
        return;
    }
    if((int32_t)val2 == INT32_MIN && (int32_t)val3 == -1){
        vpx2_log_err(11, r2);
        return;
    }
    #endif


    int32_t val1 = (int32_t)val2 % (int32_t)val3;
    vpx2_wreg(r1, (uint32_t)val1);
}

static inline void vpx2_isa_addi(){
    //===========================================
    //Add r2 and imm, write result to r1. (No carry)
    //===========================================
    //C syntax: registers[r1] = registers[r2] + imm;
    //Pseudocode: r1 <- r2 + imm
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = val2 + imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_subi(){
    //===========================================
    //Subtract r2 and imm, write result to r1.
    //===========================================
    //C syntax: registers[r1] = registers[r2] - imm;
    //Pseudocode: r1 <- r2 - imm
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = val2 - imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_muli(){
    //===========================================
    //Multiply r2 by imm, write result to r1.
    //===========================================
    //C syntax: registers[r1] = registers[r2] * imm;
    //Pseudocode: r1 <- r2 * imm
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = val2 * imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_udivi(){
    //===========================================
    //Divide r2 by imm, write result to r1. (Unsigned)
    //===========================================
    //C syntax: registers[r1] = registers[r2] / imm;
    //Pseudocode: r1 <- r2 / imm
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);
    #ifdef VPX_SAFE
    if(imm == 0){
        //Division by 0 error
        vpx2_log_err(VPX_ERR_DIV_BY_ZERO, 0);
        return;
    }
    #endif

    uint32_t val1 = val2 / imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_sdivi(){
    //===========================================
    //Divide r2 by imm, write result to r1. (Signed)
    //===========================================
    //C syntax: registers[r1] = registers[r2] / imm;
    //Pseudocode: r1 <- r2 / imm
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    #ifdef VPX_SAFE
    //Signed version is stricter.
    if((imm == 0)){
        //Division by 0 error (But signed)
        vpx2_log_err(VPX_ERR_DIV_BY_ZERO_S, 0);
        return;
    }
    if((int32_t)val2 == INT32_MIN && (int32_t)imm == -1){
        vpx2_log_err(VPX_ERR_DIV_INT32_MAX_N1, r2); //Signed conversion error.
        return;
    }
    #endif


    int32_t val1 = (int32_t)val2 / (int32_t)imm;
    vpx2_wreg(r1, (uint32_t)val1);
}
static inline void vpx2_isa_uremi(){
    //===========================================
    //Modulo/Remainder of r2 by imm, write result to r1. (Unsigned)
    //===========================================
    //C syntax: registers[r1] = registers[r2] % imm;
    //Pseudocode: r1 <- r2 % imm
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);
    #ifdef VPX_SAFE
    if(imm == 0){
        //Division by 0 error
        vpx2_log_err(9, 0);
        return;
    }
    #endif

    uint32_t val1 = val2 % imm;
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_sremi(){
    //===========================================
    //Modulo/Remainder of r2 by imm, write result to r1. (Signed)
    //===========================================
    //C syntax: registers[r1] = registers[r2] % imm;
    //Pseudocode: r1 <- r2 % imm
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    #ifdef VPX_SAFE
    //Signed version is stricter.
    if((imm == 0)){
        //Division by 0 error (But signed)
        vpx2_log_err(10, 0);
        return;
    }
    if((int32_t)val2 == INT32_MIN && (int32_t)imm == -1){
        vpx2_log_err(11, r2);
        return;
    }
    #endif


    int32_t val1 = (int32_t)val2 % (int32_t)imm;
    vpx2_wreg(r1, (uint32_t)val1);
}

static inline void vpx2_isa_ld8(){
    //===========================================
    //Load 1B to r1 based on address in r2 + PC (Relative offset)
    //===========================================
    //C syntax: registers[r1] = mem_r8(registers[r2] + registers[PC]);
    //Pseudocode: r1 <- mem[r2 + PC]
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);

    uint8_t val1 = vpx2_mem_r8(val2 + pc);
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_ld16(){
    //===========================================
    //Load 2B to r1 based on address in r2 + PC (Relative offset)
    //===========================================
    //C syntax: registers[r1] = mem_r16(registers[r2] + registers[PC]);
    //Pseudocode: r1 <- mem[r2 + PC]
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);

    uint16_t val1 = vpx2_mem_r16(val2 + pc);
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_ld32(){
    //===========================================
    //Load 4B to r1 based on address in r2 + PC (Relative offset)
    //===========================================
    //C syntax: registers[r1] = mem_r32(registers[r2] + registers[PC]);
    //Pseudocode: r1 <- mem[r2 + PC]
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = vpx2_mem_r32(val2 + pc);
    vpx2_wreg(r1, val1);
}

static inline void vpx2_isa_st8(){
    //===========================================
    //Stores 1B from r1 (LSB) to address r2 + PC (Relative offset)
    //===========================================
    //C syntax: mem_w8(registers[r2] + registers[PC], registers[r1] & 0xff);
    //Pseudocode: mem[r2 + PC] <- r1
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    vpx2_mem_w8(val2 + pc, val1);
}
static inline void vpx2_isa_st16(){
    //===========================================
    //Stores 2B from r1 (LSW) to address r2 + PC (Relative offset)
    //===========================================
    //C syntax: mem_w16(registers[r2] + registers[PC], registers[r1] & 0xffff);
    //Pseudocode: mem[r2 + PC] <- r1
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    vpx2_mem_w16(val2 + pc, val1);
}
static inline void vpx2_isa_st32(){
    //===========================================
    //Stores 4B from r1 (LSW) to address r2 + PC (Relative offset)
    //===========================================
    //C syntax: mem_w32(registers[r2] + registers[PC], registers[r1] & 0xffffffff);
    //Pseudocode: mem[r2 + PC] <- r1
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    vpx2_mem_w32(val2 + pc, val1);
}

static inline void vpx2_isa_ld8r(){
    //===========================================
    //Load 1B to r1 based on address in r2 + imm (Relative/Absolute offset)
    //===========================================
    //C syntax: registers[r1] = mem_r8(registers[r2] + imm);
    //Pseudocode: r1 <- mem[r2 + imm]
    //===========================================

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = vpx2_mem_r8(val2 + imm);
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_ld16r(){
    //===========================================
    //Load 2B to r1 based on address in r2 + imm (Relative/Absolute offset)
    //===========================================
    //C syntax: registers[r1] = mem_r16(registers[r2] + imm);
    //Pseudocode: r1 <- mem[r2 + imm]
    //===========================================

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = vpx2_mem_r16(val2 + imm);
    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_ld32r(){
    //===========================================
    //Load 4B to r1 based on address in r2 + imm (Relative/Absolute offset)
    //===========================================
    //C syntax: registers[r1] = mem_r32(registers[r2] + imm);
    //Pseudocode: r1 <- mem[r2 + imm]
    //===========================================

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint8_t imm = vpx2_mem_f32();

    uint32_t val2 = vpx2_rreg(r2);

    uint32_t val1 = vpx2_mem_r32(val2 + imm);
    vpx2_wreg(r1, val1);
}

static inline void vpx2_isa_st8r(){
    //===========================================
    //Stores 1B from r1 (LSB) to address r2 + imm (Relative/Absolute offset)
    //===========================================
    //C syntax: mem_w8(registers[r2] + imm, registers[r1] & 0xff);
    //Pseudocode: mem[r2 + imm] <- r1
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    vpx2_mem_w8(val2 + imm, val1);
}
static inline void vpx2_isa_st16r(){
    //===========================================
    //Stores 2B from r1 (LSW) to address r2 + imm (Relative/Absolute offset)
    //===========================================
    //C syntax: mem_w16(registers[r2] + imm, registers[r1] & 0xffff);
    //Pseudocode: mem[r2 + imm] <- r1
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    vpx2_mem_w16(val2 + imm, val1);
}
static inline void vpx2_isa_st32r(){
    //===========================================
    //Stores 4B from r1 to address r2 + imm (Relative/Absolute offset)
    //===========================================
    //C syntax: mem_w32(registers[r2] + imm, registers[r1]);
    //Pseudocode: mem[r2 + imm] <- r1
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    vpx2_mem_w32(val2 + imm, val1);
}

static inline void vpx2_isa_jmp(){
    //===========================================
    //Jumps (Long) to specific address that is PC += imm
    //===========================================
    //C syntax: registers[PC] = registers[PC] + imm;
    //Pseudocode: PC <- PC + imm
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint32_t imm = vpx2_mem_f32();


    vpx2_wreg(VPX_RPC, pc + imm);
}
static inline void vpx2_isa_jmpr(){
    //===========================================
    //Jumps (Long) to specific address that is PC = imm + r1
    //===========================================
    //C syntax: registers[PC] = r1 + imm;
    //Pseudocode: PC <- r1 + imm
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint8_t r1 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();

    uint32_t val1 = vpx2_rreg(r1);


    vpx2_wreg(VPX_RPC, val1 + imm);
}

static inline void vpx2_isa_jmps(){
    //===========================================
    //Jumps (Short) to specific address that is PC += imm
    //===========================================
    //C syntax: registers[PC] = registers[PC] + imm;
    //Pseudocode: PC <- PC + imm
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint32_t imm = vpx2_mem_f16(); //16 bits instead of 32


    vpx2_wreg(VPX_RPC, pc + imm);
}
static inline void vpx2_isa_jmprs(){
    //===========================================
    //Jumps (Short) to specific address that is PC = imm + r1
    //===========================================
    //C syntax: registers[PC] = r1 + imm;
    //Pseudocode: PC <- r1 + imm
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    uint8_t r1 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f16(); //16 bits instead, less memory usage.

    uint32_t val1 = vpx2_rreg(r1);


    vpx2_wreg(VPX_RPC, val1 + imm);
}

static inline void vpx2_isa_zjmp(){
    //===========================================
    //Jumps (Long) to specific address that is PC = PC + imm
    //IFF the condition is met: r1 = 0.
    //===========================================
    //C syntax: if(registers[r1] == 0){
    //registers[PC] = registers[PC] + imm;
    //}
    //Pseudocode: PC <- PC + imm : r1 == 0
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    
    uint8_t r1 = vpx2_mem_f8();

    uint32_t imm = vpx2_mem_f32();

    //[[ CONDITION CHECK ]]
    uint32_t val1 = vpx2_rreg(r1);

    if(val1 == 0){
        vpx2_wreg(VPX_RPC, pc + imm);
    }
}
static inline void vpx2_isa_ejmp(){
    //===========================================
    //Jumps (Long) to specific address that is PC = PC + imm
    //IFF the condition is met: r1 = r2.
    //===========================================
    //C syntax: if(registers[r1] == registers[r2]){
    //registers[PC] = registers[PC] + imm;
    //}
    //Pseudocode: PC <- PC + imm : r1 == r2
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t imm = vpx2_mem_f32();

    //[[ CONDITION CHECK ]]
    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    if(val1 == val2){
        vpx2_wreg(VPX_RPC, pc + imm);
    }
}
static inline void vpx2_isa_nejmp(){
    //===========================================
    //Jumps (Long) to specific address that is PC = PC + imm
    //IFF the condition is met: r1 != r2.
    //===========================================
    //C syntax: if(registers[r1] != registers[r2]){
    //registers[PC] = registers[PC] + imm;
    //}
    //Pseudocode: PC <- PC + imm : r1 != r2
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t imm = vpx2_mem_f32();

    //[[ CONDITION CHECK ]]
    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    if(val1 != val2){
        vpx2_wreg(VPX_RPC, pc + imm);
    }
}
static inline void vpx2_isa_gjmp(){
    //===========================================
    //Jumps (Long) to specific address that is PC = PC + imm
    //IFF the condition is met: r1 > r2.
    //===========================================
    //C syntax: if(registers[r1] > registers[r2]){
    //registers[PC] = registers[PC] + imm;
    //}
    //Pseudocode: PC <- PC + imm : r1 > r2
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t imm = vpx2_mem_f32();

    //[[ CONDITION CHECK ]]
    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    if(val1 > val2){
        vpx2_wreg(VPX_RPC, pc + imm);
    }
}
static inline void vpx2_isa_gejmp(){
    //===========================================
    //Jumps (Long) to specific address that is PC = PC + imm
    //IFF the condition is met: r1 >= r2.
    //===========================================
    //C syntax: if(registers[r1] >= registers[r2]){
    //registers[PC] = registers[PC] + imm;
    //}
    //Pseudocode: PC <- PC + imm : r1 >= r2
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t imm = vpx2_mem_f32();

    //[[ CONDITION CHECK ]]
    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    if(val1 >= val2){
        vpx2_wreg(VPX_RPC, pc + imm);
    }
}
static inline void vpx2_isa_sjmp(){
    //===========================================
    //Jumps (Long) to specific address that is PC = PC + imm
    //IFF the condition is met: r1 < r2.
    //===========================================
    //C syntax: if(registers[r1] < registers[r2]){
    //registers[PC] = registers[PC] + imm;
    //}
    //Pseudocode: PC <- PC + imm : r1 < r2
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t imm = vpx2_mem_f32();

    //[[ CONDITION CHECK ]]
    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    if(val1 < val2){
        vpx2_wreg(VPX_RPC, pc + imm);
    }
}
static inline void vpx2_isa_sejmp(){
    //===========================================
    //Jumps (Long) to specific address that is PC = PC + imm
    //IFF the condition is met: r1 <= r2.
    //===========================================
    //C syntax: if(registers[r1] <= registers[r2]){
    //registers[PC] = registers[PC] + imm;
    //}
    //Pseudocode: PC <- PC + imm : r1 <= r2
    //===========================================

    uint32_t pc = vpx2_rreg(VPX_RPC) - 1; //Gets PC - 1 to remove opcode index

    
    uint8_t r1 = vpx2_mem_f8();
    uint8_t r2 = vpx2_mem_f8();

    uint32_t imm = vpx2_mem_f32();

    //[[ CONDITION CHECK ]]
    uint32_t val1 = vpx2_rreg(r1);
    uint32_t val2 = vpx2_rreg(r2);

    if(val1 <= val2){
        vpx2_wreg(VPX_RPC, pc + imm);
    }
}

static inline void vpx2_isa_cjmp(){
    //===========================================
    //General conditional jump.
    //Calls corresponding jmp condition instruction depending in provided argument.
    //WARNING: this is almost fully useless. So it may be depracated.
    //===========================================
    //C syntax: switch(condition){case 0: ...}
    //Pseudocode: f(x, y) ?: cond
    //===========================================

    uint8_t con = vpx2_mem_f8(); //Get condition code.

    switch(con){
        default: vpx2_log_err(VPX_ERR_CJMP_INVALID, con); //invalid conditon
        case 0: vpx2_isa_zjmp(); break;
        case 1: vpx2_isa_ejmp(); break;
        case 2: vpx2_isa_nejmp(); break;
        case 3: vpx2_isa_gjmp(); break;
        case 4: vpx2_isa_gejmp(); break;
        case 5: vpx2_isa_sjmp(); break;
        case 6: vpx2_isa_sejmp(); break;
    }






}

static inline void vpx2_isa_push8(){
    //===========================================
    //Push 1B value from r1 (LSB) into the stack.
    //===========================================
    //C syntax: append_stack(registers[r1])
    //Pseudocode: mem[sp] <- r1 : sp+=1
    //===========================================
    uint8_t r1 = vpx2_mem_f8();


    uint32_t val1 = vpx2_rreg(r1);

    vpx2_mem_pu8(val1); //Push

}
static inline void vpx2_isa_push16(){
    //===========================================
    //Push 2B value from r1 (LSW) into the stack.
    //===========================================
    //C syntax: append_stack(registers[r1])
    //Pseudocode: mem[sp] <- r1 : sp+=2
    //===========================================
    uint8_t r1 = vpx2_mem_f8();


    uint32_t val1 = vpx2_rreg(r1);

    vpx2_mem_pu16(val1); //Push

}
static inline void vpx2_isa_push32(){
    //===========================================
    //Push 4B value from r1 into the stack.
    //===========================================
    //C syntax: append_stack(registers[r1])
    //Pseudocode: mem[sp] <- r1 : sp+=4
    //===========================================
    uint8_t r1 = vpx2_mem_f8();


    uint32_t val1 = vpx2_rreg(r1);

    vpx2_mem_pu32(val1); //Push
}

static inline void vpx2_isa_pop8(){
    //===========================================
    //Pop value (1B) from stack and write to r1
    //===========================================
    //C syntax: sp--; registers[r1] = mem[sp];
    //Pseudocode: sp-=1 : r1 <- mem[sp]
    //===========================================
    uint8_t r1 = vpx2_mem_f8();

    uint32_t val1 = vpx2_mem_po8();

    vpx2_wreg(r1, val1);

}
static inline void vpx2_isa_pop16(){
    //===========================================
    //Pop value (2B) from stack and write to r1
    //===========================================
    //C syntax: sp-=2; registers[r1] = mem[sp];
    //Pseudocode: sp-=2 : r1 <- mem[sp]
    //===========================================
    uint8_t r1 = vpx2_mem_f8();

    uint32_t val1 = vpx2_mem_po16();

    vpx2_wreg(r1, val1);
}
static inline void vpx2_isa_pop32(){
    //===========================================
    //Pop value (4B) from stack and write to r1
    //===========================================
    //C syntax: sp-=4; registers[r1] = mem[sp];
    //Pseudocode: sp-=4 : r1 <- mem[sp]
    //===========================================
    uint8_t r1 = vpx2_mem_f8();

    uint32_t val1 = vpx2_mem_po32();

    vpx2_wreg(r1, val1);
}

static inline void vpx2_isa_call(){
    //===========================================
    //Call address that is defined by imm + PC
    //after storing return address to the stack.
    //===========================================
    //C syntax: append_stack(registers[PC]); registers[PC] = registers[PC] + imm;
    //Pseudocode: mem[sp] <- PC : PC <- PC + imm 
    //===========================================
    uint32_t rpc = vpx2_rreg(VPX_RPC) - 1; //Relative increment PC.
    
    uint32_t imm = vpx2_mem_f32();
    uint32_t spc = vpx2_rreg(VPX_RPC); //Gets PC after the fetch for next instruction's address.

    vpx2_wreg(VPX_RPC, rpc + imm);

    //Push to stack
    vpx2_mem_pu32(spc);
}
static inline void vpx2_isa_callr(){
    //===========================================
    //Call address that is defined by PC = r1 + imm
    //after storing return address to the stack.
    //===========================================
    //C syntax: append_stack(registers[PC]); registers[PC] = registers[r1] + imm;
    //Pseudocode: mem[sp] <- PC : PC <- r1 + imm 
    //===========================================
    uint8_t r1 = vpx2_mem_f8();
    uint32_t imm = vpx2_mem_f32();
    uint32_t val1 = vpx2_rreg(r1);


    uint32_t pc = vpx2_rreg(VPX_RPC); //Gets PC after the fetch for next instruction's address.

    vpx2_wreg(VPX_RPC, val1 + imm);

    //Push to stack
    vpx2_mem_pu32(pc);
}
static inline void vpx2_isa_ret(){
    //===========================================
    //Return to address that is in the stack. PC = mem[sp]
    //===========================================
    //C syntax: registers[PC] = mem[sp];
    //Pseudocode: PC <- mem[sp];
    //===========================================



    uint32_t pc = vpx2_mem_po32();

    vpx2_wreg(VPX_RPC, pc); //Return to address.

}

//[[ 64 BIT EXTENSION ]]
#ifdef VPX_ISA_64
static inline uint64_t vpx2_rreg_64(uint8_t reg){
    //Strictly safe version only. For now
    if(reg >= 32){
        vpx2_log_err(VPX_ERR_RREG_64, reg);
        return 0;
    }
    uint64_t val;
    memcpy(&val, vpx2_registers + (reg * 8), 8);
    return val;
}
static inline void vpx2_wreg_64(uint8_t reg, uint64_t val){
    if(reg >= 32){

        //Log attempted register write
        vpx2_log_err(VPX_ERR_WREG_64, reg);
        return;
    }
    uint64_t tmp = val;
    memcpy(vpx2_registers + (reg * 8), &tmp, 8);
}

//[[ ISA ]]

#endif




//[[ FPU EXTENSION ]]
#ifdef VPX_ISA_FPU



#endif

#ifdef VPX_ISA_FPU_64


#endif






//[[ ISA PRIMARY EXEC ]]

#ifdef VPX_SAFE
static inline uint8_t vpx2_exec(){
    //Triggers error on invalid opcode.
    uint8_t opcode = vpx2_mem_f8(); //Fetch opcode.

    switch(opcode){
        default: {
            //Log error and exit.
            vpx2_log_err(VPX_ERR_INVALID_OPCODE, opcode);
            
            return 1; //Error!

        }
        case 0: return 0; //Does nothing, NOP
        case 1: return 255; //Hostcall.
        case 2: vpx2_isa_cpuid(); break;
        case 3: vpx2_isa_mov(); break;
        case 4: vpx2_isa_movi(); break;
        case 5: vpx2_isa_inc(); break;
        case 6: vpx2_isa_dec(); break;
        case 7: vpx2_isa_or(); break;
        case 8: vpx2_isa_xor(); break;
        case 9: vpx2_isa_and(); break;
        case 10: vpx2_isa_not(); break;
        case 11: vpx2_isa_ori(); break;
        case 12: vpx2_isa_xori(); break;
        case 13: vpx2_isa_andi(); break;
        case 14: vpx2_isa_sll(); break;
        case 15: vpx2_isa_srl(); break;
        case 16: vpx2_isa_sra(); break;
        case 17: vpx2_isa_slli(); break;
        case 18: vpx2_isa_srli(); break;
        case 19: vpx2_isa_srai(); break;
        case 20: vpx2_isa_add(); break;
        case 21: vpx2_isa_sub(); break;
        case 22: vpx2_isa_mul(); break;
        case 23: vpx2_isa_udiv(); break;
        case 24: vpx2_isa_sdiv(); break;
        case 25: vpx2_isa_urem(); break;
        case 26: vpx2_isa_srem(); break;
        case 27: vpx2_isa_addi(); break;
        case 28: vpx2_isa_subi(); break;
        case 29: vpx2_isa_muli(); break;
        case 30: vpx2_isa_udivi(); break;
        case 31: vpx2_isa_sdivi(); break;
        case 32: vpx2_isa_uremi(); break;
        case 33: vpx2_isa_sremi(); break;
        case 34: vpx2_isa_ld8(); break;
        case 35: vpx2_isa_ld16(); break;
        case 36: vpx2_isa_ld32(); break;
        case 37: vpx2_isa_st8(); break;
        case 38: vpx2_isa_st16(); break;
        case 39: vpx2_isa_st32(); break;
        case 40: vpx2_isa_ld8r(); break;
        case 41: vpx2_isa_ld16r(); break;
        case 42: vpx2_isa_ld32r(); break;
        case 43: vpx2_isa_st8r(); break;
        case 44: vpx2_isa_st16r(); break;
        case 45: vpx2_isa_st32r(); break;
        case 46: vpx2_isa_jmp(); break;
        case 47: vpx2_isa_jmpr(); break;
        case 48: vpx2_isa_jmps(); break;
        case 49: vpx2_isa_jmprs(); break;
        case 50: vpx2_isa_zjmp(); break;
        case 51: vpx2_isa_ejmp(); break;
        case 52: vpx2_isa_nejmp(); break;
        case 53: vpx2_isa_gjmp(); break;
        case 54: vpx2_isa_gejmp(); break;
        case 55: vpx2_isa_sjmp(); break;
        case 56: vpx2_isa_sejmp(); break;
        case 57: vpx2_isa_cjmp(); break;
        case 58: vpx2_isa_push8(); break;
        case 59: vpx2_isa_push16(); break;
        case 60: vpx2_isa_push32(); break;
        case 61: vpx2_isa_pop8(); break;
        case 62: vpx2_isa_pop16(); break;
        case 63: vpx2_isa_pop32(); break;
        case 64: vpx2_isa_call(); break;
        case 65: vpx2_isa_callr(); break;
        case 66: vpx2_isa_ret(); break;

        



        #ifdef VPX_ISA_64
        //64 bit versions

        #endif

        #ifdef VPX_ISA_FPU


        #endif


        #ifdef VPX_ISA_FPU_64

        #endif


        
    }
    if(vpx2_err_code){return 1;} //error!
    return 0; //Successful execution
}


#else

static inline uint8_t vpx2_exec(){
    //Doesn't check invalid opcodes
    //Treats them as a NOP
    uint8_t opcode = vpx2_mem_f8(); //Fetch opcode.


    switch(opcode){
        default: {
            //Uusally should log errors and whatever
            //This is unsafe though, No errors
            return 0; //NOP
            break;

        }
        case 0: return 0; break; //Does nothing, NOP
        case 1: return 255; break; //Hostcall. (ECALL, environment call)
        case 2: vpx2_isa_cpuid(); break;
        case 3: vpx2_isa_mov(); break;
        case 4: vpx2_isa_movi(); break;
        case 5: vpx2_isa_inc(); break;
        case 6: vpx2_isa_dec(); break;
        case 7: vpx2_isa_or(); break;
        case 8: vpx2_isa_xor(); break;
        case 9: vpx2_isa_and(); break;
        case 10: vpx2_isa_not(); break;
        case 11: vpx2_isa_ori(); break;
        case 12: vpx2_isa_xori(); break;
        case 13: vpx2_isa_andi(); break;
        case 14: vpx2_isa_sll(); break;
        case 15: vpx2_isa_srl(); break;
        case 16: vpx2_isa_sra(); break;
        case 17: vpx2_isa_slli(); break;
        case 18: vpx2_isa_srli(); break;
        case 19: vpx2_isa_srai(); break;
        case 20: vpx2_isa_add(); break;
        case 21: vpx2_isa_sub(); break;
        case 22: vpx2_isa_mul(); break;
        case 23: vpx2_isa_udiv(); break;
        case 24: vpx2_isa_sdiv(); break;
        case 25: vpx2_isa_urem(); break;
        case 26: vpx2_isa_srem(); break;
        case 27: vpx2_isa_addi(); break;
        case 28: vpx2_isa_subi(); break;
        case 29: vpx2_isa_muli(); break;
        case 30: vpx2_isa_udivi(); break;
        case 31: vpx2_isa_sdivi(); break;
        case 32: vpx2_isa_uremi(); break;
        case 33: vpx2_isa_sremi(); break;
        case 34: vpx2_isa_ld8(); break;
        case 35: vpx2_isa_ld16(); break;
        case 36: vpx2_isa_ld32(); break;
        case 37: vpx2_isa_st8(); break;
        case 38: vpx2_isa_st16(); break;
        case 39: vpx2_isa_st32(); break;
        case 40: vpx2_isa_ld8r(); break;
        case 41: vpx2_isa_ld16r(); break;
        case 42: vpx2_isa_ld32r(); break;
        case 43: vpx2_isa_st8r(); break;
        case 44: vpx2_isa_st16r(); break;
        case 45: vpx2_isa_st32r(); break;
        case 46: vpx2_isa_jmp(); break;
        case 47: vpx2_isa_jmpr(); break;
        case 48: vpx2_isa_jmps(); break;
        case 49: vpx2_isa_jmprs(); break;
        case 50: vpx2_isa_zjmp(); break;
        case 51: vpx2_isa_ejmp(); break;
        case 52: vpx2_isa_nejmp(); break;
        case 53: vpx2_isa_gjmp(); break;
        case 54: vpx2_isa_gejmp(); break;
        case 55: vpx2_isa_sjmp(); break;
        case 56: vpx2_isa_sejmp(); break;
        case 57: vpx2_isa_cjmp(); break;
        case 58: vpx2_isa_push8(); break;
        case 59: vpx2_isa_push16(); break;
        case 60: vpx2_isa_push32(); break;
        case 61: vpx2_isa_pop8(); break;
        case 62: vpx2_isa_pop16(); break;
        case 63: vpx2_isa_pop32(); break;
        case 64: vpx2_isa_call(); break;
        case 65: vpx2_isa_callr(); break;
        case 66: vpx2_isa_ret(); break;

        #ifdef VPX_ISA_64
        //64 bit versions

        #endif

        #ifdef VPX_ISA_FPU


        #endif


        #ifdef VPX_ISA_FPU_64

        #endif

    }

    //No error checks due to this being the unsafe version.



    
    return 0; //Success
}


#endif

//[[ PRIMARY FUNCTIONS ]]

static inline uint8_t vpx2_init(uint8_t* mem_ptr, uint32_t mem_size){
    if(mem_ptr == VPXNULL){
        return 1; //Fail
    }
    if(mem_size == 0){
        return 1; //Fail
    }
    vpx2_mem_ptr = mem_ptr;
    vpx2_mem_size = mem_size;

    return 0; //Success


}
static inline uint8_t vpx2_start(){
    while(1){
        uint8_t rt = vpx2_exec();
        if(rt == 1){return 1;} //error exit
        if(rt == 255){return 0;} //hostcall successful exit.

    }
    return 0;


}

//[[ DEFINE MACRO ]]
#define VPX_DEFINED