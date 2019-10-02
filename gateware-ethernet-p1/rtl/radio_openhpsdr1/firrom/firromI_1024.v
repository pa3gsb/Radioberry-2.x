
`ifdef USE_ALTSYNCRAM

module firromI_1024 (
  address,
  clock,
  q);

  parameter init_file = "missing_file.mif";

  input [9:0]  address;
  input   clock;
  output  [17:0]  q;

  wire [17:0] sub_wire0;
  wire [17:0] q = sub_wire0[17:0];

  altsyncram  altsyncram_component (
    .address_a (address),
    .clock0 (clock),
    .q_a (sub_wire0),
    .aclr0 (1'b0),
    .aclr1 (1'b0),
    .address_b (1'b1),
    .addressstall_a (1'b0),
    .addressstall_b (1'b0),
    .byteena_a (1'b1),
    .byteena_b (1'b1),
    .clock1 (1'b1),
    .clocken0 (1'b1),
    .clocken1 (1'b1),
    .clocken2 (1'b1),
    .clocken3 (1'b1),
    .data_a ({18{1'b1}}),
    .data_b (1'b1),
    .eccstatus (),
    .q_b (),
    .rden_a (1'b1),
    .rden_b (1'b1),
    .wren_a (1'b0),
    .wren_b (1'b0));
  defparam
    altsyncram_component.address_aclr_a = "NONE",
    altsyncram_component.clock_enable_input_a = "BYPASS",
    altsyncram_component.clock_enable_output_a = "BYPASS",
    altsyncram_component.init_file = init_file,
    altsyncram_component.intended_device_family = "Cyclone IV E",
    altsyncram_component.lpm_hint = "ENABLE_RUNTIME_MOD=NO",
    altsyncram_component.lpm_type = "altsyncram",
    altsyncram_component.numwords_a = 1024,
    altsyncram_component.operation_mode = "ROM",
    altsyncram_component.outdata_aclr_a = "NONE",
    altsyncram_component.outdata_reg_a = "CLOCK0",
    altsyncram_component.widthad_a = 10,
    altsyncram_component.width_a = 18,
    altsyncram_component.width_byteena_a = 1;
endmodule


`else

module firromI_1024 (
  address,
  clock,
  q);

  parameter init_file = "missing.txt";

  input [9:0]  address;
  input   clock;
  output  reg [17:0]  q;

  reg [17:0] rom[1023:0];

  initial
  begin
    $readmemb(init_file, rom);
    q = 0;
  end

  always @ (posedge clock)
  begin
    q <= rom[address];
  end

endmodule

`endif