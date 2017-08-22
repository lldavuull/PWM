/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

extern void Sweep_PWM(void);
extern void Sweep_PWM_init(void);
//volatile char RxAr[450];
volatile char *RxArPtr;
volatile int i = 0;

typedef union {
    unsigned int DC[256];
    struct{
    unsigned DCL :8;
    unsigned DCH :8;
    }PWM[256];
}PWM_16;

const PWM_16 PWM={
0x0000,	0x008C,	0x008E,	0x0091,	0x0094,	0x0096,	0x0099,	0x009C,	0x009F,	0x00A2,	//9
0x00A5,	0x00A8,	0x00AC,	0x00AF,	0x00B2,	0x00B6,	0x00B9,	0x00BC,	0x00C0,	0x00C4,	//19
0x00C7,	0x00CB,	0x00CF,	0x00D3,	0x00D7,	0x00DB,	0x00DF,	0x00E3,	0x00E8,	0x00EC,	//29
0x00F1,	0x00F5,	0x00FA,	0x00FF,	0x0103,	0x0108,	0x010D,	0x0112,	0x0118,	0x011D,	//39
0x0122,	0x0128,	0x012D,	0x0133,	0x0139,	0x013F,	0x0145,	0x014B,	0x0151,	0x0158,	//49
0x015E,	0x0165,	0x016C,	0x0173,	0x017A,	0x0181,	0x0188,	0x0190,	0x0197,	0x019F,	//59
0x01A7,	0x01AF,	0x01B7,	0x01BF,	0x01C8,	0x01D0,	0x01D9,	0x01E2,	0x01EB,	0x01F4,	//69
0x01FE,	0x0208,	0x0211,	0x021C,	0x0226,	0x0230,	0x023B,	0x0246,	0x0251,	0x025C,	//79
0x0267,	0x0273,	0x027F,	0x028B,	0x0297,	0x02A4,	0x02B1,	0x02BE,	0x02CB,	0x02D8,	//89
0x02E6,	0x02F4,	0x0303,	0x0311,	0x0320,	0x032F,	0x033F,	0x034E,	0x035E,	0x036F,	//99
0x037F,	0x0390,	0x03A2,	0x03B3,	0x03C5,	0x03D7,	0x03EA,	0x03FD,	0x0410,	0x0424,	//109
0x0438,	0x044D,	0x0461,	0x0477,	0x048C,	0x04A2,	0x04B9,	0x04D0,	0x04E7,	0x04FF,	//119
0x0517,	0x0530,	0x0549,	0x0562,	0x057D,	0x0597,	0x05B2,	0x05CE,	0x05EA,	0x0607,	//129
0x0624,	0x0642,	0x0660,	0x067F,	0x069E,	0x06BE,	0x06DF,	0x0700,	0x0722,	0x0745,	//139
0x0768,	0x078C,	0x07B1,	0x07D6,	0x07FC,	0x0822,	0x084A,	0x0872,	0x089B,	0x08C5,	//149
0x08EF,	0x091B,	0x0947,	0x0974,	0x09A1,	0x09D0,	0x0A00,	0x0A30,	0x0A61,	0x0A94,	//159
0x0AC7,	0x0AFB,	0x0B30,	0x0B67,	0x0B9E,	0x0BD6,	0x0C10,	0x0C4A,	0x0C86,	0x0CC2,	//169
0x0D00,	0x0D3F,	0x0D7F,	0x0DC1,	0x0E03,	0x0E47,	0x0E8C,	0x0ED3,	0x0F1B,	0x0F64,	//179
0x0FAE,	0x0FFA,	0x1048,	0x1097,	0x10E7,	0x1139,	0x118D,	0x11E2,	0x1238,	0x1291,	//189
0x12EA,	0x1346,	0x13A4,	0x1403,	0x1464,	0x14C6,	0x152B,	0x1592,	0x15FA,	0x1665,	//199
0x16D1,	0x1740,	0x17B0,	0x1823,	0x1898,	0x190F,	0x1989,	0x1A05,	0x1A83,	0x1B03,	//209
0x1B86,	0x1C0B,	0x1C93,	0x1D1E,	0x1DAB,	0x1E3A,	0x1ECD,	0x1F62,	0x1FFA,	0x2095,	//219
0x2133,	0x21D4,	0x2278,	0x231F,	0x23C9,	0x2476,	0x2527,	0x25DB,	0x2693,	0x274D,	//229
0x280C,	0x28CE,	0x2994,	0x2A5D,	0x2B2A,	0x2BFB,	0x2CD1,	0x2DAA,	0x2E87,	0x2F68,	//239
0x304E,	0x3138,	0x3227,	0x331A,	0x3411,	0x350E,	0x360F,	0x3715,	0x381F,	0x392F,	//249
0x3A44,	0x3B5F,	0x3C7E,	0x3DA4,	0x3ECE,	0x3FFF,		
};


char *PWMDCLptr[4];
char *PWMDCHptr[4];
char *PWMLDCONptr[4];

//#define PWMDCH[0] PWM1DCH
//#define PWMDCL[0] PWM1DCH
//#define PWMDCH[1] PWM2DCH
//#define PWMDCL[1] PWM2DCH
//#define PWMDCH[2] PWM3DCH
//#define PWMDCL[2] PWM3DCH
//#define PWMDCH[3] PWM4DCH
//#define PWMDCL[3] PWM4DCH
//
//#define PWMLDCON[0]=PWM1LDCON
//#define PWMLDCON[1]=PWM2LDCON
//#define PWMLDCON[2]=PWM3LDCON
//#define PWMLDCON[3]=PWM4LDCON



//0x0000, 0x0046, 0x0046, 0x0046, 0x0046, 0x0046, 0x0046, 0x0048, 0x0048, 0x0048,
//    0x0048, 0x0049, 0x0049, 0x0049, 0x004A, 0x004A, 0x004C, 0x004D, 0x004D, 0x004E,
//    0x0050, 0x0052, 0x0053, 0x0054, 0x0056, 0x0058, 0x005A, 0x005C, 0x005E, 0x0060,
//    0x0062, 0x0064, 0x0067, 0x006A, 0x006C, 0x006F, 0x0072, 0x0076, 0x007A, 0x007C,
//    0x0080, 0x0083, 0x0088, 0x008A, 0x008E, 0x0092, 0x0098, 0x009E, 0x00A4, 0x00AC,
//    0x00B4, 0x00BC, 0x00C0, 0x00C8, 0x00D0, 0x00DE, 0x00E4, 0x00EC, 0x00F6, 0x0106,
//    0x010E, 0x0116, 0x011E, 0x012C, 0x0134, 0x013C, 0x0146, 0x0156, 0x0160, 0x0168,
//    0x0176, 0x0182, 0x018A, 0x0196, 0x01A6, 0x01B2, 0x01BE, 0x01CC, 0x01D8, 0x01E4,
//    0x01F6, 0x0202, 0x020E, 0x0220, 0x0230, 0x0240, 0x024C, 0x0262, 0x0270, 0x0280,
//    0x0292, 0x02A2, 0x02B6, 0x02C6, 0x02D6, 0x02EA, 0x0300, 0x031C, 0x0326, 0x0336,
//    0x0348, 0x0360, 0x0376, 0x0386, 0x039E, 0x03B0, 0x03C8, 0x03DC, 0x03F4, 0x0406,
//    0x041C, 0x043A, 0x044C, 0x0468, 0x047E, 0x0496, 0x04B2, 0x04C8, 0x04E0, 0x04F8,
//    0x0514, 0x0530, 0x054C, 0x0566, 0x0580, 0x059C, 0x05B8, 0x05D8, 0x05F4, 0x060E,
//    0x0628, 0x0648, 0x0668, 0x0688, 0x06A4, 0x06C4, 0x06DC, 0x0704, 0x0724, 0x0740,
//    0x0764, 0x0784, 0x07A4, 0x07C8, 0x07EC, 0x0810, 0x0830, 0x0850, 0x0870, 0x0898,
//    0x08C0, 0x08E8, 0x0908, 0x0930, 0x0958, 0x0980, 0x09A0, 0x09C8, 0x09F0, 0x0A18,
//    0x0A40, 0x0A68, 0x0A94, 0x0ABC, 0x0AE4, 0x0B10, 0x0B38, 0x0B60, 0x0B8C, 0x0BB4,
//    0x0BE8, 0x0C16, 0x0C40, 0x0C6C, 0x0C98, 0x0CCC, 0x0CFC, 0x0D28, 0x0D58, 0x0D88,
//    0x0DB8, 0x0DE8, 0x0E18, 0x0E48, 0x0E78, 0x0EA8, 0x0EE0, 0x0F10, 0x0F48, 0x0F7C,
//    0x0FB0, 0x0FE4, 0x1018, 0x1050, 0x1088, 0x10BC, 0x10F4, 0x112C, 0x1164, 0x1198,//200
//    0x11D0, 0x120C, 0x1244, 0x127C, 0x12B4, 0x12F0, 0x1328, 0x136C, 0x13A8, 0x13E4,
//    0x1420, 0x145C, 0x149C, 0x14DC, 0x1518, 0x1558, 0x159C, 0x15DC, 0x161C, 0x165C,
//    0x169C, 0x16E0, 0x1728, 0x1768, 0x17A8, 0x17F0, 0x1838, 0x1878, 0x18C0, 0x1904,
//    0x1948, 0x1998, 0x19E0, 0x1A28, 0x1A70, 0x1AB8, 0x1B00, 0x1B48, 0x1B90, 0x1BE0,
//    0x1C30, 0x1C80, 0x1CC8, 0x1D18, 0x1D68, 0x1D80, 0x1DB8, 0x1E08, 0x1E50, 0x1EA0,
//    0x1EE8, 0x1F30, 0x1F78, 0x1FC0, 0x1FE8, 0x1FFF