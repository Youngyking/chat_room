#include "sv_msq.h"
#include "sv_msq_client.h"

extern int ser_msq_id;
extern int cli_msq_id;

void upgrade(const char *user_name){
    struct type3 t3;
    struct type4 t4;

    t3.type = TYPE3;

    if(msgsnd(cli_msq_id, &t3, T3SIZE, 0) == -1)
        errExit("msgsnd");
    //printf("upgrade:1.\n");
    if(msgrcv(cli_msq_id, &t4, T4SIZE, TYPE4, 0) == -1)
        errExit("msgrcv");
    //printf("upgrade:2.\n");
    printUsrAndRoom(t4.userList, t4.roomList);
}