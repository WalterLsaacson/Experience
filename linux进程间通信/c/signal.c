#include <signal.h>
#include <stdio.h>
#include <unistd.h>

//调用signal函数只能处理一次，恢复默认行为后下次信号将是默认行为处理
void ouch(int signal){
    printf("\nGot signal %d \n",signal);
    //恢复信号的默认行为
    signal(SIGINT,SIG_DFL);
}

int main(int argc, char *argv[]){
    //信号将先由ouch函数处理
    signal(SIGINT,ouch);
    while(1){
        printf("Running in main thread.\n");
        sleep(1);
    }
}