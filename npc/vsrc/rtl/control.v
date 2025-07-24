`include "../include/defines.vh"
`include "../templates/MuxKeyWithDefault.v"

module control(
    input clk,
    input [31:0] inst, 
    input [31:0] data_x10, // data of reg x10, we need this value for ret.
    input BrLt,   // branch less than from module branch
    input BrEq,   // branch equal from module branch
    output PCSrc,  
    output [2:0] ImmType,  
    output RegWEn, 
    output ASrc, 
    output BSrc, 
    output [3:0] ALUOp, 
    output MemRW, 
    output [1:0] WriteSrc, 
    output IsSigned, // whether the operaton is signed 
    output [1:0] DataSize  // data size of load/store instructions 
);
    /* Slice instrcution */
    wire [6:0] opcode;
    wire [2:0] funct3;
    wire [6:0] funct7;
    wire [4:0] rd;
 
 
    assign opcode = inst[6:0];
    assign funct3 = inst[14:12];
    assign funct7 = inst[31:25];
    assign rd     = inst[11:7];


    /* Level one decoder of all kinds instructions */ 
    wire [2:0] imm_type_wire;
    
    
    /* Transfer opcode slice of instrucions to immediate value type */
    MuxKeyWithDefault #(11, 7, 3) imm_type_mux ( // MuxKeyWithDefault is combitional logic 
        .out(imm_type_wire),
        .key(opcode),
        .default_out(`UNUSED_TYPE),
        .lut({
            7'b011_0111, `U_TYPE, // lui
            7'b001_0111, `U_TYPE, // auipc
            7'b110_1111, `J_TYPE, // jal
            7'b110_0111, `I_TYPE, // jalr
            7'b001_0011, `I_TYPE, // (logic and arithmetic oprations)
            7'b000_0011, `I_TYPE, // (load instructions)
            7'b000_1111, `I_TYPE, // (fence related)
            7'b111_0011, `I_TYPE, // (privileged ISA)
            7'b110_0011, `B_TYPE, // (branch instructions)
            7'b010_0011, `S_TYPE, // (store instructions)
            7'b011_0011, `R_TYPE  // (logic and arithmetic oprations)
            })
    );
 

    /* Level two decoder of certain type instruction */
    localparam [13:0] DEFAULT_CTRL_SIGNALS = 14'h0; // safe control signals for illegal instrutions
    
    /* U-type instructions decoder */
    wire [13:0] utype_ctrl_wire; // excluing immediate value type signals
    
    MuxKeyWithDefault #(2, 1, 14) utype_ctrl_mux (
        .out(utype_ctrl_wire),
        .key(opcode[5]),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            1'b1, {`PC_FROM_SNPC        , `REG_WRITABLE , `OPA_FROM_NCARE , 
                    `OPB_FROM_IMM       , `ALU_PASS     , `MEM_READ       ,
                    `WRITEBACK_FROM_ALU , `TYPE_NCARE   , `DATASIZE_NCARE},  // lui

            1'b0, {`PC_FROM_SNPC        , `REG_WRITABLE , `OPA_FROM_PC   ,
                    `OPB_FROM_IMM       , `ALU_ADD      , `MEM_READ      ,
                    `WRITEBACK_FROM_ALU , `TYPE_NCARE   , `DATASIZE_NCARE}  // auipc
        })
        

    );

    /* J-type instrcutions decoder */
    wire [13:0] jtype_ctrl_wire;

    MuxKeyWithDefault #(1, 3, 14) jtype_ctrl_mux (
        .out(jtype_ctrl_wire),
        .key(imm_type_wire),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            `J_TYPE, {`PC_FROM_ALU ,        `REG_WRITABLE, `OPA_FROM_PC    , 
                      `OPB_FROM_IMM,        `ALU_ADD     , `MEM_READ       ,
                      `WRITEBACK_FROM_SNPC, `TYPE_NCARE  , `DATASIZE_NCARE}
        })
    );


    /* I-type instructions decoder */
    wire [13:0] itype_ctrl_wire;

    localparam ITYPE_UNCONDJUMP = 3'b000; // unconditional jump
    localparam ITYPE_LOAD       = 3'b001;
    localparam ITYPE_OPERATION  = 3'b010; // including logic and arithmetic operations
    localparam ITYPE_DEFAULT    = 3'b011; // fencexx/csrxx/exxx clasified here

    // 1. Clasify I-type instructions to 4 sub types 
    wire [2:0] itype_subtype_wire;
    MuxKeyWithDefault #(5, 7, 3) itype_subtype_mux ( 
        .out(itype_subtype_wire),
        .key(opcode),
        .default_out(ITYPE_DEFAULT),
        .lut({
            7'b110_0111, ITYPE_UNCONDJUMP,
            7'b000_0011, ITYPE_LOAD      , 
            7'b001_0011, ITYPE_OPERATION , 
            7'b000_1111, ITYPE_DEFAULT   , // TODO: support fencexx
            7'b111_0011, ITYPE_DEFAULT     // TODO: suppoert exxx, csrcc   
        })
    );

    // 2. Decoder for subtype  of I-type

    //ITYPE_UNCONDJUM
    wire[13:0] ijump_ctrl_wire;
    assign ijump_ctrl_wire = {`PC_FROM_ALU        ,  `REG_WRITABLE, `OPA_FROM_RS1    , 
                              `OPB_FROM_IMM       ,  `ALU_ADD     , `MEM_READ        ,
                              `WRITEBACK_FROM_SNPC,  `TYPE_NCARE  , `DATASIZE_NCARE  };


    // ITYPE_LOAD 
    wire[13:0] iload_ctrl_wire;
    MuxKeyWithDefault #(5, 3, 14) iload_ctrl_mux(
        .out(iload_ctrl_wire),
        .key(funct3),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            3'b000, {`PC_FROM_SNPC       ,  `REG_WRITABLE, `OPA_FROM_RS1    , 
                     `OPB_FROM_IMM       ,  `ALU_ADD     , `MEM_READ        ,
                     `WRITEBACK_FROM_MEM ,  `TYPE_SIGNED , `DATASIZE_BYTE   }, // lb

            3'b001, {`PC_FROM_SNPC       ,  `REG_WRITABLE, `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       ,  `ALU_ADD     , `MEM_READ        ,
                     `WRITEBACK_FROM_MEM ,  `TYPE_SIGNED , `DATASIZE_HALWORD}, // lh

            3'b010, {`PC_FROM_SNPC       ,  `REG_WRITABLE, `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       ,  `ALU_ADD     , `MEM_READ        ,
                     `WRITEBACK_FROM_MEM ,  `TYPE_SIGNED , `DATASIZE_WORD   }, // lw

            3'b100, {`PC_FROM_SNPC       , `REG_WRITABLE , `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       , `ALU_ADD      , `MEM_READ        ,
                     `WRITEBACK_FROM_MEM , `TYPE_UNSIGNED, `DATASIZE_BYTE   }, // lbu

            3'b101, {`PC_FROM_SNPC       , `REG_WRITABLE , `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       , `ALU_ADD      , `MEM_READ        ,
                     `WRITEBACK_FROM_MEM , `TYPE_UNSIGNED, `DATASIZE_HALWORD} // lhu

        })
    );

    // ITYPE_OPERATION 
    wire[13:0] iop_ctrl_wire;

    // ITYPE_OPERATION need to divide to two part
    wire[13:0] iop_main_ctrl_wire;
    wire[13:0] iop_shiftr_ctrl_wire;
  
    // Main ITYPE_OPERATION 
    MuxKeyWithDefault #(7, 3, 14) iop_main_ctrl_mux( 
        .out(iop_main_ctrl_wire), 
        .key(funct3),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            3'b000, {`PC_FROM_SNPC       ,  `REG_WRITABLE, `OPA_FROM_RS1    , 
                     `OPB_FROM_IMM       ,  `ALU_ADD     , `MEM_READ        ,
                     `WRITEBACK_FROM_ALU ,  `TYPE_NCARE  , `DATASIZE_NCARE  }, // addi

            3'b010, {`PC_FROM_SNPC       ,  `REG_WRITABLE, `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       ,  `ALU_SLT     , `MEM_READ        ,
                     `WRITEBACK_FROM_ALU ,  `TYPE_SIGNED , `DATASIZE_NCARE  }, // slti

            3'b011, {`PC_FROM_SNPC       ,  `REG_WRITABLE , `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       ,  `ALU_SLT      , `MEM_READ        ,
                     `WRITEBACK_FROM_ALU ,  `TYPE_UNSIGNED, `DATASIZE_NCARE }, // sltiu

            3'b100, {`PC_FROM_SNPC       , `REG_WRITABLE , `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       , `ALU_XOR      , `MEM_READ        ,
                     `WRITEBACK_FROM_ALU , `TYPE_NCARE   , `DATASIZE_NCARE },  // xori

            3'b110, {`PC_FROM_SNPC       , `REG_WRITABLE , `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       , `ALU_OR       , `MEM_READ        ,
                     `WRITEBACK_FROM_ALU , `TYPE_NCARE   , `DATASIZE_NCARE  },  // ori

            3'b111, {`PC_FROM_SNPC       , `REG_WRITABLE , `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       , `ALU_AND      , `MEM_READ        ,
                     `WRITEBACK_FROM_ALU , `TYPE_NCARE   , `DATASIZE_NCARE },  // andi

            3'b001, {`PC_FROM_SNPC       , `REG_WRITABLE , `OPA_FROM_RS1    ,
                     `OPB_FROM_IMM       , `ALU_SHIFL    , `MEM_READ        ,
                     `WRITEBACK_FROM_ALU , `TYPE_NCARE   , `DATASIZE_NCARE }  // slli
        })
    );

    // Shift right instruction of subtype ITYPE_OPERATION   
    MuxKeyWithDefault #(2, 1, 14) iop_shiftr_ctrl_mux( 
        .out(iop_shiftr_ctrl_wire),
        .key(funct7[5]),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            1'b0, {`PC_FROM_SNPC       , `REG_WRITABLE , `OPA_FROM_RS1    ,
                   `OPB_FROM_IMM       , `ALU_SHIFR    , `MEM_READ        ,
                   `WRITEBACK_FROM_ALU , `TYPE_UNSIGNED, `DATASIZE_NCARE },  // srli

            1'b1, {`PC_FROM_SNPC       , `REG_WRITABLE , `OPA_FROM_RS1    ,
                   `OPB_FROM_IMM       , `ALU_SHIFR    , `MEM_READ        ,
                   `WRITEBACK_FROM_ALU , `TYPE_SIGNED  , `DATASIZE_NCARE }  // srai        
        })
    );

    // select final iop control signals
    assign iop_ctrl_wire = (funct3 == 3'b101) ? iop_shiftr_ctrl_wire : iop_main_ctrl_wire;


    // I-type final control signals mux
    MuxKeyWithDefault #(4, 3, 14) itype_ctrl_mux( 
        .out(itype_ctrl_wire),
        .key(itype_subtype_wire),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            ITYPE_UNCONDJUMP, ijump_ctrl_wire,
            ITYPE_LOAD      , iload_ctrl_wire,
            ITYPE_OPERATION , iop_ctrl_wire  ,
            ITYPE_DEFAULT   , DEFAULT_CTRL_SIGNALS       // unsupported i-type instrction output default  
        })                                               // control signals
    );



    /* S-type instructions decoder */
    wire [13:0] stype_ctrl_wire;

    MuxKeyWithDefault #(3, 3, 14) stype_ctrl_mux( 
        .out(stype_ctrl_wire),
        .key(funct3),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            3'b000, {`PC_FROM_SNPC       , `REG_UNWRITABLE , `OPA_FROM_RS1   ,
                     `OPB_FROM_IMM       , `ALU_ADD        , `MEM_WRITE      ,
                     `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_BYTE },  // sb

            3'b001, {`PC_FROM_SNPC       , `REG_UNWRITABLE , `OPA_FROM_RS1      ,
                     `OPB_FROM_IMM       , `ALU_ADD        , `MEM_WRITE         ,
                     `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_HALWORD },  // sh

            3'b010, {`PC_FROM_SNPC       , `REG_UNWRITABLE , `OPA_FROM_RS1  ,
                     `OPB_FROM_IMM       , `ALU_ADD        , `MEM_WRITE     ,
                     `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_WORD }  // sw
        })
    );    

    /* R-type instructions decode */

    wire [13:0] rtype_ctrl_wire;

    MuxKeyWithDefault #(10, 4, 14) rtype_ctrl_mux(
        .out(rtype_ctrl_wire),
        .key({funct3, funct7[5]}),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            {3'b000, 1'b0}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_ADD        , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_NCARE },  // add

            {3'b000, 1'b1}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_SUB        , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_NCARE },  // sub

            {3'b001, 1'b0}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_SHIFL      , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_NCARE },  // sll

            {3'b010, 1'b0}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_SLT        , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_SIGNED    , `DATASIZE_NCARE },  // slt

            {3'b011, 1'b0}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_SLT        , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_UNSIGNED  , `DATASIZE_NCARE },  // sltu

            {3'b100, 1'b0}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_XOR        , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_NCARE },  // xor

            {3'b101, 1'b0}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_SHIFR      , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_UNSIGNED  , `DATASIZE_NCARE },  // srl

            {3'b101, 1'b1}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_SHIFR      , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_SIGNED    , `DATASIZE_NCARE },  // sra

            {3'b110, 1'b0}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_OR         , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_NCARE },  // or

            {3'b111, 1'b0}, {`PC_FROM_SNPC       , `REG_WRITABLE   , `OPA_FROM_RS1    ,
                             `OPB_FROM_RS2       , `ALU_AND        , `MEM_READ        ,
                             `WRITEBACK_FROM_ALU , `TYPE_NCARE     , `DATASIZE_NCARE }   // and         
        })
    );
    
    /* B-type instructions decoder */
    wire [13:0] btype_default_ctrl_wire;

    // PCsrc signals in b type instructions decoded to PC_FROM_SNPC as default.
    MuxKeyWithDefault #(6, 3, 14) btype_ctrl_mux(
        .out(btype_default_ctrl_wire),
        .key(funct3),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            3'b000,    {`PC_FROM_SNPC        , `REG_UNWRITABLE , `OPA_FROM_PC     ,
                        `OPB_FROM_IMM        , `ALU_ADD        , `MEM_READ        ,
                        `WRITEBACK_FROM_NCARE, `TYPE_NCARE     , `DATASIZE_NCARE },  // beq

            3'b001,    {`PC_FROM_SNPC        , `REG_UNWRITABLE , `OPA_FROM_PC     ,
                        `OPB_FROM_IMM        , `ALU_ADD        , `MEM_READ        ,
                        `WRITEBACK_FROM_NCARE, `TYPE_NCARE     , `DATASIZE_NCARE },  // bne

            3'b100,    {`PC_FROM_SNPC        , `REG_UNWRITABLE , `OPA_FROM_PC     ,
                        `OPB_FROM_IMM        , `ALU_ADD        , `MEM_READ        ,
                        `WRITEBACK_FROM_NCARE, `TYPE_SIGNED     , `DATASIZE_NCARE },  // blt

            3'b101,    {`PC_FROM_SNPC        , `REG_UNWRITABLE , `OPA_FROM_PC     ,
                        `OPB_FROM_IMM        , `ALU_ADD        , `MEM_READ        ,
                        `WRITEBACK_FROM_NCARE, `TYPE_SIGNED    , `DATASIZE_NCARE },  // bge

            3'b110,    {`PC_FROM_SNPC        , `REG_UNWRITABLE , `OPA_FROM_PC     ,
                        `OPB_FROM_IMM        , `ALU_ADD        , `MEM_READ        ,
                        `WRITEBACK_FROM_NCARE, `TYPE_UNSIGNED  , `DATASIZE_NCARE },  // bltu

            3'b111,    {`PC_FROM_SNPC        , `REG_UNWRITABLE , `OPA_FROM_PC     ,
                        `OPB_FROM_IMM        , `ALU_ADD        , `MEM_READ        ,
                        `WRITEBACK_FROM_NCARE, `TYPE_UNSIGNED  , `DATASIZE_NCARE }  // bgeu

        })
    );
    
    /* Branch taken signals*/
    wire  branch_taken;  
    MuxKeyWithDefault #(6, 3, 1) branch_taken_mux(
        .out(branch_taken),
        .key(funct3),
        .default_out(1'b0),
        .lut({
            3'b000,  BrEq,  // beq
            3'b001, ~BrEq,  // bne
            3'b100,  BrLt,  // blt
            3'b101, ~BrLt,  // bge
            3'b110,  BrLt,  // bltu
            3'B111, ~BrLt   // bgeu
        })
    );

    /* Btype final control signals */
    wire[13:0] btype_ctrl_wire;
    
    assign btype_ctrl_wire[13] = branch_taken ? `PC_FROM_ALU : btype_default_ctrl_wire[13];
    assign btype_ctrl_wire[12:0] = btype_default_ctrl_wire[12:0]; 
     


    /* Ctrl signals mux */
    wire [13:0] ctrl_wire;

    MuxKeyWithDefault #(6, 3, 14) final_ctrl_mux (
        .out(ctrl_wire),
        .key(imm_type_wire),
        .default_out(DEFAULT_CTRL_SIGNALS),
        .lut({
            `U_TYPE, utype_ctrl_wire,
            `I_TYPE, itype_ctrl_wire,
            `J_TYPE, jtype_ctrl_wire,
            `S_TYPE, stype_ctrl_wire,
            `B_TYPE, btype_ctrl_wire,
            `R_TYPE, rtype_ctrl_wire
        })
    );
    

    /* Connect to ouput signals */
    assign {PCSrc, RegWEn, ASrc, BSrc, ALUOp, MemRW, WriteSrc, IsSigned, DataSize} = ctrl_wire; 
    assign ImmType = imm_type_wire;




    /* ------------------------------- DPI-C --------------------------------*/

    /*  Ret instructions logic */
    import "DPI-C" function void npc_reach_ret(input int code);
    always @(posedge clk) begin
       
        if (inst == 32'h00100073) begin // ebreak
            npc_reach_ret($signed(data_x10));  // send x10 to simulator
        end
    end


    /* Send current instruction to simulator */
    reg [31:0] inst_buffer;

    always @(*) begin
        inst_buffer = inst;
    end

    export "DPI-C" function npc_send_inst;
    function int unsigned npc_send_inst();
        npc_send_inst = inst_buffer;
    endfunction


   /* Send 1 bit signal to indicate whether instcution is a unconditional jump */
    reg [7:0] is_uncond_jump; // extend 1 bit to nearest unsigned type
    reg [7:0] rd_buffer; // extend 5 bits to nearest unsigned type

    always @(posedge clk) begin
        rd_buffer <= {3'b000, rd}; // store rd and extend to 8bits
        if(imm_type_wire == `J_TYPE)  // jal
            is_uncond_jump <= {7'h0, 1'b1}; 
        else if(opcode == 7'b110_0111 ) // jalr
            is_uncond_jump <= {7'h0, 1'b1};
        else 
            is_uncond_jump <= {7'h0, 1'b0};   
    end

    export "DPI-C" function npc_send_rd;
    function byte unsigned npc_send_rd();
        npc_send_rd = rd_buffer;
    endfunction

    export "DPI-C" function npc_send_is_uncondjump;
    function byte unsigned npc_send_is_uncondjump();
        npc_send_is_uncondjump = is_uncond_jump;
    endfunction
   
   

    
endmodule
