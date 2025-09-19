/*此文件提供客户端与服务端共同的协议*/

#ifndef SV_MSQ_H
#define SV_MSQ_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define NAMELEN_MAX 20 //最大用户名字长度
#define NAMENUM_MAX 50 //最大同时在线用户数量
#define ROOMLEN_MAX 20 //最大房间名字长度
#define ROOMNUM_MAX 10 //最大房间数量
#define SAY_MAX 512 //最大单次聊天内容长度

#define TYPE1 1 //消息类型1：用户名字与客户端创建的消息队列id(char[], int)
#define TYPE2 2 //消息类型2：成功与否（int)
#define TYPE3 3 //消息类型3：请求最新信息（void）
#define TYPE4 4 //消息类型4：目前在线用户信息（char []，各个名字以#号分隔），已有房间名（char []，各个名字以#号分隔）
#define TYPE5 5 //消息类型5：创建(0)/删除(1)/加入(2)房间申请（int, char[])
#define TYPE6 6 //消息类型6：待发送的聊天消息(char[], char[]）
#define TYPE7 7 //消息类型7：待接受的聊天消息（int, char[], char[])
#define TYPE8 8 //消息类型8：退出房间申请（void)

//消息类型1：用户名字与客户端创建的消息队列id(char[], int)
struct type1{
    long type;
    char usrName[NAMELEN_MAX];
    int msqID;
};
#define T1SIZE (sizeof(struct type1) - offsetof(struct type1, usrName))

//消息类型2：成功则is_success为1
struct type2{
    long type;
    int is_success;
};
#define T2SIZE (sizeof(struct type2) - offsetof(struct type2, is_success))

//消息类型3：请求最新信息（void）
struct type3{
    long type;
};
#define T3SIZE 0

//消息类型4：目前在线用户信息（char []，各个名字以#号分隔），已有房间名（char []，各个名字以#号分隔）
struct type4{
    long type;
    char userList[(NAMELEN_MAX + 1) * NAMENUM_MAX];
    char roomList[(ROOMLEN_MAX + 1) * ROOMNUM_MAX];
};
#define T4SIZE (sizeof(struct type4) - offsetof(struct type4, userList))

//消息类型5：创建(CREAT_ROOM)/删除(REMOV_ROOM)/加入(ADDIN_ROOM)房间申请（int, char[])
struct type5{
    long type;
    int operation;
    char room[ROOMLEN_MAX + 1];
};
#define T5SIZE (sizeof(struct type5) - offsetof(struct type5, operation))
#define CREAT_ROOM 0
#define REMOV_ROOM 1
#define ADDIN_ROOM 2

//消息类型6：客户端待发送的聊天消息(char[], char[]）
struct type6{
    long type;
    char name[NAMELEN_MAX + 1];
    char message[SAY_MAX];
};
#define T6SIZE (sizeof(struct type6) - offsetof(struct type6, name))

//消息类型7：客户端待接受的聊天消息（int, char[], char[]),is_chat为1代表是聊天消息，为0即房间已被关闭
struct type7
{
    long type;
    int is_chat;
    char name[NAMELEN_MAX + 1];
    char saying[SAY_MAX];
};
#define T7SIZE (sizeof(struct type7) - offsetof(struct type7, is_chat))

//消息类型8：退出房间申请（void)
struct type8
{
    long type;
};
#define T8SIZE 0

#define SEV_KEY 0x114514
#define CLI_KEY getpid()
#define max(x, y) ((x > y) ? x : y)

#endif