#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/fcntl.h>

#define MSG_W 0200
#define BUF_SIZE 512

typedef struct msgbuf{
    long mtype;
    char mdata[BUF_SIZE];
}mymsg_t;

int main(int argc,char** argv){
    int mqid; //消息队列的描述符
    size_t msglen; //消息的长度
    long msgtype; //消息的类型
    mymsg_t* ptr; //消息结构的指针

    //未按格式输入
    if(argc != 3)
        puts("usage: send <pathname> <type>");

    //atoi把字符串转换为整形数
    msgtype = atoi(argv[2]);

    //创建消息队列
    mqid = msgget(ftok(argv[1],0),MSG_W);

    //构造消息
    //1.分配内存
    ptr = calloc(sizeof(long) + msglen,sizeof(char));
    //2.有命令行指定的消息类型
    ptr->mtype = msgtype;
    //3.填充字符数组
    snprintf(ptr->mdata,BUF_SIZE,"Hello");

    //发送消息到消息队列
    msglen = strlen(ptr->mdata);
    msgsnd(mqid,ptr,msglen,0);

    exit(0);
}