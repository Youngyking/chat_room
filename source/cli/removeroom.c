#include "sv_msq.h"
#include "sv_msq_client.h"

extern int ser_msq_id;
extern int cli_msq_id;

int removeroom(void){
    struct type5 t5; //待发送的消息
    struct type2 t2; //接受消息缓冲区

    t5.type = TYPE5;
    t5.operation = REMOV_ROOM;

    char roomname[ROOMLEN_MAX + 1];

    printf("Enter the room's name:");
    fgets(roomname, ROOMLEN_MAX + 1, stdin);
    remove_chline(roomname);
    strcpy(t5.room, roomname);

    if(msgsnd(cli_msq_id, &t5, T5SIZE, 0) == -1)
        errExit("msgsnd");

    if(msgrcv(cli_msq_id, &t2, T2SIZE, TYPE2, 0) != T2SIZE)
        errExit("msgrcv");

    if(t2.is_success)
        return 0;
    else 
        return -1;
}