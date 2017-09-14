module shift_1xN 
#( 
   parameter SHIFT_LENGTH =  32
 )   
 (                       
   reset,
   clk,
   sr_in,
   sr_out
 );
  
input reset, clk;
input sr_in;
output sr_out;

reg [SHIFT_LENGTH-1:0] sr;

assign sr_out = sr[SHIFT_LENGTH-1];


always@(posedge clk or posedge reset)
begin
  if(reset)
    sr<=1'b0;
  else begin       
    if(SHIFT_LENGTH>1)  
      sr[SHIFT_LENGTH-1:1] <= sr[SHIFT_LENGTH-2:0];
	 sr[0] <= sr_in;
	end
end

endmodule
module lt24_ctrl
(
  input                          clk,
  input                          reset,

  input                          ctrl_write,
  input      [31:0]              ctrl_writedata,
  input                          ctrl_read,
  output reg [31:0]              ctrl_readdata, 
  input      [2:0]               ctrl_address,
  output reg                     ctrl_waitrequest,  

  output wire                    master_read,
  input  wire [31:0]             master_readdata,
  output reg  [31:0]             master_address,
  input  wire                    master_waitrequest,
  input  wire                    master_readdatavalid,

  output wire                    master_burstcount,
  output wire [3:0]              master_byteenable,
  
  output reg                     lt24_cs_n,
  output reg                     lt24_rs,//command/data
  output reg                     lt24_rd_n,
  output reg                     lt24_wr_n,
  output reg  [15:0]             lt24_data,
  output reg                     lt24_reset_n,
  output reg                     lt24_lcd_on   
);														
					
localparam ADRR_REG_VIDEO_MEM_ADDR    =3'd0;
localparam ADRR_REG_VIDEO_MEM_SIZE    =3'd1;					
localparam ADRR_REG_REFRESH_RATE      =3'd2;
localparam ADRR_REG_CTRL              =3'd3;	
localparam ADRR_REG_CMD               =3'd4;
localparam ADRR_REG_DATA              =3'd5;


localparam CTRL_REG_RESET             =0;	
localparam CTRL_REG_LCD_ON            =1;	
localparam CTRL_REG_REFRESH_ON        =2;	

enum int unsigned {SM_IDLE              ='h0001,
						 SM_REFRESH_PAUSE     ='h0002, 	
                   SM_REFRESH_START     ='h0004,						 
                   SM_REFRESH_RUN       ='h0008,
                   SM_REFRESH_HOLD      ='h0010
						 } state, next_state;
				

											
reg [31:0]   start_address;
reg [31:0]   counter;
reg [31:0]   input_counter;
reg [31:0]   output_counter;
reg [31:0]   video_mem_size;
//reg [31:0]   data;
//reg          update_address;
reg          hi_bit_input_counter_delayed;
reg          ctrl_read_delayed;
reg [31:0]   refresh_rate_timer;
reg [31:0]   refresh_rate;
reg          refresh_on;
reg          refresh_req;
reg [31:0]   fifo_level_in; 
reg [31:0]   fifo_level_out; 
reg          read_ready; 
reg          fifo_read;
reg          lt24_write_n;
wire         lt24_write_n_delayed;

wire         fifo_up_level_in=fifo_level_in>32;
wire         fifo_bottom_level_in=fifo_level_in<16;
wire         fifo_out_not_empty=|fifo_level_out;


wire [15:0]  fifo_data_out;

fifo fifo_ins(
     .aclr ( reset ),
     .data(master_readdata),
     .rdclk(clk),
     .rdreq(~lt24_write_n),
     .wrclk(clk),
     .wrreq(master_readdatavalid),
     .q(fifo_data_out)
);

shift_1xN #(5) lt24_wr_n_del(
     .clk(clk),
     .reset(reset),
     .sr_in(lt24_write_n),
     .sr_out(lt24_write_n_delayed)
); 

assign master_burstcount=1'b1;
assign master_byteenable=4'hF;//////////////////


always @(posedge clk, posedge reset)
if(reset)
  start_address<=32'h0000_0000;
else
  if(ctrl_write & (ctrl_address==ADRR_REG_VIDEO_MEM_ADDR))
	 start_address<=ctrl_writedata;

//always @(posedge clk)
//  update_address<=(ctrl_write & (ctrl_address==ADRR_REG_VIDEO_MEM_ADDR));	 

assign master_read=~counter[31] & read_ready;		
		
always @(posedge clk)
hi_bit_input_counter_delayed<=input_counter[31];	
	
	 
always @(posedge clk, posedge reset)
if(reset)
  counter<=32'hFFFF_FFFF;
else
  if(state==SM_REFRESH_START)
	 counter<=video_mem_size;
  else
    if(master_read & ~master_waitrequest)
      counter<=counter-32'd1;
		
		
always @(posedge clk, posedge reset)
if(reset)
  read_ready<=1'b0;
else
  if(state==SM_REFRESH_RUN) 
    if(fifo_up_level_in)
      read_ready<=1'b0;
    else
      read_ready<=(master_read | fifo_bottom_level_in);

always @(posedge clk, posedge reset)
if(reset)
  fifo_level_in<=32'd0;
else		
  case( {master_read & ~master_waitrequest, fifo_out_not_empty & ~lt24_write_n} )
    2'b01 : fifo_level_in<=fifo_level_in - 32'd1;
    2'b10 : fifo_level_in<=fifo_level_in + 32'd2;
    2'b11 : fifo_level_in<=fifo_level_in + 32'd1;
  endcase

always @(posedge clk, posedge reset)
if(reset)
  fifo_level_out<=32'd0;
else		
  case( {master_readdatavalid, fifo_out_not_empty & ~lt24_write_n} )
    2'b01 : fifo_level_out<=fifo_level_out - 32'd1;
    2'b10 : fifo_level_out<=fifo_level_out + 32'd2;
    2'b11 : fifo_level_out<=fifo_level_out + 32'd1;
  endcase
  
always @(posedge clk, posedge reset)
if(reset)
  input_counter<=32'hFFFF_FFFF;
else
  if(state==SM_REFRESH_START)
    input_counter<=video_mem_size;
  else
    if(master_readdatavalid)
      input_counter<=input_counter-32'd1;	

always @(posedge clk, posedge reset)
if(reset)
  output_counter<=32'hFFFF_FFFF;
else
  if(state==SM_REFRESH_START)
	 output_counter<={video_mem_size[30:0],1'b1};
  else
    if(~lt24_write_n_delayed)
      output_counter<=output_counter-32'd1;							
				
always @(posedge clk, posedge reset)
if(reset)
  video_mem_size<=32'hFFFF_FFFF;
else
  if(ctrl_write & (ctrl_address==ADRR_REG_VIDEO_MEM_SIZE))
	 video_mem_size<=ctrl_writedata;


				
always @(posedge clk, posedge reset)
if(reset)
  master_address<=32'h0000_0000;
else
  if(state==SM_REFRESH_START)
    master_address<=start_address;
  else  
    if(master_read & ~master_waitrequest)
      master_address<=master_address +32'd4;  

always @(posedge clk)
  ctrl_waitrequest<=1'b0;


always @(posedge clk)
if(state==SM_REFRESH_START)
  refresh_rate_timer<=refresh_rate;
else
  refresh_rate_timer<=refresh_rate_timer-32'd1;


always @(posedge clk)
  ctrl_read_delayed<=ctrl_read;

	  
always @(posedge clk)
ctrl_readdata<=32'hFFFF_FFFF;


always @(posedge clk, posedge reset)
if(reset)
  refresh_on<=1'b0;
else
  if(ctrl_write & (ctrl_address==ADRR_REG_CTRL))
    refresh_on<=ctrl_writedata[CTRL_REG_REFRESH_ON];

	 
always @(posedge clk, posedge reset)
if(reset)
  refresh_rate<=1'b0;
else
  if(ctrl_write & (ctrl_address==ADRR_REG_REFRESH_RATE))
    refresh_rate<=ctrl_writedata;
	 
always @(posedge clk, posedge reset)
if(reset)
  refresh_req<=1'b0;
else
  if((refresh_rate_timer==32'h0000_0000) & refresh_on)
    refresh_req<=1'b1;
  else
    if(state==SM_REFRESH_START)
      refresh_req<=1'b0;
	 
	 
//////////////////////////////////
always @(posedge clk, posedge reset)
if(reset)
  lt24_reset_n<=1'b0;
else
  if(ctrl_write & (ctrl_address==ADRR_REG_CTRL))
    lt24_reset_n<=~ctrl_writedata[CTRL_REG_RESET];


always @(posedge clk, posedge reset)
if(reset)
  lt24_lcd_on<=1'b0;
else
  if(ctrl_write & (ctrl_address==ADRR_REG_CTRL))
    lt24_lcd_on<=ctrl_writedata[CTRL_REG_LCD_ON];
	 

always @(posedge clk, posedge reset)
if(reset)
  lt24_cs_n<=1'b1;
else
  if((state==SM_IDLE) || (state==SM_REFRESH_HOLD))
    lt24_cs_n<=~(ctrl_write & ((ctrl_address==ADRR_REG_CMD)|(ctrl_address==ADRR_REG_DATA)));
  else
    lt24_cs_n<=lt24_write_n_delayed;
  

always @(posedge clk)
if((state==SM_IDLE) || (state==SM_REFRESH_HOLD))
  lt24_rs<=(ctrl_address==ADRR_REG_DATA);
else
  lt24_rs<=1'b1;


always @(posedge clk)
  lt24_rd_n<=1'b1;
  

always @(posedge clk, posedge reset)
if(reset)
  lt24_wr_n<=1'b1;
else
  if((state==SM_IDLE) || (state==SM_REFRESH_HOLD))
    lt24_wr_n<=~(ctrl_write & ((ctrl_address==ADRR_REG_CMD)|(ctrl_address==ADRR_REG_DATA)));
  else
    lt24_wr_n<=lt24_write_n_delayed;
    
  

always @(posedge clk, posedge reset)
if(reset)
  lt24_data<=16'h0000;
else
  if((state==SM_IDLE) || (state==SM_REFRESH_HOLD))
    lt24_data<=ctrl_writedata[15:0];
  else
    lt24_data<=fifo_data_out;

//////////////////////////////////
always @(posedge clk, posedge reset)
if(reset)
  lt24_write_n<=1'b1;
else
  if(fifo_out_not_empty)
    lt24_write_n<=~lt24_write_n;
  else
    lt24_write_n<=1'b1;

always_ff @(posedge clk, posedge reset)
if(reset)     
  state<=SM_IDLE;
else
  state<= next_state;



always_comb
unique case (state) 
    SM_IDLE          : if(refresh_on)
                         next_state=SM_REFRESH_START;
                       else
                         next_state=SM_IDLE;  
								 
    SM_REFRESH_START : next_state=SM_REFRESH_RUN; 

    SM_REFRESH_RUN   : if(output_counter[31])
                         next_state=SM_REFRESH_PAUSE; 	 
                       else
                         next_state=SM_REFRESH_RUN;  

    SM_REFRESH_PAUSE : if(!refresh_on)
                         next_state=SM_IDLE;							 
                       else		 
                         if(refresh_req)
                           next_state=SM_REFRESH_START;
                         else
                           next_state=SM_REFRESH_PAUSE;

    SM_REFRESH_HOLD  : next_state=SM_REFRESH_HOLD;
							      
    default          : next_state=SM_IDLE;             
                                                           
  endcase	 

  
endmodule