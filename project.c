/* 
I have not used C language code obtained from other students, the
Internet, and any other unauthorized sources, either modified or
unmodified. If any code in my program was obtained from an authorized
source, such as textbook or course notes, that has been clearly noted
as a citation in the comments of the program
*/
// Mina Georgoudiou
// mi343301@ucf.edu

#include "spimcore.h"

// prototype for getbits function 
unsigned getBits(unsigned num, unsigned start, unsigned end);

// prototype for is aligned function
unsigned isAligned(unsigned addr);

/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    if (ALUControl == 0)
        *ALUresult = A + B;
    else if (ALUControl == 1)
        *ALUresult = A - B;
    else if (ALUControl == 2)
        *ALUresult = (int) A < (int) B ? 1 : 0;
    else if (ALUControl == 3)
        *ALUresult = A < B ? 1 : 0;
    else if (ALUControl == 4)
        *ALUresult = A & B;
    else if (ALUControl == 5)
        *ALUresult = A | B;
    else if (ALUControl == 6)
        *ALUresult = B << 16;
    else if (ALUControl == 7)
        *ALUresult = ~A;

    *Zero = *ALUresult == 0 ? 1 : 0;
    return;
}

/* Instruction Fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // fetch the instruction
    *instruction = MEM(PC);

    // check for halt conditions
    int isHalt = 0;
    
    // if beyond range
    if (PC < 0x0000 || PC > 0xFFFF)
        isHalt = 1;
    
    // if its not word aligned
    if (!isAligned(PC))
        isHalt = 1;
    
    return isHalt;
}

/* Instruction Partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    // utilize getbits function to split up the instruction
    *op = getBits(instruction, 26, 31);
    *r1 = getBits(instruction, 21, 25);
    *r2 = getBits(instruction, 16, 20);
    *r3 = getBits(instruction, 11, 15);
    *funct = getBits(instruction, 0, 5);
    *offset = getBits(instruction, 0, 15);
    *jsec = getBits(instruction, 0, 25);

    return;
}

/* Instruction Decode */
/* 30 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{

    // defaults
    controls->RegDst = 0;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 0;
    controls->MemWrite = 0;
    controls->ALUSrc = 0;
    controls->RegWrite = 0;

    // r-type
    if (op == 0) {
        controls->RegDst = 1;
        controls->ALUOp = 7;
        controls->RegWrite = 1;
    }
    // lw
    else if (op == 35) {
        controls->MemRead = 1;
        controls->MemtoReg = 1;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
    }
    // sw
    else if (op == 43) {
        controls->MemWrite = 1;
        controls->ALUSrc = 1;
    }
    // beq
    else if (op == 4) {
        controls->Branch = 1;
        controls->ALUOp = 1;
    }
    // slti
    else if (op == 10) {
        controls->ALUOp = 2;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
    }
    // sltiu
    else if (op == 11) {
        controls->ALUOp = 3;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
    }
    // addi
    else if (op == 8) {
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
    }
    // lui
    else if (op == 15) {
        controls->ALUOp = 6;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
    }
    // jump
    else if (op == 2) {
        controls->Jump = 1;
    }
    // invalid opcode
    else {
        return 1;
    }
    // otherwise, return false for ishalt
    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    // first half of the offset int
    unsigned half = getBits(offset, 0, 15);
    // determines whether to extend with 1s or 0s
    unsigned extender = getBits(offset, 15, 15);
    if (extender) {
        // fill first 16 bits with 1
        *extended_value = 0xFFFF0000 | half;
    }
    else {
        // just return the first half
        *extended_value = 0x00000000 | half;
    }
    return;
}

/* ALU Operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	// Handle second parameter to ALU
    unsigned B = ALUSrc == 1 ? extended_value : data2;

    // Handle ALU control unit
    if (ALUOp == 7) {
        // Add
        if (funct == 0b100000)
            ALUOp = 0;
        // Subtract
        else if (funct == 0b100010)
            ALUOp = 1;
        // And
        else if (funct == 0b100100)
            ALUOp = 4;
        // Or
        else if (funct == 0b100101)
            ALUOp = 5;
        // Slt
        else if (funct == 0b101010)
            ALUOp = 2;
        // Sltu
        else if (funct == 0b101011)
            ALUOp = 3;
        // Unknown funct code
        else
            return 1;
    }

    ALU(data1, B, ALUOp, ALUresult, Zero);
    return 0;

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    // reading mem
    if (MemRead) {
        // test if aligned
        if (isAligned(ALUresult))
            *memdata = MEM(ALUresult);
        else
            return 1;
    }
    // writing mem
    if (MemWrite) {
        // test if aligned
        if (isAligned(ALUresult))
            MEM(ALUresult) = data2;
        else
            return 1;
    }
    return 0;
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (RegWrite) {
        unsigned temp;
        // if memtoreg, the register will be updated using memdata
        if (MemtoReg)
            temp = memdata;
        // otherwise its just the aluresult
        else
            temp = ALUresult;
        // when regdst is true, update r3 (rd)
        if (RegDst)
            Reg[r3] = temp;
        // otherwise, update r2 (rt)
        else
            Reg[r2] = temp;
    }
    else
        return;
}

/* PC Update */
/* 15 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // branch case
    if (Branch && Zero)
        // branch address is calculated by adding 4 and extended_value leftshifted by 2 to the PC
        *PC += 4 + (extended_value << 2);
    // jump case
    else if (Jump)
        // jump address is calculated by concatenating the left 4 bits of PC + 4 and jsec leftshifted by 2
        *PC = ((*PC + 4) & 0xF0000000) | (jsec << 2);
    
    else
        // every other case is simply add 4 to PC
        *PC += 4;

}

// helper function that returns a certain interval of bits [start, end] from num 
unsigned getBits(unsigned num, unsigned start, unsigned end) {
    // create a mask by bitshifting 1 left the right amount and then subtracting one
    unsigned mask = (1 << (end - start + 1)) - 1;
    // bitwise and it with the number thats been shifted right the appropriate amount
    return (num >> start) & mask;
}

// helper function that checks if an unsigned int is a multiple of 4
unsigned isAligned(unsigned addr) {
    if (addr % 4 == 0)
        return 1;
    return 0;
}