`include "../templates/MuxKeyWithDefault.v"
`include "../include/defines.vh"
module immgen(
    input [31:0] inst,
    input [2:0] ImmType,
    output [31:0] imm
);
    /* Default immediate value set to 32'h0 */
    localparam DEFAULT_IMM = 32'h0; 

    /* Construct immdiate value via ImmType */
    MuxKeyWithDefault #(5, 3, 32) imm_mux (
        .out(imm),
        .key(ImmType),
        .default_out(DEFAULT_IMM), 
        .lut({
        `U_TYPE, {inst[31:12], {12{1'b0}}},
        `J_TYPE, {{12{inst[31]}}, inst[19:12], inst[20], inst[30:21], {1'b0}},
        `I_TYPE, {{21{inst[31]}}, inst[30:20]},
        `B_TYPE, {{20{inst[31]}}, inst[7], inst[30:25], inst[11:8], {1'b0}},
        `S_TYPE, {{21{inst[31]}}, inst[30:25], inst[11:8], inst[7]}
        })
    );

   
endmodule

