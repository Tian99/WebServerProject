#include "minet_socket.h"
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

#define BUFSIZE 1024
#define FILENAMESIZE 100
//////////////////////////////////////////////////
//Some problem in reading the existed files
/////////////////////////////////////////////////
int handle_connection(int sock);

int main(int argc, char * argv[]) {
    int server_port = -1;
    int rc          =  0;
    int sock        = -1;

    /* parse command line args */
    if (argc != 3) {
	fprintf(stderr, "usage: http_server1 k|u port\n");
	exit(-1);
    }

    server_port = atoi(argv[2]);

    if (server_port < 1500) {
	fprintf(stderr, "INVALID PORT NUMBER: %d; can't be < 1500\n", server_port);
	exit(-1);
    }
    
    /* initialize and make socket */
	fprintf(stderr, "Initializing\n");
    	minet_init(MINET_KERNEL);
	sock = minet_socket(SOCK_STREAM); //Socket initialization
    	if(sock == -1)
	{
		//Error in creating socket
		return 1;
	}
        

    /* set server address*/
	fprintf(stderr, "Setting\n");
	struct sockaddr_in address;
	memset(&address, 0 , sizeof(address)); //Fill the address
	address.sin_family = AF_INET;
	address.sin_port = htons(server_port);
	

    /* bind listening socket */
	fprintf(stderr, "Binding\n");
 	if(minet_bind(sock, &address) == -1)
		{
			//Error binding socket
			return 1;
			
		}

    /* start listening */
	fprintf(stderr, "Listening\n");
	if(minet_listen(sock, 5) == -1)
		{
			//Too many connections
			return 1;
		}
	
	fprintf(stderr, "continue....\n");
    /* connection handling loop: wait to accept connection */
	struct sockaddr_in ad; //Temporary address
	//Socket descriptor	
	int des;//temporary socket descriptor
	fd_set readfds;//The fd_set data type represents file descriptor sets for the select function. It is actually a bit array. 
	//Time out(might not be necessary)
	struct timeval tou; //Value for timeout
	int max_connections = 100; //Maximum connection
	int socketfiles[100] = {0}; // file array
	int max; //the max number of connections
	int re; //Return value for select
	int i; //Loop

    while (1) {	
	/* create read list */
	fprintf(stderr, "Running\n");
	re = 1;
	tou.tv_sec = 10;
	tou.tv_usec = 50000;//The ellapsed time
	FD_ZERO(&readfds);//This macro initializes the file descriptor set set to be the empty set. 
	FD_SET(sock, &readfds);//This macro adds filedes to the file descriptor set set. 
	max = sock;
	
	for(i = 0; i < max_connections; i++)
	{
//	fprintf(stderr, "looping\n");
		des = socketfiles[i];
		
		if(des > 0)
		{
			FD_SET(des, &readfds);		
		}

		if(des > max)
		{
			max = des;
		}
	}
	fprintf(stderr, "Selecting\n");
	/* do a select */
	//Following the document format to select. +1 to be safe
	re = minet_select(max + 1, &readfds, NULL, NULL, NULL);
	fprintf(stderr, "finish selecting\n");
	/* process sockets that are ready */
        if(re == -1)
	{
		perror("error");	
	}
	//Do we need a no data exception?
	else
	{
		/* for the accept socket, add accepted connection to connections */
		if(FD_ISSET(sock, &readfds))
		{
			for(i = 0; i < max_connections; i++)
			{
				if(socketfiles[i] <= 0)
				{
					//Add it to connections
					socketfiles[i] = minet_accept(sock, &ad);
					break;
				}
			}
		}
		else{
			/* for a connection socket, handle the connection */
			for(i = 0; i < max_connections; i++)
			{
				if(FD_ISSET(socketfiles[i], &readfds))
				{

					fprintf(stderr, "Handling right now");
					rc = handle_connection(socketfiles[i]); //Call the function
					socketfiles[i] = 0;
				
				}		
			}
	         }
	
        }
   }
}

int handle_connection(int sock) {
   //Later truw if it could successfully read the file
    bool ok = false;

    const char * ok_response_f = "HTTP/1.0 200 OK\r\n"	\
	"Content-type: text/plain\r\n"			\
	"Content-length: %d \r\n\r\n";
    
    const char * notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"	\
	"Content-type: text/html\r\n\r\n"			\
	"<html><body bgColor=black text=white>\n"		\
	"<h2>404 FILE NOT FOUND</h2>\n"				\
	"</body></html>\n";
    
    /* first read loop -- get request and headers*/
	char buffer_1[BUFSIZE];
	memset(buffer_1, '\0', BUFSIZE);
	//Handle high latency, could pretty much be deleted
	char buffer_2[BUFSIZE];
	memset(buffer_2, '\0', BUFSIZE);
	minet_read(sock, buffer_1, BUFSIZE -1);
	int space_index = -1;
	for(int i = 4; i < strlen(buffer_1); ++i)
	{
		if (buffer_1[i] == ' ')
		{
			space_index = i;
			break;
		} 	
	}

	if(space_index == -1)
	{
		minet_close(sock);
		return -1;
	}

	char filename[space_index -4];
	strncpy(filename, &buffer_1[4], space_index - 4);
	filename[space_index - 4] = '\0';
	
	fprintf(stderr, "++++++++++++++++++++++++++++++++\n");
	fprintf(stderr, "%s\n", filename );
	fprintf(stderr, "+++++++++++++++++++++++++++++++++");

	

	strcat(buffer_2, buffer_1);
	//fprintf(stderr, "%s", buffer_1);
	memset(buffer_1, '\0', BUFSIZE);

    /* parse request to get file name */
    /* Assumption: this is a GET request and filename contains no spaces*/
	//char *str = strtok(buffer_2, "/\n");
	//fprintf(stderr, str);
	//char *file = NULL;

//FILE *f = fopen(str, "r");
//if(f)
//	fprintf(stderr, "working");
//Need to add a while loop?
//fprintf(stderr, "----------------------------\n");
//fprintf(stderr, "%s\n", str);

    /* try opening the file */
	FILE *f = fopen(filename, "r");
	long file_size;
	char *content = NULL;
	fprintf(stderr, "Choosing files\n");
	if(f != NULL){
		fprintf(stderr, "Try opening a file\n");
		if(fseek(f, 0L, SEEK_END) == 0)
		{
			file_size = ftell(f);
			if(file_size == -1) //Buffer size incorrect
				return NULL;
		
		content = (char *) malloc(sizeof(char) * (file_size + 1));
		
		if(fseek(f, 0L, SEEK_SET) != 0)
		{
			//add minet_close(sock) if you want to close it, but not necessary
			fprintf(stderr, "Seek Error");
			return NULL;
		}

		fread(content, sizeof(char), file_size, f);
		
		if(ferror(f) != 0)
		{
			fprintf(stderr, "Error reading file");
			perror("Error Reading File");
		}
		else
		{
			fprintf(stderr, "Passing file");
			content[file_size + 1] = '\0';
			ok = true;
		}
	}
	fclose(f);
    }

    /* send response */
    if (ok) {
	/* send headers */
	char output[strlen(ok_response_f)];
	sprintf(output, ok_response_f, file_size);
	minet_write(sock, output, strlen(ok_response_f) * sizeof(char));
	
	/* send file */
	minet_write(sock, content, file_size + 1);
	
    } else {
	// send error response
	char output[strlen(notok_response)];
	//sprintf
	sprintf(output, notok_response);
	minet_write(sock, output, strlen(notok_response) * sizeof(char));
    }

    /* close socket and free space */
	minet_close(sock);
	free(content);
  
    if (ok) {
	return 0;
    } else {
	return -1;
    }
}
