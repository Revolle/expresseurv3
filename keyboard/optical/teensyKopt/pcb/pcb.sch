EESchema Schematic File Version 4
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Amplifier_Operational:LM324 U?
U 1 1 5CB9E837
P 3900 2000
F 0 "U?" H 3900 2367 50  0000 C CNN
F 1 "LM324" H 3900 2276 50  0000 C CNN
F 2 "" H 3850 2100 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 3950 2200 50  0001 C CNN
	1    3900 2000
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM324 U?
U 2 1 5CB9F80D
P 3900 2800
F 0 "U?" H 3900 3167 50  0000 C CNN
F 1 "LM324" H 3900 3076 50  0000 C CNN
F 2 "" H 3850 2900 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 3950 3000 50  0001 C CNN
	2    3900 2800
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM324 U?
U 3 1 5CBA11CB
P 3850 3600
F 0 "U?" H 3850 3967 50  0000 C CNN
F 1 "LM324" H 3850 3876 50  0000 C CNN
F 2 "" H 3800 3700 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 3900 3800 50  0001 C CNN
	3    3850 3600
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM324 U?
U 4 1 5CBA2833
P 3850 4450
F 0 "U?" H 3850 4817 50  0000 C CNN
F 1 "LM324" H 3850 4726 50  0000 C CNN
F 2 "" H 3800 4550 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 3900 4650 50  0001 C CNN
	4    3850 4450
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM324 U?
U 5 1 5CBA44BF
P 3700 5350
F 0 "U?" H 3658 5396 50  0000 L CNN
F 1 "LM324" H 3658 5305 50  0000 L CNN
F 2 "" H 3650 5450 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 3750 5550 50  0001 C CNN
	5    3700 5350
	1    0    0    -1  
$EndComp
$Comp
L LED:CQY99 D?
U 1 1 5CBA5F34
P 1550 1500
F 0 "D?" V 1546 1423 50  0000 R CNN
F 1 "CQY99" V 1455 1423 50  0000 R CNN
F 2 "LED_THT:LED_D5.0mm_IRGrey" H 1550 1675 50  0001 C CNN
F 3 "https://www.prtice.info/IMG/pdf/CQY99.pdf" H 1500 1500 50  0001 C CNN
	1    1550 1500
	0    -1   -1   0   
$EndComp
$Comp
L LED:CQY99 D?
U 1 1 5CBA6844
P 1550 1950
F 0 "D?" V 1546 1873 50  0000 R CNN
F 1 "CQY99" V 1455 1873 50  0000 R CNN
F 2 "LED_THT:LED_D5.0mm_IRGrey" H 1550 2125 50  0001 C CNN
F 3 "https://www.prtice.info/IMG/pdf/CQY99.pdf" H 1500 1950 50  0001 C CNN
	1    1550 1950
	0    -1   -1   0   
$EndComp
$Comp
L LED:CQY99 D?
U 1 1 5CBA7014
P 1550 2450
F 0 "D?" V 1546 2373 50  0000 R CNN
F 1 "CQY99" V 1455 2373 50  0000 R CNN
F 2 "LED_THT:LED_D5.0mm_IRGrey" H 1550 2625 50  0001 C CNN
F 3 "https://www.prtice.info/IMG/pdf/CQY99.pdf" H 1500 2450 50  0001 C CNN
	1    1550 2450
	0    -1   -1   0   
$EndComp
$Comp
L Device:R_POT RV?
U 1 1 5CBA78A9
P 1350 4450
F 0 "RV?" H 1281 4496 50  0000 R CNN
F 1 "R_POT" H 1281 4405 50  0000 R CNN
F 2 "" H 1350 4450 50  0001 C CNN
F 3 "~" H 1350 4450 50  0001 C CNN
	1    1350 4450
	1    0    0    -1  
$EndComp
$Comp
L Device:Q_Photo_NPN Q?
U 1 1 5CBA83E5
P 1950 1550
F 0 "Q?" H 2140 1596 50  0000 L CNN
F 1 "Q_Photo_NPN" H 2140 1505 50  0000 L CNN
F 2 "" H 2150 1650 50  0001 C CNN
F 3 "~" H 1950 1550 50  0001 C CNN
	1    1950 1550
	1    0    0    -1  
$EndComp
$Comp
L Device:Q_Photo_NPN Q?
U 1 1 5CBA9872
P 1950 3350
F 0 "Q?" H 2140 3396 50  0000 L CNN
F 1 "Q_Photo_NPN" H 2140 3305 50  0000 L CNN
F 2 "" H 2150 3450 50  0001 C CNN
F 3 "~" H 1950 3350 50  0001 C CNN
	1    1950 3350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBAA2EB
P 2100 1750
F 0 "#PWR?" H 2100 1500 50  0001 C CNN
F 1 "GND" H 2105 1577 50  0000 C CNN
F 2 "" H 2100 1750 50  0001 C CNN
F 3 "" H 2100 1750 50  0001 C CNN
	1    2100 1750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBAABBC
P 2050 2800
F 0 "#PWR?" H 2050 2550 50  0001 C CNN
F 1 "GND" H 2055 2627 50  0000 C CNN
F 2 "" H 2050 2800 50  0001 C CNN
F 3 "" H 2050 2800 50  0001 C CNN
	1    2050 2800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBAD627
P 1550 2800
F 0 "#PWR?" H 1550 2550 50  0001 C CNN
F 1 "GND" H 1555 2627 50  0000 C CNN
F 2 "" H 1550 2800 50  0001 C CNN
F 3 "" H 1550 2800 50  0001 C CNN
	1    1550 2800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBADE91
P 1150 4700
F 0 "#PWR?" H 1150 4450 50  0001 C CNN
F 1 "GND" H 1155 4527 50  0000 C CNN
F 2 "" H 1150 4700 50  0001 C CNN
F 3 "" H 1150 4700 50  0001 C CNN
	1    1150 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 1750 2100 1650
Wire Wire Line
	2000 2700 2050 2700
Wire Wire Line
	2050 2700 2050 2800
Wire Wire Line
	1400 1950 1650 1950
Wire Wire Line
	1550 1700 1550 1850
Wire Wire Line
	1550 2150 1550 2350
Wire Wire Line
	1550 2650 1550 2800
$EndSCHEMATC