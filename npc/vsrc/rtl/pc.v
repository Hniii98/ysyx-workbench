`include "../templates/MuxKey.v"
module pc (
    input clk,
    input rst,
    input PCSel,     
    input [31:0] alu_result,   
    output [31:0] PC,          // current PC value
    output [31:0] SNPC // static next pc
);

    reg [31:0] pc_result_reg;         
    wire [31:0] muxpc_result_wire;       
    wire [31:0] snpc_result_wire;     

    /* Mux to select which be input of PC update logic */
    MuxKey #(2, 1, 32) pc_input_mux (
        .out(muxpc_result_wire),
        .key(PCSel),
        .lut({
            1'b0, snpc_result_wire,   
            1'b1, alu_result   
        })
    );

    // PC update logic (synchronous reset)
    always @(posedge clk) begin
        if (rst) begin
            pc_result_reg <= 32'h8000_0000;  // reset to boot address
        end else begin
            pc_result_reg <= muxpc_result_wire;
        end
    end

    // Output assignments
    
    assign snpc_result_wire = pc_result_reg + 32'h4; //  combinational logic adder
    assign SNPC = snpc_result_wire;  // for register writeback (e.g., JAL)
    assign PC = pc_result_reg;
endmodule

