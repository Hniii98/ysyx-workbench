`include "../include/defines.vh"
`include "../templates/MuxKeyWithDefault.v"
module branch(
	input [31:0] data_a,
	input [31:0] data_b,
	input [2:0] BrOp,
	output BrTaken
);


	 MuxKeyWithDefault #(6, 3, 1) branch_taken_mux ( 
        .out(BrTaken),
        .key(BrOp),
        .default_out(`BRANCH_UNTAKEN), // branch set to false as default.
        .lut({
			`BRANCH_BEQ,  data_a == data_b,   
            `BRANCH_BNE,  data_a != data_b,   
            `BRANCH_BLT,  $signed(data_a) < $signed(data_b),  
            `BRANCH_BGE,  $signed(data_a) >= $signed(data_b), 
            `BRANCH_BLTU, data_a < data_b,    
            `BRANCH_BGEU, data_a >= data_b    
            })
    );
    
    

endmodule
