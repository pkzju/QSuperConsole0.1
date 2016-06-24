#ifndef __PROTOCAL_H__
#define __PROTOCAL_H__

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef __FALSE
#define __FALSE   (0)
#endif

#ifndef __TRUE
#define __TRUE    (1)
#endif

typedef char               S8;
typedef unsigned char      U8;
typedef short              S16;
typedef unsigned short     U16;
typedef int                S32;
typedef unsigned int       U32;
typedef long long          S64;
typedef unsigned long long U64;
// Protocal Version
#define PROTOCAL_HEADER                0x5A

/**************************************** CMD List **********************************************/

#define CMD_ID_SENSOR_INFO   									0x40

/**************************************** Data Struct ****************************************/

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    unsigned char     flag;
    unsigned char     length;
    unsigned char     seq_num;
    unsigned char     type:4;
    unsigned char     id:4;
    unsigned char     cmd;
    unsigned char     crc8;
}tHeader;      //common header
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    unsigned short    crc16;
}tCheckSum;    //common tail
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct Position_Data
{
    signed short p_x:16; 
    signed short p_y:16;  
    signed short p_z:16; 
}Position_Data;
#pragma pack(pop)



#ifdef  __cpluscplus
extern "C"
{
#endif

#define PROTOCAL_FRAME_MAX_SIZE  256

unsigned char FrameUnpack(unsigned char token, unsigned char* pBuffer);
unsigned char  FramePack(unsigned char* pDataIn, unsigned char len, unsigned char* pDataOut);

#ifdef  __cpluscplus
}
#endif


#endif  //__PROTOCAL_H__
