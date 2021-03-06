EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 4
Title "Radioberry - amplifier"
Date "2020-12-18"
Rev "beta 1"
Comp "AppMind"
Comment1 "PA3GSB Johan Maas"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L hermeslite:C_Small C313
U 1 1 5FCC84B5
P 4625 4325
F 0 "C313" V 4525 4275 39  0000 L CNN
F 1 "0.1uF" V 4725 4250 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 4625 4325 50  0001 C CNN
F 3 "" H 4625 4325 50  0000 C CNN
	1    4625 4325
	0    1    1    0   
$EndComp
$Comp
L hermeslite:R R306
U 1 1 5FCC84BB
P 3475 4325
F 0 "R306" V 3400 4325 39  0000 C CNN
F 1 "12" V 3482 4326 39  0000 C CNN
F 2 "HERMESLITE:SMD-0805" V 3405 4325 30  0001 C CNN
F 3 "" H 3475 4325 30  0000 C CNN
	1    3475 4325
	0    1    1    0   
$EndComp
$Comp
L hermeslite:R R304
U 1 1 5FCC84C1
P 4925 3950
F 0 "R304" V 4850 3950 39  0000 C CNN
F 1 "270" V 4932 3951 39  0000 C CNN
F 2 "HERMESLITE:SMD-0603" V 4855 3950 30  0001 C CNN
F 3 "" H 4925 3950 30  0000 C CNN
	1    4925 3950
	0    1    1    0   
$EndComp
$Comp
L hermeslite:C_Small C311
U 1 1 5FCC84C7
P 5200 3950
F 0 "C311" V 5100 3900 39  0000 L CNN
F 1 "0.1uF" V 5300 3875 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 5200 3950 50  0001 C CNN
F 3 "" H 5200 3950 50  0000 C CNN
	1    5200 3950
	0    1    1    0   
$EndComp
$Comp
L hermeslite:RD16HHF1 Q301
U 1 1 5FCC84D4
P 5225 4325
F 0 "Q301" H 5200 4500 39  0000 R CNN
F 1 "LDMOS" H 5250 4125 39  0000 R CNN
F 2 "HERMESLITE:COMBORFGND" H 5225 4325 60  0001 C CNN
F 3 "" H 5225 4325 60  0000 C CNN
	1    5225 4325
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:+VPA #PWR0301
U 1 1 5FCC84E1
P 6825 2175
F 0 "#PWR0301" H 6825 2025 50  0001 C CNN
F 1 "+VPA" H 6800 2325 39  0000 C CNN
F 2 "" H 6825 2175 50  0000 C CNN
F 3 "" H 6825 2175 50  0000 C CNN
	1    6825 2175
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:C_Small C306
U 1 1 5FCC84E8
P 5275 3375
F 0 "C306" H 5285 3445 39  0000 L CNN
F 1 "1uF" H 5285 3295 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 5275 3375 50  0001 C CNN
F 3 "" H 5275 3375 50  0000 C CNN
F 4 "HV" H 5275 3375 60  0001 C CNN "Ext"
	1    5275 3375
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:C_Small C307
U 1 1 5FCC84EF
P 5475 3375
F 0 "C307" H 5485 3445 39  0000 L CNN
F 1 "1uF" H 5485 3295 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 5475 3375 50  0001 C CNN
F 3 "" H 5475 3375 50  0000 C CNN
F 4 "HV" H 5475 3375 60  0001 C CNN "Ext"
	1    5475 3375
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:C_Small C308
U 1 1 5FCC84F6
P 5675 3375
F 0 "C308" H 5685 3445 39  0000 L CNN
F 1 "1uF" H 5685 3295 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 5675 3375 50  0001 C CNN
F 3 "" H 5675 3375 50  0000 C CNN
F 4 "HV" H 5675 3375 60  0001 C CNN "Ext"
	1    5675 3375
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:C_Small C309
U 1 1 5FCC84FD
P 5875 3375
F 0 "C309" H 5885 3445 39  0000 L CNN
F 1 "1uF" H 5885 3295 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 5875 3375 50  0001 C CNN
F 3 "" H 5875 3375 50  0000 C CNN
F 4 "HV" H 5875 3375 60  0001 C CNN "Ext"
	1    5875 3375
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:R R305
U 1 1 5FCC8504
P 6450 4300
F 0 "R305" H 6325 4375 39  0000 C CNN
F 1 "1K" V 6457 4301 39  0001 C CNN
F 2 "HERMESLITE:SMD-0805" V 6380 4300 30  0001 C CNN
F 3 "" H 6450 4300 30  0000 C CNN
F 4 "DNI" V 6450 4300 39  0000 C CNN "Option"
	1    6450 4300
	-1   0    0    1   
$EndComp
$Comp
L hermeslite:R R308
U 1 1 5FCC850B
P 6450 4650
F 0 "R308" H 6325 4575 39  0000 C CNN
F 1 "1K" V 6457 4651 39  0001 C CNN
F 2 "HERMESLITE:SMD-0805" V 6380 4650 30  0001 C CNN
F 3 "" H 6450 4650 30  0000 C CNN
F 4 "DNI" V 6450 4650 39  0000 C CNN "Option"
	1    6450 4650
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0314
U 1 1 5FCC8517
P 5325 4550
F 0 "#PWR0314" H 5325 4300 50  0001 C CNN
F 1 "GND" H 5325 4400 50  0001 C CNN
F 2 "" H 5325 4550 50  0000 C CNN
F 3 "" H 5325 4550 50  0000 C CNN
	1    5325 4550
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:C_Small C315
U 1 1 5FCC851D
P 5850 4600
F 0 "C315" H 5875 4525 39  0000 L CNN
F 1 "0.1uF" H 5875 4675 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 5850 4600 50  0001 C CNN
F 3 "" H 5850 4600 50  0000 C CNN
	1    5850 4600
	-1   0    0    1   
$EndComp
$Comp
L hermeslite:C_Small C316
U 1 1 5FCC8523
P 6050 4600
F 0 "C316" H 6075 4525 39  0000 L CNN
F 1 "0.1uF" H 6075 4675 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 6050 4600 50  0001 C CNN
F 3 "" H 6050 4600 50  0000 C CNN
	1    6050 4600
	-1   0    0    1   
$EndComp
$Comp
L hermeslite:C_Small C317
U 1 1 5FCC8529
P 6250 4600
F 0 "C317" H 6275 4525 39  0000 L CNN
F 1 "0.1uF" H 6275 4675 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 6250 4600 50  0001 C CNN
F 3 "" H 6250 4600 50  0000 C CNN
	1    6250 4600
	-1   0    0    1   
$EndComp
$Comp
L hermeslite:C_Small C314
U 1 1 5FCC852F
P 5650 4600
F 0 "C314" H 5675 4525 39  0000 L CNN
F 1 "0.1uF" H 5675 4675 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 5650 4600 50  0001 C CNN
F 3 "" H 5650 4600 50  0000 C CNN
	1    5650 4600
	-1   0    0    1   
$EndComp
$Comp
L hermeslite:C_Small C322
U 1 1 5FCC853C
P 4625 5425
F 0 "C322" V 4525 5375 39  0000 L CNN
F 1 "0.1uF" V 4725 5350 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 4625 5425 50  0001 C CNN
F 3 "" H 4625 5425 50  0000 C CNN
	1    4625 5425
	0    1    1    0   
$EndComp
$Comp
L hermeslite:R R311
U 1 1 5FCC8542
P 4925 5050
F 0 "R311" V 4850 5050 39  0000 C CNN
F 1 "270" V 4932 5051 39  0000 C CNN
F 2 "HERMESLITE:SMD-0603" V 4855 5050 30  0001 C CNN
F 3 "" H 4925 5050 30  0000 C CNN
	1    4925 5050
	0    1    1    0   
$EndComp
$Comp
L hermeslite:C_Small C321
U 1 1 5FCC8548
P 5200 5050
F 0 "C321" V 5100 5000 39  0000 L CNN
F 1 "0.1uF" V 5300 4975 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 5200 5050 50  0001 C CNN
F 3 "" H 5200 5050 50  0000 C CNN
	1    5200 5050
	0    1    1    0   
$EndComp
$Comp
L hermeslite:RD16HHF1 Q302
U 1 1 5FCC8555
P 5225 5425
F 0 "Q302" H 5200 5600 39  0000 R CNN
F 1 "LDMOS" H 5250 5225 39  0000 R CNN
F 2 "HERMESLITE:COMBORFGND" H 5225 5425 60  0001 C CNN
F 3 "" H 5225 5425 60  0000 C CNN
	1    5225 5425
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0320
U 1 1 5FCC8568
P 5325 5650
F 0 "#PWR0320" H 5325 5400 50  0001 C CNN
F 1 "GND" H 5325 5500 50  0001 C CNN
F 2 "" H 5325 5650 50  0000 C CNN
F 3 "" H 5325 5650 50  0000 C CNN
	1    5325 5650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0315
U 1 1 5FCC856F
P 5850 4750
F 0 "#PWR0315" H 5850 4500 50  0001 C CNN
F 1 "GND" H 5850 4600 50  0001 C CNN
F 2 "" H 5850 4750 50  0000 C CNN
F 3 "" H 5850 4750 50  0000 C CNN
	1    5850 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0310
U 1 1 5FCC8575
P 5475 3525
F 0 "#PWR0310" H 5475 3275 50  0001 C CNN
F 1 "GND" H 5475 3375 50  0001 C CNN
F 2 "" H 5475 3525 50  0000 C CNN
F 3 "" H 5475 3525 50  0000 C CNN
	1    5475 3525
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:R R312
U 1 1 5FCC857B
P 3475 5425
F 0 "R312" V 3400 5425 39  0000 C CNN
F 1 "12" V 3482 5426 39  0000 C CNN
F 2 "HERMESLITE:SMD-0805" V 3405 5425 30  0001 C CNN
F 3 "" H 3475 5425 30  0000 C CNN
	1    3475 5425
	0    1    1    0   
$EndComp
$Comp
L hermeslite:C_Small C319
U 1 1 5FCC8581
P 7650 4675
F 0 "C319" V 7550 4625 39  0000 L CNN
F 1 "0.1uF" V 7750 4600 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 7650 4675 50  0001 C CNN
F 3 "" H 7650 4675 50  0000 C CNN
	1    7650 4675
	0    1    1    0   
$EndComp
$Comp
L hermeslite:TRANSF4CT T301
U 1 1 5FCC8587
P 7050 4475
F 0 "T301" H 7050 4825 50  0000 C CNN
F 1 "4:1 Z" H 7050 4225 50  0000 C CNN
F 2 "HERMESLITE:BN202V" H 7050 4475 50  0001 C CNN
F 3 "" H 7050 4475 50  0000 C CNN
	1    7050 4475
	1    0    0    -1  
$EndComp
Text Notes 7400 5625 0    60   ~ 0
Hand Wind\nB62152A4X30 or BN43-202\n4T Secondary, 1+1T Primary\nBN43-1502\n8T Secondary, 2+2T Primary\nB62152A4X30 runs hot, use 24AWG PTFE/FEP Wire
$Comp
L hermeslite:BNC RF302
U 1 1 5FCC85B7
P 9850 3275
F 0 "RF302" H 9925 3400 39  0000 C CNN
F 1 "SMA" H 10075 3275 39  0000 C CNN
F 2 "HERMESLITE:SMAEDGE_THFULL_L" H 9850 3275 50  0001 C CNN
F 3 "" H 9850 3275 50  0000 C CNN
	1    9850 3275
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0309
U 1 1 5FCC85BD
P 9850 3500
F 0 "#PWR0309" H 9850 3250 50  0001 C CNN
F 1 "GND" H 9850 3350 50  0001 C CNN
F 2 "" H 9850 3500 50  0000 C CNN
F 3 "" H 9850 3500 50  0000 C CNN
	1    9850 3500
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0307
U 1 1 5FCC85C3
P 8050 3375
F 0 "#PWR0307" H 8050 3225 50  0001 C CNN
F 1 "+3V3" V 8050 3575 39  0000 C CNN
F 2 "" H 8050 3375 50  0000 C CNN
F 3 "" H 8050 3375 50  0000 C CNN
	1    8050 3375
	0    -1   -1   0   
$EndComp
$Comp
L hermeslite:C_Small C310
U 1 1 5FCC85C9
P 8100 3600
F 0 "C310" H 8150 3675 39  0000 L CNN
F 1 "100pF" H 8150 3525 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 8100 3600 50  0001 C CNN
F 3 "" H 8100 3600 50  0000 C CNN
	1    8100 3600
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR0311
U 1 1 5FCC85CF
P 8100 3725
F 0 "#PWR0311" H 8100 3475 50  0001 C CNN
F 1 "GND" H 8100 3575 50  0001 C CNN
F 2 "" H 8100 3725 50  0000 C CNN
F 3 "" H 8100 3725 50  0000 C CNN
	1    8100 3725
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:DPDTR_GND K301
U 1 1 5FCC85FF
P 8775 3275
F 0 "K301" H 8750 3275 39  0000 C CNN
F 1 "NA3WK-K" H 8775 2975 39  0000 C CNN
F 2 "HERMESLITE:DPDT_GND" H 8475 3075 60  0001 C CNN
F 3 "" H 8475 3075 60  0000 C CNN
	1    8775 3275
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0312
U 1 1 5FCC8605
P 8575 3725
F 0 "#PWR0312" H 8575 3475 50  0001 C CNN
F 1 "GND" H 8575 3575 50  0001 C CNN
F 2 "" H 8575 3725 50  0000 C CNN
F 3 "" H 8575 3725 50  0000 C CNN
	1    8575 3725
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0306
U 1 1 5FCC860B
P 8375 2975
F 0 "#PWR0306" H 8375 2725 50  0001 C CNN
F 1 "GND" H 8375 2825 50  0001 C CNN
F 2 "" H 8375 2975 50  0000 C CNN
F 3 "" H 8375 2975 50  0000 C CNN
	1    8375 2975
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:C_Small C312
U 1 1 5FCC8613
P 6350 4075
F 0 "C312" H 6425 4150 39  0000 L CNN
F 1 "390pF" H 6375 4000 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 6350 4075 50  0001 C CNN
F 3 "" H 6350 4075 50  0000 C CNN
	1    6350 4075
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4725 4325 4750 4325
Wire Wire Line
	4750 3950 4750 4325
Connection ~ 4750 4325
Wire Wire Line
	4750 3950 4775 3950
Wire Wire Line
	5075 3950 5100 3950
Wire Wire Line
	5300 3950 5325 3950
Wire Wire Line
	5325 4125 5325 3950
Connection ~ 5325 3950
Wire Wire Line
	5325 4525 5325 4550
Wire Wire Line
	4725 5425 4750 5425
Wire Wire Line
	4750 5050 4750 5425
Connection ~ 4750 5425
Wire Wire Line
	4750 5050 4775 5050
Wire Wire Line
	5075 5050 5100 5050
Wire Wire Line
	6625 5050 6350 5050
Wire Wire Line
	5325 5225 5325 5050
Connection ~ 5325 5050
Wire Wire Line
	5325 5625 5325 5650
Wire Wire Line
	6625 3950 6625 4125
Wire Wire Line
	6625 4275 6650 4275
Wire Wire Line
	6650 4675 6625 4675
Wire Wire Line
	6625 4675 6625 4825
Wire Wire Line
	5650 4475 5850 4475
Wire Wire Line
	5650 4475 5650 4500
Wire Wire Line
	5850 4500 5850 4475
Connection ~ 5850 4475
Wire Wire Line
	6050 2250 6050 2525
Connection ~ 6050 4475
Wire Wire Line
	6250 4500 6250 4475
Connection ~ 6250 4475
Wire Wire Line
	6450 4450 6450 4475
Connection ~ 6450 4475
Wire Wire Line
	6450 4800 6450 4825
Wire Wire Line
	6450 4825 6625 4825
Connection ~ 6625 4825
Wire Wire Line
	6450 4150 6450 4125
Connection ~ 6625 4125
Wire Wire Line
	5650 4700 5650 4725
Wire Wire Line
	5650 4725 5850 4725
Wire Wire Line
	6250 4725 6250 4700
Wire Wire Line
	6050 4700 6050 4725
Connection ~ 6050 4725
Wire Wire Line
	5850 4700 5850 4725
Connection ~ 5850 4725
Wire Wire Line
	5875 3250 5875 3275
Connection ~ 5875 3250
Wire Wire Line
	5675 3250 5675 3275
Connection ~ 5675 3250
Wire Wire Line
	5475 3250 5475 3275
Connection ~ 5475 3250
Wire Wire Line
	5275 3275 5275 3250
Wire Wire Line
	5875 3500 5875 3475
Wire Wire Line
	5675 3500 5675 3475
Connection ~ 5675 3500
Wire Wire Line
	5475 3475 5475 3500
Connection ~ 5475 3500
Wire Wire Line
	5275 3475 5275 3500
Wire Wire Line
	6450 4125 6625 4125
Wire Wire Line
	5075 3500 5275 3500
Wire Wire Line
	8575 2950 8575 2975
Wire Wire Line
	8100 2950 8375 2950
Wire Wire Line
	8100 2950 8100 3275
Wire Wire Line
	8100 3275 8125 3275
Wire Wire Line
	8575 3575 8575 3725
Wire Wire Line
	8375 2950 8375 2975
Connection ~ 8375 2950
Wire Wire Line
	6350 3975 6350 3950
Connection ~ 6350 3950
Wire Wire Line
	6350 4175 6350 5050
Connection ~ 6350 5050
Connection ~ 6050 3250
Wire Wire Line
	5075 3250 5275 3250
$Comp
L hermeslite:INA199x1 U301
U 1 1 5FCC86CE
P 6500 2900
F 0 "U301" H 6500 3400 39  0000 C CNN
F 1 "INA199A1DC" H 6500 2875 39  0000 C CNN
F 2 "HERMESLITE:PE4259" H 6500 2900 60  0001 C CNN
F 3 "" H 6500 2900 60  0000 C CNN
	1    6500 2900
	-1   0    0    -1  
$EndComp
$Comp
L hermeslite:R R303
U 1 1 5FCC86D4
P 6500 2250
F 0 "R303" V 6425 2250 39  0000 C CNN
F 1 "0.04" V 6507 2251 39  0000 C CNN
F 2 "HERMESLITE:SMD-1206" V 6430 2250 30  0001 C CNN
F 3 "" H 6500 2250 30  0000 C CNN
	1    6500 2250
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0305
U 1 1 5FCC86DA
P 6175 2850
F 0 "#PWR0305" H 6175 2600 50  0001 C CNN
F 1 "GND" H 6175 2700 50  0001 C CNN
F 2 "" H 6175 2850 50  0000 C CNN
F 3 "" H 6175 2850 50  0000 C CNN
	1    6175 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 2250 6050 2250
Wire Wire Line
	6200 2525 6050 2525
Connection ~ 6050 2525
Wire Wire Line
	6200 2725 6175 2725
Wire Wire Line
	6175 2725 6175 2825
Wire Wire Line
	6200 2825 6175 2825
Connection ~ 6175 2825
Wire Wire Line
	6825 2625 6800 2625
Wire Wire Line
	6800 2525 6825 2525
Connection ~ 6825 2525
Text GLabel 6900 2825 2    39   Output ~ 0
Vipa
Wire Wire Line
	6800 2825 6900 2825
$Comp
L hermeslite:C_Small C303
U 1 1 5FCC86EF
P 6950 2375
F 0 "C303" H 6960 2445 39  0000 L CNN
F 1 "0.1uF" H 6960 2295 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 6950 2375 50  0001 C CNN
F 3 "" H 6950 2375 50  0000 C CNN
	1    6950 2375
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0303
U 1 1 5FCC86F5
P 6950 2525
F 0 "#PWR0303" H 6950 2275 50  0001 C CNN
F 1 "GND" H 6950 2375 50  0001 C CNN
F 2 "" H 6950 2525 50  0000 C CNN
F 3 "" H 6950 2525 50  0000 C CNN
	1    6950 2525
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 2250 6950 2275
Wire Wire Line
	6950 2475 6950 2525
$Comp
L hermeslite:R J301
U 1 1 5FCC8709
P 4575 4575
F 0 "J301" V 4500 4575 39  0000 C CNN
F 1 "JNC" V 4582 4576 39  0000 C CNN
F 2 "HERMESLITE:SMD-0603" V 4505 4575 30  0001 C CNN
F 3 "" H 4575 4575 30  0000 C CNN
	1    4575 4575
	0    1    1    0   
$EndComp
Wire Wire Line
	4750 4575 4725 4575
$Comp
L hermeslite:R J302
U 1 1 5FCC8710
P 4575 5675
F 0 "J302" V 4500 5675 39  0000 C CNN
F 1 "JNC" V 4582 5676 39  0000 C CNN
F 2 "HERMESLITE:SMD-0603" V 4505 5675 30  0001 C CNN
F 3 "" H 4575 5675 30  0000 C CNN
	1    4575 5675
	0    1    1    0   
$EndComp
Wire Wire Line
	4750 5675 4725 5675
$Comp
L hermeslite:C_Small C323
U 1 1 5FCC8718
P 4350 5800
F 0 "C323" H 4400 5725 39  0000 L CNN
F 1 "0.1uF" H 4375 5875 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 4350 5800 50  0001 C CNN
F 3 "" H 4350 5800 50  0000 C CNN
F 4 "DNI" H 4500 5775 39  0000 C CNN "Option"
	1    4350 5800
	-1   0    0    1   
$EndComp
$Comp
L hermeslite:C_Small C320
U 1 1 5FCC871F
P 4350 4700
F 0 "C320" H 4400 4625 39  0000 L CNN
F 1 "0.1uF" H 4375 4775 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 4350 4700 50  0001 C CNN
F 3 "" H 4350 4700 50  0000 C CNN
F 4 "DNI" H 4500 4675 39  0000 C CNN "Option"
	1    4350 4700
	-1   0    0    1   
$EndComp
Wire Wire Line
	4175 4575 4350 4575
Wire Wire Line
	4350 4600 4350 4575
Connection ~ 4350 4575
$Comp
L power:GND #PWR0316
U 1 1 5FCC8728
P 4350 4825
F 0 "#PWR0316" H 4350 4575 50  0001 C CNN
F 1 "GND" H 4350 4675 50  0001 C CNN
F 2 "" H 4350 4825 50  0000 C CNN
F 3 "" H 4350 4825 50  0000 C CNN
	1    4350 4825
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 4800 4350 4825
Wire Wire Line
	4175 5675 4350 5675
Wire Wire Line
	4350 5700 4350 5675
Connection ~ 4350 5675
$Comp
L power:GND #PWR0321
U 1 1 5FCC8732
P 4350 5925
F 0 "#PWR0321" H 4350 5675 50  0001 C CNN
F 1 "GND" H 4350 5775 50  0001 C CNN
F 2 "" H 4350 5925 50  0000 C CNN
F 3 "" H 4350 5925 50  0000 C CNN
	1    4350 5925
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 5900 4350 5925
Wire Wire Line
	8100 3500 8100 3475
Connection ~ 8100 3475
Wire Wire Line
	8100 3700 8100 3725
$Comp
L hermeslite:C_Small C305
U 1 1 5FCC878B
P 5075 3375
F 0 "C305" H 5085 3445 39  0000 L CNN
F 1 "22uF" H 5085 3295 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 5075 3375 50  0001 C CNN
F 3 "" H 5075 3375 50  0000 C CNN
	1    5075 3375
	1    0    0    -1  
$EndComp
Wire Wire Line
	5075 3250 5075 3275
Connection ~ 5275 3250
Wire Wire Line
	5075 3475 5075 3500
Connection ~ 5275 3500
Wire Wire Line
	7950 3475 8100 3475
Wire Wire Line
	8050 3375 8125 3375
Text GLabel 7950 3475 0    39   Input ~ 0
INTTR
$Comp
L hermeslite:C_Small C318
U 1 1 5FCC87C5
P 1300 4625
F 0 "C318" V 1150 4625 39  0000 L CNN
F 1 "0.1uF" V 1425 4650 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 1300 4625 50  0001 C CNN
F 3 "" H 1300 4625 50  0000 C CNN
F 4 "" H 1475 4675 39  0000 C CNN "Option"
	1    1300 4625
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0317
U 1 1 5FCC87CB
P 1025 4900
F 0 "#PWR0317" H 1025 4900 39  0001 C CNN
F 1 "GND" H 1025 4830 39  0001 C CNN
F 2 "" H 1025 4900 60  0000 C CNN
F 3 "" H 1025 4900 60  0000 C CNN
	1    1025 4900
	1    0    0    -1  
$EndComp
$Comp
L hermeslite:BNC RF303
U 1 1 5FCC87D2
P 1025 4625
F 0 "RF303" H 1225 4650 39  0000 C CNN
F 1 "SMA" H 900 4525 39  0000 C CNN
F 2 "HERMESLITE:SMAEDGE_THFULL_L" H 1025 4625 50  0001 C CNN
F 3 "" H 1025 4625 50  0000 C CNN
	1    1025 4625
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1175 4625 1200 4625
$Comp
L power:GND #PWR0308
U 1 1 5FCC87E4
P 9450 3500
F 0 "#PWR0308" H 9450 3250 50  0001 C CNN
F 1 "GND" H 9450 3350 50  0001 C CNN
F 2 "" H 9450 3500 50  0000 C CNN
F 3 "" H 9450 3500 50  0000 C CNN
	1    9450 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	9425 3375 9450 3375
Wire Wire Line
	9450 3375 9450 3475
Wire Wire Line
	9425 3475 9450 3475
Connection ~ 9450 3475
Wire Wire Line
	9850 3475 9850 3500
Wire Wire Line
	4750 4325 4750 4575
Wire Wire Line
	4750 5425 4750 5675
Wire Wire Line
	5325 5050 5300 5050
Wire Wire Line
	5850 4475 6050 4475
Wire Wire Line
	6050 4475 6050 4500
Wire Wire Line
	6050 4475 6250 4475
Wire Wire Line
	6250 4475 6450 4475
Wire Wire Line
	6450 4475 6450 4500
Wire Wire Line
	6450 4475 6650 4475
Wire Wire Line
	6625 4825 6625 5050
Wire Wire Line
	6625 4125 6625 4275
Wire Wire Line
	6050 4725 6250 4725
Wire Wire Line
	5850 4725 6050 4725
Wire Wire Line
	5850 4725 5850 4750
Wire Wire Line
	5875 3250 6050 3250
Wire Wire Line
	5675 3250 5875 3250
Wire Wire Line
	5475 3250 5675 3250
Wire Wire Line
	5675 3500 5875 3500
Wire Wire Line
	5475 3500 5475 3525
Wire Wire Line
	5475 3500 5675 3500
Wire Wire Line
	8375 2950 8575 2950
Wire Wire Line
	6350 3950 6625 3950
Wire Wire Line
	6050 3250 6050 4475
Wire Wire Line
	6050 2525 6050 3250
Wire Wire Line
	6175 2825 6175 2850
Wire Wire Line
	6825 2525 6825 2625
Wire Wire Line
	4350 4575 4425 4575
Wire Wire Line
	4350 5675 4425 5675
Wire Wire Line
	8100 3475 8125 3475
Wire Wire Line
	5275 3250 5475 3250
Wire Wire Line
	5275 3500 5475 3500
Wire Wire Line
	9450 3475 9450 3500
$Comp
L hermeslite:R W301
U 1 1 5FCC883C
P 7550 5775
F 0 "W301" V 7475 5775 39  0000 C CNN
F 1 "21" V 7557 5776 39  0000 C CNN
F 2 "" V 7480 5775 30  0001 C CNN
F 3 "" H 7550 5775 30  0000 C CNN
	1    7550 5775
	0    1    1    0   
$EndComp
Text Notes 7400 5925 0    60   ~ 0
21 cm wire required
NoConn ~ 7400 5775
NoConn ~ 7700 5775
Wire Wire Line
	7450 4275 7525 4275
Wire Wire Line
	1025 4825 1025 4900
$Comp
L power:GND #PWR0318
U 1 1 5FE6ADD2
P 2175 5100
F 0 "#PWR0318" H 2175 5100 39  0001 C CNN
F 1 "GND" H 2175 5030 39  0001 C CNN
F 2 "" H 2175 5100 60  0000 C CNN
F 3 "" H 2175 5100 60  0000 C CNN
	1    2175 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2175 5025 2175 5100
$Comp
L hermeslite:C_Small C304
U 1 1 5FEE6652
P 9550 2525
F 0 "C304" V 9675 2500 39  0000 L CNN
F 1 "0.1uF" V 9750 2500 39  0000 L CNN
F 2 "HERMESLITE:SMD-0603" H 9550 2525 50  0001 C CNN
F 3 "" H 9550 2525 50  0000 C CNN
F 4 "" H 9725 2575 39  0000 C CNN "Option"
	1    9550 2525
	0    -1   1    0   
$EndComp
$Comp
L power:GND #PWR0304
U 1 1 5FEE6658
P 9825 2800
F 0 "#PWR0304" H 9825 2800 39  0001 C CNN
F 1 "GND" H 9825 2730 39  0001 C CNN
F 2 "" H 9825 2800 60  0000 C CNN
F 3 "" H 9825 2800 60  0000 C CNN
	1    9825 2800
	-1   0    0    -1  
$EndComp
$Comp
L hermeslite:BNC RF301
U 1 1 5FEE665F
P 9825 2525
F 0 "RF301" H 9925 2650 39  0000 C CNN
F 1 "SMA" H 10000 2425 39  0000 C CNN
F 2 "HERMESLITE:SMAEDGE_THFULL_L" H 9825 2525 50  0001 C CNN
F 3 "" H 9825 2525 50  0000 C CNN
	1    9825 2525
	1    0    0    -1  
$EndComp
Wire Wire Line
	9675 2525 9650 2525
Wire Wire Line
	9825 2725 9825 2800
Wire Wire Line
	8975 2525 9300 2525
Wire Wire Line
	8975 2525 8975 2975
Text Notes 10075 2525 0    60   ~ 0
To Radioberry RX
Text Notes 10150 3475 0    60   ~ 0
Antenna
Text Notes 675  4275 0    60   ~ 0
From TX Radioberry
Wire Wire Line
	9425 3275 9700 3275
Wire Wire Line
	5325 3950 6350 3950
Wire Wire Line
	5325 5050 6350 5050
Wire Wire Line
	4750 5425 5025 5425
Wire Wire Line
	4750 4325 5025 4325
Wire Wire Line
	6650 2250 6825 2250
Wire Wire Line
	6825 2175 6825 2250
Connection ~ 6825 2250
Wire Wire Line
	6825 2250 6950 2250
Wire Wire Line
	6825 2250 6825 2525
Text GLabel 4175 4575 0    50   Input ~ 0
bias1
Text GLabel 4175 5675 0    50   Input ~ 0
bias0
Wire Wire Line
	3625 5425 4525 5425
Wire Wire Line
	3625 4325 4525 4325
Wire Wire Line
	3175 5025 3175 5425
Wire Wire Line
	3175 4625 3175 4325
Wire Wire Line
	3175 4325 3325 4325
Wire Wire Line
	3325 5425 3175 5425
$Comp
L hermeslite:R R307
U 1 1 5FEB0070
P 1800 4625
F 0 "R307" V 1725 4625 39  0000 C CNN
F 1 "5.9" V 1807 4626 39  0000 C CNN
F 2 "HERMESLITE:SMD-0805" V 1730 4625 30  0001 C CNN
F 3 "" H 1800 4625 30  0000 C CNN
	1    1800 4625
	0    1    1    0   
$EndComp
$Comp
L hermeslite:R R309
U 1 1 5FEB09FD
P 1625 4900
F 0 "R309" H 1750 4925 39  0000 C CNN
F 1 "866" H 1750 5000 39  0000 C CNN
F 2 "HERMESLITE:SMD-0805" V 1555 4900 30  0001 C CNN
F 3 "" H 1625 4900 30  0000 C CNN
	1    1625 4900
	-1   0    0    1   
$EndComp
$Comp
L hermeslite:R R310
U 1 1 5FEB1A8E
P 1975 4900
F 0 "R310" H 2125 4925 39  0000 C CNN
F 1 "866" H 2100 5000 39  0000 C CNN
F 2 "HERMESLITE:SMD-0805" V 1905 4900 30  0001 C CNN
F 3 "" H 1975 4900 30  0000 C CNN
	1    1975 4900
	-1   0    0    1   
$EndComp
Wire Wire Line
	1400 4625 1625 4625
Wire Wire Line
	1625 4750 1625 4625
Connection ~ 1625 4625
Wire Wire Line
	1625 4625 1650 4625
Wire Wire Line
	1975 4750 1975 4625
Wire Wire Line
	1625 5050 1625 5125
Wire Wire Line
	1625 5125 1800 5125
Wire Wire Line
	1975 5125 1975 5050
$Comp
L power:GND #PWR0319
U 1 1 5FECF1D0
P 1800 5125
F 0 "#PWR0319" H 1800 5125 39  0001 C CNN
F 1 "GND" H 1800 5055 39  0001 C CNN
F 2 "" H 1800 5125 60  0000 C CNN
F 3 "" H 1800 5125 60  0000 C CNN
	1    1800 5125
	1    0    0    -1  
$EndComp
Connection ~ 1800 5125
Wire Wire Line
	1800 5125 1975 5125
$Comp
L power:GND #PWR0313
U 1 1 5FF30D89
P 7525 4275
F 0 "#PWR0313" H 7525 4025 50  0001 C CNN
F 1 "GND" H 7525 4125 50  0001 C CNN
F 2 "" H 7525 4275 50  0000 C CNN
F 3 "" H 7525 4275 50  0000 C CNN
	1    7525 4275
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 4675 7550 4675
Wire Wire Line
	7750 4675 8975 4675
Wire Wire Line
	8975 3575 8975 4675
$Comp
L hermeslite:C_Small C301
U 1 1 5FD52EB5
P 8450 1775
F 0 "C301" H 8500 1850 39  0000 L CNN
F 1 "1uF" H 8500 1700 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 8450 1775 50  0001 C CNN
F 3 "" H 8450 1775 50  0000 C CNN
F 4 "DNI" H 8450 1775 50  0001 C CNN "Option"
	1    8450 1775
	-1   0    0    -1  
$EndComp
$Comp
L hermeslite:C_Small C302
U 1 1 5FD54917
P 8450 2175
F 0 "C302" H 8500 2250 39  0000 L CNN
F 1 "1uF" H 8500 2100 39  0000 L CNN
F 2 "HERMESLITE:SMD-0805" H 8450 2175 50  0001 C CNN
F 3 "" H 8450 2175 50  0000 C CNN
F 4 "DNI" H 8450 2175 50  0001 C CNN "Option"
	1    8450 2175
	-1   0    0    -1  
$EndComp
$Comp
L hermeslite:R R301
U 1 1 5FD54DB6
P 8700 1925
F 0 "R301" V 8625 1925 39  0000 C CNN
F 1 "10K" V 8700 1925 39  0000 C CNN
F 2 "HERMESLITE:SMD-0805" V 8630 1925 30  0001 C CNN
F 3 "" H 8700 1925 30  0000 C CNN
F 4 "DNI" V 8700 1925 50  0001 C CNN "Option"
	1    8700 1925
	0    1    1    0   
$EndComp
$Comp
L hermeslite:R R302
U 1 1 5FD5552B
P 8975 2125
F 0 "R302" H 8850 2100 39  0000 C CNN
F 1 "10K" V 8982 2126 39  0000 C CNN
F 2 "HERMESLITE:SMD-0805" V 8905 2125 30  0001 C CNN
F 3 "" H 8975 2125 30  0000 C CNN
F 4 "DNI" H 8975 2125 50  0001 C CNN "Option"
	1    8975 2125
	-1   0    0    1   
$EndComp
Wire Wire Line
	8450 2075 8450 1925
Wire Wire Line
	8450 1925 8550 1925
Connection ~ 8450 1925
Wire Wire Line
	8450 1925 8450 1875
Wire Wire Line
	8850 1925 8975 1925
Wire Wire Line
	8975 1925 8975 1975
Wire Wire Line
	8450 2275 8675 2275
Wire Wire Line
	8975 1925 9300 1925
Wire Wire Line
	9300 1925 9300 2525
Connection ~ 8975 1925
Connection ~ 9300 2525
Wire Wire Line
	9300 2525 9450 2525
Text Label 9550 3275 0    50   ~ 0
ant
Wire Wire Line
	8450 1675 8450 1550
Text Label 8450 1550 0    50   ~ 0
ant
$Comp
L power:GND #PWR0302
U 1 1 5FD856A4
P 8675 2375
F 0 "#PWR0302" H 8675 2125 50  0001 C CNN
F 1 "GND" H 8675 2225 50  0001 C CNN
F 2 "" H 8675 2375 50  0000 C CNN
F 3 "" H 8675 2375 50  0000 C CNN
	1    8675 2375
	1    0    0    -1  
$EndComp
Wire Wire Line
	8675 2375 8675 2275
Connection ~ 8675 2275
Wire Wire Line
	8675 2275 8975 2275
Wire Wire Line
	2975 4625 3175 4625
Wire Wire Line
	2975 5025 3175 5025
$Comp
L hermeslite:TRANSF4CT T302
U 1 1 5FE45EDE
P 2575 4825
F 0 "T302" H 2575 4400 50  0000 C CNN
F 1 "1:1" H 2575 4491 50  0000 C CNN
F 2 "HERMESLITE:TRANSFSMT" H 2575 5114 50  0001 C CNN
F 3 "" H 2575 4825 50  0001 C CNN
	1    2575 4825
	1    0    0    1   
$EndComp
Wire Wire Line
	1950 4625 1975 4625
Connection ~ 1975 4625
Wire Wire Line
	1975 4625 2175 4625
NoConn ~ 2175 4825
Text Notes 1625 5425 0    50   ~ 0
1dB  ATT
$EndSCHEMATC
