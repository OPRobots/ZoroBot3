EESchema Schematic File Version 4
LIBS:PCB_ZoroBot_v3-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 2200 4800 0    50   ~ 0
ENC_1_A_CONN
Text Label 2200 4900 0    50   ~ 0
ENC_1_B_CONN
Text Label 2200 5100 0    50   ~ 0
ENC_1_I_CONN
$Comp
L Device:C C46
U 1 1 5E471179
P 4450 3650
F 0 "C46" H 4475 3750 50  0000 L CNN
F 1 "0.1uF" H 4475 3550 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4488 3500 50  0001 C CNN
F 3 "" H 4450 3650 50  0001 C CNN
	1    4450 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C45
U 1 1 5E47117F
P 4150 3650
F 0 "C45" H 4175 3750 50  0000 L CNN
F 1 "1uF" H 4175 3550 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4188 3500 50  0001 C CNN
F 3 "" H 4150 3650 50  0001 C CNN
	1    4150 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 3500 4150 3400
Wire Wire Line
	4150 3800 4150 3900
Wire Wire Line
	4450 3900 4450 3800
Wire Wire Line
	4450 3400 4450 3500
Wire Wire Line
	2200 4800 2750 4800
Wire Wire Line
	2200 4900 2750 4900
Wire Wire Line
	2200 5100 2750 5100
Wire Wire Line
	2200 5200 2750 5200
Wire Wire Line
	4250 4700 4400 4700
Text Label 4900 4600 2    50   ~ 0
ENC_1_3.3
Wire Wire Line
	4150 3400 4300 3400
Wire Wire Line
	4300 3400 4300 3300
Connection ~ 4300 3400
Wire Wire Line
	4300 3400 4450 3400
NoConn ~ 2750 4600
NoConn ~ 2750 4700
NoConn ~ 2750 5300
$Comp
L Connector:Conn_01x05_Male J_ENC_1_CONN1
U 1 1 5E471196
P 3100 3700
F 0 "J_ENC_1_CONN1" H 3073 3630 50  0000 R CNN
F 1 "Conn_01x05_Male" H 3073 3721 50  0000 R CNN
F 2 "PCB_ZoroBot_v3:encd_fuji" H 3100 3700 50  0001 C CNN
F 3 "~" H 3100 3700 50  0001 C CNN
	1    3100 3700
	-1   0    0    1   
$EndComp
Text Label 2350 3500 0    50   ~ 0
ENC_1_A_CONN
Text Label 2350 3600 0    50   ~ 0
ENC_1_B_CONN
Text Label 2350 3700 0    50   ~ 0
ENC_1_I_CONN
Wire Wire Line
	2350 3500 2900 3500
Wire Wire Line
	2350 3600 2900 3600
Wire Wire Line
	2350 3700 2900 3700
Text Label 2350 3800 0    50   ~ 0
ENC_1_3.3
Wire Wire Line
	2350 3800 2900 3800
Text Label 2200 5200 0    50   ~ 0
ENC_1_GND
Wire Wire Line
	4250 4600 4400 4600
Wire Wire Line
	4400 4700 4400 4600
Connection ~ 4400 4600
Wire Wire Line
	4400 4600 4900 4600
Text Label 4300 3300 2    50   ~ 0
ENC_1_3.3
Wire Wire Line
	2350 3900 2900 3900
Text Label 2350 3900 0    50   ~ 0
ENC_1_GND
Text Label 6650 4800 0    50   ~ 0
ENC_2_A_CONN
Text Label 6650 4900 0    50   ~ 0
ENC_2_B_CONN
Text Label 6650 5100 0    50   ~ 0
ENC_2_I_CONN
$Comp
L Device:C C48
U 1 1 5E4711CA
P 8900 3650
F 0 "C48" H 8925 3750 50  0000 L CNN
F 1 "0.1uF" H 8925 3550 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 8938 3500 50  0001 C CNN
F 3 "" H 8900 3650 50  0001 C CNN
	1    8900 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C47
U 1 1 5E4711D0
P 8600 3650
F 0 "C47" H 8625 3750 50  0000 L CNN
F 1 "1uF" H 8625 3550 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 8638 3500 50  0001 C CNN
F 3 "" H 8600 3650 50  0001 C CNN
	1    8600 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 3500 8600 3400
Wire Wire Line
	8600 3800 8600 3900
Wire Wire Line
	8900 3900 8900 3800
Wire Wire Line
	8900 3400 8900 3500
Wire Wire Line
	6650 4800 7200 4800
Wire Wire Line
	6650 4900 7200 4900
Wire Wire Line
	6650 5100 7200 5100
Wire Wire Line
	6650 5200 7200 5200
Wire Wire Line
	8700 4700 8850 4700
Text Label 9350 4600 2    50   ~ 0
ENC_2_3.3
Wire Wire Line
	8600 3400 8750 3400
Wire Wire Line
	8750 3400 8750 3300
Connection ~ 8750 3400
Wire Wire Line
	8750 3400 8900 3400
NoConn ~ 7200 4600
NoConn ~ 7200 4700
NoConn ~ 7200 5300
NoConn ~ 8700 5000
NoConn ~ 8700 5200
NoConn ~ 8700 5300
$Comp
L Connector:Conn_01x05_Male J_ENC_2_CONN1
U 1 1 5E4711EA
P 7550 3700
F 0 "J_ENC_2_CONN1" H 7523 3630 50  0000 R CNN
F 1 "Conn_01x05_Male" H 7523 3721 50  0000 R CNN
F 2 "PCB_ZoroBot_v3:encd_fuji" H 7550 3700 50  0001 C CNN
F 3 "~" H 7550 3700 50  0001 C CNN
	1    7550 3700
	-1   0    0    1   
$EndComp
Text Label 6800 3500 0    50   ~ 0
ENC_2_A_CONN
Text Label 6800 3600 0    50   ~ 0
ENC_2_B_CONN
Text Label 6800 3700 0    50   ~ 0
ENC_2_I_CONN
Wire Wire Line
	6800 3500 7350 3500
Wire Wire Line
	6800 3600 7350 3600
Wire Wire Line
	6800 3700 7350 3700
Text Label 6800 3800 0    50   ~ 0
ENC_2_3.3
Wire Wire Line
	6800 3800 7350 3800
Text Label 6650 5200 0    50   ~ 0
ENC_2_GND
Wire Wire Line
	8700 4600 8850 4600
Wire Wire Line
	8850 4700 8850 4600
Connection ~ 8850 4600
Wire Wire Line
	8850 4600 9350 4600
Text Label 8750 3300 2    50   ~ 0
ENC_2_3.3
Wire Wire Line
	6800 3900 7350 3900
Text Label 6800 3900 0    50   ~ 0
ENC_2_GND
Text Notes 4450 2150 0    236  ~ 47
ENCODERS FUJI
Wire Wire Line
	8600 3900 8750 3900
Wire Wire Line
	4150 3900 4300 3900
Wire Wire Line
	8750 3900 8750 4000
Connection ~ 8750 3900
Wire Wire Line
	8750 3900 8900 3900
Wire Wire Line
	4300 3900 4300 4000
Connection ~ 4300 3900
Wire Wire Line
	4300 3900 4450 3900
Text Label 4300 4000 2    50   ~ 0
ENC_1_GND
Text Label 8750 4000 2    50   ~ 0
ENC_2_GND
$Comp
L AS5145B-HSST:AS5145B-HSST IC3
U 1 1 5E47122E
P 2750 4600
F 0 "IC3" H 3500 4865 50  0000 C CNN
F 1 "AS5145B-HSST" H 3500 4774 50  0000 C CNN
F 2 "AS5145B-HSST:SOP65P780X199-16N" H 4100 4700 50  0001 L CNN
F 3 "https://media.digikey.com/pdf/Data%20Sheets/Austriamicrosystems%20PDFs/AS5145H,A,B.pdf" H 4100 4600 50  0001 L CNN
F 4 "Board Mount Hall Effect / Magnetic Sensors AS5145B-HSST SSOP16 LF T&RDP" H 4100 4500 50  0001 L CNN "Description"
F 5 "1.99" H 4100 4400 50  0001 L CNN "Height"
F 6 "ams" H 4100 4300 50  0001 L CNN "Manufacturer_Name"
F 7 "AS5145B-HSST" H 4100 4200 50  0001 L CNN "Manufacturer_Part_Number"
F 8 "985-AS5145B-HSST" H 4100 4100 50  0001 L CNN "Mouser Part Number"
F 9 "https://www.mouser.com/Search/Refine.aspx?Keyword=985-AS5145B-HSST" H 4100 4000 50  0001 L CNN "Mouser Price/Stock"
F 10 "" H 4100 3900 50  0001 L CNN "RS Part Number"
F 11 "" H 4100 3800 50  0001 L CNN "RS Price/Stock"
	1    2750 4600
	1    0    0    -1  
$EndComp
NoConn ~ 2750 5000
NoConn ~ 4250 4800
NoConn ~ 4250 4900
NoConn ~ 4250 5000
NoConn ~ 4250 5200
NoConn ~ 4250 5300
Wire Wire Line
	4800 5100 4250 5100
Text Label 4800 5100 2    50   ~ 0
ENC_1_GND
$Comp
L AS5145B-HSST:AS5145B-HSST IC4
U 1 1 5E471244
P 7200 4600
F 0 "IC4" H 7950 4865 50  0000 C CNN
F 1 "AS5145B-HSST" H 7950 4774 50  0000 C CNN
F 2 "AS5145B-HSST:SOP65P780X199-16N" H 8550 4700 50  0001 L CNN
F 3 "https://media.digikey.com/pdf/Data%20Sheets/Austriamicrosystems%20PDFs/AS5145H,A,B.pdf" H 8550 4600 50  0001 L CNN
F 4 "Board Mount Hall Effect / Magnetic Sensors AS5145B-HSST SSOP16 LF T&RDP" H 8550 4500 50  0001 L CNN "Description"
F 5 "1.99" H 8550 4400 50  0001 L CNN "Height"
F 6 "ams" H 8550 4300 50  0001 L CNN "Manufacturer_Name"
F 7 "AS5145B-HSST" H 8550 4200 50  0001 L CNN "Manufacturer_Part_Number"
F 8 "985-AS5145B-HSST" H 8550 4100 50  0001 L CNN "Mouser Part Number"
F 9 "https://www.mouser.com/Search/Refine.aspx?Keyword=985-AS5145B-HSST" H 8550 4000 50  0001 L CNN "Mouser Price/Stock"
F 10 "" H 8550 3900 50  0001 L CNN "RS Part Number"
F 11 "" H 8550 3800 50  0001 L CNN "RS Price/Stock"
	1    7200 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9250 5100 8700 5100
Text Label 9250 5100 2    50   ~ 0
ENC_2_GND
NoConn ~ 8700 4900
NoConn ~ 8700 4800
NoConn ~ 7200 5000
$EndSCHEMATC
