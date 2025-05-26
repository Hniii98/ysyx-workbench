
module alu(
    input [31:0] operandA,
    input [31:0] operandB,
    output [31:0] result,
    output overflow
);
    fadder_32bit adder(
        .a(operandA),
        .b(operandB),
        .cin(0),
        .sum(result),
        .cout(overflow)
        
    );

endmodule
