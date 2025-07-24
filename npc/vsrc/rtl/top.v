module top(
    input clk, rst,
    input [31:0] inst,
    output [31:0] PC
);

    /* Defination */
    wire    [4:0] rd;
    wire    [4:0] rs1;
    wire    [4:0] rs2;
    assign rd  = inst[11:7];
    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];

    /* Control signals */
    wire        control_to_pc_wire;
    wire [2:0]  control_to_immgen_wire;
    wire        control_to_regfiles_wire;
    wire        control_to_muxa_wire;
    wire        control_to_muxb_wire;
    wire [3:0]  control_to_alu_wire;
    wire        control_to_memory_wire;
    wire [1:0]  control_to_writeback_wire;
    wire        control_is_signed_wire;
    wire [1:0]  control_datasize_wire;

    /* Writeback data wire */
    wire [31:0] writeback_to_regfiles_wire;

    /* Regfiles data wire */
    wire [31:0] regfiles_data_a_wire;
    wire [31:0] regfiles_data_b_wire;
    wire [31:0] regfiles_x10_wire;

    /* Immediate value wire */
    wire [31:0] immgen_to_muxb_wire;

    /* PC data wire */
    wire [31:0] current_pc_wire;
    wire [31:0] static_next_pc_wire;
    
    /* ALU data wire */
    wire [31:0] alu_result_wire;

    /* Memory data wire */
    wire [31:0] memory_output_wire;

    /* Branch data wire */
    wire branch_is_equal;
    wire branch_is_lessthan;


    /* PC */
    pc u_pc(
        .clk            (clk),
        .rst            (rst),
        .PCSel          (control_to_pc_wire),
        .alu_result     (alu_result_wire),
        .PC             (current_pc_wire),
        .SNPC           (static_next_pc_wire)
    );

    /* Control */
    control u_control(
        .clk            (clk),
        .inst           (inst),
        .data_x10       (regfiles_x10_wire),
        .BrLt           (branch_is_lessthan),
        .BrEq           (branch_is_equal),
        .ImmType        (control_to_immgen_wire),
        .PCSrc          (control_to_pc_wire),
        .RegWEn         (control_to_regfiles_wire),
        .ASrc           (control_to_muxa_wire),
        .BSrc           (control_to_muxb_wire),
        .ALUOp          (control_to_alu_wire),
        .MemRW          (control_to_memory_wire),
        .WriteSrc       (control_to_writeback_wire),
        .IsSigned       (control_is_signed_wire),
        .DataSize       (control_datasize_wire)
    );

    /* Regfiles */
    regfiles u_regfiles(
        .clk            (clk),
        .rst            (rst),
        .RegWEn         (control_to_regfiles_wire),
        .addr_towrite   (rd),
        .addr_rs1       (rs1),
        .addr_rs2       (rs2),
        .data_towrite   (writeback_to_regfiles_wire),
        .data_rs1       (regfiles_data_a_wire),
        .data_rs2       (regfiles_data_b_wire),
        .data_x10       (regfiles_x10_wire)
    );

    /* Immgen */
    immgen u_immgen(
        .inst           (inst),
        .ImmType        (control_to_immgen_wire),
        .imm            (immgen_to_muxb_wire)
    );

    /* Branch */
    branch u_branch(
        .data_a   	(regfiles_data_a_wire    ),
        .data_b   	(regfiles_data_b_wire    ),
        .IsSigned 	(control_is_signed_wire  ),
        .BrEq     	(branch_is_equal      ),
        .BrLt     	(branch_is_lessthan      )
    );
    
    /* ALU */
    alu u_alu(
        .data_rs1       (regfiles_data_a_wire),
        .data_rs2       (regfiles_data_b_wire),
        .ASel           (control_to_muxa_wire),
        .BSel           (control_to_muxb_wire),
        .PC             (current_pc_wire),
        .imm            (immgen_to_muxb_wire),
        .ALUOp          (control_to_alu_wire),
        .IsSigned       (control_is_signed_wire),
        .alu_result     (alu_result_wire) 
    );
    
    /* Memory */
    memory u_memory(
        .clk          	(clk           ),
        .addr         	(alu_result_wire       ),
        .data_towrite 	(  regfiles_data_b_wire),
        .MemRW        	(control_to_memory_wire        ),
        .DataSize     	(control_datasize_wire      ),
        .IsSigned     	(control_is_signed_wire     ),
        .data_out     	(memory_output_wire    )
    );
    
    /* Writeback */
    writeback u_writeback(
        .alu_result     (alu_result_wire),
        .static_nextpc  (static_next_pc_wire),
        .mem_output     (memory_output_wire),
        .WBSel          (control_to_writeback_wire),
        .writeback_data (writeback_to_regfiles_wire)
    );
    

    assign PC = current_pc_wire;
  
endmodule
