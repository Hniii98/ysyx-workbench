
module pc (
    input clk,
    input rst,
    input IsBr,
    input BrTaken,
    input IsJAL,
    input IsJALR,
    input [31:0] alu_result,   
    output [31:0] pc_current,   // current PC value for fetching instruction
    output [31:0] pc_snpc // static next pc for writeback
    
);  

    reg [31:0] pc_reg;
    reg [31:0] next_pc;

    /* Next pc update logic */
    always @(*) begin
        if (IsJALR) begin 
            next_pc = alu_result;      
        end
        else if (IsJAL) begin 
            next_pc = alu_result;      
        end
        else if (IsBr && BrTaken) begin
            next_pc = alu_result;     
        end
        else begin
            next_pc = pc_reg + 32'h4;  
        end
    end

    
    /* Current PC update logic (synchronous reset) */
    always @(posedge clk) begin
        if (rst)
            pc_reg <= 32'h8000_0000;
        else
            pc_reg <= next_pc;
    end

    /* Output assignment */
    assign pc_current = pc_reg;
    assign pc_snpc= pc_reg + 32'h4; // jal and jalr instruction need this result.



    /* ------------------------- DPI-C ------------------------------*/
    export "DPI-C" function npc_send_nextpc;
 
    function int unsigned npc_send_nextpc();
        npc_send_nextpc = next_pc;
    endfunction
 
endmodule


