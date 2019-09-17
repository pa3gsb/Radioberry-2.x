transcript off

proc dosim { simname } {
  echo $simname
}

transcript on

### create work library
if {![file exists work]} {
  vlib work
}
vmap work work

### compile sources
vlog -sv -work work ../../rtl/timescale.v
vlog -sv -work work ../../rtl/rxstream.v


### compile tb
vlog  -sv -work work ./rxstream_tb.v


### open simulation 
vsim rxstream_tb


### prepare and run
do wave.do
restart -f
run -all
