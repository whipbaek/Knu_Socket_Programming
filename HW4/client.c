#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sd;
	FILE *fp;

	char buf[BUF_SIZE];
	char *msg;
	int read_cnt;
	struct sockaddr_in serv_adr;
	struct hostent *host, *host2; // domain system
    struct sockaddr_in addr;
	socklen_t optlen;
	int state;
	int sock_type;

	if (argc < 2)
	{ // argument가 적으면
		return 1;
	}

	if (argc == 2) //domain 만 들어왔을때의 분기
    {

        host = gethostbyname(argv[1]); // domain, 1->succes / 0 -> fail
        if (!host)
        {
            return 1;
        }

        printf("gethostbyname()\n");
        printf("Official name : %s\n", host->h_name);

        for (int i = 0; host->h_aliases[i]; i++)
        {
            printf("Aliases %d : %s \n", i, host->h_aliases[i]);
        }

        printf("Address type : %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
        for (int i = 0; host->h_addr_list[i]; i++)
        {
            printf("IP addr %d: %s\n", i, inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
        }

        printf("\n");

        memset(&addr, 0, sizeof(addr));
        addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));

        host2 = gethostbyaddr((char *)&addr.sin_addr, 4, host->h_addrtype);
        printf("gethostbyaddr()\n");
        printf("Official name : %s \n", host2->h_name);

        for (int i = 0; host2->h_aliases[i]; i++)
        {
            printf("Aliases %d : %s \n", i, host2->h_aliases[i]);
        }

        printf("Address type : %s\n", (host2->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
        for (int i = 0; host2->h_addr_list[i]; i++)
        {
            printf("IP addr %d: %s\n", i, inet_ntoa(*(struct in_addr *)host2->h_addr_list[i]));
        }
    }

	else if (argc == 3)
	{
		sd = socket(PF_INET, SOCK_STREAM, 0);

		optlen = sizeof(sock_type);
        state = getsockopt(sd, SOL_SOCKET, SO_TYPE, (void*)&sock_type, &optlen);
        if(state){
            perror("getsockopt() error");
            return 1;
        }

        printf("This socket type is : %d(%d) \n",sock_type,SOCK_STREAM);

		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = inet_addr(argv[2]);
		serv_adr.sin_port = htons(atoi(argv[1]));

		connect(sd, (struct sockaddr *)&serv_adr, sizeof(serv_adr));

		read(sd, buf, BUF_SIZE);
		puts("Received file data");

		fp = fopen("copy.txt", "w+b");

		msg = buf;

		fwrite(msg, strlen(msg), 1, fp);
		write(sd, msg, strlen(msg) + 1);

		fclose(fp);
		close(sd);
	}
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}