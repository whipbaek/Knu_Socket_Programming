#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define OPSZ 4
void error_handling(char *message);
int calculate(int opnum, int opnds[], char oprator);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	int opinfo[BUF_SIZE]; // operation information array
	char operationinfo[BUF_SIZE];
	int result, opnd_cnt, i;
	int recv_cnt, recv_len;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	if (argc != 2)
	{ // argument가 적게 들어왔다면
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0); //서버쪽 소켓
	if (serv_sock == -1)
		error_handling("socket() error");

	int enable = 1;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	// initialize
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1) // 5 means size of Queue
		error_handling("listen() error");
	clnt_adr_sz = sizeof(clnt_adr);

	for(int i=0; i<5; i++){
		opnd_cnt = 0;															   // num of operand
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz); // accept client
		read(clnt_sock, &opnd_cnt, 1);											   // read num of operand count
		if (opnd_cnt <= 0 || opnd_cnt >= 128)
		{
			char check = 0;
			check = opnd_cnt;
			printf("Server close(%d)\n", check);
			close(clnt_sock);
			close(serv_sock);
			return 0;
		}
		printf("Operand count : %d\n", opnd_cnt);

		recv_len = 0;

		for (int i = 0; i < opnd_cnt; i++)
		{
			read(clnt_sock, &opinfo[i], 4);
			printf("Operand %d : %d\n", i, opinfo[i]);
		}
		int operator;
		for (int i = 0; i < opnd_cnt - 1; i++)
		{
			read(clnt_sock, &operationinfo[i], 1);
			printf("Operator %d : %c\n",i,operationinfo[i]);
		}

		int res = opinfo[0];
		int j = 0;
		int area = 1;
		while (j < opnd_cnt - 1)
		{
			switch (operationinfo[j])
			{
			case '+':
				res += opinfo[area];
				break;
			case '-':
				res -= opinfo[area];
				break;
			case '*':
				res *= opinfo[area];
				break;
			}
			j++;
			area++;
		}
		printf("Operation result : %d\n", res);
		write(clnt_sock, &res, 4);
		close(clnt_sock);
	}

	close(serv_sock);
	return 0;
}


void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}