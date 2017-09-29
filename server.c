#include "sock.h"
#include "capture.h"

void * handle_clnt(void * arg);

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

  while(1)
	{
		clnt_addr_size=sizeof(clnt.addr);
		clnt.sock=accept(serv.sock, (struct sockaddr*)&clnt.addr,&clnt_addr_size);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt.sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt.addr.sin_addr));
	}
	close(serv.sock);
	return 0;
}

void * handle_clnt(void * arg)
{
  int sock=*((int*)arg);
  int file_size;
  char buf[BUF_SIZE];

	FILE *fp;

	fp = fopen(OUTFILE_NAME, "a");

  bzero(buf,BUF_SIZE);
  while ( (file_size = recv(sock,buf,BUF_SIZE,0)) > 0 ) {
    int write_size = fwrite(buf,sizeof(char),file_size,fp);
    bzero(buf,BUF_SIZE);
  }

	fclose(fp);

	close(sock);
	return NULL;
}
