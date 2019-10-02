

module coarserom (
  address,
  clock,
  q);

  parameter init_file = "missing.txt";

  input [7:0]  address;
  input   clock;
  output reg [35:0]  q;

  reg [35:0] rom[255:0];

  initial
  begin
    $readmemb(init_file, rom);
  end

  always @ (posedge clock)
  begin
    q <= rom[address];
  end

endmodule
