#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024
#define RLT_SIZE 4
#define OPSZ 4
int SO_BRD = 1;
void error_handling(char *message);
void broad_func(int p_num, char *message);
_Bool isregister = 0;

int main(int argc, char *argv[])
{

	if (argc != 1)
	{
		printf("Usage : %s ./client \n", argv[0]);
		exit(1);
	}

	
		puts("Start to find calc server");

		int cli_sock;
		struct sockaddr_in cli_adr;
		char cli_buf[BUF_SIZE] = {0,};
		int str_len;

		int enable = 1;
		setsockopt(cli_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

		//client에서 연결후에 8080으로 메세지를 보낸다.
		broad_func(8080,"client");

        cli_sock = socket(PF_INET, SOCK_DGRAM, 0); // udp 소켓 생성 포트번호 8082
		memset(&cli_adr, 0, sizeof(cli_adr));
		cli_adr.sin_family = AF_INET;
		cli_adr.sin_addr.s_addr = htonl(INADDR_ANY);
		cli_adr.sin_port = htons(8082);

		if (bind(cli_sock, (struct sockaddr *)&cli_adr, sizeof(cli_adr)) == -1)
			error_handling("bind() error\n");

		str_len = recvfrom(cli_sock, cli_buf, BUF_SIZE - 1, 0, NULL, 0);
        if(!strcmp(cli_buf,"fail")){ //calc 포트번호가 discovery에 등록되지 않은 상태
            printf("Fail\n");
            close(cli_sock);
            return 0;
        } else{ //서버 찾음, tcp소켓으로 연결 필요
            printf("Found calc server %s\n", cli_buf);

            int sock;
            char opmsg[30];
            int result, opnd_cnt, i;
            struct sockaddr_in serv_adr;

            sock = socket(PF_INET, SOCK_STREAM, 0);
            if (sock == -1)
                error_handling("socket() error");

            memset(&serv_adr, 0, sizeof(serv_adr));
            serv_adr.sin_family = AF_INET;
            serv_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
            serv_adr.sin_port = htons(atoi(cli_buf));

            if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
                error_handling("connect() error!");

            fputs("Operand count: ", stdout);
            scanf("%d", &opnd_cnt);
            if (opnd_cnt <= 0 || opnd_cnt >= 128)
            {
                write(sock, &opnd_cnt, 4);
                close(sock);
                return 0;
            }
            opmsg[0] = (char)opnd_cnt;

            for (i = 0; i < opnd_cnt; i++)
            {
                printf("Operand %d: ", i);
                scanf("%d", (int *)&opmsg[i * OPSZ + 1]);
            }
            fgetc(stdin);
            for (i = 0; i < opnd_cnt - 1; i++)
            {
                printf("Operator %d : ", i);
                scanf("%c", &opmsg[opnd_cnt * OPSZ + (i + 1)]);
                fgetc(stdin);
            }
            write(sock, opmsg, opnd_cnt * OPSZ + 1 + (opnd_cnt - 1));
            read(sock, &result, RLT_SIZE);

            printf("Operation result: %d \n", result);
            close(sock);
            return 0;
        }
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

//브로드 캐스트 메세지를 보내기 위한 함수
void broad_func(int p_num, char *message)
{
	int bro_sock;
	struct sockaddr_in bro_adr;
	char bro_buf[BUF_SIZE] = {0,};
	
	strcpy(bro_buf, message);

	bro_sock = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&bro_adr, 0, sizeof(bro_adr));

	bro_adr.sin_family = AF_INET;
	bro_adr.sin_addr.s_addr = inet_addr("255.255.255.255");
	bro_adr.sin_port = htons(p_num);
	
	setsockopt(bro_sock, SOL_SOCKET, SO_BROADCAST, (void*)&SO_BRD, sizeof(SO_BRD));
	sendto(bro_sock, bro_buf, strlen(bro_buf), 0, (struct sockaddr*)&bro_adr, sizeof(bro_adr));
    
    close(bro_sock);
}