/*此文件提供客户端所使用的所有的函数声明*/

#ifndef SV_MSQ_CLIENT_H
#define SV_MSQ_CLIENT_H

//错误退出函数
_Noreturn void errExit (const char * func);

//清空stdin的缓冲区
void clear_stdin(void);

//去掉字符串的换行符
void remove_chline(char *str);

//打印目前在线的用户与开启的房间
void printUsrAndRoom(const char *usr, const char *room);

//欢迎用户，接受用户姓名
void welcome(char *user_name);

//向服务器发送注册请求，返回0为连接成功，-1为连接失败
int connector(const char *user_name);

//更新在线用户信息和房间信息
void upgrade(const char *user_name);

//创建房间函数，成功返回0，不成功返回-1
int creatroom(void);

//移除房间函数，成功返回0，不成功返回-1
int removeroom(void);

//会话函数，查无此房输出-1，成功返回0
int talk(const char *user_name);

#endif