#include "sv_msq.h"
#include "sv_msq_client.h"

void welcome(char *user_name){
    printf("Hello! Welcome to KingYoungy's chat room!\n\n\n");
    printf("Would it be okay if I asked for your name? :");
    fgets(user_name, NAMELEN_MAX + 1, stdin);
    remove_chline(user_name);
    printf("\n\nOK, %s, Now try to connect the server.\n\n", user_name);
}