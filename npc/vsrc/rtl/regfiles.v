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


    export "DPI-C" function npc_send_gprval;
    
    function int unsigned npc_send_gprval(int unsigned index);
        if (index < 32)
            npc_send_gprval = gpr[index];
        else
            npc_send_gprval = 0;
    endfunction
    
    reg [31:0] gpr [0:31];

    // 3. x0寄存器特殊处理
    always @(*) begin
        gpr[0] = 32'h0;  // 组合逻辑确保恒为0
    end
    

    // 4. 寄存器实例化宏
    `define REG_GENERATE(index) \
        Reg #(32, 32'h0) u_reg_``index ( \
            .clk(clk), \
            .rst(rst), \
            .wen(RegWEn && (addr_towrite == index)), \
            .din(data_towrite), \
            .dout(gpr[index]) \
        )

    // 5. 生成31个寄存器实例
    genvar i;
    generate
        for(i = 1; i < 32; i = i + 1) begin : gpr_gen_loop
            `REG_GENERATE(i);
        end
    endgenerate

    // 7. 读端口逻辑
    assign data_rs1 = gpr[addr_rs1];
    assign data_rs2 = gpr[addr_rs2];
    assign data_x10 = gpr[10];

      
endmodule
