#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "upgrade.h"


const char* boot_ver = "360-boot-VER-1.0.0";

void* com_thread(void *arg)
{

	tty1_com_thread((int*)arg);
	return  NULL;
}

int main(int argc,char *argv[])
{
	int err;
	pthread_t tid1;
	printf("%s\n",boot_ver);
	err = pthread_create(&tid1,NULL,com_thread,&argc);
	if(err != 0)
    {
		printf("can not create thread 1:%s\n",strerror(err));
	}
	err = pthread_join(tid1,NULL);
	if(err != 0)
    {
		printf("can not join thread 1:%s\n",strerror(err));
	}
	printf("exit 360 app boot!\n");
    return 0;
}
