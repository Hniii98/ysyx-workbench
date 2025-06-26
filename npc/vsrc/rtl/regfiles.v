`include "../templates/Reg.v"

module regfiles(
    input clk, 
    input rst,
    input RegWEn, // write enable
    input [4:0] addr_towrite, // address for data_towrite
    input [4:0] addr_rs1, // address for DataA
    input [4:0] addr_rs2, // address for DataB
    input [31:0] data_towrite, // data about to write
    output reg [31:0]  data_rs1, 
    output reg [31:0]  data_rs2
);

   reg [31:0] regfiles_output_reg [31:0];   

    `define REG_GENERATE(index) \
        Reg #(32, 32'h0) reg``index( \
            .clk(clk), \
            .rst(rst), \
            .wen(RegWEn && (addr_towrite == index)), \
            .din(data_towrite), \
            .dout(regfiles_output_reg[index]) \
        ); \
      
        
    genvar i;
    generate
        for(i = 0; i < 32; i = i + 1) begin : gpr_gen_loop
            if(i == 0) begin : gpr_0
                 Reg #(32, 32'h0) reg0( 
                .clk(clk),
                .rst(rst),
                .wen(1), // always writable
                .din(32'h0), // always zero
                .dout(regfiles_output_reg[0])
    );
            end
            else begin : gpr_i
                `REG_GENERATE (i)
            end
        end 
    endgenerate

    assign data_rs1 = regfiles_output_reg[addr_rs1];
    assign data_rs2 = regfiles_output_reg[addr_rs2];

endmodule
