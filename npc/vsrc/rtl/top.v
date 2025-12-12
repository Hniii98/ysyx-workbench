module top(
    input clk, rst,
    input [31:0] inst,
    output [31:0] PC
);

    /* Defination */
    wire    [4:0] inst_rd;
    wire    [4:0] inst_rs1;
    wire    [4:0] inst_rs2;
    wire    [11:0] inst_csr;
    assign inst_rd  = inst[11:7];
    assign inst_rs1 = inst[19:15];
    assign inst_rs2 = inst[24:20];
    assign inst_csr = inst[31:20];

    /* Control signals */
    wire [2:0]  control_immtype;
    wire        control_regwen;
    wire        control_asrc;
    wire        control_bsrc;
    wire [3:0]  control_aluop;
    wire        control_memrw;
    wire [1:0]  control_wbsrc;
    wire        control_signed;
    wire [1:0]  control_dsize;
    wire [2:0]  control_brop; // branch option
    wire        control_isbr;
    wire        control_isjal;
    wire        control_isjalr;  
    wire        control_csrwen;
    wire [1:0]  control_csrop;
    wire        control_isecall;
    wire        control_ismret;



    /* Writeback wire */
    wire [31:0] writeback_data_out;

    /* Regfiles wire */
    wire [31:0] regfiles_rs1_data;
    wire [31:0] regfiles_rs2_data;
    wire [31:0] regfiles_x10_data;

    /* SysRegfiles wire */
    wire [31:0] sysregfiles_read_data;
    wire [31:0] sysregfiles_mtvec_data;
    wire [31:0] sysregfiles_mepc_data;

    /* Immediate  wire */
    wire [31:0] immgen_data;

    /* PC wire */
    wire [31:0] pc_current;
    wire [31:0] pc_snpc;
    
    /* ALU data wire */
    wire [31:0] alu_result;

    /* Memory data wire */
    wire [31:0] memory_data_out;

    /* Branch wire */
    wire        branch_brtaken;

    /* PC  */
    pc u_pc(
        .clk            (clk),
        .rst            (rst), 
        .IsBr           (control_isbr),
        .BrTaken        (branch_brtaken),
        .IsJAL          (control_isjal),
        .IsJALR         (control_isjalr),
        .IsECALL        (control_isecall),
        .IsMRET         (control_ismret),
        .alu_result     (alu_result),
        .data_mtvec     (sysregfiles_read_data),
        .data_mepc      (sysregfiles_mepc_data),
        .pc_current     (pc_current),
        .pc_snpc        (pc_snpc)    
    );

    /* Control */
    control u_control(
        .clk            (clk),
        .inst           (inst),
        .data_x10       (regfiles_x10_data), 
        .ImmType        (control_immtype),
        .RegWEn         (control_regwen),
        .ASrc           (control_asrc),
        .BSrc           (control_bsrc),
        .ALUOp          (control_aluop),
        .MemRW          (control_memrw),
        .WriteSrc       (control_wbsrc),
        .IsSigned       (control_signed),
        .DataSize       (control_dsize),
        .BrOp           (control_brop),
        .IsBr           (control_isbr),
        .IsJAL          (control_isjal),
        .IsJALR         (control_isjalr),
        .CSRWEn         (control_csrwen),
        .CSROp          (control_csrop),
        .IsECALL        (control_isecall),
        .IsMRET         (control_ismret)
    );

    /* Regfiles */
    regfiles u_regfiles(
        .clk            (clk),
        .rst            (rst),
        .RegWEn         (control_regwen),
        .waddr  (inst_rd),
        .raddr1       (inst_rs1),
        .raddr2       (inst_rs2),
        .wdata   (writeback_data_out), // data from writeback module
        .rdata1       (regfiles_rs1_data),
        .rdata2       (regfiles_rs2_data),
        .x10_data      (regfiles_x10_data)
    );

    /* SysRegfiles */
    sysregfiles u_sysregfiles(
        .clk           	(clk            ),
        .rst           	(rst            ),
        .CSRWEn        	(control_csrwen         ),
        .CSROp         	(control_csrop          ),
        .IsECALL        (control_isecall),
        .IsMRET         (control_ismret),
        .wraddr        	(inst_csr         ),
        .data_rs1      	(regfiles_rs1_data       ),
        .static_nextpc 	(pc_snpc  ),
        .rdata         	(sysregfiles_read_data          ),
        .data_mtvec    	(sysregfiles_mtvec_data     ),
        .data_mepc      (sysregfiles_mepc_data)
    );
    

    /* Immgen */
    immgen u_immgen(
        .inst           (inst), // top module input
        .ImmType        (control_immtype),
        .imm            (immgen_data)
    );

    /* Branch */
    branch u_branch(
        .data_a   	(regfiles_rs1_data    ),
        .data_b   	(regfiles_rs2_data    ),
        .BrOp       (control_brop),
        .BrTaken    (branch_brtaken)
    );
    
    /* ALU */
    alu u_alu(
        .data_rs1       (regfiles_rs1_data),
        .data_rs2       (regfiles_rs2_data),
        .ASel           (control_asrc),
        .BSel           (control_bsrc),
        .PC             (pc_current),
        .imm            (immgen_data),
        .ALUOp          (control_aluop),
        .IsSigned       (control_signed),
        .alu_result     (alu_result) 
    );
    
    /* Memory */
    memory u_memory(
        .clk          	(clk           ),
        .addr         	(alu_result      ),
        .data_towrite 	(regfiles_rs2_data),
        .MemRW        	(control_memrw        ),
        .DataSize     	(control_dsize      ),
        .IsSigned     	(control_signed     ),
        .data_out     	(memory_data_out    )
    );
    
    /* Writeback */
    writeback u_writeback(
        .alu_result     (alu_result),
        .static_nextpc  (pc_snpc),
        .mem_output     (memory_data_out),
        .csr_rdata      (sysregfiles_read_data),
        .WBSel          (control_wbsrc),
        .writeback_data (writeback_data_out)
    );
    

    assign PC = pc_current;
  
endmodule
