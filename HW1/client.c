#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char ** argv){
    int sockfd;
    char buf[1024];
    char *ID = "2019118024";
    struct sockaddr_in servaddr;

    if(argc<3){
        printf("usage:./client remoteAddress remotePort");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket creation failed");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[2]);
    servaddr.sin_port = htons(atoi(argv[1]));

    printf("%s\n",ID);
    
    if(connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0 ){
        perror("connet error");
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    write(sockfd, ID, strlen(ID));
    read(sockfd, buf, sizeof(buf));
    printf("%s\n",buf);

    close(sockfd);

    return 0;
}