
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
//Response Type
enum{
    ACK=0x00,
    ACK_TIMER=0x01,
    NACK_REASON=0x02,
    ACK_OVERFLOW=0x03,
};

//Command Class
enum{
    DISCOVERY_COMMAND= 0x10,
    DISCOVERY_COMMAND_RESPONSE=0x11,
    GET_COMMAND=0x20,
    GET_COMMAND_RESPONSE=0x21,
    SET_COMMAND=0x30,
    SET_COMMAND_RESPONSE=0x31,


};

//Parameter ID (PID)
enum{
    DISC_UNIQUE_BRANCH = 0x0001,
    DISC_MUTE = 0x0002,
    DISC_UN_MUTE = 0x0003,
    SUPPORTED_PARAMETERS = 0x0050,
    PARAMETER_DESCRIPTION = 0x0051,
    DEVICE_INFO = 0x0060,
    SOFTWARE_VERSION_LABEL = 0x00C0,
    DMX_START_ADDRESS = 0x00F0,
    IDENTIFY_DEVICE = 0x1000
};


//RDM set start
/** RxStart The Start Code for a valid data stream */
const char RDM_StartCode = 0xCC;
//RDM set start
/** RxStart The Start Code for a valid data stream */
const char RDM_SubStartCode = 0x01;

/**
*  Initialises the DMX512A Library
*  Please add near the beginning of you main() function before your main loop
*/
extern void RDM_init(void);
/**
* RDM Transmit interrupt. Called as part of dmx_interrupt() but can call seperately for optimization
*/
extern void RDM_tx_interrupt(void);

extern void RDM_rx_loop(void);
/**
 *  Select the size of the transmit buffer
 *  This is the Maximum number of channels that will be sent.
 *  DMX512a requires a minimum of 20 and a maximum of 512
 *  This depends on how much RAM your PIC has available.
 */
#define TX_BUFFER_SIZE 512

#define TX_PIN      LATC4   // TX pin use RC4
#define TX_TRIS     TRISC4  // Set TX pin as output

/** TxData Transmit Data Buffer  */
//volatile uint8_t   TxData[TX_BUFFER_SIZE];
/** TxCount Counts how many bytes sent. Used in interrupt  */
uint16_t         TxCount=0;
/** TxStart Start code to transmit  */
uint8_t          TxStart=0;
/** *TxByte Pointer to data to send  */
volatile uint8_t *TxByte;
/** TxState State for the Transmit state machine  */
volatile uint8_t TxState=0;

 enum
 {
    TX_BREAK,
    TX_MAB,
    TX_START,
    TX_DATA
 };


typedef union{
    unsigned char value[24]; 
    struct{
        unsigned ML:8;      //MessageLength
        struct{
            uint16_t M;     //Manufacture
            uint32_t ID;
        }UID;   // this.device UID
        struct{
            uint16_t M;     //Manufacture
            uint32_t ID;    
        }CUID;          //Controller UID
        unsigned TN:8;      //Transaction Number
        unsigned PORT:8;     //Port ID / Response Type
        unsigned message:8;     //message count
        uint16_t subDevice;      //sub device
        unsigned CC:8;          //Command Class
        uint16_t PID;        //Parameter ID
        unsigned PDL:8;         //Parameter Data Length
        union{
            uint16_t CS;
            struct{
                unsigned CSH:8;         //ChechSum High
                unsigned CSL:8;         //ChechSum Low
            };
        };
        
    };
}RDM_Data;

volatile RDM_Data RX_RDM_Data;
volatile RDM_Data TX_RDM_Data;

//RDM Parameter Data
char PD[231];

char PDCount;

volatile char PackCount;
volatile unsigned PD_Flag;

//typedef struct {
//    int PacketType; //
//    union{
//        char value[6];
//        unsigned char Length;
//        struct{
//            uint16_t M;     //Manufacture
//            uint32_t ID;
//        }UID;       // this.device UID
//        struct{
//            uint16_t M;     //Manufacture
//            uint32_t ID;    
//        }CUID;      //Controller UID
//        unsigned int TN;//Transaction Number
//        unsigned int PORT;//Port ID / Response Type
//        unsigned int MSG;//message count
//        uint16_t subDevice;
//        unsigned int CC;//Command Class
//        uint16_t PID;//Parameter ID
//        unsigned int PDL;//Parameter Data Length
//        union{
//            struct{
//            unsigned int CSH;//ChechSum High
//            unsigned int CSL;//ChechSum Low
//            };
//            uint16_t CS;
//        };
//    };
//}RDM_pack;
//
///** AddrCount RDM counter used in the interrupt routine */
//
//typedef struct{
//    unsigned char Type; //
//    unsigned char Count;
//    unsigned char PDL;//boolean 0 or 1
//}RDM_PackNum;
//
//RDM_PackNum RDM_PackFlag;
//
//const unsigned char PackNum[12]={1,6,6,1,1,1,2,1,2,1,1,1};
//
//RDM_pack *PackPtr;
//RDM_pack RDMpack[12];

/** PARAMETER_ID is a part of Packet */
//typedef struct{
//    unsigned char PARAMETER_ID;
//    unsigned char PARAMETER;//Parameter
//    char* NextID;
//}RDM_Parameter;


//enum{
//    ML,
//    UID,
//    CUID,
//    TN,
//    PORT,
//    MSG,
//    subDevice,
//    CC,
//    PID,
//    PDL,
//    CSH,
//    CSL
//};

uint16_t checkSum;

//RDM end start
