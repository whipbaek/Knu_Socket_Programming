#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);
void broad_func(int p_num, char *message);
_Bool isregister = 0;
int SO_BRD = 1;
int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		printf("Usage : %s ./discovery or ./calc \n", argv[0]);
		exit(1);
	}

	if (!strcmp(argv[1], "discovery"))
	{
		puts("Discovery Server operating...");

		int disc_sock, disc_broad_sock;
		struct sockaddr_in disc_adr, disc_broad_adr;
		char calc_broad_buf[BUF_SIZE] = {0,};
		char disc_broad_buf[BUF_SIZE] = {0,};
		int so_brd = 1;
		int str_len;

		int enable = 1;
		setsockopt(disc_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		setsockopt(disc_broad_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

		disc_sock = socket(PF_INET, SOCK_DGRAM, 0); // udp 소켓 생성 포트번호 8080
		memset(&disc_adr, 0, sizeof(disc_adr));
		disc_adr.sin_family = AF_INET;
		disc_adr.sin_addr.s_addr = htonl(INADDR_ANY);
		disc_adr.sin_port = htons(8080);

		if (bind(disc_sock, (struct sockaddr *)&disc_adr, sizeof(disc_adr)) == -1)
			error_handling("bind() error\n");
	
		str_len = recvfrom(disc_sock, disc_broad_buf, BUF_SIZE - 1, 0, NULL, 0);
		if(!strcmp(disc_broad_buf,"client")){
			if(isregister){
				broad_func(8082,disc_broad_buf);
			}
			else{
				broad_func(8082,"fail");
				return 0;
			}
		} else{
		printf("Calc Server registered(%s)\n", disc_broad_buf);
			isregister=1;
		}

		//success or fail 메세지를 브로드캐스트로 8081 포트로 보낸다.

		if (isregister) broad_func(8081,"success");	
		else broad_func(8081,"fail");

		char new_disc_broad_buf[BUF_SIZE] = {0,};

		str_len = recvfrom(disc_sock, new_disc_broad_buf, BUF_SIZE -1, 0, NULL, 0);
		if(!strcmp(new_disc_broad_buf,"client")){
			if(isregister){
				broad_func(8082,disc_broad_buf);
			}
			else{
				broad_func(8082,"fail");
			}
		}

	}

	else if (!strcmp(argv[1], "calc"))
	{
		puts("Register calc server");

		int calc_broad_sock, calc_sock;
		struct sockaddr_in calc_broad_adr, calc_adr;
		char calc_broad_buf[BUF_SIZE] = {
			0,
		};
		char calc_buf[BUF_SIZE] = {
			0,
		};
		int so_brd = 1;
		int str_len;

		int enable = 1;
		setsockopt(calc_broad_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		setsockopt(calc_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

		// 8080으로 브로드캐스트 데이터를 보낸다.

		int rannum = (rand() % 50000) + 10000; // 10000에서 50000사이 값을 구함
		sprintf(calc_broad_buf, "%d", rannum);

		broad_func(8080,calc_broad_buf);
		
		// success 메세지를 받는다.

		calc_sock = socket(PF_INET, SOCK_DGRAM, 0); // udp 소켓 생성 포트번호 8081
		memset(&calc_adr, 0, sizeof(calc_adr));
		calc_adr.sin_family = AF_INET;
		calc_adr.sin_addr.s_addr = htonl(INADDR_ANY);
		calc_adr.sin_port = htons(8081);

		if (bind(calc_sock, (struct sockaddr *)&calc_adr, sizeof(calc_adr)) == -1)
			error_handling("bind() error\n");

		str_len = recvfrom(calc_sock, calc_buf, BUF_SIZE - 1, 0, NULL, 0);

		if(!strcmp(calc_buf,"success")){ //success 면 소켓 클라이언트와 연결할 소켓 구동
			printf("Calc Server %d operating ...\n", rannum);

			int serv_sock, clnt_sock;
			int opinfo[BUF_SIZE]; // operation information array
			char operationinfo[BUF_SIZE];
			int result, opnd_cnt, i;
			int recv_cnt, recv_len;
			struct sockaddr_in serv_adr, clnt_adr;
			socklen_t clnt_adr_sz;

			serv_sock = socket(PF_INET, SOCK_STREAM, 0); //서버쪽 소켓
			if (serv_sock == -1)
				error_handling("socket() error");

			int enable = 1;
			setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

			// initialize
			memset(&serv_adr, 0, sizeof(serv_adr));
			serv_adr.sin_family = AF_INET;
			serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
			serv_adr.sin_port = htons(rannum);

			if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
				error_handling("bind() error");
			if (listen(serv_sock, 5) == -1) // 5 means size of Queue
				error_handling("listen() error");
			clnt_adr_sz = sizeof(clnt_adr);

			for (int i = 0; i < 5; i++)
			{
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
					printf("Operator %d : %c\n", i, operationinfo[i]);
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
				printf("closed client : %d\n",getpid());
				close(clnt_sock);
			}

			close(serv_sock);
			return 0;
		} else{
			printf("Fail");
			close(calc_sock);
		}
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