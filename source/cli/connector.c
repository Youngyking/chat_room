#include "sv_msq.h"
#include "sv_msq_client.h"

extern int ser_msq_id;
extern int cli_msq_id;

int connector(const char *user_name){
    struct type1 t1; //待发送的消息
    struct type2 t2; //接受消息缓冲区

    t1.type = TYPE1;
    strcpy(t1.usrName, user_name);
    t1.msqID = cli_msq_id;

    if(msgsnd(ser_msq_id, &t1, T1SIZE, 0) == -1)
        return -1;

    if(msgrcv(cli_msq_id, &t2, T2SIZE, TYPE2, 0) != T2SIZE)
        return -1;

    if(t2.is_success == 1)
        return 0;
    else
        return -1;
}