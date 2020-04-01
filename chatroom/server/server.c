#include "../common/common.h"
#include "../common/tcp_server.h"
#include "../common/chatroom.h"
#include "../common/color.h"

struct User{
    char name[20];
    int online;
    pthread_t tid;
    int fd;
};


char *conf = "./server.conf";

struct User *client;

void recv_file(char *filename, int sockfd) {
    char buf[MAX_LINE];
    int fd_out;
    int nread, nwrite;
    
    if ((fd_out = creat(filename, 0644)) == -1) {
        perror(filename);
        return ;
    }
    while ((nread = read(sockfd, buf, sizeof(buf))) > 0) {
        int nwrite;
        if ((nwrite = write(fd_out, buf, nread)) != nread) {
            perror("write");
            exit(1);
        }
        memset(buf, 0, sizeof(buf));
        if (buf[nread] == '\0') break;
    }
    printf("end\n");
    close(fd_out); 
}


void *work(void *arg){
    int sub = *(int *)arg;
    int client_fd = client[sub].fd;
    struct RecvMsg rmsg;
    printf(GREEN"Login "NONE" : %s\n", client[sub].name);
    while (1) {
        rmsg = chat_recv(client_fd);
        if (rmsg.retval < 0) {
            printf(PINK"Logout: "NONE" %s \n", client[sub].name);
            close(client_fd);
            client[sub].online = 0;
            return NULL;
        }
        printf(BLUE"[%s]"NONE, rmsg.msg.from);
        if (rmsg.msg.message[0] == '#') {
            if (rmsg.msg.message[1] == '1') {
                int i = 0;
                i += sprintf(rmsg.msg.message + i, "在线名单如下:\n");
                for (int j = 0; j < MAX_CLIENT; j++) {
                    if (client[j].online)
                    i += sprintf(rmsg.msg.message + i, "%s\n", client[j].name);
                }
                chat_send(rmsg.msg, client_fd);
            }
            else if (rmsg.msg.message[1] == '2') {
                printf("收文件\n");
				char filename[20] = {0};
                int len = 0;
                for (int i = 0; i < strlen(rmsg.msg.message); i++) {
                    if (rmsg.msg.message[i] == ' ') break;
                    len++;
                }
                strcpy(filename, rmsg.msg.message + len + 1);
                printf("filename:%s\n", filename);
                recv_file(filename, client_fd);
            }
        }
        else if (rmsg.msg.flag == 1) {
            printf(RED"私聊"NONE" %s :%s\n", rmsg.msg.to, rmsg.msg.message);
            for (int i = 0; i < MAX_CLIENT; i++) {
                if (client[i].online && !strcmp(rmsg.msg.to, client[i].name)) {
                    chat_send(rmsg.msg, client[i].fd);
                    break;
                }
            }
        }
        else {
            printf(RED"公聊"NONE" :%s\n", rmsg.msg.message);
            for (int i = 0; i < MAX_CLIENT; i++) {
                if (client[i].online) {
                    chat_send(rmsg.msg, client[i].fd);
                }
            }
        }
    }
    return NULL;
}


int find_sub() {
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (!client[i].online) return i;
    }
    return -1;
}

bool check_online(char *name) {
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (client[i].online && !strcmp(name, client[i].name)) {
            printf(YELLOW"W"NONE": %s is online\n", name);
            return true;
        }
    }
    return false;
}


int main() {
    int port, server_listen, fd;
    struct RecvMsg recvmsg;
    struct Msg msg;
    port = atoi(get_value(conf, "SERVER_PORT"));
    client = (struct User *)calloc(MAX_CLIENT, sizeof(struct User));

    if ((server_listen = socket_create(port)) < 0) {
        perror("socket_create");
        return 1;
    }
    while (1) { 
        if ((fd = accept(server_listen, NULL, NULL)) < 0) {
            perror("accept");
            continue;
        }
        
        recvmsg = chat_recv(fd);
        if (recvmsg.retval < 0) {
            close(fd);
            continue;
        }
        if (check_online(recvmsg.msg.from)) {
            msg.flag = 3;
            strcpy(msg.message, "You have Already Login System!");
            chat_send(msg, fd);
            close(fd);
            continue;
            //拒绝连接：
        } 
        msg.flag = 2;
        strcpy(msg.message, "Welcome to this chat room!");
        chat_send(msg, fd);

        int sub;
        sub = find_sub();
        client[sub].online = 1;
        client[sub].fd =fd;
        strcpy(client[sub].name, recvmsg.msg.from);
        pthread_create(&client[sub].tid, NULL, work, (void *)&sub);
    }
    return 0;
}
