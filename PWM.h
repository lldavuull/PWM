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
    unsigned int D[256];
    struct{
    unsigned DCL :8;
    unsigned DCH :8;
    }PWM[256];
}PWM_16;

const PWM_16 PWM={
    0x0000, 0x0046, 0x0046, 0x0046, 0x0046, 0x0046, 0x0046, 0x0048, 0x0048, 0x0048,
    0x0048, 0x0049, 0x0049, 0x0049, 0x004A, 0x004A, 0x004C, 0x004D, 0x004D, 0x004E,
    0x0050, 0x0052, 0x0053, 0x0054, 0x0056, 0x0058, 0x005A, 0x005C, 0x005E, 0x0060,
    0x0062, 0x0064, 0x0067, 0x006A, 0x006C, 0x006F, 0x0072, 0x0076, 0x007A, 0x007C,
    0x0080, 0x0083, 0x0088, 0x008A, 0x008E, 0x0092, 0x0098, 0x009E, 0x00A4, 0x00AC,
    0x00B4, 0x00BC, 0x00C0, 0x00C8, 0x00D0, 0x00DE, 0x00E4, 0x00EC, 0x00F6, 0x0106,
    0x010E, 0x0116, 0x011E, 0x012C, 0x0134, 0x013C, 0x0146, 0x0156, 0x0160, 0x0168,
    0x0176, 0x0182, 0x018A, 0x0196, 0x01A6, 0x01B2, 0x01BE, 0x01CC, 0x01D8, 0x01E4,
    0x01F6, 0x0202, 0x020E, 0x0220, 0x0230, 0x0240, 0x024C, 0x0262, 0x0270, 0x0280,
    0x0292, 0x02A2, 0x02B6, 0x02C6, 0x02D6, 0x02EA, 0x0300, 0x031C, 0x0326, 0x0336,
    0x0348, 0x0360, 0x0376, 0x0386, 0x039E, 0x03B0, 0x03C8, 0x03DC, 0x03F4, 0x0406,
    0x041C, 0x043A, 0x044C, 0x0468, 0x047E, 0x0496, 0x04B2, 0x04C8, 0x04E0, 0x04F8,
    0x0514, 0x0530, 0x054C, 0x0566, 0x0580, 0x059C, 0x05B8, 0x05D8, 0x05F4, 0x060E,
    0x0628, 0x0648, 0x0668, 0x0688, 0x06A4, 0x06C4, 0x06DC, 0x0704, 0x0724, 0x0740,
    0x0764, 0x0784, 0x07A4, 0x07C8, 0x07EC, 0x0810, 0x0830, 0x0850, 0x0870, 0x0898,
    0x08C0, 0x08E8, 0x0908, 0x0930, 0x0958, 0x0980, 0x09A0, 0x09C8, 0x09F0, 0x0A18,
    0x0A40, 0x0A68, 0x0A94, 0x0ABC, 0x0AE4, 0x0B10, 0x0B38, 0x0B60, 0x0B8C, 0x0BB4,
    0x0BE8, 0x0C16, 0x0C40, 0x0C6C, 0x0C98, 0x0CCC, 0x0CFC, 0x0D28, 0x0D58, 0x0D88,
    0x0DB8, 0x0DE8, 0x0E18, 0x0E48, 0x0E78, 0x0EA8, 0x0EE0, 0x0F10, 0x0F48, 0x0F7C,
    0x0FB0, 0x0FE4, 0x1018, 0x1050, 0x1088, 0x10BC, 0x10F4, 0x112C, 0x1164, 0x1198,
    0x11D0, 0x120C, 0x1244, 0x127C, 0x12B4, 0x12F0, 0x1328, 0x136C, 0x13A8, 0x13E4,
    0x1420, 0x145C, 0x149C, 0x14DC, 0x1518, 0x1558, 0x159C, 0x15DC, 0x161C, 0x165C,
    0x169C, 0x16E0, 0x1728, 0x1768, 0x17A8, 0x17F0, 0x1838, 0x1878, 0x18C0, 0x1904,
    0x1948, 0x1998, 0x19E0, 0x1A28, 0x1A70, 0x1AB8, 0x1B00, 0x1B48, 0x1B90, 0x1BE0,
    0x1C30, 0x1C80, 0x1CC8, 0x1D18, 0x1D68, 0x1D80, 0x1DB8, 0x1E08, 0x1E50, 0x1EA0,
    0x1EE8, 0x1F30, 0x1F78, 0x1FC0, 0x1FE8, 0x1FFF
};

