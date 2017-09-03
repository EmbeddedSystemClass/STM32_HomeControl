
#ifndef _AP_300_H_
#define _AP_300_H_

//
typedef struct {
	byte level;			// 00 - Lüfterstufe (0,9,1,2,3)
	byte volume;		// 01 - Grundeinstellung Volumenstrom/10
	byte weekday;		// 02 - 2^Weekday (1=Mo, 2=Tu, 4=We, 8=Th, 16=Fr, 32=Sa, 64=Su)
	byte hour;			// 03 - Stunde
	byte minute;		// 04 - Minute
	byte unknown5;		// 05
	byte unknown6;		// 06
	byte calibrate;	// 07 - 1=Kalibrieren(write)
	byte unknown8[5];	// 08-12
	byte flame;			// 13 - 04=>Raumlaufabhängige Feuerstätte, 00=>Keine oder geschlossene Feuerstätte
	byte bypass_max;	// 14
	byte bypass_min;	// 15
	byte T1;				// 16
	byte T2;				// 17
	byte T3;				// 18
	byte T4;				// 19
	byte S1;				// 20 - S1/2
	byte S2;				// 21 - S2/2
	byte unknown22;	// 22
	byte unknown23;	// 23
	byte heat_reg;		// 24 - Vorheizregisterstatus
	byte calibration;	// 25 - Kalibrierungsstatus 0=unkalibriert, 2=kalibrierung aktiv, 3=kalibrierung abgeschlossen
	byte unknown26[];	// 26 - 37
	byte cntdwn_st_h;	// 38 - H-Byte Startup Countdown -> Register 1043
	byte cntdwn_st_l;	// 39 - L-Byte Startup Countdown -> Register 1043
} ap300_reg_t;
//
typedef struct {
	byte lvl;		// Stufe
	byte weekdays;	// Aktive Wochentag(e) Bitmask 1=Montag 2=Dienstag 3=Montag+Dienstag...
	byte st_hour;	// Startstunde
	byte st_min;	// Startminute
	byte end_hour;	// Endestunde
	byte end_min;	// Endeminute
} prog_t;
//
typedef struct {
	prog_t prog[20];	// 512-517, 518-523, ...
} ap300_prog_t;
//
// Register >0x400 (1024) sind komprimierte Register und werden von der Fernbedieung abgefragt.
// Im Register 0x400 steht im H-Byte Register 0 und im L-Byte Register 1.
// Im Register 0x401 steht im H-Byte Register 2 und im L-Byte Register 3 u.s.w.
typedef struct {
	int reg01_reg00;	// 1024
	int reg03_reg02;	// 1025
	int reg05_reg04;	// 1026
	int reg07_reg06;	// 1027
	int reg09_reg08;	// 1028
	int reg11_reg10;	// 1029
	int reg13_reg12;	// 1030
	int reg15_reg14;	// 1031
	int reg17_reg16;	// 1032
	int reg19_reg18;	// 1033
	int reg21_reg20;	// 1034
	int reg23_reg22;	// 1035
	int reg25_reg24;	// 1036
	int reg27_reg26;	// 1037
	int reg29_reg28;	// 1038
	int reg31_reg30;	// 1039
	int reg33_reg32;	// 1040
	int reg35_reg34;	// 1041
	int reg37_reg36;	// 1042
	int reg39_reg38;	// 1043
} ap300_compressed_reg_t;


#endif	// _AP_300_H_


