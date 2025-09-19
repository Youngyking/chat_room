#include "sv_msq.h"
#include "sv_msq_sev.h"

extern user_linklist ul;
extern pthread_mutex_t ul_mutex;

void * thread_2_userremover(void * thread_2_arg){
    printf("线程2:启动\n");
    user_linknode * uln;
    user_linknode * temp;
    
    struct msqid_ds md; //没有什么用

    while(1){
        //printf("线程2:加用户链表锁以保证独占接下来对用户链表的更新\n");
        if(pthread_mutex_lock(&ul_mutex) != 0)
            errExit("pthread_mutex_lock");
        uln = ul.firstnode;
        temp = ul.firstnode;
        //遍历链表
        while(uln != NULL){
            temp = uln;
            uln = uln->next;
            if(msgctl(temp->id, IPC_STAT, &md) == -1){
                //printf("用户%s已经退出\n", temp->name);
                user_remove_by_name(&ul, temp->name);
            }
        }
        //printf("线程2:更新完成后放用户链表锁\n");
        if(pthread_mutex_unlock(&ul_mutex) != 0)
            errExit("pthread_mutex_unlock");
            
        sleep(3); //这段时间别的线程可能向已经不存在的客户端消息队列发信息，这就要求不进行错误检查。
    }
    
}