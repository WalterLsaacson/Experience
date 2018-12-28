#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *arry;
};

static int sem_id = 0;

static int set_semvalue();
static void del_semvalue();
static int semaphore_p();
static int semaphore_v();

/*
gcc Semaphore.c -o sem.exe

./sem.exe 0 & ./sem.exe
[1] 28511
XXXX00XX00XX00XX00XX00XX00XX00XXXX000000
28512-finished
guanyin@software:~/tt/c$
28511-finished

[1]+  已完成               ./sem.exe 0

*/
int main(int argc,char *argv[]){
    char message = 'X';
    int i = 0;

    //创建信号量(唯一标识，信号量数量一般为1，标志)
    //1.获取信号量标识符,后续的操作均针对该标识符进行
    sem_id = semget((key_t)1234,1,0666|IPC_CREAT);

    if(argc > 1){
        //2.首次运次初始化信号量
        if(!set_semvalue()){
            fprintf(stderr,"Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
        //设置将要输出到屏幕的信息
        message = argv[1][0];
        sleep(2);
    }

    for(i = 0;i < 10;++i){
        //3.进入临界区,执行P操作获取信号量实现互斥
        if(!semaphore_p())
            exit(EXIT_FAILURE);

        printf("%c",message);
        sleep(rand()%3);
        printf("%c",message);
        fflush(stdout);

        //4.执行V操作释放信号量，使其他进程访问该临界区
        if(!semaphore_v())
            exit(EXIT_FAILURE);
        sleep(rand()%2);
    }

    sleep(10);
    printf("\n%d-finished\n",getpid());

    //5.首次运行需要删除信号量，防止之后创建信号量是报错
    if(argc > 1){
        sleep(3);
        del_semvalue();
    }
    exit(EXIT_SUCCESS);
}

static int set_semvalue(){
    union semun sem_union;
    sem_union.val = 1;
    //调用semctl函数，第三个参数为SETVAL
    //用来把信号量初始化为一个已知的值
    if(semctl(sem_id,0,SETVAL,sem_union) == -1)
        return 0;
    return 1;
}

static void del_semvalue(){
    union semun sem_union;
    //IPC_RMID：用于删除一个已经无需继续使用的信号量标识符。
    if(semctl(sem_id,0,IPC_RMID,sem_union) == -1)
        fprintf(stderr,"Failed to delete semaphore\n");
}

static int semaphore_p(){
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;//P
    sem_b.sem_flg = SEM_UNDO;
    //（信号量标识符，sembuf数据结构，信号量的数量）
    if(semop(sem_id,&sem_b,1)==-1){
        fprintf(stderr,"semaphore_p failed\n");
        return 0;
    }
    return 1;
}

static int semaphore_v(){
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;//V
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id,&sem_b,1)==-1){
        fprintf(stderr,"semphore_v failed\n");
        return 0;
    }
    return 1;
}














