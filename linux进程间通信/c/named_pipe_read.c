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
        fprintf(stderr,"usage : %s argv[1].\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    //创建管道，第一个参数为filename，第二个参数指定管道文件在文件系统中的权限
    if(mkfifo(argv[1],0666) < 0 && errno != EEXIST){
        fprintf(stderr,"Fail to mkfifo %s : %s.\n",argv[1],strerror(errno));
        exit(EXIT_FAILURE);
    }

    //以只读方式打开该管道文件
    if((fd = open(argv[1],O_RDONLY)) < 0){
        fprintf(stderr,"Fail to open %s : %s.\n",argv[1],strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("open for read success.\n");

    while(1){
        printf(">");
        n = read(fd,buf,n);
        if (n != -1){
            printf("Read %d bytes %s.\n",n,buf);
        }
    }
    return 0;
}