#include "sock.h"

void error_handling(const char* msg)
{
	fputs(msg,stderr);
	fputc('\n',stderr);
	exit(1);
}

int initSocketTCPServer(uint16_t port)
{
		sock_info serv;

		serv.sock=socket(PF_INET, SOCK_STREAM,0);
		if( -1 == serv.sock)
		{
				error_handling("socket() error");
		}

		serv.addr.sin_family = AF_INET;
		serv.addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv.addr.sin_port = htons( port );
		if( -1==bind(serv.sock, (struct sockaddr*)&serv.addr,sizeof(serv.addr)))
		{
			error_handling("bind() error");
		}

		if( -1 == listen(serv.sock, 5) )
		{
				error_handling("listen() error");
		}

		return serv.sock;
}

int initSocketTCPConnect(const char* ip,  uint16_t port)
{
	  sock_info serv;

		serv.sock = socket(PF_INET, SOCK_STREAM,0);
		if( -1 == serv.sock)
		{
				error_handling("socket() error");
		}

		serv.addr.sin_family = AF_INET;
		serv.addr.sin_addr.s_addr = inet_addr(ip);
		serv.addr.sin_port  = htons( port );
		if( -1 == connect(serv.sock,(struct sockaddr*)&serv.addr,sizeof(serv.addr)))
	   {
			  error_handling("connect() error");
	   }
	   return serv.sock;
}
