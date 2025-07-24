`include "../templates/MuxKeyWithDefault.v"
`include "../include/defines.vh"
module writeback(
	input [31:0] alu_result,
	input [31:0] static_nextpc,
	input [31:0] mem_output,
	input [1:0]  WBSel,
	output [31:0] writeback_data
);

	localparam DEFAULT_WBDATA = 32'h0;

	/* Mux output data depends on control signals WBSel */
	MuxKeyWithDefault #(3, 2, 32) writeback_mux(
		.out(writeback_data),
		.key(WBSel),
		.default_out(DEFAULT_WBDATA),
		.lut({
			`WRITEBACK_FROM_ALU ,	alu_result,	 
			`WRITEBACK_FROM_SNPC,	static_nextpc,
			`WRITEBACK_FROM_MEM ,   mem_output
		})
	);
	
	
endmodule
