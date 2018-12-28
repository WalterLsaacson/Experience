#include <stdio.h>
#include <unistd.h>//建立管道需要用到的头文件
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define N 10
#define MAX 100

//当管道数据满时，此时再向管道写数据，写端将阻塞。
//当读端不存在时，写端写数据，内核将向其发送SIGPIPE信号/*信号通信*/，默认是终止进程

int child_read_pipe(int fd){

    //匿名管道传送的是无格式的字节流，读写段需要明确数据的格式
    //这里表示数据大小为10是一个消息
    char buf[N];
    int n = 0;

    while(1){
        //从哪里读，放哪里，读多大
        n = read(fd,buf,sizeof(buf));
        // 收个字符为空字符
        // \0转义表示空字符
        buf[n] = '\0';

        printf("Read %d bytes : %s.\n",n,buf);

        //谁和谁比较，比较多长内容
        if (strncmp(buf,"quit",4) == 0 )
            break;
    }

    return 0;
}

int father_write_pipe(int fd){
    char buf[MAX] = {0};

    while(1){
        printf(">");
        //从标准输入中获取内容
        fgets(buf,sizeof(buf),stdin);
        //末尾添加空字符
        buf[strlen(buf)-1] = '\0';
        //写入管道
        write(fd,buf,strlen(buf));
        usleep(500);
        if(strncmp(buf,"quit",4) == 0)
            break;
    }

    return 0;
}

int main(){
    int pid;
    int fd[2];

    //pipe将直接创建管道并将文件描述符返回
    //成功为0，失败为-1
    if(pipe(fd) < 0){
        perror("Fail to pipe");
        exit(EXIT_FAILURE);
    }

    //调用fork函数将返回两次
    //0表示子进程
    if((pid=fork()) < 0){
        perror("Fail to fork");
        exit(EXIT_FAILURE);
    }else if(pid == 0){
        close(fd[1]);
        child_read_pipe(fd[0]);
    }else{
        close(fd[0]);
        father_write_pipe(fd[1]);

    }
    exit(EXIT_SUCCESS);
}

/*int main(){
    int fd[2];
    int count = 0;
    
    if (pipe(fd) < 0){
    perror("Fail to create pipe");
    exit(EXIT_FAILURE);
    }
    
    while(1){
    write(fd[1],"a",sizeof(char));
    printf("count = %d.\n",++count);
    usleep(20000);
    }

    return 0;
}*/
