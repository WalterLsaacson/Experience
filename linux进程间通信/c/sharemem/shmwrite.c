#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include "shmdata.h"

#define MAX 100

int main(){
    int running = 1;
    void *shm = NULL;//共享内存的首地址
    struct shared_use_st *shared;//指向shm
    int shmid;//共享内存标识符
    char buf[MAX];

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
    printf("\nMemory attached at %X\n",(int)shm);

    //3.读共享内存
    shared = (struct shared_use_st*)shm;
    while(running){
        //written是互斥量
        while(shared->written == 1){
            sleep(1);
            printf("Wating ...\n");
        }
        printf("Plz enter the text:\n>");
        //读取用户输入并写入共享内存
        fgets(buf,sizeof(buf),stdin);
        //buf[strlen(buf) - 1] = '\0';
        printf("Write to shm :%s",buf);
        strncpy(shared->text, buf, TEXT_SZ);
        shared->written = 1;
        if(strcmp(shared->text,"end",3)==0)
            running = 0;
    }
    //4.把共享内存从当前进程分离
    if(shmdt(shm)==-1){
        fprintf(stderr,"shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}


