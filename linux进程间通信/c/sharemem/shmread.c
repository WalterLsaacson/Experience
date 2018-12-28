#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "shmdata.h"

int main(){
    int running = 1;
    void *shm = NULL;//共享内存的首地址
    struct shared_use_st *shared;//指向shm
    int shmid;//共享内存标识符

    //1.创建共享内存，创建之后共享内存还不能被访问
    //@创建共享内存的id
    //@共享内存的大小
    //@可以理解为权限，IPC_CREAT将在没有该共享内存时创建
    shmid = shmget((key_t)1234,sizeof(struct shared_use_st),0666|IPC_CREAT);
    if(shmid == -1){
        //cteate failure
        fprintf(stderr,"shmget failed\n");
        exit(EXIT_FAILURE);
    }

    //2.启动对该共享内存的访问，将地址进行连接
    //@共享内存标识符
    //@连接到当前进程中的内存地址位置，通常为空，将由系统进行选择
    //@标志位通常为零
    shm = shmat(shmid,0,0);
    if(shm == (void*)-1){
        fprintf(stderr,"shmat faile\n");
        exit(EXIT_FAILURE);
    }
    printf("\nMemory attached at %X\n",(unsigned int)shm);

    //3.读共享内存
    shared = (struct shared_use_st*)shm;
    shared->written = 0;
    while(running){
        //written是互斥量
        if(shared->written != 0){
            printf("Read from memory:%s",shared->text);
            shared->written = 0;
            if(strncmp(shared->text,"end",3)==0)
                running = 0;
        }
        else
            sleep(1);
    }
    //4.把共享内存从当前进程分离
    if(shmdt(shm)==-1){
        fprintf(stderr,"shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    //5.删除共享内存
    if(shmctl(shmid,IPC_RMID,0)==-1){
        fprintf(stderr,"shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}



