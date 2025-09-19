#include "sv_msq.h"
#include "sv_msq_sev.h"
#include <pthread.h>

user_linklist ul;
room_linklist rl;
saying_queue  sq;
room_opration ro;
int sev_id; //服务器端消息队列id
pthread_t t1, t2, t3;
pthread_attr_t attr;
pthread_mutex_t ul_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rl_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sq_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ro_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond     = PTHREAD_COND_INITIALIZER;

//信号处理器
void handler(int sig){
    write(STDERR_FILENO, "SYSV_chatroom: bye~\n", 21);
    exit(EXIT_SUCCESS);
}

void clean_up(void){
    pthread_cancel(t1);
    pthread_cancel(t2);
    pthread_cancel(t3);
    msgctl(sev_id, IPC_RMID, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&ul_mutex);
    pthread_mutex_destroy(&rl_mutex);
    pthread_mutex_destroy(&sq_mutex);
    pthread_mutex_destroy(&ro_mutex);
    pthread_cond_destroy(&cond);
    user_delete_linklist(&ul);
    room_delete_linklist(&rl);
    saying_delete_queue(&sq);
}

int main(){
    signal(SIGINT, handler);
    signal(SIGSEGV, handler);
    signal(SIGTERM, handler);
        
    if(atexit(clean_up) == -1)
        errExit("atexit");
    
    ul = user_init_list();
    rl = room_init_list();
    sq = saying_init_queue();
    struct type1 rcv_buf;
    struct type2 snd_buf;
    snd_buf.type = TYPE2;
    user_linknode *temp;
    ro.operation = ROOM_NONE;
    
    if((sev_id = msgget(SEV_KEY, IPC_CREAT | IPC_EXCL)) == -1)
        errExit("msgget");

    if(pthread_create(&t1, NULL, thread_1_msgsender, NULL) != 0)
        errExit("pthread_create");
    if(pthread_create(&t2, NULL, thread_2_userremover, NULL) != 0)
        errExit("pthread_create");
    if(pthread_create(&t3, NULL, thread_3_roommanager, NULL) != 0)
        errExit("pthread_create");
    pthread_detach(t3);
    if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
        errExit("pthread_attr_setdetachstate");
    
    while(1){
        if(msgrcv(sev_id, &rcv_buf, T1SIZE, TYPE1, 0) == -1)
            errExit("msgrcv");
        if((ul.usernum >= NAMENUM_MAX) || (user_is_exist(ul, rcv_buf.usrName))){
            //保证用户数未满且无重名用户
            snd_buf.is_success = 0;
            printf("有重名用户或用户已满。\n");
            if(msgsnd(rcv_buf.msqID, &snd_buf, T2SIZE, 0) == -1)
                errExit("msgsnd");
            continue;
        }
        //printf("可以创建新用户。\n");
        snd_buf.is_success = 1;
        temp = user_add_by_name(&ul, rcv_buf.usrName, rcv_buf.msqID);
        if(msgsnd(rcv_buf.msqID, &snd_buf, T2SIZE, 0) == -1)
            errExit("msgsnd");
        printf("发送成功。\n");
        pthread_t serv;
        if(pthread_create(&serv, &attr, thread_serv, temp) != 0)
            errExit("pthread_create");
    }
}