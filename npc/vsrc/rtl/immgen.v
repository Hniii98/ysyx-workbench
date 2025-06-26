`include "../templates/MuxKeyWithDefault.v"
`include "../include/defines.vh"
module immgen(
    input [31:0] inst,
    input [1:0] ImmType,
    output [31:0] imm
);
   localparam UNKNOWN_TYPE_IMM = 32'h0; 
    /* determine the output imm by low 7 bits of inst, default value 32'h0 */
    MuxKeyWithDefault #(3, 2, 32) imm_mux (
        .out(imm),
        .key(ImmType),
        .default_out(UNKNOWN_TYPE_IMM), 
        .lut({
        `U_TYPE, {inst[31:12], {12{1'b0}}},
        `J_TYPE, {{12{inst[31]}}, inst[19:12], inst[20], inst[30:21], {1'b0}},
        `I_TYPE, {{21{inst[31]}}, inst[30:20]}
        })
    );


endmodule
