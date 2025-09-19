/*此头文件专属于服务器，用于声明服务器各个程序文件共享的函数。
此外还会定义一些服务器共享的全局数据结构。*/
#include "sv_msq.h"
#include <pthread.h>
#ifndef SV_MSQ_SEV_H
#define SV_MSQ_SEV_H

#define MESGMAXLEN 1024 //可容纳客户端发来的任何一种消息正文的大小
#define SAYING_QUEUE_LEN 32 //消息队列数据结构的空间大小
#define ROOM_NONE 0 //用于room_opration，对room什么也不做
#define ROOM_ADD 1 //用于room_operation，将room添加至room链表
#define ROOM_REMOVE 2 //用于room_operation，将room移出room链表

typedef struct{
    char room[ROOMLEN_MAX + 1];
    int operation; //对room的操作
}room_opration;

//清空stdin缓冲区
void clear_stdin(void);

//去掉字符串结尾的换行符
void remove_chline(char *str);

//错误退出函数
_Noreturn void errExit (const char * func);

//线程1
void * thread_1_msgsender(void * thread_1_arg);

//线程2
void * thread_2_userremover(void * thread_2_arg);

//线程3
void * thread_3_roommanager(void * thread_3_arg);

//服务线程
void * thread_serv(void * thread_serv_arg);

/*---------------------------------以下为数据结构定义---------------------------------*/
//用户信息链表结点
typedef struct user_linknode{
    char name[NAMELEN_MAX + 1];
    char room[ROOMLEN_MAX + 1];
    int id; //客户端消息队列id
    struct user_linknode * next;
}user_linknode;

//用户信息链表句柄
typedef struct{
    int usernum; //目前用户数量
    user_linknode * firstnode;
}user_linklist;

//房间链表结点
typedef struct room_linknode{
    char room[ROOMLEN_MAX + 1];
    struct room_linknode * next;
}room_linknode;

//房间链表句柄
typedef struct{
    int roomnum; //目前房间数量
    room_linknode * firstnode;
}room_linklist;

//队列数据结构结点
typedef struct{
    char user[NAMELEN_MAX + 1]; //用户名称
    char room[ROOMLEN_MAX + 1]; //来源房间
    char info[SAY_MAX]; //聊天信息
}saying_queue_node;

//队列数据结构句柄
typedef struct{
    saying_queue_node *base; //队列存储空间的基地址
    int front;
    int rear;
}saying_queue;

/*----------以下为用户链表数据结构操作，需与互斥量配合使用------------*/

//初始化一个用户信息链表，返回链表句柄
user_linklist  user_init_list(void);

//判断用户链表中是否有一个用户名为user_name
int user_is_exist(user_linklist ul, const char * user_name);

//为新用户添加一个结点并返回该结点地址，使用前需判断用户数量是否已满，否则别的缓冲区将无法容纳，使用前保证用户不存在
user_linknode * user_add_by_name(user_linklist * ul, const char * user_name, int id);

//根据用户名称查找结点并返回结点指针，使用前保证用户存在
user_linknode * user_find_by_name(user_linklist ul, const char * user_name);

//根据房间名称查找结点并返回结点指针，需循环调用，若返回NULL，表明全部找完
user_linknode * user_find_by_room(user_linklist ul, const char * room_name);

//根据用户名称删除一个结点
void user_remove_by_name(user_linklist * ul, const char * user_name);

//销毁用户信息链表
void user_delete_linklist(user_linklist * ul);

/*----------以下为房间链表数据结构操作，需与互斥量配合使用------------*/

//初始化一个房间链表，返回链表句柄
room_linklist room_init_list(void);

//判断房间链表中是否有一个房间名为room_name
int room_is_exist(room_linklist rl, const char * room_name);

//删除一个名为room_name的结点，不管存不存在都成功
void room_remove_by_name(room_linklist * rl, const char * room_name);

//增加一个名为room_name的结点（需先调用room_is_exist，否则将创建重名的房间）
void room_add_by_name(room_linklist *rl, const char * room_name);

//销毁一个房间链表
void room_delete_linklist(room_linklist * rl);

/*----------以下为聊天消息队列数据结构操作，需与互斥量配合使用------------*/

//初始化一个聊天消息队列，返回聊天消息队列句柄
saying_queue  saying_init_queue(void);

//消息入队，不返回成功失败消息，队列满就丢包
void saying_enter_queue(saying_queue * sq, saying_queue_node *sqn);

//消息出队，使用前需调用saying_is_empty检查是否为空
saying_queue_node saying_leave_queue(saying_queue * sq);

//判断队列是否为空，1为空，0为不空
int saying_is_empty(saying_queue sq);

//销毁一个聊天消息队列
void saying_delete_queue(saying_queue * sq);

#endif