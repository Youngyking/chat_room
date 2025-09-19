/*只要只有一个线程持有互斥量并等待另一个互斥量，那么就不会死锁
因此，无论如何都不能让别的线程再像该线程3一样持有并等待锁了。*/

#include "sv_msq.h"
#include "sv_msq_sev.h"

extern user_linklist ul;
extern room_opration ro;
extern room_linklist rl;
extern pthread_mutex_t ul_mutex;
extern pthread_mutex_t ro_mutex;
extern pthread_mutex_t rl_mutex;
extern pthread_cond_t  cond;

void * thread_3_roommanager(void * thread_3_arg){
    printf("线程3:启动\n");
    user_linknode * temp;
    struct type7 snd_buf;
    snd_buf.is_chat = 0;
    snd_buf.type = TYPE7;
    while(1){
        printf("线程3：进入循环\n");
        if(pthread_mutex_lock(&ro_mutex) != 0)
            errExit("pthread_mutex_lock");
        while(ro.operation == ROOM_NONE){
            if(pthread_cond_wait(&cond, &ro_mutex) != 0)
                errExit("pthread_cond_wait");
        }
        printf("线程3:被唤醒\n");
        if(ro.operation == ROOM_ADD){
            printf("线程3:居然是开房操作\n");
            if(pthread_mutex_lock(&rl_mutex) != 0)
                errExit("pthread_mutex_lock");
            room_add_by_name(&rl, ro.room);
            printf("线程3:开房成功，房间名为%s\n", ro.room);
        }else{
            printf("线程3:居然是退房操作\n");
            if(pthread_mutex_lock(&ul_mutex) != 0)
                errExit("pthread_mutex_lock");
            while((temp = user_find_by_room(ul, ro.room)) != NULL){
                msgsnd(temp->id, &snd_buf, T7SIZE, 0);
            }
            printf("线程3:成功赶走该房间的所有人\n");
            if(pthread_mutex_unlock(&ul_mutex) != 0)
                errExit("pthread_mutex_unlock");
                room_remove_by_name(&rl, ro.room);
            if(pthread_mutex_lock(&rl_mutex) != 0)
                errExit("pthread_mutex_lock");
        }
        ro.operation = ROOM_NONE;
        if(pthread_mutex_unlock(&rl_mutex) != 0)
            errExit("pthread_mutex_lock");
        if(pthread_mutex_unlock(&ro_mutex) != 0)
            errExit("pthread_mutex_lock");
        printf("线程3:活干完了睡觉\n");
        sleep(1);
    }
}