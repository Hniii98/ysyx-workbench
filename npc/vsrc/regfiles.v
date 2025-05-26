module regfiles(
    input clk, 
    input rst,
    input wen, // write enable
    input [4:0] addrW,
    input [4:0] addrA,
    input [4:0] addrB,
    input [31:0] dataW,
    output reg [31:0]  dataA,
    output reg [31:0]  dataB,
    output reg [31:0]  pc
);

    wire [31:0] reg_out [31:0];   // conncet to reg0-reg31

    `define REG_INST(num) \
        Reg #(32, 32'b0) reg``num( \
            .clk(clk), \
            .rst(rst), \
            .wen(wen & (addrW == 5'(num))), \
            .din(dataW), \
            .dout(reg_out[5'(num)]) \
        ); \
      
        
    genvar i;
    generate
        for(i = 0; i < 32; i = i + 1) begin : gprloop
            if(i == 0) begin : gpr_zero
                 Reg #(32, 32'h0) reg0( 
                .clk(clk),
                .rst(rst),
                .wen(1), // always writable
                .din(32'h0),
                .dout(reg_out[0])
    );
            end
            else begin : gpr_other
                `REG_INST(i)
            end
        end 
    endgenerate

    always @(posedge clk) begin
        if(rst) begin
            pc <= 32'h80000000;
        end 
        else begin
            pc <= pc + 4;
        end     
    end

    assign dataA = reg_out[addrA]; // combination logic, mux 32 to 1.
    assign dataB = reg_out[addrB];

endmodule
