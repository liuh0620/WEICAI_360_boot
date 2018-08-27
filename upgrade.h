#ifndef UPGRADE_H
#define UPGRADE_H

#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include "types.h"

#define MAX_REC_SIZE 2060
#define MAX_SEND_SIZE 1024
#define MSG_HEAD             0xF0
#define MSG_DATA_OFFSET      7
#define MSG_LEN_OFFSET       6
#define MSG_CMD_OFFSET       1
#define MSG_HID_OFFSET       2
#define MSG_HEAD_SIZE        7
#define MSG_FRAME_OTHER_SIZE        8


#define COMMUNICATION_FRAMES  0x01  
#define DATA_FRAMES           0x02 

#define START_UPDATE_CMD      0x01
#define SEND_PACKAGE_NUM_CMD      0x02
#define SEND_PACKAGE_DONE_CMD         0x03


#define  ARM_READY_CMD            0x01
#define SEND_PACKAGE_NUM_ACK_CMD      0x02
#define SEND_PACKAGE_DONE_ACK_CMD         0x03
#define ARM_TIMEOUT_CMD                 0x04          

#define	CM_UNVTRANS_TO_ARM				0x16
#define	CM_UNVTRANS_TO_MCU				0x8F
#define	CM_GENERAL_ACK		    0x00
#define	CM_ARM_READY			    0x81
#define	CM_ARM_READY_ACK		    0x01
#define	CM_ACC_OFF				    0x02	//¹ØACC
#define	CM_SEND_KEY			    0x03
#define	CM_SEND_VEHICLE_INFO      0x04
#define	CM_SET_MODE			     0x05
#define	CM_SET_WAKE			     0x08
#define	CM_SET_CVBS_DISP		     0x84
#define   CM_ARM_TO_MCU_UPGRADE                0x85
#define	CM_MCU_TO_ARM_UPGRADE	    		0x09

#define	CM_MCU_APP_VERSION				0x86
#define	CM_MCU_APP_VERSION_ACK			0x0A

#define	CM_MCU_BOOT_VERSION				0x87	
#define	CM_MCU_BOOT_VERSION_ACK		0x0B	


#define	CM_RESET_SYSTEM				       0x88


#define	CM_ARM_APP_VERSION				0x0C
#define	CM_ARM_APP_VERSION_ACK			0x89	

#define	CM_ARM_ARI_VERSION				0x0D	
#define	CM_ARM_ARI_VERSION_ACK			0x8A	
#define	CM_HEARTBEAT				    0x8B
#define	CM_SET_DISP_TYPE				       0x0E
#define	CM_SET_DISP_TYPE_ACK				0x8C

#define CM_VIDEO_CALIBRATION        0x0F

#define CM_GET_AVM_INFO             0x10
#define	CM_GET_AVM_INFO_ACK				0x8E

#define CM_UPDATE_ARM_READY                0x30
#define CM_UPDATE_ARM_READY_ACK            0x31
#define CM_UPDATE_ARM_START                0x32
#define CM_UPDATE_ARM_START_ACK            0x33
#define CM_UPDATE_ARM_DATATRANS            0x34
#define CM_UPDATE_ARM_DATATRANS_ACK        0x35
#define CM_UPDATE_ARM_DATATRANS_END        0x36
#define CM_UPDATE_ARM_DATATRANS_END_ACK    0x37
#define CM_UPDATE_ARM_FINISH               0x38
#define CM_UPDATE_ARM_ABORT                0x39





#define SUCCEED_ACK          0x01
#define FAILED_ACK           0x00
#define VALID_FLAG_PATH              "/home/chinagps/app_valid_flag.ini"
#define UPDATE_APP_PATH              "/home/chinagps/update/t6a_app"
#define UPDATE_KERNEL_PATH           "/home/chinagps/update/zImage"
#define UPDATE_DTB_PATH              "/home/chinagps/update/imx6q-sabresd.dtb"
#define UPDATE_SUANFA_PATH           "/home/chinagps/update/suanfa.tar.bz2"
#define UPDATE_UBOOT_PATH            "/home/chinagps/update/u-boot-imx6qsabresd_sd.imx"



#define UPDATE_APP_PATCH             "/home/chinagps/update/t6a_app.patch"
#define UPDATE_KERNEL_PATCH          "/home/chinagps/update/zImage.patch"
#define UPDATE_DTB_PATCH             "/home/chinagps/update/imx6q-sabresd.dtb.patch"
#define UPDATE_SUANFA_PATCH          "/home/chinagps/update/suanfa.tar.bz2.patch" 
#define UPDATE_UBOOT_PATCH           "/home/chinagps/update/u-boot-imx6qsabresd_sd.imx.patch"


int SendPkgStruct(uchar cmd,uint Hid,char IsAck, uchar* msg,uchar msgLen);
#endif
