#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define RLT_SIZE 4
#define OPSZ 4
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	char opmsg[BUF_SIZE];
	int result, opnd_cnt, i;
	struct sockaddr_in serv_adr;
	if (argc != 3)
	{
		printf("Usage : %s <port> <ID>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[2]);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error!");
	
	fputs("Operand count: ", stdout);
	scanf("%d", &opnd_cnt);
	if(opnd_cnt <= 0 || opnd_cnt >= 128){ 
		write(sock, &opnd_cnt, 4);
		close(sock);
		return 0;
	}
	opmsg[0] = (char)opnd_cnt; //operand 개수 저장

	for (i = 0; i < opnd_cnt; i++)//operand를 입력받고, 저장.
	{
		printf("Operand %d: ", i);
		scanf("%d", (int *)&opmsg[i * OPSZ + 1]);
	}
	fgetc(stdin);
	for (i = 0; i < opnd_cnt - 1; i++)//operator를 입력받고, 저장
	{
		printf("Operator %d : ", i);
		scanf("%c", &opmsg[opnd_cnt * OPSZ + (i + 1)]);
		fgetc(stdin);
	}
	//server쪽으로 정보를 보냄.
	write(sock, opmsg, opnd_cnt * OPSZ + 1 + (opnd_cnt - 1));
	
	//server측에서 내준 결과를 받아온다.
	read(sock, &result, RLT_SIZE);

	printf("Operation result: %d \n", result);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}