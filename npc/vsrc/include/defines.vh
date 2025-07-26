`ifndef _DEFINES_VH_
`define _DEFINES_VH_  

// Some components must in a certain status, otherwise could be harmful
// to result. eg: regfiles only can be writable when it need to write. 
// The others if current instruction do need a fucntion of certian part 
// and it would not WRITE TO SOMEWHERE, we can set it to donnot care.

// default signals set to all zeor, in this circumstance everthing are unwritable.


/* Six instructions type in RV32I */
`define U_TYPE  	 3'b000
`define J_TYPE  	 3'b001
`define I_TYPE  	 3'b010
`define B_TYPE  	 3'b011
`define S_TYPE  	 3'b100
`define R_TYPE  	 3'b101
`define UNUSED_TYPE  3'b110  

/* Control signals to choose which could be next pc (static or dynamic) */
`define PC_FROM_SNPC 1'b0
`define PC_FROM_ALU  1'b1

/* Control signals to decide whether regfiles is writable */
`define REG_WRITABLE	1'b1
`define REG_UNWRITABLE	1'b0  

/* Control signal to decide operater of ALU */
// operand a for alu
`define OPA_FROM_PC		1'b1
`define OPA_FROM_RS1	1'b0
`define OPA_FROM_NCARE	1'bx
// operand b for alu
`define OPB_FROM_IMM	1'b1
`define OPB_FROM_RS2	1'b0
`define	OPB_FROM_NCARE  1'bx

/* Control signal to decide operation of ALU */
// logic and arithmetic oprations
`define ALU_ADD		4'b0000
`define ALU_SUB		4'b0001
`define ALU_AND		4'b0010
`define ALU_OR		4'b0011
`define ALU_XOR		4'b0100
// shift operation 
`define ALU_SHIFL	4'b0101
`define ALU_SHIFR	4'b0110
`define ALU_SLT     4'b0111  // set less than
// others
`define ALU_PASS 	4'b1000  //  pass oprand b to output directly
`define ALU_NCARE	4'bxxxx

/* Read or write control to memory block */
`define MEM_READ  1'b0
`define MEM_WRITE 1'b1


/* Branch instructions type */
// aligned to funct3 part in B type instructions
`define BRANCH_BEQ  3'b000
`define BRANCH_BNE  3'b001
`define BRANCH_BLT  3'b100
`define BRANCH_BGE  3'b101
`define BRANCH_BLTU 3'b110
`define BRANCH_BGEU 3'b111

`define BRANCH_TAKEN 	1'b1
`define BRANCH_UNTAKEN  1'b0


/* Control the source of data that write to regfiles */
`define WRITEBACK_FROM_SNPC		2'b00
`define WRITEBACK_FROM_ALU		2'b01
`define WRITEBACK_FROM_MEM      2'b10
`define WRITEBACK_FROM_NCARE 	2'bxx


/* Control signals to decide whether operator is unsigned */

// when compare two numbers and shift right a number
// component need sign information.
`define TYPE_SIGNED 	1'b1 
`define TYPE_UNSIGNED 	1'b0 
`define TYPE_NCARE 	1'bx 

/* Define data size of load/store instructions */
`define DATASIZE_BYTE 		2'b00
`define DATASIZE_HALWORD	2'b01
`define DATASIZE_WORD		2'b10
`define DATASIZE_NCARE      2'bxx



`endif
