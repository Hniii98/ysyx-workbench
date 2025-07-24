`include "../templates/MuxKeyWithDefault.v"
`include "../include/defines.vh"

module alu(
    input [31:0] data_rs1,    
    input [31:0] data_rs2,
    input ASel,
    input BSel,
    input [31:0] PC,
    input [31:0] imm,
    input [3:0] ALUOp,
    input IsSigned,
    output [31:0] alu_result
);
    wire[31:0] muxa_result_wire; // output of oprand_a_mux
    wire[31:0] muxb_result_wire; // output of oprand_b_mux

   /* Mux to select which be operand A to ALU */ 
   MuxKeyWithDefault #(2, 1, 32) operand_a_mux (
        .out(muxa_result_wire),
        .key(ASel),
        .default_out(32'h0),
        .lut({
            `OPA_FROM_RS1, data_rs1,
            `OPA_FROM_PC, PC
        })
   );

   /*Mux to select whice be operand A to ALU */
   MuxKeyWithDefault #(2, 1, 32) operand_b_mux (
        .out(muxb_result_wire),
        .key(BSel),
        .default_out(32'h0),
        .lut({
            `OPB_FROM_RS2, data_rs2,
            `OPB_FROM_IMM, imm
        })
   );
  
    /* ALU without compute logic  */

    wire [31:0] alu_wosign_wire;
    // computation that donot need signature
    MuxKeyWithDefault #(7, 4, 32) alu_without_sign_logic (
        .out(alu_wosign_wire),
        .key(ALUOp),
        .default_out(32'h0),
        .lut({
            `ALU_ADD    , muxa_result_wire + muxb_result_wire, 
            `ALU_SUB    , muxa_result_wire + (~muxb_result_wire + 32'h1), 
            `ALU_AND    , muxa_result_wire & muxb_result_wire,  
            `ALU_OR     , muxa_result_wire | muxb_result_wire,
            `ALU_XOR    , muxa_result_wire ^ muxb_result_wire,
            `ALU_SHIFL  , muxa_result_wire << (muxb_result_wire & 32'h1F),
            `ALU_PASS   , muxb_result_wire
        })
    );
    
    // set less than logic
    wire signed_lessthan;
    wire unsigned_lessthan;

    assign signed_lessthan = $signed(muxa_result_wire) < $signed(muxb_result_wire);
    assign unsigned_lessthan = muxa_result_wire < muxb_result_wire;

    wire [31:0] alu_sign_wire;
    // computation that do need signature
    MuxKeyWithDefault #(4, 5, 32) alu_with_sign_logic (
        .out(alu_sign_wire),
        .key({ALUOp, IsSigned}),
        .default_out(32'h0),
        .lut({
            {`ALU_SHIFR, `TYPE_SIGNED}   , $signed(muxa_result_wire) >> (muxb_result_wire & 32'h1F),
            {`ALU_SHIFR, `TYPE_UNSIGNED} , muxa_result_wire >> (muxb_result_wire & 32'h1F),
            {`ALU_SLT  , `TYPE_SIGNED}   , {31'h0, signed_lessthan}, // extend to 32 bits
            {`ALU_SLT  , `TYPE_UNSIGNED} , {31'h1, unsigned_lessthan} // extend to 32 bits
         })
    );
    

    assign alu_result = (ALUOp == `ALU_SHIFR || ALUOp == `ALU_SLT) ? alu_sign_wire : alu_wosign_wire;

  
   
endmodule
