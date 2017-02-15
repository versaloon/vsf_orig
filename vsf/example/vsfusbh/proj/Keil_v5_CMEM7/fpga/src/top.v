module top(
	clk_i,
	gpio0,
);

input  clk_i;
inout [31:0] gpio0;

wire [31:0] gpio_0_out_o;
wire [31:0] gpio_0_oe_o;
wire [31:0] gpio_0_in_i;
assign gpio_0_in_i = gpio0;
assign gpio0[0] = (gpio_0_oe_o[0])? gpio_0_out_o[0] : 1'bz;
assign gpio0[1] = (gpio_0_oe_o[1])? gpio_0_out_o[1] : 1'bz;
assign gpio0[2] = (gpio_0_oe_o[2])? gpio_0_out_o[2] : 1'bz;
assign gpio0[3] = (gpio_0_oe_o[3])? gpio_0_out_o[3] : 1'bz;
assign gpio0[4] = (gpio_0_oe_o[4])? gpio_0_out_o[4] : 1'bz;
assign gpio0[5] = (gpio_0_oe_o[5])? gpio_0_out_o[5] : 1'bz;
assign gpio0[6] = (gpio_0_oe_o[6])? gpio_0_out_o[6] : 1'bz;
assign gpio0[7] = (gpio_0_oe_o[7])? gpio_0_out_o[7] : 1'bz;
assign gpio0[8] = (gpio_0_oe_o[8])? gpio_0_out_o[8] : 1'bz;
assign gpio0[9] = (gpio_0_oe_o[9])? gpio_0_out_o[9] : 1'bz;
assign gpio0[10] = (gpio_0_oe_o[10])? gpio_0_out_o[10] : 1'bz;
assign gpio0[11] = (gpio_0_oe_o[11])? gpio_0_out_o[11] : 1'bz;
assign gpio0[12] = (gpio_0_oe_o[12])? gpio_0_out_o[12] : 1'bz;
assign gpio0[13] = (gpio_0_oe_o[13])? gpio_0_out_o[13] : 1'bz;
assign gpio0[14] = (gpio_0_oe_o[14])? gpio_0_out_o[14] : 1'bz;
assign gpio0[15] = (gpio_0_oe_o[15])? gpio_0_out_o[15] : 1'bz;
assign gpio0[16] = (gpio_0_oe_o[16])? gpio_0_out_o[16] : 1'bz;
assign gpio0[17] = (gpio_0_oe_o[17])? gpio_0_out_o[17] : 1'bz;
assign gpio0[18] = (gpio_0_oe_o[18])? gpio_0_out_o[18] : 1'bz;
assign gpio0[19] = (gpio_0_oe_o[19])? gpio_0_out_o[19] : 1'bz;
assign gpio0[20] = (gpio_0_oe_o[20])? gpio_0_out_o[20] : 1'bz;
assign gpio0[21] = (gpio_0_oe_o[21])? gpio_0_out_o[21] : 1'bz;
assign gpio0[22] = (gpio_0_oe_o[22])? gpio_0_out_o[22] : 1'bz;
assign gpio0[23] = (gpio_0_oe_o[23])? gpio_0_out_o[23] : 1'bz;
assign gpio0[24] = (gpio_0_oe_o[24])? gpio_0_out_o[24] : 1'bz;
assign gpio0[25] = (gpio_0_oe_o[25])? gpio_0_out_o[25] : 1'bz;
assign gpio0[26] = (gpio_0_oe_o[26])? gpio_0_out_o[26] : 1'bz;
assign gpio0[27] = (gpio_0_oe_o[27])? gpio_0_out_o[27] : 1'bz;
assign gpio0[28] = (gpio_0_oe_o[28])? gpio_0_out_o[28] : 1'bz;
assign gpio0[29] = (gpio_0_oe_o[29])? gpio_0_out_o[29] : 1'bz;
assign gpio0[30] = (gpio_0_oe_o[30])? gpio_0_out_o[30] : 1'bz;
assign gpio0[31] = (gpio_0_oe_o[31])? gpio_0_out_o[31] : 1'bz;

wire clk_200m;
wire clk_12m;
wire locked;
pll_v1 u_pll (
    .clkin0 (clk_i      ),
    .clkout0(clk_200M   ),
    .clkout1(clk_12M    ),
    .locked (locked     )
);
armcm3_v1 u_arm(
    .fp2soc_rst_n   (locked         ),
    .fp_clk_sys     (clk_200M       ),
    .fp_clk_arm     (clk_200M       ),
    .fp_clk_usb     (clk_12M        ),
    .gpio_0_out_o   (gpio_0_out_o   ),
    .gpio_0_oe_o    (gpio_0_oe_o    ),
    .gpio_0_in_i    (gpio_0_in_i    )
);

endmodule