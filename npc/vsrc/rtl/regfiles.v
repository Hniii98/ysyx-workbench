`include "../templates/Reg.v"
module regfiles(
    input clk, 
    input rst,
    input RegWEn,
    input [4:0] waddr,
    input [4:0] raddr1,
    input [4:0] raddr2,
    input [31:0] wdata,
    output [31:0] rdata1, 
    output [31:0] rdata2,
    output [31:0] x10_value // for DPI-C return value in control.v
); 
    localparam REG_WIDTH = 32;
    localparam REG_DEPTH = 32;

    /* General purpose regfiles */
    reg [REG_WIDTH-1:0] gpr [0:REG_DEPTH-1];

    
    /* Define marco */
    `define REG_GENERATE_MACRO(index) \
        Reg #(32, 32'h0) u_reg``index ( \
            .clk(clk), \
            .rst(rst), \
            .wen(RegWEn && (waddr == index)), \
            .din(wdata), \
            .dout(gpr[index]) \
        )

    /* 
     * Register 0 is hardwired to zero in RISC-V
     * Registers 1-31 are implemented as flip-flops
     */
    assign gpr[0] = 32'h0;
 
    genvar i;
    generate
        for(i = 1; i < 32; i = i + 1) begin : gpr_gen_loop
            `REG_GENERATE_MACRO(i);
        end
    endgenerate

    /* Assign output */
    assign rdata1 = gpr[raddr1];
    assign rdata2 = gpr[raddr2];
    assign x10_value = gpr[10];


    /*----------------------- DPI-C -----------------------*/
    export "DPI-C" function npc_send_gprval;
 
    function int unsigned npc_send_gprval(int unsigned index);
        if (index < REG_DEPTH)
            npc_send_gprval = gpr[index];
        else
            npc_send_gprval = 0;
    endfunction

      
endmodule
