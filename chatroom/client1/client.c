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

int send_file(char *filename, int sockfd) {
    int fd_in, nread;
	if ((fd_in = open(filename, O_RDONLY)) == -1) {
        perror(filename);
        return -1;
    }
    char buf[MAX_LINE];
	
	while ((nread = read(fd_in, buf, sizeof(buf))) > 0) {
        int nwrite;
        if ((nwrite = write(sockfd, buf, nread)) != nread) {
            perror("write");
            exit(1);
        }
        memset(buf, 0, sizeof(buf));
    } 
    write(sockfd, '\0', sizeof('\0'));
    printf("end\n");
    close(fd_in);
    return 0;
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
            else if (tmp[0] == '#' && tmp[1] == '2') {
                strcpy(msg.message, tmp + len);
                chat_send(msg, sockfd);
                char filename[20] = {0};
                for (int i = 0; i < strlen(msg.message); i++) {
                    if (msg.message[i] == ' ') break;
                    len++;
                                                                               
                }
                strcpy(filename, msg.message + len + 1);
                printf("filename:%s\n", filename);
                send_file(filename, sockfd);
                continue;
            }
            else{
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
			fp=fopen("./Log_File","a");
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


