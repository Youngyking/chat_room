#!/bin/bash

# 定义项目根目录路径
PROJECT="/root/Myprogram/local_chat_room"

#-----------------------------------------------------客户端-----------------------------------------------------------
# 进入客户端源码目录
cd $PROJECT/source/cli/

# 编译目标文件
gcc -c connector.c removeroom.c tool.c welcome.c creatroom.c talk.c upgrade.c -fPIC

# 生成共享库
gcc -shared -o $PROJECT/lib/libcliFunc.so.1.1 *.o -lncursesw -L$PROJECT/lib/ -Wl,-rpath,'$ORIGIN'/

# 清理目标文件
rm -f *.o

# 进入库目录
cd $PROJECT/lib

# 清楚旧符号链接
rm -f libcliFunc.so.1 libcliFunc.so

# 创建符号链接
ln -s libcliFunc.so.1.1 libcliFunc.so.1
ln -s libcliFunc.so.1 libcliFunc.so

# 进入源码目录
cd $PROJECT/source/cli/

# 编译客户端程序
gcc sv_msq_client.c -lpthread -lerrFunc -lcliFunc -lncursesw -L$PROJECT/lib/ -Wl,-rpath,'$ORIGIN'/../lib -o $PROJECT/exec/sv_msq_client

#------------------------------------------------------服务端------------------------------------------------------

# 进入服务端源码目录
cd $PROJECT/source/sev/

# 编译目标文件
gcc -c data_struct.c thread* -fPIC

# 生成共享库
gcc -shared -o $PROJECT/lib/libsevFunc.so.1.1 *.o

# 清理目标文件
rm -f *.o

# 进入库目录
cd $PROJECT/lib

# 清楚旧符号链接
rm -f libsevFunc.so.1 libsevFunc.so

# 创建符号链接
ln -s libsevFunc.so.1.1 libsevFunc.so.1
ln -s libsevFunc.so.1 libsevFunc.so

# 进入源码目录
cd $PROJECT/source/sev/

# 编译客户端程序
gcc sv_msq_sev.c -lpthread -lerrFunc -lsevFunc -L$PROJECT/lib/ -Wl,-rpath,'$ORIGIN'/../lib -o $PROJECT/exec/sv_msq_server
