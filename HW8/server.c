#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/uio.h>


#define BUF_SIZE 100
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL); // Critical Section 충돌 방지를 위함
	serv_sock=socket(PF_INET, SOCK_STREAM, 0); // TCP Socket

	// Socket initalize & Setting
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx); //mutex lock 
		clnt_socks[clnt_cnt++]=clnt_sock; // Client Socket 정보를 저장해둔다.
		pthread_mutex_unlock(&mutx); 
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client Port: %d \n", (clnt_adr.sin_port));
	}
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg); // Client Socket fd를 인자로 받아서 설정
	int str_len=0, i;
	char msg[BUF_SIZE];
	
	struct iovec vec[2];
	char buf1[255]={0,};
	char buf2[255]={0,};

	vec[0].iov_base = buf1;
	vec[0].iov_len = 4;
	vec[1].iov_base = buf2;
	vec[1].iov_len = 255;

	while ((str_len = readv(clnt_sock, vec, 2)) != -1) // socket에서 데이터 읽음
	{
		int opnd_cnt = (int)buf2[0];
		int opinfo[255];
		char operationinfo[255];
		char *newmsg = buf1;
		
		if (opnd_cnt <= 0 || opnd_cnt >= 128)
		{
			printf("Closed Client\n");
			char check = opnd_cnt; 
			close(clnt_sock);
			return 0;
		}

		int opidx = 1;
		for (int i = 0; i < opnd_cnt; i++)
		{
			opinfo[i] = (int)buf2[opidx];
			opidx+=4;
		}
		int operator;
		for (int i = 0; i < opnd_cnt - 1; i++)
		{
			operationinfo[i] = buf2[opidx++];
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
		char temp1[255];
		sprintf(temp1,"%d",res);
		char temp2[255] = " result = ";
		strcat(temp2, temp1);
		strcat(newmsg, temp2);
		char final[255] = "\n";
		strcat(final, newmsg);


		send_msg(final, strlen(final));		
	}
	pthread_mutex_lock(&mutx);


	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1){
				clnt_socks[i]=clnt_socks[i+1];
			}
			break;
		}
	}

	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	printf("Closed Client");
	close(clnt_sock);

	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}