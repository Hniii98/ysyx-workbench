module fadder_32bit(
    input [31:0] a, b,
    input cin,
    output [31:0] sum, 
    output cout
    );

    wire [31:0] carry;

    assign cout = carry[31]; // msb carry out

    fadder fa[31:0](
        .a(a),
        .b(b),
        .cin({carry[30:0], cin}),
        .cout(carry),
        .sum(sum)
    );


endmodule
