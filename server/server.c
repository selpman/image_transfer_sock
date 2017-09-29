#include "../sock.h"
#include "../capture.h"

void * handle_clnt(void * arg);
static char passwd[3];
typedef enum{
	BLOCK,
	PASS
} state_t;


int main(int argc, char *argv[])
{
	sock_info serv, clnt;
  int clnt_addr_size;
	pthread_t t_id;

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

  serv.sock = initSocketTCPServer(atoi(argv[1]));
	
  	printf("config password ### >> \n");
	scanf("%s",passwd);
  	printf("password >> %s \n",passwd);
	

  while(1)
	{
		clnt_addr_size=sizeof(clnt.addr);
		clnt.sock=accept(serv.sock, (struct sockaddr*)&clnt.addr,&clnt_addr_size);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt);
		pthread_detach(t_id);
	}
	close(serv.sock);
	return 0;
}

void * handle_clnt(void * arg)
{
	sock_info clnt = *((sock_info*)arg);
  	int file_size;
  	int state;
	char buf[BUF_SIZE];

	FILE *fp;
		
	printf(" %s >> wait passwd from client  \n", inet_ntoa(clnt.addr.sin_addr));
	write(clnt.sock,passwd,sizeof(passwd));

	recv(clnt.sock,&state,sizeof(int),0);
	printf("%d \n", state);
	if(state)
	{
		printf(" %s >> correct passwd. open door  \n", inet_ntoa(clnt.addr.sin_addr));
	}
	else
	{
		fp = fopen(OUTFILE_NAME, "w");

  		bzero(buf,BUF_SIZE);
  		while ( (file_size = recv(clnt.sock,buf,BUF_SIZE,0)) > 0 ) {
    			int write_size = fwrite(buf,sizeof(char),file_size,fp);
    			bzero(buf,BUF_SIZE);
  		}
		printf(" %s >> alert! passwd dismatch. close door  \n", inet_ntoa(clnt.addr.sin_addr));
		fclose(fp);
	}

	close(clnt.sock);
	return NULL;
}
