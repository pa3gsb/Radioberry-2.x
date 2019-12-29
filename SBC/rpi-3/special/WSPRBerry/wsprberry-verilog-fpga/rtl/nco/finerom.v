

module finerom (
  address,
  clock,
  q);

  parameter init_file = "missing.txt";

  input [8:0]  address;
  input   clock;
  output reg [8:0]  q;

  reg [8:0] rom[511:0];

  initial
  begin
    $readmemb(init_file, rom);
  end

  always @ (posedge clock)
  begin
    q <= rom[address];
  end

endmodule
