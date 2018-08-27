#ifndef __types_H__
#define __types_H__

#define LIUH_DBG
#ifdef LIUH_DBG
    #define DBG(fmt, args...)  printf("Dbg: " fmt, ## args)
#else
    #define DBG(fmt, args...) /* empty debug slot */
#endif

typedef unsigned int  uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
#ifndef NULL
#define NULL ((void*)0)
#endif
typedef enum{bFALSE,bTRUE}TBOOL;		
typedef enum cCodeType{
	TImx6App=0,
    TArithmetic,
    Tkernel,
    TDtb,
    TUboot,
}CodeType;

typedef  enum {
    T_START,
    T_CMD,
    T_HID,
    T_LEN,
    T_DATA,
    T_LRC,
}RecType;


typedef enum cInterval{
	T100=0,
    T1000,
    T6000,
    T10000,
}AckInterval;

typedef struct cWriteFileInfo{
   uchar*  filename;
   int fd;
   uint pos;
}WriteFileInfo;

typedef struct cOneFrameData{
   uchar data[2048];
}OneFrameData;

#define QUEUE_SIZE    6000

typedef struct CQueue{
    uint mQueueCtr;
    OneFrameData *mQueueInPtr;
    OneFrameData *mQueueOutPtr;
    OneFrameData mQueueBuf[QUEUE_SIZE];
}Queue;

typedef struct cUpdateInfo{
   uchar init;      
   uchar diff_update;       //1 差分升级    0 整包升级
   CodeType code_type;      
   uint packageNum;       //包序号
   uint  filesize;          //文件总长度
   uint packageCount;         //包总个数
   uint packageSize;        //包大小
   uint crc32;
   uint alreadyRecPkg;
   WriteFileInfo writeFileInfo;
   Queue queue;
}UpdateInfo;

#endif
