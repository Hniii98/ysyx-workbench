`include "../templates/MuxKeyWithDefault.v"
`include "../include/defines.vh"
module writeback(
	input [31:0] alu_result,
	input [31:0] SNPC,
	input WBSel,
	output [31:0] writeback_data
);

	localparam DEFAULT_WBDATA = 32'h0;

	MuxKeyWithDefault #(2, 1, 32) writeback_mux(
		.out(writeback_data),
		.key(WBSel),
		.default_out(DEFAULT_WBDATA),
		.lut({
			`WRITEBACK_FROM_ALU ,	alu_result,	 
			`WRITEBACK_FROM_SNPC,	SNPC
		})
	);
	
	
endmodule
