#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

/*struct msqid_ds {
      struct ipc_perm msg_perm ;
      struct msg*    msg_first ; //指向队列中的第一个消息
      struct msg*    msg_last ; //指向队列中的最后一个消息
      ……
  } ;*/
int main(int argc, char** argv){
    int c,flags;
    int mqid;

    flags = IPC_CREAT;

    while((c = getopt(argc,argv,"e")) != -1){
        switch(c){
            case 'e':
            flags |= IPC_EXCL;
            break;
        }
    }

    if(optind != argc -1)
        puts("请按格式输入：[-e] [-m maxmsg] [-z msgsize] <name>");

    //ftok 根据文件名和权限控制信息，生成唯一标识
    //msgget生成一个消息队列，返回消息队列的标识符
    mqid = msgget(ftok(argv[optind],0),flags);

    /*int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
    int msgctl(int msqid, int cmd, struct msqid_ds *buf)
    ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,int msgflg);*/

    exit(0);
}