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
    char opinfo[BUF_SIZE]; // get all 
	int result, opnd_cnt, i;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	if (argc != 2)
	{ // argument가 적게 들어왔다면
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_DGRAM, 0); //서버쪽 소켓
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

	clnt_adr_sz = sizeof(clnt_adr);

	for(int i=0; i<5; i++){
															 
        recvfrom(serv_sock, &opinfo, BUF_SIZE, 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        
        opnd_cnt = opinfo[0];
		if (opnd_cnt <= 0 || opnd_cnt >= 128)
		{
			char check = 0;
			check = opnd_cnt;
			printf("Server close(%d)\n", check);
			close(clnt_sock);
			close(serv_sock);
			return 0;
		}
        printf("Operand count : %d\n",opnd_cnt);
        int tempidx = 0;
        int opPoint = OPSZ *opnd_cnt;
        for(int i=1; i<=opPoint; i+=4){
            printf("Operand %d : %d\n",tempidx,opinfo[i]);
            tempidx++;
        }
        tempidx = 0;
        opPoint +=1;
        
        for(int i= opPoint; i< opPoint + (opnd_cnt - 1); i++){
             printf("Operator %d : %c\n",tempidx,opinfo[i]);
            tempidx++;
        }

		int res = opinfo[1];
		int area = 5;
		while (opnd_cnt != 1)
		{
			switch (opinfo[opPoint])
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
			opnd_cnt--;
            opPoint++;
			area+=4;
		}
		printf("Operation result : %d\n", res);
        sendto(serv_sock, &res, OPSZ, 0, 
								(struct sockaddr*)&clnt_adr, clnt_adr_sz);
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