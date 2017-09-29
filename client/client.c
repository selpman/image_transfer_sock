#include "../sock.h"
#include "../capture.h"
#include "../gpio.h"

void * send_image(void * arg);
typedef enum{
	BLOCK,
	PASS
} state_t;

int main(int argc, char *argv[])
{
	sock_info serv;
	pthread_t snd_thread;

	if(argc!=3) {
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	 }

	serv.sock = initSocketTCPConnect(argv[1],atoi(argv[2]));
	gpio_init();

	pthread_create(&snd_thread, NULL, send_image, (void*)&serv.sock);
	pthread_join(snd_thread, NULL);
	close(serv.sock);
	return 0;
}

void * send_image(void * arg)   // send thread main
{
	int sock=*((int*)arg);
  int file_size;
  int state =BLOCK;
  char buf[BUF_SIZE];
  FILE *fp;

  read(sock,buf,sizeof(buf));
  printf("server send passwd : %s \n",buf);
	get_button();
	
	if(cmp_button_passwd(buf))
	{
		printf("correct password\n");
		printf("Door opend\n");
		bzero(buf,BUF_SIZE);
		state = PASS;
		write(sock,&state,sizeof(int));
	}
	else
	{
  		captureBM();
		printf("Alert! password dismatched\n");
		state = BLOCK;
		write(sock,&state,sizeof(int));
		fp = fopen(OUTFILE_NAME, "rb");

  		bzero(buf,BUF_SIZE);
  		while ( (file_size = fread(buf,sizeof(char),BUF_SIZE,fp)) > 0 ) {
    			send(sock,buf,file_size,0);
    		bzero(buf,BUF_SIZE);
		}
	}
	fclose(fp);

	return NULL;
}
