#include "sv_msq.h"
#include "sv_msq_client.h"
#include <pthread.h>
#include <ncurses.h>

int ser_msq_id;
int cli_msq_id;
extern WINDOW *top_win; //接收消息的窗口，供recver线程使用
extern WINDOW *bottom_win; //发送消息的窗口，供sender线程使用
extern pthread_t sender; //发送聊天消息的线程
extern pthread_t recver; //接受返回消息的线程
extern pthread_mutex_t win_mutex;

void handle(int sig){
    exit(EXIT_SUCCESS);
}

void clean_up(void){
    msgctl(cli_msq_id, IPC_RMID, NULL);
}

int main(){
    char user_name[NAMELEN_MAX + 1]; //留一个字节给'\0'
    char ch;
    char choice;
    atexit(clean_up);
    signal(SIGINT, handle);
    signal(SIGTERM, handle);
    signal(SIGHUP, handle);
    setvbuf(stdout, NULL, _IOLBF, 0);

    welcome(user_name);

    if((ser_msq_id = msgget(SEV_KEY, 0)) == -1){
        sleep(2);
        printf("\n\nServer is not open.\n\n");
        exit(EXIT_FAILURE);
    }

    if((cli_msq_id = msgget(CLI_KEY, IPC_CREAT | IPC_EXCL)) == -1)
        errExit("msgget");

    while(1){
        if(connector(user_name) == 0)
            break;
        else{
            //sleep(2);
            printf("\n\nConnection failed. Try again?(Y/N)\n\n");
            ch = getchar();
            if(ch == 'Y' || ch == 'y'){
                clear_stdin();
                continue;
            }else if(ch == 'N' || ch == 'n'){
                clear_stdin();
                exit(EXIT_SUCCESS);
            }
        }
    }
    sleep(2);
    printf("\n\nSuccessfully Connect!\n\n");
    sleep(1);
    while(1){
        printf("\n\n****************************************************\n");
        printf("\n\nYou have following choices:\n");
        printf("A.update the online users and rooms.\n");
        printf("B.create a new room.\n");
        printf("C.remove a exist room.\n");
        printf("D.join in a room for chatting.\n");
        printf("Others: quit\n");
        printf("\n\n****************************************************\n\n");
        printf("Your choice: ");
        ch = getchar();
        clear_stdin();
        switch (ch)
        {
        case 'A':
        case 'a':
            upgrade(user_name);
            break;
        case 'B':
        case 'b':
            if(creatroom() == -1){
                printf("Failed to create room.\n");
                sleep(3);
            }else{
                printf("success to create room.\n");
                sleep(3);
            }
            break;
        case 'C':
        case 'c':
            if(removeroom() == -1){
                printf("Failed to remove room.\n");
                sleep(3);
            }else{
                printf("success to remove room.\n");
                sleep(3);
            }
            break;
        case 'D':
        case 'd':
            talk(user_name);
            break;
        default:
            exit(EXIT_SUCCESS);
        }
    }

    exit(EXIT_SUCCESS);
}