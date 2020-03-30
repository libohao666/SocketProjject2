/*************************************************************************
	> File Name: client.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 日  3/29 18:16:00 2020
 ************************************************************************/

#include "../common/chatroom.h"
#include "../common/tcp_client.h"
#include "../common/common.h"
#include "../common/color.h"

char *conf = "./client.conf";
int sockfd;

void *logout(int signalnum) {
    close(sockfd);
    exit(1);
    printf("recv a signal");
}


int main() {
    system("clear");
    int port;
    struct Msg msg;
    char ip[20] = {0};
    port = atoi(get_value(conf, "SERVER_PORT"));
    strcpy(ip, get_value(conf, "SERVER_IP"));
    printf("ip = %s , port = %d\n", ip, port);
    
    if ((sockfd = socket_connect(ip, port)) < 0) {
        perror("socket_connect");
        return 1;
    }
    strcpy(msg.from, get_value(conf, "MY_NAME"));
    printf("%s\n", msg.from);
    msg.flag = 2;
    if (chat_send(msg, sockfd) < 0) {
        return 2;
    }
    
    struct RecvMsg rmsg = chat_recv(sockfd);
    
    if (rmsg.retval < 0) {
        fprintf(stderr, "Error!\n");
        return 1;
    }

    printf(GREEN"Server "NONE": %s\n", rmsg.msg.message);

    if (rmsg.msg.flag == 3) {
        close(sockfd);
        return 1;
    }

    pid_t pid;
    int x;
    for (int i = 0; i < 2; i++) {
        if ((pid = fork()) < 0){
            perror("fork");
        }
        if (pid == 0) {
            x = i;
            break;
        }
    }
    if (pid == 0 && x == 0) {
        signal(SIGINT, logout);
        while (1) {
            char tmp[533];
            scanf("%[^\n]s", tmp);
            getchar();
            int len = 0;
            if (tmp[0] == '@') {
                msg.flag = 1;
                while (tmp[len] != ' ' && len < strlen(tmp)) {
                    len++;
                }
                strncpy(msg.to, tmp + 1, len - 1);
                msg.to[len - 1] = '\0';
            }
            else {
                msg.flag = 0;
            }
            strcpy(msg.message, tmp + len);
            chat_send(msg, sockfd);
            memset(msg.message, 0, sizeof(msg.message));
        }
    } 
    else if (pid == 0 && x == 1) {
        while(1) {
            rmsg = chat_recv(sockfd);
			FILE *fp;
			fp=fopen("./Log_File","a");//参数a表示追加写入
            fprintf(fp, BLUE"[%s]"NONE, rmsg.msg.from);
            if (rmsg.msg.flag == 1) {
                fprintf(fp, "私聊 %s :%s\n", rmsg.msg.to, rmsg.msg.message);
            }
            else {
     	       fprintf(fp, "公聊:%s\n", rmsg.msg.message);
            }
			fclose(fp);
            system("clear");
            system("tail -10 ./Log_File");
            printf(L_PINK"Please Input Message:"NONE"\n");
        }
    } 
    else{
        wait(NULL);
        close(sockfd);
    }
    return 0;
}


