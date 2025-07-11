`include "../include/defines.vh"
`include "../templates/MuxKeyWithDefault.v"

module control(
    input clk,
    input [31:0] inst, 
    input [31:0] data_x10,
    output [1:0] ImmType,
    output PCSrc,
    output RegWEn,
    output ASrc,
    output BSrc,
    output [1:0] ALUOp,
    output WriteSrc
);

    wire [1:0] imm_type_wire;
    wire [6:0] opcode;
 
    assign opcode = inst[6:0];
    localparam UNKNOWN_IMM_TYPE =  2'b11; // 

    /* level one decode of inst */ 
    MuxKeyWithDefault #(9, 7, 2) imm_type_mux (
        .out(imm_type_wire),
        .key(opcode),
        .default_out(UNKNOWN_IMM_TYPE),
        .lut({
            7'b011_0111, `U_TYPE, // lui
            7'b001_0111, `U_TYPE, // auipc
            7'b110_1111, `J_TYPE,
            7'b110_0111, `I_TYPE,
            7'b000_0011, `I_TYPE,
            7'b001_0011, `I_TYPE,
            7'b000_1111, `I_TYPE,
            7'b111_0011, `I_TYPE,
            7'b010_0011, `S_TYPE})
    );
 
    /*
      level 2 decode output signal in sequence  
      {PCSrc, RegWEn, ASrc, BSrc, ALUOp, WriteDest}
    */
    
    localparam [6:0] UNKNOWN_CTRL_SIGN = 7'b000_0000;
    
    /* level two decode unit of U-type instructions */ 
    wire [6:0] utype_ctrl_signals;
    wire utype_sel = inst[5];
   
    MuxKeyWithDefault #(2, 3, 7) utype_ctrl_mux (
        .out(utype_ctrl_signals),
        .key({imm_type_wire, utype_sel}),
        .default_out(UNKNOWN_CTRL_SIGN),
        .lut({
            // 格式: {key}, {value}
            {`U_TYPE, 1'b1}, {`PC_FROM_SNPC, `REG_WRITABLE, `OPA_FROM_NCARE  ,`OPB_FROM_IMM, `ALU_PASS, `WRITEBACK_FROM_ALU}, // lui
            {`U_TYPE, 1'b0}, {`PC_FROM_SNPC, `REG_WRITABLE, `OPA_FROM_PC     ,`OPB_FROM_IMM, `ALU_ADD , `WRITEBACK_FROM_ALU}  // auipc
        })
        

    );

    /* level two decode unit of J-type instructions */
    wire [6:0] jtype_ctrl_signals;

    MuxKeyWithDefault #(1, 2, 7) jtype_ctrl_mux (
        .out(jtype_ctrl_signals),
        .key(imm_type_wire),
        .default_out(UNKNOWN_CTRL_SIGN),
        .lut({
            `J_TYPE, {`PC_FROM_ALU, `REG_WRITABLE, `OPA_FROM_PC, `OPB_FROM_IMM, `ALU_ADD, `WRITEBACK_FROM_SNPC}
        })
    );


    /* level two decode unit of I-type instructions */
    wire [6:0] itype_ctrl_signals;

    MuxKeyWithDefault #(2, 7, 7) itype_ctrl_mux (
        .out(itype_ctrl_signals),
        .key(opcode),
        .default_out(UNKNOWN_CTRL_SIGN),
        .lut({
            7'b110_0111, {`PC_FROM_ALU,    `REG_WRITABLE, `OPA_FROM_RS1, `OPB_FROM_IMM, `ALU_ADD, `WRITEBACK_FROM_SNPC}, // jalr
            7'b001_0011, {`PC_FROM_SNPC,  `REG_WRITABLE, `OPA_FROM_RS1, `OPB_FROM_IMM, `ALU_ADD, `WRITEBACK_FROM_ALU } // addi
        })
    );

    /* level two decode unit of s-type instructiongs */
    wire [6:0] stype_ctrl_signals;
    // hard encode to unharmful empty for sw.
    assign stype_ctrl_signals = {`PC_FROM_SNPC, `REG_UNWRITABLE, `OPA_FROM_RS1, `OPB_FROM_RS2, `ALU_ADD, `WRITEBACK_FROM_ALU};


    /* output signal mux */
    reg [6:0] signals_mux_wire;
    
    MuxKeyWithDefault #(4, 2, 7) final_signals_mux (
        .out(signals_mux_wire),
        .key(imm_type_wire),
        .default_out(UNKNOWN_CTRL_SIGN),
        .lut({
            `U_TYPE, utype_ctrl_signals,
            `I_TYPE, itype_ctrl_signals,
            `J_TYPE, jtype_ctrl_signals,
            `S_TYPE, stype_ctrl_signals
        })
    );

  
    assign {PCSrc, RegWEn, ASrc, BSrc, ALUOp, WriteSrc} = signals_mux_wire;
    assign ImmType = imm_type_wire;


    /* DPI-C */
    /*  ret instructions logic */
    import "DPI-C" function void npc_reach_ret(input  int code);
    always @(posedge clk) begin
        /* ebreak instructions */
        if (inst == 32'h00100073) begin
            npc_reach_ret($signed(data_x10));  // send reg10 as params
        end
    end

    
    /* send current instruction to simulator */
    reg [31:0] inst_buffer;

    always @(*) begin
        inst_buffer = inst;
    end

    export "DPI-C" function npc_send_inst;
    function int unsigned npc_send_inst();
        npc_send_inst = inst_buffer;
    endfunction

   
   

    
endmodule
