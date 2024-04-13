#include "spimcore.h"

// prototype for getbits function 
unsigned getBits(unsigned num, unsigned start, unsigned end);

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
    if (PC % 4 != 0)
        isHalt = 1;
    
    return isHalt;
}

/* Instruction Partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
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

}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{

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

}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{

}

/* PC Update */
/* 15 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{

}

// main function for testing stuff
/*
int main() {
    unsigned test = 0b110010001110;
    printf("%o %o %o", getBits(test, 0, 11), getBits(test, 6, 11), getBits(test, 0, 5));
}
*/

// helper function that returns a certain interval of bits [start, end] from num 
unsigned getBits(unsigned num, unsigned start, unsigned end) {
    // create a mask by bitshifting 1 left the right amount and then subtracting one
    unsigned mask = (1 << (end - start + 1)) - 1;
    // bitwise and it with the number thats been shifted right the appropriate amount
    return (num >> start) & mask;
}