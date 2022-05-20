#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/uio.h>
	
#define BUF_SIZE 100
#define NAME_SIZE 20
#define OPSZ 4
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
	
char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];
char * ID;
char opmsg[255];

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread; //2가지의 쓰레드를 생성
	void * thread_return;
	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	sprintf(name, "[%s]", argv[3]);
	ID = argv[3];
	sock=socket(PF_INET, SOCK_STREAM, 0);
	

	// 소켓 연결
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	//스레드 생성

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return); // 쓰레드의 종료를 대기
	pthread_join(rcv_thread, &thread_return); // 쓰레드의 종료를 대기
	close(sock);  
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg); //소켓 받아옴
	char name_msg[NAME_SIZE+BUF_SIZE];
	while(1) 
	{
		struct iovec vec[2];
		
		vec[0].iov_base = ID; // argv
		vec[0].iov_len = 4;
		
		int opnd_cnt;
		int i;

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

		vec[1].iov_base = opmsg;
		vec[1].iov_len = sizeof(opmsg);

		writev(sock, vec, 2);		
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		fputs(name_msg, stdout);
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
