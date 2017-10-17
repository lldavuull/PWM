
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include "rdm_define.h"

//Response Type
enum{
    ACK=0x00,
    ACK_TIMER=0x01,
    NACK_REASON=0x02,
    ACK_OVERFLOW=0x03,
};

//Command Class
//enum{
//    DISCOVERY_COMMAND= 0x10,
//    DISCOVERY_COMMAND_RESPONSE=0x11,
//    GET_COMMAND=0x20,
//    GET_COMMAND_RESPONSE=0x21,
//    SET_COMMAND=0x30,
//    SET_COMMAND_RESPONSE=0x31,
//};
//enum{
//    DISC_UNIQUE_BRANCH = 0x0001,
//    DISC_MUTE = 0x0002,
//    DISC_UN_MUTE = 0x0003,
//};

//Parameter ID (PID)
//enum{
//    SUPPORTED_PARAMETERS = 0x0050,
//    PARAMETER_DESCRIPTION = 0x0051,
//    DEVICE_INFO = 0x0060,
//    SOFTWARE_VERSION_LABEL = 0x00C0,
//    DMX_START_ADDRESS = 0x00F0,
//    IDENTIFY_DEVICE = 0x1000
//};




////RDM set start
///** RxStart The Start Code for a valid data stream */
//const char RDM_StartCode = 0xCC;
////RDM set start
///** RxStart The Start Code for a valid data stream */
//const char RDM_SubStartCode = 0x01;

/**
*  Initialises the DMX512A Library
*  Please add near the beginning of you main() function before your main loop
*/
extern void RDM_init(void);
/**
* RDM Transmit interrupt. Called as part of dmx_interrupt() but can call seperately for optimization
*/
extern void RDM_tx_interrupt(void);

extern void RDM_tx_TimerBreak(void);
extern void RDM_discovery_CC(void);
extern void RDM_GET_CC(void);
extern void RDM_SET_CC(void);
extern void TX_RDM_Response_Set(void);
extern void RDM_rx_loop(void);
extern void RDM_TXSTART(void);
//extern void RDM_disc_tx_TimerBreak(void);

#define DiscoveryLength 23
/**
 *  Select the size of the transmit buffer
 *  This is the Maximum number of channels that will be sent.
 *  RDM requires a minimum of 24 and a maximum of 257
 *  This depends on how much RAM your PIC has available.
 */
//char TX_SIZE=26;
#define TX_SIZE     24

#define TX_PIN      LATC4   // TX pin use RC4
#define TX_TRIS     TRISC4  // Set TX pin as output
#define TX_PPS      RC4PPS  //RC4
#define RXTX_SWITCH_PIN      LATC3   // RX TX Switcher pin use RC3; RX=0, TX=1

/** TxData Transmit Data Buffer  */
//volatile uint8_t   TxData[TX_BUFFER_SIZE];
/** TxCount Counts how many bytes sent. Used in interrupt  */
uint16_t         TxCount=0;
///** TxStart Start code to transmit  */
//const uint8_t          TxStart=0;
/** *TxByte Pointer to data to send  */
volatile uint8_t *BytePtr;
/** TxState State for the Transmit state machine  */
volatile uint8_t TxState=0;

#define METEOR 0x08BA
#define DriverID 0x12345678
const char UID[6]={0x08,0xBA,0x12,0x34,0x56,0x78};


const char SOFTWARE_VERSION_LABEL[]="Meteor Lighting v171012";
#define SOFTWARE_VERSION_LABEL_Size      sizeof(SOFTWARE_VERSION_LABEL)
//{
//0x4D,	0x65,	0x74,	0x65,	0x6F,	0x72,	0x20,	0x4C,	0x69,	0x67,	//Meteor Lighting v171012
//0x68,	0x74,	0x69,	0x6E,	0x67,	0x20,	0x76,	0x31,	0x37,	0x31,	
//0x30,	0x31,	0x32,								
//};

enum
{
//    TX_MARK_BEFORE_BREAK,
//    TX_MAB,
    TX_DISCOVERY,
    TX_START,
    TX_SART_DISCOVERY,
    TX_DATA,
    TX_RDM_PD
};


typedef union{
    unsigned char value[24]; 
    struct{
        union{
            uint16_t CS;                //value[0~1]
            struct{
                unsigned CSL:8;         //ChechSum Low
                unsigned CSH:8;         //ChechSum High
            };
        };
        
        unsigned PDL:8;         //Parameter Data Length         //value[2]
        uint16_t PID;        //Parameter ID     //value[3~4]
        
        unsigned CC:8;          //Command Class     //value[5]
        uint16_t subDevice;      //sub device   //value[6~7]
        unsigned message:8;     //message count     //value[8]
        unsigned PORT:8;     //Port ID / Response Type  //value[9]
        unsigned TN:8;      //Transaction Number    //value[10]
        
        struct{
            uint32_t ID;
            uint16_t M;     //Manufacture
        }SUID;          //Source UID    //value[11~16]
        
        struct{
            uint32_t ID;
            uint16_t M;     //Manufacture
        }DUID;   // Destination UID      //value[17~22]
        
        unsigned ML:8;      //MessageLength     //value[23]
    };
}RDM_Data;

extern uint16_t RDM_get_checkSum(RDM_Data, char);
extern uint16_t RDM_set_checkSum(RDM_Data);


//char pd_c=0;
volatile RDM_Data RX_RDM_Data;
volatile RDM_Data TX_RDM_Data;
//volatile RDM_Data DISCOVERY_RDM_Data;
char DISCOVERY_RDM_Data[DiscoveryLength+1];

#define TX_PD_LEN      (PD_LEN>>1)   // Parameter Data Length
#define TX_PD_u16      (TX_PD_LEN>>1)   // Parameter Data Length
#define TX_PD_u32      (TX_PD_LEN>>2)   // Parameter Data Length

#define PD_LEN      100   // Parameter Data Length
#define PD_u16      (PD_LEN>>1)   // Parameter Data Length
#define PD_u32      (PD_LEN>>2)   // Parameter Data Length
//#define PD1  PD_LEN-2
//#define PD5  PD_LEN-6
//#define PD7  PD_LEN-8
//#define PD11  PD_LEN-12
//RDM Parameter Data
typedef union{
    uint8_t u8[PD_LEN];
    uint16_t u16[PD_u16];
    uint32_t u32[PD_u32];
}PD_call;
PD_call PD;

extern void PD_init(void);
extern void PD_write_u8(uint8_t);
extern void PD_write_u16(uint16_t);
extern void PD_write_u32(uint32_t);
extern uint8_t* PD_Read_u8ptr(void);
extern uint16_t* PD_Read_u16ptr(void);
extern uint32_t* PD_Read_u32ptr(void);
uint16_t *PD_Manu_ptr;
uint32_t *PD_ID_ptr;

char PDCount;
char TX_PDCount;

volatile char PackCount;
volatile unsigned PD_Flag;
volatile unsigned TX_PD_Flag;
volatile unsigned TX_Discovery_Flag;

uint16_t checkSum;

char IDENTIFY_MODE=0xff;

static const uint16_t PID_DEFINITIONS[] = {
//    E120_QUEUED_MESSAGE,
//    E120_PRODUCT_DETAIL_ID_LIST,
//    E120_DEVICE_MODEL_DESCRIPTION,
//    E120_MANUFACTURER_LABEL,
//    E120_DEVICE_LABEL,
//    E120_FACTORY_DEFAULTS,
//    E120_LANGUAGE_CAPABILITIES,
//    E120_LANGUAGE,
//    E120_BOOT_SOFTWARE_VERSION_ID,
//    E120_BOOT_SOFTWARE_VERSION_LABEL,
    E120_DMX_PERSONALITY,
    E120_DMX_PERSONALITY_DESCRIPTION,
//    E120_SENSOR_DEFINITION,
//    E120_SENSOR_VALUE,
//    E120_RECORD_SENSORS,
//    E120_DEVICE_HOURS,
//    E120_REAL_TIME_CLOCK,
//    E120_RESET_DEVICE,
//    E137_1_IDENTIFY_MODE,  //done
};


const char PERSONALITY_1[]="1 LOG";
const char PERSONALITY_2[]="2 LOG";
const char PERSONALITY_3[]="3 LOG";
const char PERSONALITY_4[]="4 LOG";
const char PERSONALITY_5[]="1 LOG, 1 CCT";
const char PERSONALITY_6[]="2 LOG, 2 CCT";

typedef struct _personality_definition
{
	const uint8_t personality;
//	void (*get_handler)(uint16_t sub_device);
//	void (*set_handler)(bool was_broadcast, uint16_t sub_device);
	const uint8_t footprint;
    const char* description;
    const char descriptionsize;
//	const bool include_in_supported_params;
};
static const struct _personality_definition PERSONALITY_DEFINITIONS[]  = {
    {1,1,&PERSONALITY_1[0],sizeof(PERSONALITY_1)},
    {2,2,&PERSONALITY_2[0],sizeof(PERSONALITY_2)},
    {3,3,&PERSONALITY_3[0],sizeof(PERSONALITY_3)},
    {4,4,&PERSONALITY_4[0],sizeof(PERSONALITY_4)},
    {5,2,&PERSONALITY_5[0],sizeof(PERSONALITY_5)},
    {6,4,&PERSONALITY_6[0],sizeof(PERSONALITY_6)},
};
char PERSONALITY ;
#define PERSONALITY_SIZE      sizeof(PERSONALITY_DEFINITIONS)