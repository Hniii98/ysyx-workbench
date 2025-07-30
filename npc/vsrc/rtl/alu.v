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
    input IsSigned, // whether operation need sign
    output [31:0] alu_result
);
    wire[31:0] mux_a_wire; // output of oprand_a_mux
    wire[31:0] mux_b_wire; // output of oprand_b_mux

   /* Mux to select which be operand A to ALU */ 
   MuxKeyWithDefault #(2, 1, 32) operand_a_mux (
        .out(mux_a_wire),
        .key(ASel),
        .default_out(32'h0),
        .lut({
            `OPA_FROM_RS1, data_rs1,
            `OPA_FROM_PC, PC
        })
   );

   /*Mux to select whice be operand A to ALU */
   MuxKeyWithDefault #(2, 1, 32) operand_b_mux (
        .out(mux_b_wire),
        .key(BSel),
        .default_out(32'h0),
        .lut({
            `OPB_FROM_RS2, data_rs2,
            `OPB_FROM_IMM, imm
        })
   );
  
    /* ALU without compute logic  */

    wire [31:0] alu_unsigned_wire;

    wire [4:0] shamt = mux_b_wire[4:0];
    // computation that donot need signature
    MuxKeyWithDefault #(7, 4, 32) alu_without_sign_logic (
        .out(alu_unsigned_wire),
        .key(ALUOp),
        .default_out(32'hxxxx_xxxx), // for simulation debug
        .lut({
            `ALU_ADD    , mux_a_wire + mux_b_wire, 
            `ALU_SUB    , mux_a_wire - mux_b_wire, 
            `ALU_AND    , mux_a_wire & mux_b_wire,  
            `ALU_OR     , mux_a_wire | mux_b_wire,
            `ALU_XOR    , mux_a_wire ^ mux_b_wire,
            `ALU_SHIFL  , mux_a_wire << shamt,
            `ALU_PASS   , mux_b_wire
        })
    );
    
    // set less than logic
    wire signed_lessthan;
    wire unsigned_lessthan;

    assign signed_lessthan = $signed(mux_a_wire) < $signed(mux_b_wire);
    assign unsigned_lessthan = mux_a_wire < mux_b_wire;

    wire [31:0] alu_signed_wire;
    // computation that do need signature
    MuxKeyWithDefault #(4, 5, 32) alu_with_sign_logic (
        .out(alu_signed_wire),
        .key({ALUOp, IsSigned}),
        .default_out(32'hxxxx_xxxx),
        .lut({
            {`ALU_SHIFR, `TYPE_SIGNED}   , $signed(mux_a_wire) >>> shamt,
            {`ALU_SHIFR, `TYPE_UNSIGNED} , mux_a_wire >> shamt,
            {`ALU_SLT  , `TYPE_SIGNED}   , {31'b0, signed_lessthan}, // extend to 32 bits
            {`ALU_SLT  , `TYPE_UNSIGNED} , {31'b0, unsigned_lessthan} // extend to 32 bits
         })
    );
    
    /* ALU output data logic */
    wire need_sign_logic;

    assign need_sign_logic = (ALUOp == `ALU_SHIFR || ALUOp == `ALU_SLT);
    assign alu_result = need_sign_logic ? alu_signed_wire : alu_unsigned_wire;

    

  
   
endmodule
