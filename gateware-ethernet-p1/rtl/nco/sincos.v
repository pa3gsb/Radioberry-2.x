
`timescale 1us/1ns

module sincos (
  clk,
  angle,
  cos,
  sin
);

parameter CALCTYPE = 0;

input                       clk;
input               [19:0]  angle;
output logic        [18:0]  sin;
output logic        [18:0]  cos;

//Pipestage 1
logic   [7:0]   coarse;
logic   [9:0]   fine;
logic   [1:0]   quadrant;
assign {quadrant,coarse,fine} = angle;

logic   [7:0]   coarse_addr;
logic           sin_sign, cos_sign;

logic   [8:0]   fine_addr;
logic           fine_sign;

always @(posedge clk) begin
  coarse_addr <= quadrant[0] ? ~coarse : coarse;
  sin_sign    <= quadrant[1] ? 1'b1 : 1'b0;
  cos_sign    <= (quadrant[1] ^ quadrant[0]) ? 1'b1 : 1'b0;

  fine_addr   <= fine[9] ? fine[8:0] : ~fine[8:0];
  fine_sign   <= fine[9] ? 1'b0 : 1'b1;
end 


//Pipestage2
//Sign magnitude values to make full use of Cyclone IV multipliers
logic  [18:0]   coarse_sin, coarse_cos, coarse_sinp, coarse_cosp;
logic           fine_sign_d1, fine_sign_pd1;
          
always @(posedge clk) begin
  coarse_sinp[18]          <=  sin_sign;
  coarse_cosp[18]          <=  cos_sign;
  fine_sign_pd1            <=  fine_sign;
end // always @(posedge clk)

coarserom #(.init_file("sincos_coarse.txt")) coarserom_i (
  .address(coarse_addr),
  .clock(clk),
  .q({coarse_sinp[17:0],coarse_cosp[17:0]})
);

// Extra pipe to map into optimum RAM and multiplier
always @(posedge clk) begin
  coarse_sin <= coarse_sinp;
  coarse_cos <= coarse_cosp;
  fine_sign_d1 <= fine_sign_pd1;
end


//Pipestage2,3
logic  [9:0]   scsf, ccsf;
logic  [35:0]   scsf_ff, ccsf_ff;

logic  unsigned [17:0]  sin_opa, sin_opb, cos_opa, cos_opb;
logic  unsigned [35:0]  sin_mult_res, cos_mult_res;

generate
  if (CALCTYPE == 0) begin: CALCROM
    
    logic   [8:0]   fine_sin;

    finerom #(.init_file("sin_fine.txt")) finerom_i (
      .address(fine_addr),
      .clock(clk),
      .q(fine_sin)
    );

    assign sin_opa = coarse_sinp[17:0];
    assign sin_opb = {9'h00,fine_sin};
    assign scsf_ff = sin_mult_res;
    assign cos_opa = coarse_cosp[17:0];
    assign cos_opb = {9'h00,fine_sin};
    assign ccsf_ff = cos_mult_res;
    
    assign scsf = scsf_ff[26:17];
    assign ccsf = ccsf_ff[26:17];

  end else if (CALCTYPE == 1) begin: CALCMULT

    logic  [17:0]   fine_sin;

    // 9x9 unsigned multiply
    always @(posedge clk) begin
      // 1'b1 is used below as tables are centered around midpoint, 
      // it will be half after shifting
      // 9'h193 is an approximation of 2*pi*64
      fine_sin <= 9'h193 * {fine_addr[8:1],1'b1};
    end

    assign sin_opa = coarse_sinp[17:0];
    assign sin_opb = fine_sin;
    assign scsf_ff = sin_mult_res;
    assign cos_opa = coarse_cosp[17:0];
    assign cos_opb = fine_sin;
    assign ccsf_ff = cos_mult_res;

    // Only 10 bits required for correction factor
    assign scsf = scsf_ff[35:26];
    assign ccsf = ccsf_ff[35:26];

  end else if (CALCTYPE == 2) begin: CALCLOGIC8

    logic  [15:0]   fine_sin;

    // 8x8 unsigned multiply in logic
    always @(posedge clk) begin
      fine_sin <= 8'hca * {fine_addr[8:2],1'b1};
    end

    // Restrict output to 8 bits to reduce LUT usage
    assign sin_opa = coarse_sinp[17:0];
    assign sin_opb = {10'h00,fine_sin[15:8]};
    assign scsf_ff = sin_mult_res;
    assign cos_opa = coarse_cosp[17:0];
    assign cos_opb = {10'h00,fine_sin[15:8]};
    assign ccsf_ff = cos_mult_res;

    // Only 10 bits required for correction factor
    assign scsf = scsf_ff[25:16];
    assign ccsf = ccsf_ff[25:16];

  end else if (CALCTYPE == 3) begin: CALCLOGIC7

    logic  [13:0]   fine_sin;

    always @(posedge clk) begin
      // 7x7
      fine_sin <= 7'h65 * {fine_addr[8:3],1'b1};
    end

    // Restrict output to 7 bits to reduce LUT usage
    assign sin_opa = coarse_sinp[17:0];
    assign sin_opb = {11'h00,fine_sin[13:7]};
    assign scsf_ff = sin_mult_res;
    assign cos_opa = coarse_cosp[17:0];
    assign cos_opb = {11'h00,fine_sin[13:7]};
    assign ccsf_ff = cos_mult_res;
 
    // Only 10 bits required for correction factor
    assign scsf = scsf_ff[24:15];
    assign ccsf = ccsf_ff[24:15];

  end else if (CALCTYPE == 4) begin: CALCLOGIC7_10

    logic  [16:0]   fine_sin;

    always @(posedge clk) begin
      // 7x10
      fine_sin <= 7'h65 * {fine_addr[8:0],1'b1};
    end

    // Restrict output to 8 bits to reduce LUT usage
    assign sin_opa = coarse_sinp[17:0];
    assign sin_opb = {10'h00,fine_sin[16:9]};
    assign scsf_ff = sin_mult_res;
    assign cos_opa = coarse_cosp[17:0];
    assign cos_opb = {10'h00,fine_sin[16:9]};
    assign ccsf_ff = cos_mult_res;
 
    // Only 10 bits required for correction factor
    assign scsf = scsf_ff[25:16];
    assign ccsf = ccsf_ff[25:16];

  end

endgenerate

`ifdef SIMULATION

logic  unsigned [17:0]  isin_opa, isin_opb, icos_opa, icos_opb;

always @(posedge clk) begin
  isin_opa <= sin_opa;
  isin_opb <= sin_opb;
  sin_mult_res <= isin_opa * isin_opb;
end

always @(posedge clk) begin
  icos_opa <= cos_opa;
  icos_opb <= cos_opb;
  cos_mult_res <= icos_opa * icos_opb;
end


`else

lpm_mult #( .lpm_hint("DEDICATED_MULTIPLIER_CIRCUITRY=YES,MAXIMIZE_SPEED=5"),
            .lpm_pipeline(2),
            .lpm_representation("UNSIGNED"),
            .lpm_type("LPM_MULT"),
            .lpm_widtha(18),
            .lpm_widthb(18),
            .lpm_widthp(36))
sinmult (   .clock(clk),
            .dataa(sin_opa),
            .datab(sin_opb),
            .result(sin_mult_res),
            .aclr(1'b0),
            .clken(1'b1),
            .sclr(1'b0),
            .sum(1'b0));


lpm_mult #( .lpm_hint("DEDICATED_MULTIPLIER_CIRCUITRY=YES,MAXIMIZE_SPEED=5"),
            .lpm_pipeline(2),
            .lpm_representation("UNSIGNED"),
            .lpm_type("LPM_MULT"),
            .lpm_widtha(18),
            .lpm_widthb(18),
            .lpm_widthp(36)) 
cosmult (   .clock(clk),
            .dataa(cos_opa),
            .datab(cos_opb),
            .result(cos_mult_res),
            .aclr(1'b0),
            .clken(1'b1),
            .sclr(1'b0),
            .sum(1'b0));

`endif

//    assign sin_mult_res = sin_opa * sin_opb;
//    always @(posedge clk) begin
//      sin_opa <= coarse_sinp[17:0];
//      sin_opb <= {9'h00,fine_sin};
//      scsf_ff <= sin_mult_res;
//    end
//
//    assign cos_mult_res = cos_opa * cos_opb;
//    always @(posedge clk) begin
//      cos_opa <= coarse_cosp[17:0];
//      cos_opb <= {9'h00,fine_sin};
//      ccsf_ff <= cos_mult_res;
//    end


// Pipestate3
logic           cop, sop;
logic  [18:0]   sc, cc;
always @(posedge clk) begin
  sc <= coarse_sin;
  cc <= coarse_cos;
end

//Compute operation for correction factor
always @(posedge clk) begin
  case ({coarse_cos[18],coarse_sin[18],fine_sign_d1})
    3'b000: cop <= 1'b1;
    3'b001: cop <= 1'b0;
    3'b010: cop <= 1'b0;
    3'b011: cop <= 1'b1;
    3'b100: cop <= 1'b0;
    3'b101: cop <= 1'b1;
    3'b110: cop <= 1'b1;
    3'b111: cop <= 1'b0;
  endcase 
end

//Compute operation for correction factor
always @(posedge clk) begin
  case ({coarse_sin[18],coarse_cos[18],fine_sign_d1})
    3'b000: sop <= 1'b0;
    3'b001: sop <= 1'b1;
    3'b010: sop <= 1'b1;
    3'b011: sop <= 1'b0;
    3'b100: sop <= 1'b1;
    3'b101: sop <= 1'b0;
    3'b110: sop <= 1'b0;
    3'b111: sop <= 1'b1;
  endcase
end


//Pipestage4
always @(posedge clk) begin
  sin[17:0] <= sop ? (sc[17:0] - {1'b0,ccsf}) : (sc[17:0] + {1'b0,ccsf});
  cos[17:0] <= cop ? (cc[17:0] - {1'b0,scsf}) : (cc[17:0] + {1'b0,scsf});

  sin[18]   <= sc[18];
  cos[18]   <= cc[18];
end

endmodule