#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
void error_handling(char *message);
void read_childproc(int sig);
void exitinput(int sig);
int forexit;
int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int fds[2];
	int fds2[2];

	pid_t pid;
	struct sigaction act;
	socklen_t adr_sz;
	int str_len, state;

	int opnd_cnt = 129;
	int opinfo[BUF_SIZE];
	char operationinfo[BUF_SIZE];

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	int enable = 1;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	// zombie process handling
	act.sa_handler = read_childproc; // chlid process가 종료되면 실행될 함수
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	state = sigaction(SIGCHLD, &act, 0);

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)
		puts("Error");

	pipe(fds); //파이프 생성
	pipe(fds2);

	pid = fork(); //자식 프로세스 생성
	if (pid == 0) //자식 프로세스에서 할 일
	{
		FILE *fp = fopen("log.txt", "wb");
		char msgbuf[BUF_SIZE]="";
		char ch;
		int i, len;
	
		while(1)
		{
			read(fds[0], msgbuf, sizeof(msgbuf));
			if(strlen(msgbuf)<=3) {
				break;
			}
			char * newbuf = msgbuf;
			fwrite(newbuf, strlen(newbuf), 1, fp);
			fwrite("\n",1,1,fp);
		}
		fclose(fp);
		return 0;
	}

	while (1) // process를 받는곳
	{
		adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz); // client측 소켓을 연결함.
		if (clnt_sock == -1)
			continue;
		else
			puts("new client connected...");

		pid = fork();
		if (pid == 0) //서비스를 제공하는곳, 자식 프로세스
		{
			close(serv_sock);

			char buf[BUF_SIZE];
			char *temp;
			char forid[BUF_SIZE];

			sprintf(buf, "%d: ", getpid());

			opnd_cnt = 0;

			read(clnt_sock, &opnd_cnt, 1); // read num of operand count
			if (opnd_cnt <= 0 || opnd_cnt>128)
			{
				printf("Save File(%d)\n",opnd_cnt);
				sprintf(buf,"%d",opnd_cnt);
				write(fds[1], buf, sizeof(buf));
			}
			else
			{
				for (int i = 0; i < opnd_cnt; i++)
				{
					read(clnt_sock, &opinfo[i], 4);
				}

				for (int i = 0; i < opnd_cnt - 1; i++)
				{
					read(clnt_sock, &operationinfo[i], 1);
				}

				int res = opinfo[0];
				sprintf(forid, "%d", opinfo[0]);
				strcat(buf, forid);
				memset(forid, 0, sizeof(forid));
				strncat(buf, temp, 1);

				int j = 0;
				int area = 1;
				while (j < opnd_cnt - 1)
				{
					//연산자 정보 붙이기
					temp[0] = operationinfo[j];
					strncat(buf, temp, 1);
					memset(temp, 0, sizeof(temp));

					//피연산자 정보 붙이기
					sprintf(forid, "%d", opinfo[area]);
					strcat(buf, forid);
					memset(forid, 0, sizeof(forid));
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

				sprintf(forid, "=%d", res);
				strcat(buf, forid);
				memset(forid, 0, sizeof(forid));
				printf("%s\n",buf);
				write(clnt_sock, &res, 4);
				write(fds[1], buf, sizeof(buf));
			}

			close(clnt_sock);
			return 0;
		}
		else
			close(clnt_sock);
	}
	close(serv_sock);
	return 0;
}

void read_childproc(int sig) // chlid process가 종료될때 실행될 함수
{
	pid_t pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG);
	printf("removed proc id: %d \n", pid);
}
void error_handling(char *message)
{
	fputc('\n', stderr);
	exit(1);
}