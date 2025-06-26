`include "../templates/MuxKey.v"
`include "../templates/MuxKeyWithDefault.v"

module alu(
    input [31:0] data_rs1,    
    input [31:0] data_rs2,
    input ASel,
    input BSel,
    input [31:0] PC,
    input [31:0] imm,
    input [1:0] ALUOp,
    output [31:0] alu_result
);
    wire[31:0] muxa_result_wire; // output of oprand_a_mux
    wire[31:0] muxb_result_wire; // output of oprand_b_mux

   /* Mux to select which be operand A to ALU */ 
   MuxKey #(2, 1, 32) operand_a_mux (
        .out(muxa_result_wire),
        .key(ASel),
        .lut({
            1'b0, data_rs1,
            1'b1, PC
        })
   );

   /*Mux to select whice be operand A to ALU */
   MuxKey #(2, 1, 32) operand_b_mux (
        .out(muxb_result_wire),
        .key(BSel),
        .lut({
            1'b0, data_rs2,
            1'b1, imm
        })
   );
    
    /* ALU compute logic  */
    MuxKeyWithDefault #(3, 2, 32) alu_logic (
        .out(alu_result),
        .key(ALUOp),
        .default_out(muxa_result_wire + muxb_result_wire),
        .lut({
            2'b00, muxa_result_wire + muxb_result_wire, // add
            2'b01, muxa_result_wire + (~muxb_result_wire + 32'h1), // sub
            2'b10, muxb_result_wire
        })
    );

  

endmodule
