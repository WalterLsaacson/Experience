#ifndef _SHMDATA_H_HEADER
#define _SHMDATA_H_HEADER

#define TEXT_SZ 2048

struct shared_use_st{
    int written;//互斥量，非零可读，零可写
    char text[TEXT_SZ];//写入和读取的文本
};

#endif //C_SHMDATA_H
