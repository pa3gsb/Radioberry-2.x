## Quartus tools quartus_sh and quartus_cpf must be in your path

all: hl2b5up hl2b3to4 hl2b2 hl2b5upce15

hl2b5up/hl2b5up.sof:
	quartus_sh --flow compile hermeslite -c hl2b5up

hl2b5up/hl2b5up.jic: hl2b5up/hl2b5up.sof
	quartus_cpf -c hl2b5up.cof

hl2b5up/hl2b5upjic.jam: hl2b5up/hl2b5up.jic
	quartus_cpf -c hl2b5up/hl2b5up.jic hl2b5up/hl2b5upjic.jam

hl2b5up/hl2b5upsof.jam: hl2b5up/hl2b5up.sof
	quartus_cpf -c hl2b5up/hl2b5up.sof hl2b5up/hl2b5upsof.jam

hl2b5up/hl2b5upsof.svf: hl2b5up/hl2b5up.sof
	quartus_cpf -c -q 12.0MHz -g 3.3 -n p hl2b5up/hl2b5up.sof hl2b5up/hl2b5upsof.svf

hl2b5up: hl2b5up/hl2b5up.sof hl2b5up/hl2b5up.jic hl2b5up/hl2b5upjic.jam hl2b5up/hl2b5upsof.jam hl2b5up/hl2b5upsof.svf
	mkdir -p release/hl2b5up
	cp hl2b5up/hl2b5up.sof release/hl2b5up
	cp hl2b5up/hl2b5up.rbf release/hl2b5up
	cp hl2b5up/hl2b5up.jic release/hl2b5up
	cp hl2b5up/hl2b5upjic.jam release/hl2b5up
	cp hl2b5up/hl2b5upsof.jam release/hl2b5up
	cp hl2b5up/hl2b5upsof.svf release/hl2b5up

hl2b3to4/hl2b3to4.sof:
	quartus_sh --flow compile hermeslite -c hl2b3to4

hl2b3to4/hl2b3to4.jic: hl2b3to4/hl2b3to4.sof
	quartus_cpf -c hl2b3to4.cof

hl2b3to4/hl2b3to4jic.jam: hl2b3to4/hl2b3to4.jic
	quartus_cpf -c hl2b3to4/hl2b3to4.jic hl2b3to4/hl2b3to4jic.jam

hl2b3to4/hl2b3to4sof.jam: hl2b3to4/hl2b3to4.sof
	quartus_cpf -c hl2b3to4/hl2b3to4.sof hl2b3to4/hl2b3to4sof.jam

hl2b3to4/hl2b3to4sof.svf: hl2b3to4/hl2b3to4.sof
	quartus_cpf -c -q 12.0MHz -g 3.3 -n p hl2b3to4/hl2b3to4.sof hl2b3to4/hl2b3to4sof.svf

hl2b3to4: hl2b3to4/hl2b3to4.sof hl2b3to4/hl2b3to4.jic hl2b3to4/hl2b3to4jic.jam hl2b3to4/hl2b3to4sof.jam hl2b3to4/hl2b3to4sof.svf
	mkdir -p release/hl2b3to4
	cp hl2b3to4/hl2b3to4.sof release/hl2b3to4
	cp hl2b3to4/hl2b3to4.rbf release/hl2b3to4
	cp hl2b3to4/hl2b3to4.jic release/hl2b3to4
	cp hl2b3to4/hl2b3to4jic.jam release/hl2b3to4
	cp hl2b3to4/hl2b3to4sof.jam release/hl2b3to4
	cp hl2b3to4/hl2b3to4sof.svf release/hl2b3to4


hl2b2/hl2b2.sof:
	quartus_sh --flow compile hermeslite -c hl2b2

hl2b2/hl2b2.jic: hl2b2/hl2b2.sof
	quartus_cpf -c hl2b2.cof

hl2b2/hl2b2jic.jam: hl2b2/hl2b2.jic
	quartus_cpf -c hl2b2/hl2b2.jic hl2b2/hl2b2jic.jam

hl2b2/hl2b2sof.jam: hl2b2/hl2b2.sof
	quartus_cpf -c hl2b2/hl2b2.sof hl2b2/hl2b2sof.jam

hl2b2/hl2b2sof.svf: hl2b2/hl2b2.sof
	quartus_cpf -c -q 12.0MHz -g 3.3 -n p hl2b2/hl2b2.sof hl2b2/hl2b2sof.svf

hl2b2: hl2b2/hl2b2.sof hl2b2/hl2b2.jic hl2b2/hl2b2jic.jam hl2b2/hl2b2sof.jam hl2b2/hl2b2sof.svf
	mkdir -p release/hl2b2
	cp hl2b2/hl2b2.sof release/hl2b2
	cp hl2b2/hl2b2.rbf release/hl2b2
	cp hl2b2/hl2b2.jic release/hl2b2
	cp hl2b2/hl2b2jic.jam release/hl2b2
	cp hl2b2/hl2b2sof.jam release/hl2b2
	cp hl2b2/hl2b2sof.svf release/hl2b2


hl2b5upce15/hl2b5upce15.sof:
	quartus_sh --flow compile hermeslite -c hl2b5upce15

hl2b5upce15/hl2b5upce15.jic: hl2b5upce15/hl2b5upce15.sof
	quartus_cpf -c hl2b5upce15.cof

hl2b5upce15/hl2b5upce15jic.jam: hl2b5upce15/hl2b5upce15.jic
	quartus_cpf -c hl2b5upce15/hl2b5upce15.jic hl2b5upce15/hl2b5upce15jic.jam

hl2b5upce15/hl2b5upce15sof.jam: hl2b5upce15/hl2b5upce15.sof
	quartus_cpf -c hl2b5upce15/hl2b5upce15.sof hl2b5upce15/hl2b5upce15sof.jam

hl2b5upce15/hl2b5upce15sof.svf: hl2b5upce15/hl2b5upce15.sof
	quartus_cpf -c -q 12.0MHz -g 3.3 -n p hl2b5upce15/hl2b5upce15.sof hl2b5upce15/hl2b5upce15sof.svf

hl2b5upce15: hl2b5upce15/hl2b5upce15.sof hl2b5upce15/hl2b5upce15.jic hl2b5upce15/hl2b5upce15jic.jam hl2b5upce15/hl2b5upce15sof.jam hl2b5upce15/hl2b5upce15sof.svf
	mkdir -p release/hl2b5upce15
	cp hl2b5upce15/hl2b5upce15.sof release/hl2b5upce15
	cp hl2b5upce15/hl2b5upce15.rbf release/hl2b5upce15
	cp hl2b5upce15/hl2b5upce15.jic release/hl2b5upce15
	cp hl2b5upce15/hl2b5upce15jic.jam release/hl2b5upce15
	cp hl2b5upce15/hl2b5upce15sof.jam release/hl2b5upce15
	cp hl2b5upce15/hl2b5upce15sof.svf release/hl2b5upce15


hl2b5uphd/hl2b5uphd.sof:
	quartus_sh --flow compile hermeslite -c hl2b5uphd

hl2b5uphd/hl2b5uphd.jic: hl2b5uphd/hl2b5uphd.sof
	quartus_cpf -c hl2b5uphd.cof

hl2b5uphd/hl2b5uphdjic.jam: hl2b5uphd/hl2b5uphd.jic
	quartus_cpf -c hl2b5uphd/hl2b5uphd.jic hl2b5uphd/hl2b5uphdjic.jam

hl2b5uphd/hl2b5uphdsof.jam: hl2b5uphd/hl2b5uphd.sof
	quartus_cpf -c hl2b5uphd/hl2b5uphd.sof hl2b5uphd/hl2b5uphdsof.jam

hl2b5uphd/hl2b5uphdsof.svf: hl2b5uphd/hl2b5uphd.sof
	quartus_cpf -c -q 12.0MHz -g 3.3 -n p hl2b5uphd/hl2b5uphd.sof hl2b5uphd/hl2b5uphdsof.svf

hl2b5uphd: hl2b5uphd/hl2b5uphd.sof hl2b5uphd/hl2b5uphd.jic hl2b5uphd/hl2b5uphdjic.jam hl2b5uphd/hl2b5uphdsof.jam hl2b5uphd/hl2b5uphdsof.svf
	mkdir -p release/hl2b5uphd
	mkdir -p release/hl2b5uphd
	cp hl2b5uphd/hl2b5uphd.sof release/hl2b5uphd
	cp hl2b5uphd/hl2b5uphd.rbf release/hl2b5uphd
	cp hl2b5uphd/hl2b5uphd.jic release/hl2b5uphd
	cp hl2b5uphd/hl2b5uphdjic.jam release/hl2b5uphd
	cp hl2b5uphd/hl2b5uphdsof.jam release/hl2b5uphd
	cp hl2b5uphd/hl2b5uphdsof.svf release/hl2b5uphd


hl2b3to4hd/hl2b3to4hd.sof:
	quartus_sh --flow compile hermeslite -c hl2b3to4hd

hl2b3to4hd/hl2b3to4hd.jic: hl2b3to4hd/hl2b3to4hd.sof
	quartus_cpf -c hl2b3to4hd.cof

hl2b3to4hd/hl2b3to4hdjic.jam: hl2b3to4hd/hl2b3to4hd.jic
	quartus_cpf -c hl2b3to4hd/hl2b3to4hd.jic hl2b3to4hd/hl2b3to4hdjic.jam

hl2b3to4hd/hl2b3to4hdsof.jam: hl2b3to4hd/hl2b3to4hd.sof
	quartus_cpf -c hl2b3to4hd/hl2b3to4hd.sof hl2b3to4hd/hl2b3to4hdsof.jam

hl2b3to4hd/hl2b3to4hdsof.svf: hl2b3to4hd/hl2b3to4hd.sof
	quartus_cpf -c -q 12.0MHz -g 3.3 -n p hl2b3to4hd/hl2b3to4hd.sof hl2b3to4hd/hl2b3to4hdsof.svf

hl2b3to4hd: hl2b3to4hd/hl2b3to4hd.sof hl2b3to4hd/hl2b3to4hd.jic hl2b3to4hd/hl2b3to4hdjic.jam hl2b3to4hd/hl2b3to4hdsof.jam hl2b3to4hd/hl2b3to4hdsof.svf
	mkdir -p release/hl2b3to4hd
	cp hl2b3to4hd/hl2b3to4hd.sof release/hl2b3to4hd
	cp hl2b3to4hd/hl2b3to4hd.rbf release/hl2b3to4hd
	cp hl2b3to4hd/hl2b3to4hd.jic release/hl2b3to4hd
	cp hl2b3to4hd/hl2b3to4hdjic.jam release/hl2b3to4hd
	cp hl2b3to4hd/hl2b3to4hdsof.jam release/hl2b3to4hd
	cp hl2b3to4hd/hl2b3to4hdsof.svf release/hl2b3to4hd

halfduplex: hl2b5uphd hl2b3to4hd

clean:
	rm -rf hl2b2 hl2b3to4 hl2b3to4hd hl2b5up hl2b5uphd hl2b5upce15 release

realclean:
	rm -rf db incremental_db

.PHONY: hl2b5up hl2b3to4 hl2b2 hl2b5uphd hl2b3to4hd hl2b5upce15 halfduplex all clean
