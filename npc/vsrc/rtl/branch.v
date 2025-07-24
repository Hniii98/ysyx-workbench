module branch(
	input [31:0] data_a,
	input [31:0] data_b,
	input IsSigned,
	output BrEq, // branch equal
	output BrLt	 // branch less than
);

    wire signed [31:0] signed_a = data_a;
    wire signed [31:0] signed_b = data_b;
    
    assign BrEq = data_a == data_b;
    assign BrLt = IsSigned ? (signed_a < signed_b)  : (data_a < data_b);

endmodule
