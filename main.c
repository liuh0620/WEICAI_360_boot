#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "upgrade.h"


const char* boot_ver = "360-boot-VER-1.0.0";

void* com_thread(void* arg)
{

	tty1_com_thread();
	return  NULL;
}

int main(void)
{
	int32_t err;
	pthread_t tid1;
	printf("%s\n",boot_ver);
    err = pthread_create(&tid1,NULL,com_thread,NULL);
    if(err == 0)
    {
        err = pthread_join(tid1,NULL);
        if(err == 0)
        {
            printf("pthread_join is succeed!\n");
        }
        else
        {
            printf("can not join thread 1:%s\n",strerror(err));
        }
    }
    else
    {
        printf("can not create thread 1:%s\n",strerror(err));
        return -1;
    }
    
	printf("exit 360 app boot!\n");
    return 0;
}
