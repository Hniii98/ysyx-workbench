
module top(
    input clk, rst,
    input [31:0] inst,
    output [31:0] PC, // pc for cpp simulator
    output [31:0] alu_result
);

    /* definations */
    wire    [4:0] rd;
    wire    [4:0] rs1;
    wire    [4:0] rs2;
    assign rd  = inst[11:7];
    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];

    /* control output wires */
    wire [1:0]  control_to_immgen_wire;
    wire        control_to_pc_wire;
    wire        control_to_regfiles_wire;
    wire        control_to_muxa_wire;
    wire        control_to_muxb_wire;
    wire        control_to_writeback_wire;
    wire [1:0]  control_to_alu_wire;

    /* writeback output wires */
    wire [31:0] writeback_to_regfiles_wire;

    /* regfiles output wires */
    wire [31:0] regfiles_to_muxa_wire;
    wire [31:0] regfiles_to_muxb_wire;

    /* immgen output wires */
    wire [31:0] immgen_to_muxb_wire;

    /* pc output wires */
    wire [31:0] current_pc_wire;
    wire [31:0] static_next_pc_wire;
    

    /* alu output wires */
    wire[31:0] alu_result_wire;

    pc  u_pc(
        .clk            (clk),
        .rst            (rst),
        .PCSel          (control_to_pc_wire),
        .alu_result     (alu_result_wire),
        .PC             (current_pc_wire),
        .SNPC           (static_next_pc_wire)
        
    );

    control u_control(
        .inst   	(inst    ),
        .ImmType    (control_to_immgen_wire),
        .PCSrc      (control_to_pc_wire),
        .RegWEn     (control_to_regfiles_wire),
        .ASrc       (control_to_muxa_wire),
        .BSrc       (control_to_muxb_wire),
        .ALUOp      (control_to_alu_wire),
        .WriteSrc   (control_to_writeback_wire)
    );

    regfiles u_regfiles(
        .clk          	(clk    ),
        .rst          	(rst    ),
        .RegWEn         (control_to_regfiles_wire),
        .addr_towrite   (rd),
        .addr_rs1       (rs1),
        .addr_rs2       (rs2),
        .data_towrite   (writeback_to_regfiles_wire),
        .data_rs1       (regfiles_to_muxa_wire),
        .data_rs2       (regfiles_to_muxb_wire)
    );
  
    immgen u_immgen(
        .inst 	    (inst ),
        .ImmType    (control_to_immgen_wire),
        .imm        (immgen_to_muxb_wire)
    );
    
    always @(posedge clk) begin
        $display("Current control signals is: %b", {control_to_pc_wire, 
                                                    control_to_regfiles_wire,
                                                    control_to_muxa_wire,
                                                    control_to_muxb_wire,
                                                    control_to_alu_wire,
                                                    control_to_writeback_wire});

        $display("[verilog] IMMGEN: inst=%08x ImmType=%b  imm num=%08x  alu result:%h", inst, control_to_immgen_wire, immgen_to_muxb_wire, alu_result_wire);
    end


    alu u_alu(
       .data_rs1(regfiles_to_muxa_wire),
       .data_rs2(regfiles_to_muxb_wire),
       .ASel(control_to_muxa_wire),
       .BSel(control_to_muxb_wire),
       .PC(current_pc_wire),
       .imm(immgen_to_muxb_wire),
       .ALUOp(control_to_alu_wire),
       .alu_result(alu_result_wire) 
    );
    

    writeback u_writeback(
        .alu_result     	(alu_result_wire      ),
        .SNPC 	(static_next_pc_wire  ),
        .WBSel          	(control_to_writeback_wire         ),
        .writeback_data 	(writeback_to_regfiles_wire )
    );
    
    assign alu_result = alu_result_wire;
    assign PC = current_pc_wire;
   
    
endmodule
