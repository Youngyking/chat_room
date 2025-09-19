#include "sv_msq.h"
#include "sv_msq_sev.h"

extern saying_queue  sq;
extern user_linklist ul;
extern pthread_mutex_t sq_mutex;
extern pthread_mutex_t ul_mutex;


void * thread_1_msgsender(void * thread_1_arg){
    //printf("线程1:启动\n");
    saying_queue_node sqn;
    user_linknode * uln;
    struct type7 buf;
    buf.is_chat = 1;
    buf.type = TYPE7;

    while(1){
        //printf("线程1:加消息队列锁\n");
        if(pthread_mutex_lock(&sq_mutex) != 0)
            errExit("pthread_mutex_lock");
        if(saying_is_empty(sq) == 1){
            if(pthread_mutex_unlock(&sq_mutex) != 0)
                errExit("pthread_mutex_unlock");
            //printf("线程1:放消息队列锁\n");
            sleep(2);
            continue;
        }
        //printf("线程1:发现有消息\n");
        sqn = saying_leave_queue(&sq);

        if(pthread_mutex_unlock(&sq_mutex) != 0)
            errExit("pthread_mutex_unlock");
        //printf("线程1:放消息队列锁\n");
        //printf("线程1:加用户链表锁\n");
        if(pthread_mutex_lock(&ul_mutex) != 0)
            errExit("pthread_mutex_lock");
        //printf("线程1:即将开始循环以找到房间%s下的所有用户\n", sqn.room);
        while((uln = user_find_by_room(ul, sqn.room)) != NULL){
            //printf("线程1:找到用户%s,将给其发包\n", uln->name);
            if(pthread_mutex_unlock(&ul_mutex) != 0)
                errExit("pthread_mutex_unlock");

            strcpy(buf.name, sqn.user);
            strcpy(buf.saying, sqn.info);
            //printf("线程1：发的包的内容为%s\n", buf.saying);
            msgsnd(uln->id, &buf, T7SIZE, IPC_NOWAIT); //不阻塞，不能发就直接丢包，不进行错误检查
            //printf("线程1:放用户链表锁\n");
            if(pthread_mutex_lock(&ul_mutex) != 0)
                errExit("pthread_mutex_lock");
        }
        //printf("线程1:结束本轮循环并放用户链表锁\n");
        if(pthread_mutex_unlock(&ul_mutex) != 0)
            errExit("pthread_mutex_unlock");
    }
}