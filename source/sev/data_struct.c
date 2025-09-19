#include "sv_msq.h"
#include "sv_msq_sev.h"

//为在用户链表中按房间名查找结点的函数使用，保存每次调用该函数后下次调用开始的结点指针
static user_linknode * tempnode_for_user_find_by_room = NULL;

//为在用户链表中按房间名查找结点的函数使用，若为1，则表明下次调用函数是从头开始。
static volatile int is_ok = 1;

/*----------以下为用户链表数据结构操作，需与互斥量配合使用------------*/

//初始化一个用户信息链表，返回链表句柄
user_linklist  user_init_list(void){
    user_linklist ul;
    ul.firstnode = NULL;
    ul.usernum = 0;
    return ul;
}


//判断用户链表中是否有一个用户名为user_name
int user_is_exist(user_linklist ul, const char * user_name){
    user_linknode * temp = ul.firstnode;
    while(1){
        if(temp == NULL)
            return 0;
        if(strcmp(temp->name, user_name) == 0)
            return 1;
        temp = temp->next;
    }
}

//为新用户添加一个结点并返回该结点地址，使用前需判断用户数量是否已满，否则别的缓冲区将无法容纳，使用前保证用户不存在
user_linknode * user_add_by_name(user_linklist * ul, const char * user_name, int id){
    user_linknode * temp = ul->firstnode;
    user_linknode * ul_node = malloc(sizeof(user_linknode));
    ul_node->id = id;
    strcpy(ul_node->name, user_name);
    ul_node->next = NULL;
    if(temp == NULL){
        ul->firstnode = ul_node;
    }else{
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = ul_node;
    }
    ul->usernum ++;
    return ul_node;
}

//根据用户名称查找结点并返回结点指针，使用前保证用户存在
user_linknode * user_find_by_name(user_linklist ul, const char * user_name){
    user_linknode * temp = ul.firstnode;
    while(1){
        if(temp == NULL)
            return NULL;
        if(strcmp(temp->name, user_name) == 0)
            return temp;
        temp = temp->next;
    }
}

//根据房间名称查找结点并返回结点指针，需循环调用，若返回NULL，表明全部找完
user_linknode * user_find_by_room(user_linklist ul, const char * room_name){
    user_linknode * node_now = NULL;
    if(is_ok == 1){
        //对用户链表首次调用该函数，则从头结点开始
        //printf("根据房间名搜索用户的函数:首次调用\n");
        node_now = ul.firstnode;
    }else{
        //对用户链表不是首次调用该函数，则从tempnode_for_user_find_by_room开始
        //printf("根据房间名搜索用户的函数:非首次调用\n");
        node_now = tempnode_for_user_find_by_room;
    }
    is_ok = 0; //提示下一次调用就不是从头开始了
    //printf("根据房间名搜索用户的函数:用户链表非空，即将开始遍历\n");
    //开始从node_now遍历用户链表
    while(1){
        if(node_now == NULL){
            //为空结点，表明遍历结束
            //printf("根据房间名搜索用户的函数:到达链表末尾\n");
            is_ok = 1; //通知下一次从头开始
            tempnode_for_user_find_by_room = NULL;
            return NULL; //返回NULL通知调用者遍历结束
        }
        
        //正常有数据的结点
        if(strcmp(node_now->room, room_name) == 0){
            //找到
            //printf("根据房间名搜索用户的函数:找到一个用户%s\n", node_now->name);
            tempnode_for_user_find_by_room = node_now->next;
            return node_now;
        }
        //printf("该用户%s的房间%s不是我要找的.\n", node_now->name, node_now->room);
        node_now = node_now->next;
    }
}

//根据用户名称删除一个结点
void user_remove_by_name(user_linklist * ul, const char * user_name){
    if(ul->firstnode == NULL)
        return;

    if((ul->firstnode->next == NULL) && (strcmp(ul->firstnode->name, user_name) == 0)){
        //链表只有一个结点，该结点就是该用户
        free(ul->firstnode);
        ul->firstnode = NULL;
        ul->usernum = 0;
        return;
    }

    //链表有两个结点以上
    user_linknode * temp = ul->firstnode->next;
    user_linknode * previous = ul->firstnode;
    if(strcmp(previous->name, user_name) == 0){
        ul->firstnode = temp;
        free(previous);
    }
    
    while (temp != NULL)
    {
        if(strcmp(user_name, temp->name) == 0){
            previous->next = temp->next;
            free(temp);
            ul->usernum --;
            return;
        }else{
            temp = temp->next;
            previous = previous->next;
        }
    }
    return;
}

//销毁用户信息链表
void user_delete_linklist(user_linklist * ul){
    if(ul->firstnode == NULL)
        return;

    user_linknode * for_free;
    user_linknode * next_node = ul->firstnode;
    while(next_node != NULL){
        for_free = next_node;
        next_node = next_node->next;
        free(for_free);
    }
    return;
}

/*----------以下为房间链表数据结构操作，需与互斥量配合使用------------*/

//初始化一个房间链表，返回链表句柄
room_linklist room_init_list(void){
     room_linklist rll;
     rll.firstnode = NULL;
     rll.roomnum = 0;
     return rll;
}

//判断房间链表中是否有一个房间名为room_name
int room_is_exist(room_linklist rl, const char * room_name){
    room_linknode * temp = rl.firstnode;
    while(1){
        if(temp == NULL)
            return 0;
        if(strcmp(temp->room, room_name) == 0)
            return 1;
        temp = temp->next;
    }
}

//删除一个名为room_name的结点，不管存不存在都成功
void room_remove_by_name(room_linklist * rl, const char * room_name){
    if(rl->firstnode == NULL)
        return;

    if((rl->firstnode->next == NULL) && (strcmp(rl->firstnode->room, room_name) == 0)){
        //链表只有一个结点，该结点就是该房间
        free(rl->firstnode);
        rl->firstnode = NULL;
        rl->roomnum = 0;
        return;
    }

    //链表有两个结点以上
    room_linknode * temp = rl->firstnode->next;
    room_linknode * previous = rl->firstnode;
    if(strcmp(previous->room, room_name) == 0){
        rl->firstnode = temp;
        free(previous);
    }

    while (temp != NULL)
    {
        if(strcmp(room_name, temp->room) == 0){
            previous->next = temp->next;
            free(temp);
            rl->roomnum --;
            return;
        }else{
            temp = temp->next;
            previous = previous->next;
        }
    }
    return;
}

////增加一个名为room_name的结点（需先调用room_is_exist，否则将创建重名的房间）
void room_add_by_name(room_linklist *rl, const char * room_name){
    if(rl->firstnode == NULL){
        rl->firstnode = malloc(sizeof(room_linknode));
        strcpy(rl->firstnode->room, room_name);
        rl->firstnode->next = NULL;
    }else{
        room_linknode * node = rl->firstnode;
        while(node->next != NULL){
            node = node->next;
        }
        node->next = malloc(sizeof(room_linknode));
        strcpy(node->next->room, room_name);
        node->next->next = NULL;
    }
    rl->roomnum ++;
}

//销毁一个房间链表
void room_delete_linklist(room_linklist * rl){
    if(rl->firstnode == NULL)
        return;

    room_linknode * for_free;
    room_linknode * next_node = rl->firstnode;
    while(next_node != NULL){
        for_free = next_node;
        next_node = next_node->next;
        free(for_free);
    }
    return;
}

/*----------以下为聊天消息队列数据结构操作，需与互斥量配合使用------------*/

//初始化一个聊天消息队列，返回聊天消息队列句柄
saying_queue  saying_init_queue(void){
    saying_queue sq;
    sq.base = calloc(SAYING_QUEUE_LEN, sizeof(saying_queue_node));
    sq.front = 0;
    sq.rear = 0;
    return sq;
}

//消息入队，不返回成功失败消息，队列满就丢包
void saying_enter_queue(saying_queue * sq, saying_queue_node *sqn){
    //printf("入队函数:开始\n");
    if(((sq->front + 1) % SAYING_QUEUE_LEN) == sq->rear){
        //printf("入队函数:队列已满\n");
        return;
    }
    //printf("入队函数:队列未满,可以开始\n");
    strcpy(sq->base[sq->rear].room, sqn->room);
    strcpy(sq->base[sq->rear].info, sqn->info);
    strcpy(sq->base[sq->rear].user, sqn->user);
    //printf("入队函数:新增一个结点,用户为%s,房间为%s,聊天消息为%s\n", sq->base[sq->rear].user, sq->base[sq->rear].room, sq->base[sq->rear].info);
    sq->rear = (sq->rear + 1) % SAYING_QUEUE_LEN;
}

//消息出队，使用前需调用saying_is_empty检查是否为空
saying_queue_node saying_leave_queue(saying_queue * sq){
    saying_queue_node sqn;
    sqn = sq->base[sq->front];
    sq->front = (sq->front + 1) % SAYING_QUEUE_LEN;
    //printf("出队函数:成功出队一个结点,聊天消息为%s,用户为%s,房间为%s.\n", sqn.info, sqn.user, sqn.room);
    return sqn;
}

//判断队列是否为空，1为空，0为不空
int saying_is_empty(saying_queue sq){
    if(sq.front == sq.rear)
        return 1;
    else
        return 0;
}

//销毁一个聊天消息队列
void saying_delete_queue(saying_queue * sq){
    free(sq->base);
    sq->front = 0;
    sq->rear = 0;
}