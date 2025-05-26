module immgen(
    input [11:0] inst,
    output [31:0] imm
);
    assign imm = { {21{inst[11]}}, inst[10:0] }; // I type imm    
endmodule
