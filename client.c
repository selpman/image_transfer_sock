#include "sock.h"
#include "capture.h"

void * send_image(void * arg);

int main(int argc, char *argv[])
{
	sock_info serv;
	pthread_t snd_thread;

	if(argc!=3) {
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	 }

	serv.sock = initSocketTCPConnect(argv[1],atoi(argv[2]));

	pthread_create(&snd_thread, NULL, send_image, (void*)&serv.sock);
	pthread_join(snd_thread, NULL);
	close(serv.sock);
	return 0;
}

void * send_image(void * arg)   // send thread main
{
	int sock=*((int*)arg);
  int file_size;
  char buf[BUF_SIZE];
  FILE *fp;

  captureBM();
	fp = fopen(OUTFILE_NAME, "rb");

  bzero(buf,BUF_SIZE);
  while ( (file_size = fread(buf,sizeof(char),BUF_SIZE,fp)) > 0 ) {
    send(sock,buf,file_size,0);
    bzero(buf,BUF_SIZE);
  }

	fclose(fp);

	return NULL;
}
