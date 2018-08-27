
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <stdint.h>
#include "queue.h"
#include "upgrade.h" 


//全局变量
int stopped = 0;
static int comfd = -1;
static UpdateInfo updateInfo;
static pthread_mutex_t f_lock = PTHREAD_MUTEX_INITIALIZER;

#define   MAKE_DWORD(a,b,c,d)  ((a<<24)|(b<<16)|(c<<8)|(d))
#define   MAKE_WORD(a,b)       ((a<<8)|(b))

#define REC_DATA_ONE_BY_ONE		 1

void InitialUpdateInfo(void){
    updateInfo.init = 0;
    updateInfo.diff_update = 0;
    updateInfo.code_type = 0;
    updateInfo.packageNum = 0;
    updateInfo.filesize = 0;
    updateInfo.packageCount = 0;
    updateInfo.crc32 = 0;
	updateInfo.alreadyRecPkg = 1;      
    if(updateInfo.writeFileInfo.fd > 0){
		close(updateInfo.writeFileInfo.fd);
	}
	updateInfo.writeFileInfo.fd = 0;
	updateInfo.writeFileInfo.filename = 0;
	CQueueInit(&updateInfo.queue);
	return;
}

uint fwriten(int fd, void *vptr, uint n)
{
    int nleft;
    int nwrite;
    char  *ptr;
    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwrite = write(fd, ptr, nleft)) <= 0) {
            perror("fwrite: ");
            return (-1);			/* error */
        }

        nleft -= nwrite;
        ptr   += nwrite;
    }
    return (n);
} /* end fwriten */

void WriteDataFull(void)
{	
	OneFrameData outElemPtr;
	while(IsEmpty(&updateInfo.queue) == bFALSE){
		DeQueue(&updateInfo.queue,&outElemPtr);
		fwriten(updateInfo.writeFileInfo.fd, &outElemPtr, updateInfo.packageSize);
	}
	 /* fdatasync(updateInfo.writeFileInfo.fd); */
	return;	
}

void WriteDataByInterval(void)
{

	if(updateInfo.alreadyRecPkg%updateInfo.packageSize == 0){
		WriteDataFull();
	}
	
	return;
}

static const uint32_t crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL 
};
uint32_t calc_crc(int fd, uint32_t count)
{
	uint32_t crc;
	uchar ptr;
    crc = 0xFFFFFFFF;
    while(count--){
        read(fd, &ptr, 1);
        crc = crc32tab[(crc ^ ptr) & 0xff] ^ (crc >> 8);
     }
    DBG("liuh>>>[%s:%d] the checksum is %x ............\n",__func__,__LINE__, crc^0xFFFFFFFF);
     return crc^0xFFFFFFFF;
}

int WriteAppFlag(uchar u){//u=0 有效
	int Parafd = -1,ret= -1;
	int app_flag = MAKE_DWORD('F','A','I','L');
	Parafd = open(VALID_FLAG_PATH,O_RDWR);
    if(Parafd < 0)
    {
        printf("open app flag err!\n");
        ret = -1;
    }
	else{
	 	lseek(Parafd,0,SEEK_SET);
		if(u == 0){
			app_flag = MAKE_DWORD('V','A','L','I');
		}
      	write(Parafd,&app_flag,4);
		close(Parafd);
		ret = 0;
	}
	return ret;
}

int ProcessSendDone(uint Hid, uchar* pMsgData)
{	
	int ret = 2;
	int fd;
	char str[50] ={0};
    uchar SendMsgBuf[20] = {0};
    updateInfo.crc32 = MAKE_DWORD(pMsgData[0], pMsgData[1], pMsgData[2], pMsgData[3]);
	close(updateInfo.writeFileInfo.fd);
	fd = open(updateInfo.writeFileInfo.filename,O_RDONLY);
	if(fd < 0){
        printf("open %s fail!\n", updateInfo.writeFileInfo.fd);
		return ret;
	}
    DBG("liuh>>>[%s:%d] updateInfo.crc32 = %x............\n",__func__,__LINE__,updateInfo.crc32);
	if(calc_crc(fd,updateInfo.filesize) == updateInfo.crc32){
        DBG("liuh>>>[%s:%d] the checksum is OK............\n",__func__,__LINE__);
        SendMsgBuf[0] = 0x01;
        SendPkgStruct( CM_UPDATE_ARM_DATATRANS_END_ACK,Hid,1,SendMsgBuf,1  );
        DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_END_ACK>\n",__func__,__LINE__);
		ret = 1;
		sprintf(str,"/home/chinagps/t6a_update_by_can.sh %d %d",updateInfo.code_type,updateInfo.diff_update);
		system(str); //判断成功?
		if(updateInfo.code_type  == TImx6App){
			WriteAppFlag(0);
		}
		stopped = 1;
	}
	else{
        DBG("liuh>>>[%s:%d] the checksum is ERR............\n",__func__,__LINE__);
		ret = 2;
		if(updateInfo.code_type  == TImx6App){
			WriteAppFlag(1);
		}
        SendMsgBuf[0] = 0x02;
        SendPkgStruct( CM_UPDATE_ARM_DATATRANS_END_ACK,Hid,1,SendMsgBuf,1  );
	}
	close(fd);
	return ret;
}



/*---------------------------------------------------------------
* 函数原型：int setCom(const char *Dev)
* 函数功能：设置串口通信速率
* 参数说明：
  
* 输入参数：设备名称 
* 返 回 值：fd
* 作者：    zd
*---------------------------------------------------------------*/

int setCom(const char *Dev)
{
        int fd;
        struct termios termios_new;
        fd = open(Dev, O_RDWR|O_NDELAY|O_NOCTTY);
        if (-1 == fd)
        {
            printf("open com dev filed!\n");
            return -1;
        }
        bzero(&termios_new, sizeof(termios_new));
        cfmakeraw(&termios_new);
        termios_new.c_cflag = (B576000);
        //termios_new.c_cflag = (B115200);
        termios_new.c_cflag |= CLOCAL | CREAD;
        termios_new.c_cflag &= ~CSIZE;
        termios_new.c_cflag |= CS8;
        termios_new.c_cflag &= ~PARENB;
        termios_new.c_cflag &= ~CSTOPB;
#if REC_DATA_ONE_BY_ONE		 
        termios_new.c_cc[VTIME] = 0;
        termios_new.c_cc[VMIN] = 0;
#else
		termios_new.c_cc[VTIME] = 1;
        termios_new.c_cc[VMIN] = 0;
#endif
        tcflush(fd, TCIOFLUSH);
        tcsetattr(fd, TCSANOW, &termios_new);
        return fd;
}



/*---------------------------------------------------------------
* 函数原型：int WriteCom(char* buf,int n)
* 函数功能：串口写数据
* 参数说明：
  
* 输入参数：
* 返 回 值：无
* 作者：    zd
*---------------------------------------------------------------*/
int WriteCom(char* buf,int n)
{
    int nleft;
    int nwrite;
    char  *ptr;
    ptr = buf;
    nleft = n;
    pthread_mutex_lock(&f_lock);
    while (nleft > 0) {
        if ( (nwrite = write(comfd, ptr, nleft)) <= 0) {
	     pthread_mutex_unlock(&f_lock);
            perror("write: ");
            return (-1);			/* error */
        }

        nleft -= nwrite;
        ptr   += nwrite;
    }
    pthread_mutex_unlock(&f_lock);
    return (n);
}

/*---------------------------------------------------------------
* 函数原型：int ReadCom(char* buf)
* 函数功能：串口读数据
* 参数说明：
  
* 输入参数：
* 返 回 值：读的大小
* 作者：    zd
*---------------------------------------------------------------*/
int ReadCom(char* buf)
{
    int size = 0;
	pthread_mutex_lock(&f_lock);
    while(read(comfd,buf,1) > 0)
    {
        printf("%x ", *buf);
        buf++;
        size++;
        if(size == MAX_REC_SIZE)
        {
            break;
        }
    }
	pthread_mutex_unlock(&f_lock);
    return size;
}

int ReadOneChar(char* c){
	return read(comfd,c,1);
}



int ReadOneFrame(char* buf)
{
    static RecType f_level = T_START;
    static uint  index = 0,hid_size = 0,len_size = 0,ex_len_flag = 0;  
    static int len = 0, len_bak = 0;
    char data = 0,size = 0;
    int ret = -1;
    pthread_mutex_lock(&f_lock);
    size = ReadOneChar(&data);
    pthread_mutex_unlock(&f_lock);

    if(size < 0)
    {
        return 0;
    }

    switch(f_level)
    {
        case T_START:
            if((uchar)data == MSG_HEAD)
            {
                buf[index++] = data;
                f_level = T_CMD;
            }
            break;
        case T_CMD:
            buf[index++] = data;
            if(data == CM_UPDATE_ARM_DATATRANS)
            {
                len_size = 2;
                ex_len_flag = 1;
            }
            else
            {
                len_size = 1;
                ex_len_flag = 0;
            }
            hid_size = 4;
            f_level = T_HID;
            break;
        case T_HID:
            buf[index++] = data;
            hid_size--;
            if(hid_size == 0)
            {
                f_level = T_LEN;
            }
            break;
        case T_LEN:
            if(data < (MAX_REC_SIZE - MSG_FRAME_OTHER_SIZE))
            {
                buf[index++] = data;
                len_size--;
                if(len_size == 0)
                {
                    f_level = T_DATA;
                    if(ex_len_flag)
                    {
                        len =  data << 8 | buf[index-2];  //接收data时需要减一 因为已经接收到一个了
                        len_bak = len;
                    }
                    else
                    {
                        len = data;
                        len_bak = len;
                    }
                }
            }
            else
            {
                index = 0;
                f_level = T_START;
            }
            break;
        case T_DATA:
            buf[index++] = data;
            len--;
            if(ex_len_flag)
            {
                if(len == 1)
                { //少接收一个数据
                    f_level = T_LRC;
                }
            }
            else
            {
                if(len == 0)
                {
                    f_level = T_LRC;
                }
            }
            break;
        case T_LRC:
            buf[index++] = data;
            index = 0;
            f_level = T_START;  
            ret = len_bak + MSG_FRAME_OTHER_SIZE;
            break;
        default: 
            index = 0;
            f_level = T_START;
            break;
    }
    return ret;
}


/*---------------------------------------------------------------
* 函数原型：void flush_buf(int type)
* 函数功能：冲洗数据
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
void flush_buf(int type)
{
	tcflush(comfd, type);	
}

#define PRINTFBUF(size,buf)   do{ \
    printf("[REC_BUF]"); \
    int s=size; \
    char* b = buf; \
    while(s--){ \
        printf("%x ",*b++); \
    } \
    printf("\n");\
    }while(0)



/*---------------------------------------------------------------
* 函数原型：uchar* arrchr(uchar* pBuf,uchar ch,int size)
* 函数功能：查找特定字符
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
uchar* arrchr(uchar* pBuf,uchar ch,int size)
{

    if(pBuf == NULL)
    {
        return NULL;
    }
    else if(*pBuf == ch)
    {
        return pBuf;
    }
    size--;
    while(size--)
    {
        if(*(++pBuf) == ch)
        {

            return pBuf;
        }
    }
    return NULL;
}

/*---------------------------------------------------------------
* 函数原型：void ParseGeneralAckCmd(uchar* msg)
* 函数功能：解析回应命令是否成功
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
void ParseGeneralAckCmd(uchar* msg)
{
    uchar cmdId = msg[0];
    uchar result = msg[1];
    if(result == FAILED_ACK)
    {
        printf("failed ID:%d\n",cmdId);
    }
    return;

}





//return NULL if not found  
/*---------------------------------------------------------------
* 函数原型：char* memstr(char* full_data, int full_data_len, const char* substr) 
* 函数功能：查找字符串
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
char* memstr(char* full_data, int full_data_len, const char* substr)  
{  
    if (full_data == NULL || full_data_len <= 0 || substr == NULL) {  
        return NULL;  
    }  
  
    if (*substr == '\0') {  
        return NULL;  
    }  
  
    int sublen = strlen(substr);  
  
    int i;  
    char* cur = full_data;  
    int last_possible = full_data_len - sublen + 1;  
    for (i = 0; i < last_possible; i++) {  
        if (*cur == *substr) {  
            //assert(full_data_len - i >= sublen);  
            if (memcmp(cur, substr, sublen) == 0) {  
                //found  
                return cur;  
            }  
        }  
        cur++;  
    }  
  
    return NULL;  
}  




/*---------------------------------------------------------------
* 函数原型：int SendPkgStruct(uchar cmd,uint Hid,char IsAck, uchar* msg,uchar msgLen)
* 函数功能：构建发送包
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
int SendPkgStruct(uchar cmd,uint Hid,char IsAck, uchar* msg,uchar msgLen)
{
    static unsigned int SendCount = 1;
    int i,ret;
    uchar crc;
    uchar SendBuf[MAX_SEND_SIZE];
    SendBuf[0] = MSG_HEAD;
    SendBuf[MSG_CMD_OFFSET] = cmd;
    if(IsAck)
    {
        SendBuf[MSG_HID_OFFSET] = (Hid>>24)&0xFF;
        SendBuf[MSG_HID_OFFSET+1] = (Hid>>16)&0xFF;
        SendBuf[MSG_HID_OFFSET+2] = (Hid>>8)&0xFF;
        SendBuf[MSG_HID_OFFSET+3] = Hid&0xFF;
    }
    else
    {
        SendBuf[MSG_HID_OFFSET] = (SendCount>>24)&0xFF;
        SendBuf[MSG_HID_OFFSET+1] = (SendCount>>16)&0xFF;
        SendBuf[MSG_HID_OFFSET+2] = (SendCount>>8)&0xFF;
        SendBuf[MSG_HID_OFFSET+3] = SendCount&0xFF;
    }
    SendBuf[MSG_LEN_OFFSET] = msgLen;
    if(msgLen > 0)
    {
        memcpy(&SendBuf[MSG_DATA_OFFSET],msg,msgLen);
    }
    crc = 0;
    for(i = 1;i<(msgLen + MSG_HEAD_SIZE);i++)
    {
        crc ^= SendBuf[i];
    }
    SendBuf[msgLen+MSG_HEAD_SIZE] = crc;
    ret = WriteCom((char*)SendBuf,MSG_HEAD_SIZE+msgLen+1);
    if(ret < 0)
    {
        return -1;
    }
    if(!IsAck)
    {
        SendCount++;
    }
    return 0;
}


/*---------------------------------------------------------------
* 函数原型：void SendGeneralAck(uchar cmd,uint Hid,uchar isSucc)
* 函数功能：发送通用应答
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
void SendGeneralAck(uchar cmd,uint Hid,uchar isSucc)
{
    uchar SendMsgBuf[MAX_SEND_SIZE];
    SendMsgBuf[0] = cmd;
    SendMsgBuf[1] = isSucc;
    SendPkgStruct(CM_GENERAL_ACK,Hid,1,SendMsgBuf,2);
    return;
}



/*--------------------------------------------------------------------------
 *  Function:
 *  MainWindow.ParseProtocol
 *
 *  Parameters:
 *
 *
 *  Returns value:
 *
 *
 *  Description:
 * 		解析串口协议
 *
 *--------------------------------------------------------------------------*/
void ParseProtocol(uchar* msgData,uchar cmd)
{
    uint Hid = (msgData[0]<<24)|(msgData[1]<<16)|(msgData[2]<<8)| msgData[3];
    uchar* pMsgData = &msgData[5];
	uchar SendMsgBuf[20] = {0};
	int ret = 0;
    
    switch(cmd)
    {
        case CM_GENERAL_ACK:
            ParseGeneralAckCmd(pMsgData);
            break;

        case CM_ACC_OFF:
            break;     

        case CM_SET_WAKE:
		    SendGeneralAck(cmd,Hid,SUCCEED_ACK);
	        break;

        case CM_UPDATE_ARM_START:
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_START>\n",__func__,__LINE__);
            UpdateARM_Start(Hid, pMsgData);
	        break;

        case CM_UPDATE_ARM_DATATRANS:
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS>\n",__func__,__LINE__);
            UpdateARM_DataTrans(Hid, pMsgData);
            break;

        case CM_UPDATE_ARM_DATATRANS_END:
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_END>\n",__func__,__LINE__);
            UpdateARM_DataTrans_End(Hid, pMsgData);
            break;

        case CM_UPDATE_ARM_ABORT:
            UpdateARM_Abort(Hid, pMsgData);
            break;
        default: 
		    printf("parse cmd default!\n");
	        break;
    }
    return;
}

/*---------------------------------------------------------------
* 函数原型：void ProcessComHandle(char* buf ,int size)
* 函数功能：解析串口协议
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/

void ProcessComHandle(char* buf ,int size)
{
    uchar cmd;
    uchar* pbuf;
    uchar* Srcbuf;
    uchar crc;
    int CalCnt, i,DataLen;

    //PRINTFBUF(size,buf);

    if(size <= MSG_HEAD_SIZE)
    {
        return;
    }

    Srcbuf =(uchar*)buf;

    while((pbuf= arrchr(Srcbuf,MSG_HEAD,(size- (Srcbuf - (uchar*)buf)))) != NULL) //粘包处理
    {

        cmd = pbuf[MSG_CMD_OFFSET];
        if(cmd == CM_UPDATE_ARM_DATATRANS)
        {
            DataLen = (pbuf[MSG_LEN_OFFSET+1] << 8) | pbuf[MSG_LEN_OFFSET];
        }
        else
        {
            DataLen = pbuf[MSG_LEN_OFFSET];                    
        }
        if((pbuf - (uchar*)buf + DataLen + MSG_HEAD_SIZE +1 ) > size )
        {
            break;
        }

        crc = 0;
        CalCnt = DataLen + MSG_HEAD_SIZE - 1;
        i = 1;
        while(CalCnt--)
        {
            crc ^= pbuf[i++];
        }

        if(pbuf[DataLen + MSG_HEAD_SIZE] != crc)
        {
            printf("check ecc err!\n");
            break;
        }
        ParseProtocol(&pbuf[MSG_HID_OFFSET], pbuf[MSG_CMD_OFFSET]);
        Srcbuf = pbuf + DataLen + MSG_HEAD_SIZE +1;
        if(Srcbuf > ((uchar*)&buf[size - 1] - MSG_HEAD_SIZE))
        {
            break;
        }
    }
    return;
}


/*---------------------------------------------------------------
* 函数原型：Update_ARM_Ready(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int Filed_Filename()
{

    int ret = 0;
    
    if(updateInfo.code_type == TImx6App)
    {
        updateInfo.writeFileInfo.filename = UPDATE_APP_PATH;
		if(updateInfo.diff_update == 1)
        {
		    updateInfo.writeFileInfo.filename = UPDATE_APP_PATCH;
		}
	}
	else if(updateInfo.code_type == TArithmetic)
    {
		updateInfo.writeFileInfo.filename = UPDATE_SUANFA_PATH;
		if(updateInfo.diff_update == 1)
        {
			updateInfo.writeFileInfo.filename = UPDATE_SUANFA_PATCH;
		}
	}
	else if(updateInfo.code_type == Tkernel)
    {
		updateInfo.writeFileInfo.filename = UPDATE_KERNEL_PATH;
		if(updateInfo.diff_update == 1)
        {          
			updateInfo.writeFileInfo.filename = UPDATE_KERNEL_PATCH;
		}
	}
	else if(updateInfo.code_type == TDtb)
    {
		updateInfo.writeFileInfo.filename = UPDATE_DTB_PATH;
		if(updateInfo.diff_update == 1)
        {
			updateInfo.writeFileInfo.filename = UPDATE_DTB_PATCH;
		}
	}
   	else if(updateInfo.code_type == TUboot)
    {
   		updateInfo.writeFileInfo.filename = UPDATE_UBOOT_PATH;
   		if(updateInfo.diff_update == 1)
        {
   		    updateInfo.writeFileInfo.filename = UPDATE_UBOOT_PATCH;
   		}
   	}
   	else
   	{
   		updateInfo.init = 0;
        printf("filled file Error!\n");
        ret = -1;
   	}
    return ret;
}

/*---------------------------------------------------------------
* 函数原型：Update_ARM_Ready(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int UpdateARM_Start(uint Hid, uchar* msg)
{
    int32_t ret = 0;
    uchar SendMsgBuf[20] = {0};
    uchar* pMsgData = msg;

    if( 1 == updateInfo.init )
    {
        return ret;
    }
    InitialUpdateInfo();
	updateInfo.diff_update = pMsgData[0] & 0x01;    //1 差分升级    2 整包升级
    DBG("[%s:%d]updateInfo.diff_update: %d\n",__func__,__LINE__,updateInfo.diff_update);
    updateInfo.code_type = (pMsgData[0] >> 1) & 0x07;
    DBG("[%s:%d]code_type: %d\n",__func__,__LINE__,updateInfo.code_type);
	updateInfo.filesize = MAKE_DWORD(pMsgData[1],pMsgData[2],pMsgData[3],pMsgData[4]);
    if( 0x01 == pMsgData[5] )
    {
        updateInfo.packageSize = 1024;
    }
    else if( 0x02 == pMsgData[5] )
    {
        updateInfo.packageSize = 2048;
    }
    updateInfo.packageCount = updateInfo.filesize / updateInfo.packageSize + (updateInfo.filesize % updateInfo.packageSize == 0? 0 : 1);   
    DBG("[%s:%d]filesize = %d, packageCount= %d\n",__func__,__LINE__,updateInfo.filesize, updateInfo.packageCount);
	updateInfo.init = 1;
    SendMsgBuf[0] = 0x01;//1:准备好 2:未准备好 
    ret = Filed_Filename();	
    if( -1 == ret )
    {
        SendMsgBuf[0] = 0x02;
    }
   	SendPkgStruct(CM_UPDATE_ARM_START_ACK,Hid,1,SendMsgBuf,1);
    DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_START_ACK>\n",__func__,__LINE__);
   	system("rm /home/chinagps/update/*");
   	updateInfo.writeFileInfo.fd = open(updateInfo.writeFileInfo.filename, O_CREAT | O_RDWR | O_TRUNC,
       S_IRWXU | S_IRWXG | S_IRWXO | O_NONBLOCK);
   	if(updateInfo.writeFileInfo.fd < 0)
    {
   		/* SendMsgBuf[0] = 0x02; */
   		updateInfo.init = 0;
        printf("open %s fail!\n",updateInfo.writeFileInfo.filename);
   	}

    return ret;
}

/*---------------------------------------------------------------
* 函数原型：UpdateARM_DataTrans(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int UpdateARM_DataTrans(uint Hid, uchar* pMsgData)
{
    uchar SendMsgBuf[20] = {0};
    int ret = 0;

    if( 0 == updateInfo.init )    
    {
        return ;
    }

    updateInfo.packageNum = MAKE_DWORD(0, pMsgData[1], pMsgData[2], pMsgData[3]);    
    DBG("%s[%d]packageCount = %d, packageNum = %x\n",__func__,__LINE__,updateInfo.packageCount,updateInfo.packageNum);
    if(updateInfo.packageNum == updateInfo.alreadyRecPkg)
    {
        if(updateInfo.alreadyRecPkg == updateInfo.packageCount)
        {
            SendMsgBuf[0] = 0x02;
            SendPkgStruct(CM_UPDATE_ARM_DATATRANS_ACK,Hid,1,SendMsgBuf,1); 
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_ACK>\n",__func__,__LINE__);
            ret = fwriten(updateInfo.writeFileInfo.fd, &pMsgData[4], (updateInfo.filesize % updateInfo.packageSize == 0)? updateInfo.packageSize:(updateInfo.filesize % updateInfo.packageSize)); 
            fdatasync(updateInfo.writeFileInfo.fd);
            printf("rec last pkg!!!!!!!!!\n");
        }
        else if(updateInfo.alreadyRecPkg < updateInfo.packageCount)
        {
            SendMsgBuf[0] = 0x02;
            SendPkgStruct(CM_UPDATE_ARM_DATATRANS_ACK,Hid,1,SendMsgBuf,1); 
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_ACK>\n",__func__,__LINE__);
            if(IsFull(&updateInfo.queue) == bFALSE){
            EnQueue(&updateInfo.queue,(const OneFrameData *)&pMsgData[4]);
            WriteDataFull();
            }else{
                printf("queue is full!!!\n");
                return -1;
            }
            updateInfo.alreadyRecPkg++;
        }
        else
        {
            printf("Already recive pakage is greater than package count!\n");
            SendMsgBuf[0] = 0x01;
            SendPkgStruct(CM_UPDATE_ARM_DATATRANS_ACK,Hid,1,SendMsgBuf,1); 
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_ACK>\n",__func__,__LINE__);
        }
    }
    else
    {
        printf("The package number is error!\n");
        SendMsgBuf[0] = 0x01;
    }
    return ret;
}


/*---------------------------------------------------------------
* 函数原型：vUpdateARM_DataTrans_End(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int UpdateARM_DataTrans_End(uint Hid, uchar* pMsgData)
{
    int fd = -1;
    uchar SendMsgBuf[20] = {0};
    if( updateInfo.alreadyRecPkg == updateInfo.packageCount )
    {
        if( 2 == ProcessSendDone(Hid, pMsgData) )
        {
            printf("ProcessSendDone is Fail!\n");
            goto Fail;
        }
    }
    else 
    {
        printf("lost package!!!\n");
        goto Fail;
    }
    sleep(2);
    SendMsgBuf[0] = 0x01;
    SendPkgStruct( CM_UPDATE_ARM_FINISH,Hid,1,SendMsgBuf,1  );
    DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_FINISH>\n",__func__,__LINE__);
    return 0;
Fail:
    SendMsgBuf[0] = 0x02;
    SendPkgStruct( CM_UPDATE_ARM_FINISH,Hid,1,SendMsgBuf,1  );
    DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_FINISH>\n",__func__,__LINE__);
    return -1;
}



/*---------------------------------------------------------------
* 函数原型：vUpdate_ARM_Ready(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int UpdateARM_Abort(uint Hid, uchar* pMsgData)
{
    printf("Update is abort......\n");
    return 0;
}

/*---------------------------------------------------------------
* 函数原型：void InitSavePara(void)
* 函数功能：tty1线程循环处理 
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
 void tty1_com_thread(int* arg)
{

	int size;
	char buf[MAX_REC_SIZE];
	int app_flag = MAKE_DWORD('F','A','I','L');//失效
	int Parafd = -1;
    uchar SendMsgBuf[MAX_SEND_SIZE];
    if(access(VALID_FLAG_PATH,F_OK) >= 0)
    {
        Parafd = open(VALID_FLAG_PATH,O_RDWR);
	    if(Parafd < 0)
	    {
	        printf("open app flag err!\n");
	        goto error1;
	    }
		lseek(Parafd,0,SEEK_SET);
        read(Parafd,&app_flag,4);
		close(Parafd);
	}
    else
    {
        Parafd = open(VALID_FLAG_PATH,O_CREAT|O_RDWR,777);
	    if(Parafd < 0)
	    {
	        printf("open app flag err!\n");
	        goto error1;
	    }
	    lseek(Parafd,0,SEEK_SET);
        write(Parafd,&app_flag,4);
		close(Parafd);
	}

	if(app_flag == MAKE_DWORD('V','A','L','I'))
	{
		system("/home/chinagps/t6a_app &");//jump to app
		return;
	}
lable_1:
	comfd = setCom("/dev/ttymxc1");
	if(comfd < 0)
    {
		printf("open com failed!\n");
	    goto error1;
	}
    SendMsgBuf[0] = 0x01;
	SendPkgStruct(CM_UPDATE_ARM_READY_ACK,0,1, SendMsgBuf,1);
    DBG("%s[%d]Send CMD: CM_UPDATE_ARM_READY_ACK is OK....\n",__func__,__LINE__);
	while(!stopped)
    {
#if REC_DATA_ONE_BY_ONE		
        size = ReadOneFrame(buf);
#else
		size = ReadCom(buf);
#endif 
		if(size > 0)
        {
		    ProcessComHandle(buf,size);
		}
		else if(size == 0)
		{
			usleep(1000);
		}
		
	}
	close(comfd);
	comfd= -1;
	system("/home/chinagps/t6a_app &");//jump to app
error1:
    SendMsgBuf[0] = 0x00;
	SendPkgStruct(CM_UPDATE_ARM_READY_ACK,0,0, SendMsgBuf,1);
	return;
}


