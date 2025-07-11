`include "../templates/Reg.v"
module regfiles(
    input clk, 
    input rst,
    input RegWEn,
    input [4:0] addr_towrite,
    input [4:0] addr_rs1,
    input [4:0] addr_rs2,
    input [31:0] data_towrite,
    output [31:0] data_rs1, 
    output [31:0] data_rs2,
    output [31:0] data_x10
); 
    reg [31:0] gpr [0:31];

    /* reg 0  */
    always @(*) begin
        gpr[0] = 32'h0;  
    end
    
    /* define marco */
    `define REG_GENERATE(index) \
        Reg #(32, 32'h0) u_reg_``index ( \
            .clk(clk), \
            .rst(rst), \
            .wen(RegWEn && (addr_towrite == index)), \
            .din(data_towrite), \
            .dout(gpr[index]) \
        )

    /* reg 0 ~ Reg31 */
    genvar i;
    generate
        for(i = 1; i < 32; i = i + 1) begin : gpr_gen_loop
            `REG_GENERATE(i);
        end
    endgenerate

    /* assign output */
    assign data_rs1 = gpr[addr_rs1];
    assign data_rs2 = gpr[addr_rs2];
    assign data_x10 = gpr[10];



     export "DPI-C" function npc_send_gprval;
 
    function int unsigned npc_send_gprval(int unsigned index);
        if (index < 32)
            npc_send_gprval = gpr[index];
        else
            npc_send_gprval = 0;
    endfunction

      
endmodule
