/* 
     the Weather daemon

     tcp service for interactive weather 
     
     features
        IPv4
	TCP
	iterative
*/

 
#include        "wthlib.h"


int
main(int argc, char **argv)
{
        /* network */
	int		     listenfd, connfd;
        int                  servport;
	struct sockaddr_in   servaddr;
	char		     readline[MAXLINE];

	/* data */
        struct cmd           cmd; 
        int                  ndata = 0;
        int                  err,c;
        char                 data[MAXBUFF];


        servport = atoi(WPORT);
        daemon_init(argv[0], 0);

        /* parse commandline */
	while ((c = getopt(argc, argv, "p:")) != -1) {
	    switch(c) {
		case 'p':
		    servport = atoi(optarg);
		    break;
		case '?':
		    usaged(1,"command line error","");
		    break;
	    }
	}

        
	listenfd = Socket(AF_INET, SOCK_STREAM, 0); 

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	servaddr.sin_port        = htons(servport);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for ( ; ; ) {
	     connfd = accept(listenfd, (SA *) NULL, NULL); 
	     Read(connfd, readline, MAXLINE);
             cmd.command = atoi(readline);
             cmd.argcmd  = 0;
             cmd.netflg  = 0;
             
             err = getsrd( data, &ndata, &cmd);
	     
             Write(connfd, data, ndata);
	     
	     Close(connfd);
	}
}








