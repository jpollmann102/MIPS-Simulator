// Joshua Pollmann
// Teron Lewis
// Robert Stevens-Olmo

// CDA3100

#include "spimcore.h"

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero)
{
	switch(ALUControl)
	{
		case 0 :
			// add
			*ALUresult = A + B;
			break;
		case 1 :
			// sub
			*ALUresult = A - B;
			break;
		case 2 :
			// slt
			if((signed)A < (signed)B) *ALUresult = 1;
			else *ALUresult = 0;
			break;
		case 3 :
			// sltu
			if(A < B) *ALUresult = 1;
			else *ALUresult = 0;
			break;
		case 4 :
			// and
			*ALUresult = A & B;
			break;
		case 5 :
			// or
			*ALUresult = A | B;
			break;
		case 6 :
			// shift B left by 16 bits
			*ALUresult = B << 16;
			break;
		case 7 :
			// not
			*ALUresult = ~A;
			break;
		default:
			;
	}

	// if zero = 1, then the value is 0
	if(*ALUresult == 0) *Zero = 1;
	else *Zero = 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction)
{
	// check for alignment
	if(PC % 4 == 0) *instruction = Mem[PC >> 2];
	// if misaligned, halt
	else return 1;
	return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
	unsigned r = 0x1f;
	unsigned functOp = 0x0000003f;
	unsigned offsetPartition = 0x0000ffff;
	unsigned jsecPartition = 0x0000ffff;
	
	*op = (instruction >> 26) & functOp;		// get bits 31-26
	*r1 = (instruction >> 21) & r;				// get bits 25-21
	*r2 = (instruction >> 16) & r;				// get bits 20-16
	*r3 = (instruction >> 11) & r;				// get bits 15-11
	*funct = instruction & functOp;				// get bits 5-0
	*offset = instruction & offsetPartition;	// get bits 15-0
	*jsec = instruction & jsecPartition;		// get bits 25-0

}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls)
{
	if(op == 0)
	{
		// r-type
		controls->RegWrite = 1;
		controls->MemWrite = 0;
		controls->MemtoReg = 0;
		controls->Branch = 2;
		controls->MemRead = 0;
		controls->RegDst = 1;
		controls->ALUSrc = 0;
		controls->ALUOp = 7;
		controls->Jump = 0;
	}else if(op == 2)
	{
		// j-type
		controls->RegWrite = 0;
		controls->MemWrite = 0;
		controls->MemtoReg = 2;
		controls->Branch = 0;
		controls->MemRead = 0;
		controls->RegDst = 2;
		controls->ALUSrc = 1;
		controls->ALUOp = 0;
		controls->Jump = 1;
	}else
	{
		// i-type
		controls->RegWrite = 1;
		controls->MemWrite = 0;
		controls->MemtoReg = 0;
		controls->Branch = 0;
		controls->MemRead = 0;
		controls->RegDst = 0;
		controls->ALUSrc = 1;
		controls->ALUOp = 0;
		controls->Jump = 2;

		// change controls based on op
		switch(op) 
		{
			case 4: // beq
		    	controls->Jump = 0;
				controls->ALUOp = 1;
				controls->ALUSrc = 0;
				controls->RegWrite = 0;
				controls->Branch = 0;
				controls->MemtoReg = 2;
				break;
			case 8: // addi
		    	controls->ALUOp = 0;
		    	break;
			case 9: // addiu
				controls->ALUOp = 0;
				break;
			case 10: // slti
				controls->ALUOp = 2;
				break;
			case 11: // sltiu
				controls->ALUOp = 3;
				break;
			case 15: // lui
		    	controls->MemRead = 0;
				controls->MemWrite = 0;
				controls->MemtoReg = 0;
				controls->ALUOp = 6;
				break;
			case 35: // lw
		   		controls->MemRead = 1;
				controls->MemWrite = 0;
				controls->MemtoReg = 1;
				break;
			case 43: // sw
		   		controls->MemRead = 0;
				controls->MemWrite = 1;
				controls->MemtoReg = 1;
				controls->RegWrite = 0;
				break;
			default:
				//no op found, return false
				return 1;
		}
	}
	return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2)
{
	*data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value)
{
	*extended_value = offset;
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero)
{	
	if(ALUSrc == 1) data2 = extended_value;

	if(ALUOp == 7)
	{
		switch(funct)
		{
			case 32:
				// add
				ALUOp = 0;
				break;
			case 34:
				// sub
				ALUOp = 1;
				break;
			case 42:
				// slt
				ALUOp = 2;
				break;
			case 43:
				// sltu
				ALUOp = 3;
				break;
			case 36:
				// and
				ALUOp = 4;
				break;
			case 37:
				// or
				ALUOp = 5;
				break;
			case 6:
				// shift left
				ALUOp = 6;
				break;
			case 39:
				// nor
				ALUOp = 7;
				break;
			default:
				// improper function, halt
				return 1;
		}
		// call ALU
		ALU(data1, data2, ALUOp, ALUresult, Zero);
	} else ALU(data1, data2, ALUOp, ALUresult, Zero);

	return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem)
{
	// get address by shifting ALUresult right 2
    unsigned address = ALUresult >> 2;
    // if writing to memory
	if(MemWrite == 1) {
        // check for correct word shift
		if(ALUresult % 4 != 0) return 1;
		// assign value to memory
		Mem[address] = data2;
	}
    // if reading
	if(MemRead == 1) 
	{
        // check for correct word shift
		if(ALUresult % 4 != 0) return 1;
		// read value from memory
		*memdata = Mem[address];
	}
	return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg)
{
	// if write and mem are 1, memdata is used
    if (RegWrite == 1 && MemtoReg == 1) 
    {
        if(RegDst == 0) Reg[r2] = memdata;
        else Reg[r3] = memdata;
    }
    // if write is 1 and mem is 0, ALUresult is used
    if (RegWrite == 1 && MemtoReg == 0)
    {
        if(RegDst == 0) Reg[r2] = ALUresult;
        else Reg[r3] = ALUresult;
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC)
{
 	int check = 0;
    // if there is a jump, set PC to jsec shifted 2 bits
	if(Jump == 1) 
	{
		*PC = jsec << 2;
		check = 1;
	}
	// if branch is not ignored, shift PC to the extended value shifted by 2
	else if(Branch == 1 || Branch == 0) 
	{
		if(Zero == 1) 
		{
			*PC = extended_value >> 2;
			check = 1;
		}
	}
	// if no branches or jumps, increment PC by 4
	if(check == 0)  *PC += 4;
}