#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;


	char buffer[256];
	int ticket[10], 			// 10 tickets sold
		t_sold[10];				// Checks if the ticket was sold
	int n, i, t_count = 0;		// Counters
	int to_cancel;				// Ticket that'll be canceled

	/* *************************************************************
	Adds random numbers (ie the tickets) and adds 0 (ticket not sold)
	to the int array of ticket and t_sold
	************************************************************** */
	srand(time(NULL));			// Randomize the seed
	for(i = 0; i < 10; i++) {
		// Adds random numbers to tickets
		ticket[i] = rand() % 90000 + 10000;
		t_sold[i] = 0;			// Assigns no tickets sold 
	}


	if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    	error("ERROR on binding");

    while(1) {
    	listen(sockfd, 5);
    	clilen = sizeof(cli_addr);
    	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    	if(newsockfd < 0) 
    		error("ERROR on accept");
    	bzero(buffer, 256);

    	n = read(newsockfd, buffer, 255);
    	if(n < 0) error("ERROR on read");

    	if(buffer[0] == 'b' && buffer[1] == 'u' && buffer[2] == 'y') {
    		for(i = 0; i < 10; i++) {
    			if(t_sold[i] == 0) {
    				t_sold[i] = 1;	// Assigns 1 meaing its sold
    	// Converts the int ticket number to characters and puts it in buffer
    				snprintf(buffer, 10, "%d", ticket[i]);
    				printf("Sold: %s\n", buffer);
    				t_count++;// Counter to keep track of tickets sold
    				goto t_sold;
    			}
    			else if(t_count == 10) {
    				n = write(newsockfd, "No more tickets available", 25);
    				if(n < 0) error("ERROR on write");
    				goto end;
    			}
    		}
    		t_sold:;
    		n = write(newsockfd, buffer, 255);
    	}
    	end:;

    	if(buffer[0] == 'c' && buffer[1] == 'a' && buffer[2] == 'n' && buffer[3] == 'c' && buffer[4] == 'e' && buffer[5] == 'l') {  		
    		n = read(newsockfd, buffer, 255);
    		if(n < 0) error("ERROR reading from socket");
			to_cancel = atoi(buffer);        // Converts character buffer to int
    		for(i = 0; i < 10; i++) {
    			if(ticket[i] == to_cancel) {
    				t_sold[i] = 0;			// Sets location to not sold
					printf("Ticket %d cancelled", to_cancel);
    			}
    		}
    	}
    	bzero(buffer, 256);
    	close(newsockfd);
    }
    close(sockfd);
    return 0;
}
