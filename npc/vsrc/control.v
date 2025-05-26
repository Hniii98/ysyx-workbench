import "DPI-C" function void end_loop();
module control(
    input [31:0] inst,
    output RegWEn
);

    wire [6:0] optype;
    wire [2:0] funct3;

    

    assign optype = inst[6:0];
    assign funct3 = inst[14:12];

    assign RegWEn = (optype == 7'b0010011) && (funct3 == 3'b000);
   

   always @(*) begin
    /* ebreak inst */
    if (inst == 32'h00100073) begin
        end_loop();  // DPI-C calling
    end
end
    

    
endmodule
