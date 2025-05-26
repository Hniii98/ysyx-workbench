module top(
    input clk, rst,
    input [31:0] inst,
    output [31:0] pc,
    output overflow,
    output [31:0] result
);
    // output declaration of module control
    wire RegWEn;
    
    control u_control(
        .inst   	(inst    ),
        .RegWEn 	(RegWEn  )
    );
    
    regfiles u_regfiles(
        .clk   	(clk    ),
        .rst   	(rst    ),
        .wen   	( RegWEn    ),
        .addrW 	(inst[11:7]  ),
        .addrA 	(inst[19:15]  ),
        .addrB 	(inst[24:20]  ),
        .dataW 	(alu2reg),
        .dataA 	(reg2aluA),
        .dataB 	(reg2aluB),
        .pc    	(pc     )
    );

    wire [31:0] alu2reg;
    wire [31:0] reg2aluA, reg2aluB;
    wire [31:0] imm2aluB;
    
    immgen u_immgen(
        .inst 	(inst[31:20]  ),
        .imm  	(imm2aluB )
    );
    

    alu u_alu(
        .operandA 	(reg2aluA  ),
        .operandB 	(imm2aluB  ),
        .result   	(alu2reg    ),
        .overflow 	( overflow )
    );
    
    assign result = alu2reg;

endmodule
