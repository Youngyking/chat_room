#include "sv_msq.h"
#include "sv_msq_client.h"

void clear_stdin(void){
    char ch;
    while((ch = getchar()) != '\n')
        continue;
}

void remove_chline(char *str){
    size_t len = strlen(str);
    str[len - 1] = '\0';
}

void printUsrAndRoom(const char *usr, const char *room){
    char buf[NAMELEN_MAX + 1];
    char buf2[ROOMLEN_MAX + 1];

    printf("\n\n--------------------------------------------------\n");
    printf("User Online:\n");
    //打印用户
    for(int i = 0, j = 0; i < strlen(usr); i++){
        if(usr[i] != '#'){
            buf[j] = usr[i];
            j++;
        }else{
            buf[j] = '\0';
            printf("%-20s", buf);
            j = 0;
        }
    }
    printf("\n--------------------------------------------------\n");
    printf("Room exist:\n");
    for(int i = 0, j = 0; i < strlen(room); i++){
        if(room[i] != '#'){
            buf2[j] = room[i];
            j++;
        }else{
            buf2[j] = '\0';
            printf("%-20s", buf2);
            j = 0;
        }
    }
}