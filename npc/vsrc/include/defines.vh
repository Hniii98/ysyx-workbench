`ifndef _DEFINES_VH_
`define _DEFINES_VH_  


`define U_TYPE 2'b00
`define J_TYPE 2'b01
`define I_TYPE 2'b10
`define S_TYPE 2'b11

`define PC_FROM_SNPC 1'b0
`define PC_FROM_ALU  1'b1

`define REG_WRITABLE	1'b1
`define REG_UNWRITABLE	1'b0  

`define OPA_FROM_PC		1'b1
`define OPA_FROM_RS1	1'b0
`define OPA_FROM_NCARE	1'b0

`define OPB_FROM_IMM	1'b1
`define OPB_FROM_RS2	1'b0
`define	OPB_FROM_NCARE  1'b0

`define ALU_ADD		2'b00
`define ALU_SUB		2'b01
`define ALU_PASS 	2'b10
`define ALU_NCARE	2'b00

`define WRITEBACK_FROM_SNPC		1'b1
`define WRITEBACK_FROM_ALU		1'b0
`define WRITEBACK_FROM_NCARE 	1'b0

`define X 1'b0 // inst do need all control signals, so set 0 to which don't care


`endif
