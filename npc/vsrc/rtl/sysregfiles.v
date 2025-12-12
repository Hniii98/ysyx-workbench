`include "../templates/Reg.v"
`include "../templates/MuxKeyWithDefault.v"
`include "../include/defines.vh"

module  sysregfiles(
	input clk,
	input rst,
	input CSRWEn,
	input [1:0] CSROp,
	input IsECALL,
	input IsMRET,
	input [11:0] wraddr, // write & read address
	input [31:0] data_rs1,
	input [31:0] static_nextpc, // for ecall inst
	output [31:0] rdata,
	output [31:0] data_mtvec,
	output [31:0] data_mepc
);

wire [CSR_BITS-1:0] csr [0:CSR_NUMS-1];

/* CSR indices mapping logic */
localparam IDX_MSTATUS = 2'b00;
localparam IDX_MTVEC = 2'b01;
localparam IDX_MEPC = 2'b10;
localparam IDX_MCAUSE = 2'b11;


localparam CSR_BITS = 32; 
localparam CSR_NUMS = 4; 

wire [1:0] mapped_index;
wire map_valid;

MuxKeyWithDefault #(4, 12, 2) csr_index_mapping (
	.out(mapped_index),
	.key(wraddr),
	.default_out(IDX_MCAUSE), 
	.lut({
		12'h300, IDX_MSTATUS,
		12'h305, IDX_MTVEC  ,
		12'h341, IDX_MEPC   ,
		12'h342, IDX_MCAUSE  
	})
);

assign map_valid = 	(wraddr == 12'h300) ||
					(wraddr == 12'h305) ||
					(wraddr == 12'h341) ||
					(wraddr == 12'h342);


// mcause value for environment call from M-mode
localparam [CSR_BITS-1:0] CAUSE_M_MODE = 32'd11;
localparam CSR_DEFAULT_INPUT = 32'h00000000; 

// Compute writing data for exception */
wire except_active = (IsECALL == 1'b1);
wire [CSR_BITS-1:0] mepc_wdata = static_nextpc;
wire [CSR_BITS-1:0] mcause_wdata = CAUSE_M_MODE;


/* CSR writing logic */
reg  final_wen [CSR_NUMS-1];
reg [CSR_BITS-1:0] final_wdata [CSR_NUMS-1];

integer i;
always @(*) begin
	// default: no write
	for (i = 0; i < CSR_NUMS; i = i + 1) begin
		final_wen[i] = 1'b0;
		final_wdata[i] = 32'h0;
	end

	if (except_active) begin
		// exception writes
		final_wen[IDX_MEPC]   = 1'b1;
		final_wdata[IDX_MEPC] = mepc_wdata;

		final_wen[IDX_MCAUSE]   = 1'b1;
		final_wdata[IDX_MCAUSE] = mcause_wdata;
	end
	else if (CSRWEn && map_valid) begin
        final_wen[mapped_index]   = 1'b1;

        case (CSROp)
            `CSROp_WRITE:      final_wdata[mapped_index] = data_rs1;
            `CSROp_SETBITS:    final_wdata[mapped_index] = csr[mapped_index] | data_rs1;
            `CSROp_CLEARBITS:  final_wdata[mapped_index] = csr[mapped_index] & ~data_rs1;
            default:         final_wdata[mapped_index] = csr[mapped_index];
        endcase
    end
	
end


/* Define csr generate marco */
`define CSR_GENERATE_MACRO(index, default_val) \
	Reg #(CSR_BITS, default_val) u_csr``index ( \
		.clk(clk), \
		.rst(rst), \
		.wen(final_wen[index]), \
		.din(final_wdata[index]), \
		.dout(csr[index]) \
	)

/* instantiate registers (Reg module) with per-entry wen/data */

Reg #(CSR_BITS, 32'h00001800) u_csr_mstatus (
	.clk(clk), 
	.rst(rst),
	.wen(final_wen[IDX_MSTATUS]),
	.din(final_wdata[IDX_MSTATUS]),
	.dout(csr[IDX_MSTATUS])
);

Reg #(CSR_BITS, 32'h00000000) u_csr_mtvec (
	.clk(clk), 
	.rst(rst),
	.wen(final_wen[IDX_MTVEC]),
	.din(final_wdata[IDX_MTVEC]),
	.dout(csr[IDX_MTVEC])
);

Reg #(CSR_BITS, 32'h00000000) u_csr_mepc (
	.clk(clk), 
	.rst(rst),
	.wen(final_wen[IDX_MEPC]),
	.din(final_wdata[IDX_MEPC]),
	.dout(csr[IDX_MEPC])
);

Reg #(CSR_BITS, 32'h00000000) u_csr_mcause (
	.clk(clk), 
	.rst(rst),
	.wen(final_wen[IDX_MCAUSE]),
	.din(final_wdata[IDX_MCAUSE]),
	.dout(csr[IDX_MCAUSE])
);
	
/* Assignment */
assign rdata = csr[mapped_index];
assign data_mtvec = csr[IDX_MTVEC];
assign data_mepc = csr[IDX_MEPC];

endmodule

