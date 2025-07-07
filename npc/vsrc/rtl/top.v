module top(
    input clk, rst,
    input [31:0] inst,
    output [31:0] PC,
    output [31:0] alu_result
);

    /* 定义 */
    wire    [4:0] rd;
    wire    [4:0] rs1;
    wire    [4:0] rs2;
    assign rd  = inst[11:7];
    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];

    /* 控制信号 */
    wire [1:0]  control_to_immgen_wire;
    wire        control_to_pc_wire;
    wire        control_to_regfiles_wire;
    wire        control_to_muxa_wire;
    wire        control_to_muxb_wire;
    wire        control_to_writeback_wire;
    wire [1:0]  control_to_alu_wire;

    /* 写回信号 */
    wire [31:0] writeback_to_regfiles_wire;

    /* 寄存器文件信号 */
    wire [31:0] regfiles_to_muxa_wire;
    wire [31:0] regfiles_to_muxb_wire;
    wire [31:0] regfiles_to_control_wire;

    /* 立即数生成信号 */
    wire [31:0] immgen_to_muxb_wire;

    /* PC信号 */
    wire [31:0] current_pc_wire;
    wire [31:0] static_next_pc_wire;
    
    /* ALU信号 */
    wire [31:0] alu_result_wire;

    /* PC模块 */
    pc u_pc(
        .clk            (clk),
        .rst            (rst),
        .PCSel          (control_to_pc_wire),
        .alu_result     (alu_result_wire),
        .PC             (current_pc_wire),
        .SNPC           (static_next_pc_wire)
    );

    /* 控制模块 */
    control u_control(
        .clk            (clk),
        .inst           (inst),
        .data_x10       (regfiles_to_control_wire),
        .ImmType        (control_to_immgen_wire),
        .PCSrc          (control_to_pc_wire),
        .RegWEn         (control_to_regfiles_wire),
        .ASrc           (control_to_muxa_wire),
        .BSrc           (control_to_muxb_wire),
        .ALUOp          (control_to_alu_wire),
        .WriteSrc       (control_to_writeback_wire)
    );

    /* 寄存器文件模块 - 关键修改 */
    regfiles u_regfiles(
        .clk            (clk),
        .rst            (rst),
        .RegWEn         (control_to_regfiles_wire),
        .addr_towrite   (rd),
        .addr_rs1       (rs1),
        .addr_rs2       (rs2),
        .data_towrite   (writeback_to_regfiles_wire),
        .data_rs1       (regfiles_to_muxa_wire),
        .data_rs2       (regfiles_to_muxb_wire),
        .data_x10       (regfiles_to_control_wire)
    );

    /* 立即数生成模块 */
    immgen u_immgen(
        .inst           (inst),
        .ImmType        (control_to_immgen_wire),
        .imm            (immgen_to_muxb_wire)
    );

    /* ALU模块 */
    alu u_alu(
        .data_rs1       (regfiles_to_muxa_wire),
        .data_rs2       (regfiles_to_muxb_wire),
        .ASel           (control_to_muxa_wire),
        .BSel           (control_to_muxb_wire),
        .PC             (current_pc_wire),
        .imm            (immgen_to_muxb_wire),
        .ALUOp          (control_to_alu_wire),
        .alu_result     (alu_result_wire) 
    );
    
    /* 写回模块 */
    writeback u_writeback(
        .alu_result     (alu_result_wire),
        .SNPC           (static_next_pc_wire),
        .WBSel          (control_to_writeback_wire),
        .writeback_data (writeback_to_regfiles_wire)
    );
    
    /* 输出分配 */
    assign alu_result = alu_result_wire;
    assign PC = current_pc_wire;
   
  
endmodule