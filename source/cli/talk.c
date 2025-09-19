#include "sv_msq_client.h"
#include "sv_msq.h"
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>

extern int cli_msq_id;
WINDOW *top_win = NULL;
WINDOW *bottom_win = NULL;
pthread_t recver, sender;
struct type6 t6;
static struct type8 t8;

/* 线程安全锁 */
static pthread_mutex_t win_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 信号处理：窗口尺寸变化 */
void sigwinch_handler(int sig) {
    endwin();
    refresh();
    clear();
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    pthread_mutex_lock(&win_mutex);
    if(top_win) delwin(top_win);
    if(bottom_win) delwin(bottom_win);
    
    // 创建自适应窗口[1](@ref)
    top_win = newwin(height - 4, width, 0, 0);
    bottom_win = newwin(4, width, height -4, 0);
    
    box(top_win, 0, 0);
    box(bottom_win, 0, 0);
    wrefresh(top_win);
    wrefresh(bottom_win);
    pthread_mutex_unlock(&win_mutex);
}

/* 接收消息线程 */
static void *receive_thread(void *arg) {
    struct type7 t7;
    int max_y, max_x;
    
    while(1) {
        if(msgrcv(cli_msq_id, &t7, T7SIZE, TYPE7, 0) == -1)
            errExit("msgrcv");

        pthread_mutex_lock(&win_mutex);
        getmaxyx(top_win, max_y, max_x);
        
        if(t7.is_chat) {
            // 支持宽字符输出[1](@ref)
            wprintw(top_win, "[%s] %s\n", t7.name, t7.saying);
            wrefresh(top_win);
        } else {
            mvwprintw(top_win, max_y-1, 1, "房间已关闭，按任意键退出...");
            wrefresh(top_win);
            pthread_cancel(sender);
            break;
        }
        pthread_mutex_unlock(&win_mutex);
    }
    return NULL;
}

/* 发送消息线程 */
static void *send_thread(void *arg) {
    char ch;
    char saying[SAY_MAX + 1] = {0};
    int index = 0;
    const char *username = (char*)arg;

    strcpy(t6.name, username);
    t6.type = TYPE6;
    t8.type = TYPE8;

    while(1) {
        pthread_mutex_lock(&win_mutex);
        werase(bottom_win);
        box(bottom_win, 0, 0);
        mvwprintw(bottom_win, 1, 1, "@%s> ", username); // 固定提示符位置
        wmove(bottom_win, 1, strlen(username)+3);       // 光标定位到输入起始点[1](@ref)
        wrefresh(bottom_win);
        pthread_mutex_unlock(&win_mutex);

        index = 0;
        memset(saying, 0, sizeof(saying));
        while(index < SAY_MAX) {
            ch = wgetch(bottom_win);
            
            if(ch == '\n' || ch == '\r') { 
                // 发送消息
                strcpy(t6.message, saying);
                if(msgsnd(cli_msq_id, &t6, T6SIZE, 0) == -1)
                    errExit("msgsnd");
                break;
            } else if(ch == KEY_BACKSPACE || ch == 127) { 
                // 退格处理[3](@ref)
                if(index > 0) {
                    index--;
                    saying[index] = '\0';
                    mvwaddch(bottom_win, 1, strlen(username)+3 + index, ' ');
                    wrefresh(bottom_win);
                }
            } else if(isprint(ch)) { 
                // 可打印字符检测[1](@ref)
                saying[index] = ch;
                mvwaddch(bottom_win, 1, strlen(username)+3 + index, ch);
                index++;
                wrefresh(bottom_win);
            }

            // 退出检测
            if(index >=4 && strncmp(saying + index -4, "!exit",4) == 0) {
                pthread_cancel(recver);
                if(msgsnd(cli_msq_id, &t8, T8SIZE, 0) == -1)
                    errExit("msgsnd");
                return NULL;
            }
        }
    }
}

/* 主会话函数 */
int talk(const char *user_name) {
    struct type5 t5 = {.type = TYPE5, .operation = ADDIN_ROOM};
    struct type2 t2;

    // 获取房间名称
    printf("输入要加入的房间名：");
    fgets(t5.room, ROOMLEN_MAX+1, stdin);
    remove_chline(t5.room);

    // 发送加入请求
    if(msgsnd(cli_msq_id, &t5, T5SIZE, 0) == -1)
        errExit("msgsnd");

    // 等待响应
    if(msgrcv(cli_msq_id, &t2, T2SIZE, TYPE2, 0) != T2SIZE)
        errExit("msgrcv");
    if(!t2.is_success) return -1;

    signal(SIGWINCH, sigwinch_handler);
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);

    // 创建窗口
    int height, width;
    getmaxyx(stdscr, height, width);
    top_win = newwin(height -4, width, 0, 0);
    bottom_win = newwin(4, width, height -4, 0);
    box(top_win, 0, 0);
    box(bottom_win, 0, 0);
    wrefresh(top_win);
    wrefresh(bottom_win);

    // 启动线程
    pthread_create(&recver, NULL, receive_thread, NULL);
    pthread_create(&sender, NULL, send_thread, (void*)user_name);

    // 等待线程结束
    pthread_join(recver, NULL);
    pthread_join(sender, NULL);

    // 清理资源
    pthread_mutex_destroy(&win_mutex);
    delwin(top_win);
    top_win = NULL;
    delwin(bottom_win);
    bottom_win = NULL;
    endwin();
    return 0;
}