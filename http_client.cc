#include "minet_socket.h"
#include <stdlib.h>
#include <ctype.h>

#define BUFSIZE 1024

int main(int argc, char * argv[]) {

    char * server_name = NULL;
    int server_port    = -1;
    char * server_path = NULL;
    char * req         = NULL;
    bool ok            = false;
    int read_header;
    char c[1];
    

    //Define useful variables
    struct hostent *host_address;

    /*parse args */
    if (argc != 5) {
	fprintf(stderr, "usage: http_client k|u server port path\n");
	exit(-1);
    }

    server_name = argv[2];
    server_port = atoi(argv[3]);
    server_path = argv[4];

    req = (char *)malloc(strlen("GET  HTTP/1.0\r\n\r\n") 
			 + strlen(server_path) + 1);  

    /* initialize */
    if (toupper(*(argv[1])) == 'K') { 
	minet_init(MINET_KERNEL);
    } else if (toupper(*(argv[1])) == 'U') { 
	minet_init(MINET_USER);
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }

    /* make socket */
	int sk = minet_socket(SOCK_STREAM);
    /* Hint: use gethostbyname() */
	host_address = gethostbyname(server_name);

    /* set address */
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_port = htons(server_port);
	sa.sin_family = AF_INET;
        memcpy(&sa.sin_addr.s_addr, host_address->h_addr, host_address->h_length);
    /* connect to the server socket */
	minet_connect(sk,(struct sockaddr_in *) &sa);

    /* send request message */
	if(server_path[0] = '/')
    		sprintf(req, "GET %s HTTP/1.0\r\n\r\n", server_path);
	else
		sprintf(req, "GET /%s HTTP/1.0\r\n\r\n", server_path);
	minet_write(sk, req, strlen(req)*sizeof(char));
	

    /* wait till socket can be read. */
	fd_set fdset;
	FD_ZERO(&fdset);
    	FD_SET(sk, &fdset);
	minet_select(sk+1, &fdset, NULL, NULL, NULL);

    /* Hint: use select(), and ignore timeout for now. */

    /* first read loop -- read headers */
	int result;
	char header[12];
	minet_read(sk, header, 12);
	result = atoi(header + 9);

    /* examine return code */   

    //Skip "HTTP/1.0"
    //remove the '\0'

    // Normal reply has return code 200
	
    /* print first part of response: header, error code, etc. */
	if(result == 200)
	{
		printf("%s", header);

    /* second read loop -- print out the rest of the response: real web content */
	int read_header;
    	char c[1];
	do{
	   	read_header = minet_read(sk, c, 1);
		if(read_header > 0)
			printf("%c", c[0]);
		}while(read_header > 0);
		
	
	}
	else{
		printf("%s", header);
		do{
       	 		read_header = minet_read(sk, c, 1);
        		if (read_header > 0)
          			printf("%c", c[0]);
      		} while(read_header > 0);
   	 }	
	minet_close(sk);


    /*close socket and deinitialize */

    if (ok) {
	return 0;
    } else {
	return -1;
    }
}
