#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>//建立管道需要用到的头文件

#define MAX 100

int main(int argc, char *argv[]){
    int fd,n;
    char buf[MAX];

    if(argc < 2){
        fprintf(stderr,"Usage : %s argv[1].\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    if(mkfifo(argv[1],0666) < 0 && errno != EEXIST){
        fprintf(stderr,"Fail to mkfifo %s : %s.\n",argv[1],strerror(errno));
        exit(EXIT_FAILURE);
    }

    if((fd = open(argv[1],O_WRONLY)) < 0){
        fprintf(stderr,"Fail to open %s : %s.\n",argv[1],strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("open for write success.\n");

    while(1){
        printf(">");
        //从标准输入中获取内容
        fgets(buf,sizeof(buf),stdin);
        buf[strlen(buf) - 1] = '\0';
        printf("Write %s to %s.\n",buf,argv[1]);
        write(fd,buf,n);
    }

    exit(EXIT_SUCCESS);
}