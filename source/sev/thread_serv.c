#include "sv_msq.h"
#include "sv_msq_sev.h"
#include <errno.h>

extern user_linklist ul;
extern room_linklist rl;
extern saying_queue  sq;
extern room_opration ro;
extern pthread_mutex_t ul_mutex;
extern pthread_mutex_t rl_mutex;
extern pthread_mutex_t sq_mutex;
extern pthread_mutex_t ro_mutex;
extern pthread_cond_t  cond    ;

void * thread_serv(void * thread_serv_arg){
    user_linknode * uln = (user_linknode *)thread_serv_arg;
    //printf("%s的服务线程被创建.\n", uln->name);
    char *rcv_buf = malloc(sizeof(long) + MESGMAXLEN);
    char *snd_user_buf = calloc(NAMENUM_MAX, sizeof(NAMELEN_MAX + 1));
    char *snd_room_buf = calloc(ROOMNUM_MAX, sizeof(ROOMLEN_MAX + 1));
    char * user_buf_pointor = snd_user_buf;
    char * room_buf_pointor = snd_room_buf;
    user_linknode * user_temp_node;
    room_linknode * room_temp_node;
    size_t len;
    struct type2 t2;
    struct type4 t4;
    t2.type = TYPE2;
    t4.type = TYPE4;
    struct msqid_ds buf;

    while(1){
        errno = 0;
        //printf("%s的服务线程:开始一轮循环.\n", uln->name);

        if(msgrcv(uln->id, rcv_buf, MESGMAXLEN, TYPE3, IPC_NOWAIT) != -1){
            user_buf_pointor = snd_user_buf;
            room_buf_pointor = snd_room_buf;
            //printf("%s的服务线程:收到请求在线用户与房间信息.\n", uln->name);
            //将用户链表的每个结点记录的名字打印到snd_user_buf
            if(pthread_mutex_lock(&ul_mutex) != 0) //加锁
                errExit("pthread_mutex_lock");
            
            if(ul.firstnode == NULL){
                strcpy(snd_user_buf, "none#");
            }else{
                user_temp_node = ul.firstnode;
                while(user_temp_node != NULL){
                    strcpy(user_buf_pointor, user_temp_node->name);
                    len = strlen(user_buf_pointor);
                    user_buf_pointor[len] = '#'; //以#号分割
                    user_buf_pointor += len + 1; //下一次开始赋值的位置
                    user_temp_node = user_temp_node->next;
                }
                *user_buf_pointor = '\0';

            }
            //printf("%s的服务线程:现在，在线用户为：%s.\n", uln->name, snd_user_buf);
            if(pthread_mutex_unlock(&ul_mutex) != 0) //解锁
                errExit("pthread_mutex_unlock");

            //将房间链表的每个结点记录的房间名打印到snd_room_buf
            if(pthread_mutex_lock(&rl_mutex) != 0) //加锁
                errExit("pthread_mutex_lock");
            
            if(rl.firstnode == NULL){
                strcpy(snd_room_buf, "none#");
            }else{
                room_temp_node = rl.firstnode;
                while(room_temp_node != NULL){
                    strcpy(room_buf_pointor, room_temp_node->room);
                    len = strlen(room_buf_pointor);
                    room_buf_pointor[len] = '#'; //以#号分割
                    room_buf_pointor += len + 1; //下一次开始赋值的位置
                    room_temp_node = room_temp_node->next;
                }
                *room_buf_pointor = '\0';

            }
            //printf("%s的服务线程:现在，在线房间为：%s.\n", uln->name, snd_room_buf);
            if(pthread_mutex_unlock(&rl_mutex) != 0) //解锁
                errExit("pthread_mutex_unlock");

            strcpy(t4.userList, snd_user_buf);
            strcpy(t4.roomList, snd_room_buf);
            msgsnd(uln->id, &t4, T4SIZE, IPC_NOWAIT);
            //printf("%s的服务线程:发送成功.\n", uln->name);
        }

        if(msgrcv(uln->id, rcv_buf, MESGMAXLEN, TYPE5, IPC_NOWAIT) != -1){
            struct type5 * info = (struct type5 *)rcv_buf;
            //printf("%s的服务线程:收到房间操作消息，房间为%s.\n", uln->name, info->room);
            strcpy(ro.room, info->room);
            if(info->operation == CREAT_ROOM){
                if(pthread_mutex_lock(&rl_mutex) != 0) //加锁
                    errExit("pthread_mutex_lock");
                if(room_is_exist(rl, info->room) == 0){ //房间不存在，可以创建
                    t2.is_success = 1; 
                    msgsnd(uln->id, &t2, T2SIZE, IPC_NOWAIT);
                    if(pthread_mutex_unlock(&rl_mutex) != 0)
                        errExit("pthread_mutex_lock");
                    if(pthread_mutex_lock(&ro_mutex) != 0) //加锁
                        errExit("pthread_mutex_lock");
                    ro.operation = ROOM_ADD;
                }else{
                    t2.is_success = 0; 
                    msgsnd(uln->id, &t2, T2SIZE, IPC_NOWAIT);
                    if(pthread_mutex_unlock(&rl_mutex) != 0)
                        errExit("pthread_mutex_lock");
                }
            }else if(info->operation == REMOV_ROOM){
                if(pthread_mutex_lock(&rl_mutex) != 0) //加锁
                    errExit("pthread_mutex_lock");
                if(room_is_exist(rl, info->room) == 1){ //房间存在，可以删除
                    t2.is_success = 1; 
                    msgsnd(uln->id, &t2, T2SIZE, IPC_NOWAIT);
                    if(pthread_mutex_unlock(&rl_mutex) != 0)
                        errExit("pthread_mutex_lock");
                    if(pthread_mutex_lock(&ro_mutex) != 0) //加锁
                        errExit("pthread_mutex_lock");
                    ro.operation = ROOM_REMOVE;
                }else{
                    t2.is_success = 0; 
                    msgsnd(uln->id, &t2, T2SIZE, IPC_NOWAIT);
                    if(pthread_mutex_unlock(&rl_mutex) != 0)
                        errExit("pthread_mutex_lock");
                }
            }
            if(pthread_mutex_unlock(&ro_mutex) != 0) //加锁
                errExit("pthread_mutex_unlock");
            pthread_cond_signal(&cond);

            if(info->operation == ADDIN_ROOM){
                if(pthread_mutex_lock(&rl_mutex) != 0)
                    errExit("pthread_mutex_lock");
                if(room_is_exist(rl, info->room) == 0){
                    if(pthread_mutex_unlock(&rl_mutex) != 0)
                        errExit("pthread_mutex_unlock");
                    //printf("%s的服务线程:要加入的房间不存在.\n",uln->name);
                    t2.is_success = 0;
                    msgsnd(uln->id, &t2, T2SIZE, IPC_NOWAIT);
                }else{
                    if(pthread_mutex_unlock(&rl_mutex) != 0)
                        errExit("pthread_mutex_unlock");
                    //printf("%s的服务线程:已找到要加入的房间.\n",uln->name);
                    if(pthread_mutex_lock(&ul_mutex) != 0)
                        errExit("pthread_mutex_lock");
                    strcpy(uln->room, info->room); //发生bug的可能是用户在发请求后在收到回复之前退出且清理线程已经把该结点删了
                    if(pthread_mutex_unlock(&ul_mutex) != 0)
                        errExit("pthread_mutex_unlock");
                    t2.is_success = 1;
                    msgsnd(uln->id, &t2, T2SIZE, IPC_NOWAIT);
                }
            }
        }

        if(msgrcv(uln->id, rcv_buf, MESGMAXLEN, TYPE6, IPC_NOWAIT) != -1){
            //printf("%s的服务线程:收到聊天消息.\n",uln->name);
            struct type6 * info = (struct type6 *)rcv_buf;
            saying_queue_node temp;
            strcpy(temp.info, info->message);
            strcpy(temp.room, uln->room);
            strcpy(temp.user, info->name); //这里重复了，info->name或uln->name均可
            if(pthread_mutex_lock(&sq_mutex) != 0)
                errExit("pthread_mutex_lock");
            saying_enter_queue(&sq, &temp);
            if(pthread_mutex_unlock(&sq_mutex) != 0)
                errExit("pthread_mutex_unlock");
            // break;
        }

        if(msgrcv(uln->id, rcv_buf, MESGMAXLEN, TYPE8, IPC_NOWAIT) != -1){
            //printf("%s的服务线程:收到退出房间申请.\n",uln->name);
            if(pthread_mutex_lock(&ul_mutex) != 0)
                errExit("pthread_mutex_lock");
            strcpy(uln->room, "none");
            if(pthread_mutex_unlock(&ul_mutex) != 0)
                errExit("pthread_mutex_unlock");
        }

        if(msgctl(uln->id, IPC_STAT, &buf) == -1){
            //printf("%s的服务线程:监测到用户已经退出.\n",uln->name);
            free(rcv_buf);
            free(snd_user_buf);
            free(snd_room_buf);
            return NULL;
        }
        //printf("%s的服务线程:开始休眠.\n",uln->name);
        sleep(2);
    }
}