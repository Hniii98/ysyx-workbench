
import "DPI-C" function int unsigned npc_pmem_read(input  int unsigned  raddr);
import "DPI-C" function void npc_pmem_write(input int unsigned waddr,
										input int unsigned  wdata,
										input byte wmask);

`include "../include/defines.vh"
`include "../templates/MuxKeyWithDefault.v"

module memory(
	input clk,
	input [31:0] addr,
	input [31:0] data_towrite,
	input MemRW, // memory read or write signals
	input [1:0] DataSize,
	input IsSigned,
	output [31:0] data_out
);

	
	/* Memory read logic */
	wire [31:0] raw_data;
	wire [31:0] signed_byte;
	wire [31:0] signed_halfword;
	wire [31:0] unsigned_byte;
	wire [31:0] unsigned_halfword;

	assign signed_byte   = ({{24{raw_data[7]}} , raw_data[7:0]});
	assign signed_halfword   = ({{16{raw_data[15]}}, raw_data[15:0]});
	assign unsigned_byte = ({{24{1'b0}} , raw_data[7:0]});
	assign unsigned_halfword = ({{16{1'b0}} , raw_data[15:0]});

	MuxKeyWithDefault #(4, 3, 32) data_out_mux ( 
			.out(data_out),
			.key({IsSigned, DataSize}),
			.default_out(raw_data),
			.lut({
				{`TYPE_SIGNED,   `DATASIZE_BYTE}, signed_byte, // signed byte
				{`TYPE_UNSIGNED, `DATASIZE_BYTE}, unsigned_byte, // unsigned byte
				{`TYPE_SIGNED,   `DATASIZE_HALWORD}, signed_halfword, // signed  half word
				{`TYPE_UNSIGNED, `DATASIZE_HALWORD}, unsigned_halfword // unsigned half word
			})
	);

	always @(*) begin
		if(MemRW == `MEM_READ) begin
			raw_data = npc_pmem_read(addr & ~32'h3); // align to four byte
		end
		else begin
			raw_data = 32'h0;
		end
	end

	
	/* Memory write logic */

	wire [7:0] mask;

	MuxKeyWithDefault #(3, 2, 8) datasize_mask_mux ( 
			.out(mask),
			.key(DataSize),
			.default_out(8'hFF), // default : write all bytes
			.lut({
				`DATASIZE_BYTE	  , 8'h3 ,    // write lowwest byte
				`DATASIZE_HALWORD , 8'hF ,   // wirte lowwest half word
				`DATASIZE_WORD	  , 8'hFF   // write word	    
				})
	);


	always @(posedge clk) begin
		if(MemRW == `MEM_WRITE) begin
			npc_pmem_write(addr, data_towrite, mask);
		end
	end



							
endmodule

