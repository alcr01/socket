#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define BUFFSIZE 10
#define WEBSIZE 100

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

// Program 3 Variables
struct tCPSegment {  
    short int destPort;                 // 16 bit destination port
    short int sourcePort;               // 16 bit source port
    short int reserve;                  // 6 bit reserved section
    short int flags;                 	// Combined fields of d, e, and f
    short int window;              		// 16 bit receive window for flow control
    short int checksum;                 // 16 bit checksum
    int sequence;                       // 32 bit sequence number
    int ack;                            // 32 bit acknowledgement number
    int options;                    	// 32 bit options
};

int main()
{
	 // Variables
    int createSocket, n, run = 1;
    struct sockaddr_in servaddr;        // Servers connection
    struct hostent *server;			
    socklen_t addrlen;
    char *website = malloc(WEBSIZE);    
    int *buffer = (int*) malloc(BUFFSIZE * sizeof(int));    // Buffer where everything will be saved
	struct tCPSegment t;
	unsigned short int cksum_arr[12];	// Checksum
	unsigned int sum = 0, i, cksum;	
	int s_bit = 0, a_bit = 0;
	int f_bit = 0;			

	// Assign initial values for the first TCP Segment being sent
	srand(time(NULL));					// Randomize the random seed
	t.sequence = random() % BUFFSIZE;	// Random sequence number
	t.ack = 0;
	t.destPort = 2200;
	t.sourcePort = 21334;
	t.window = 0;
	t.checksum = 0;
	t.reserve = 0;
	t.options = 0;
	/*
		// Set 0 for all the flags
	for(n = 0; n < 6; n++)
		t.flags[n] = 0;
	t.flags[4] = 1;						// Sets SYNBIT to 1
	*/
	memcpy(cksum_arr, &t, 24); 			//Copying 24 bytes
	for (i=0;i<12;i++)               	// Compute sum
		sum = sum + cksum_arr[i];

	cksum = sum >> 16;              	// Fold once
	sum = sum & 0x0000FFFF; 
	sum = cksum + sum;

	cksum = sum >> 16;             		// Fold once more
	sum = sum & 0x0000FFFF;
	cksum = cksum + sum;

	cksum = 0xFFFF^cksum;

	t.checksum = cksum;

    // Creates the socket and checks if it created successfully
    if((createSocket = socket(AF_INET, SOCK_STREAM, 0)) > 0)
        printf("Socket created!\n");
   
	// Will get the server for the client to connect to
    printf("Server CSE Machine: ");
    scanf("%s", website);

	server = gethostbyname(website);
	if(server == NULL) {
		error("ERROR, no such host\n");
	}
	bzero((char *) &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr, server->h_length);
	servaddr.sin_port = htons(t.destPort);

	if(connect(createSocket, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		error("ERROR connecting");
	for(n = 0; n < sizeof(buffer); n++)
		buffer[n] = 0;

	// TCP Connection
	if(run == 1)
	{
		buffer[0] = t.sourcePort;	// Source Port
		buffer[1] = t.destPort;		// Destination Port
		buffer[2] = t.sequence;		// Sequence Number
		buffer[3] = t.ack;			// Ack number
			// SYN AND ACK BITS
		s_bit = 1;
		buffer[4] = s_bit;			// SYNBIT
		buffer[5] = a_bit;			// ACKBIT
		buffer[6] = t.window;		// Window
		buffer[7] = t.checksum;		// Checksum
		buffer[8] = t.options;		// Options
		run++;						// Second transition ready

		// Send to the server
		n = write(createSocket, buffer, sizeof(buffer));
		if(n < 0) error("ERROR writing to socket");

		// PRINT OUT WHAT THE CLIENT SENDS TO SERVER
		printf("CLIENT -> SERVER SENT\n");
		printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
		printf("Seq: %d\n", t.sequence);
		printf("ACK: %d\n", t.ack);
		printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
		printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
		printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d\n", s_bit, f_bit, t.window);
		printf("CHECKSUM: 0x%04X\n", t.checksum);
		printf("OPTIONS: %d\n DATA: 0\n", t.options);
		printf("\n\n");
	}
	// Second run
	if(run == 2)
	{	
		n = read(createSocket, buffer, sizeof(buffer));
		if(n < 0) error("ERROR on reading from socket");

		t.sequence =  t.sequence + 1;	// Sequence number plus 1
		t.ack = buffer[2];				// Changes the ACK number
		a_bit = 1;						// Changes the ack bit

		memcpy(cksum_arr, &t, 24); 		//Copying 24 bytes
		for (i=0;i<12;i++)              // Compute sum
			sum = sum + cksum_arr[i];

		cksum = sum >> 16;             	// Fold once
		sum = sum & 0x0000FFFF; 
		sum = cksum + sum;

		cksum = sum >> 16;             	// Fold once more
		sum = sum & 0x0000FFFF;
		cksum = cksum + sum;
		cksum = 0xFFFF^cksum;

		t.checksum = cksum;
		run++;

		buffer[2] = t.sequence;
		buffer[3] = t.ack;
		buffer[5] = a_bit;
		buffer[7] = t.checksum;

		// Send to the server
		n = write(createSocket, buffer, sizeof(buffer));
		if(n < 0) error("ERROR writing to socket");

		// PRINT OUT WHAT THE CLIENT SENDS TO SERVER
		printf("\nWHAT CLIENT SENDS TO SERVER\n");
		printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
		printf("Seq: %d\n", t.sequence);
		printf("ACK: %d\n", t.ack);
		printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
		printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
		printf("\tSYN: %d \n\tFINBIT: 0 \n\tWINDOW: %d", s_bit, t.window);
		printf("CHECKSUM: 0x%04X\n", t.checksum);
		printf("OPTIONS: %d\n DATA: 0\n", t.options);
		printf("\n\n");
	}

	
	//Closing TCP connection
	if(run == 3)
	{
		srand(time(NULL));					// Randomize the random seed
		t.sequence = random() % BUFFSIZE;
		t.ack = 0;
		f_bit = 1;

		memcpy(cksum_arr, &t, 24); 		//Copying 24 bytes
		for (i=0;i<12;i++)              // Compute sum
			sum = sum + cksum_arr[i];

		cksum = sum >> 16;             	// Fold once
		sum = sum & 0x0000FFFF; 
		sum = cksum + sum;

		cksum = sum >> 16;             	// Fold once more
		sum = sum & 0x0000FFFF;
		cksum = cksum + sum;
		cksum = 0xFFFF^cksum;

		t.checksum = cksum;
		run++;

		buffer[2] = t.sequence;		// Sequence Number
		buffer[3] = t.ack;			// Ack number
			// SYN AND ACK BITS
		f_bit = 1;
		a_bit = 0;
		buffer[4] = f_bit;			// FINBIT
		buffer[5] = a_bit;			// ACKBIT
		buffer[6] = t.window;		// Window
		buffer[7] = t.checksum;		// Checksum
		buffer[8] = t.options;		// Options

		// Send to the server
		n = write(createSocket, buffer, sizeof(buffer));
		if(n < 0) error("ERROR writing to socket");

		// PRINT OUT WHAT THE CLIENT SENDS TO SERVER
		printf("\n\nCLOSING TCP\n");
		printf("CLIENT -> SERVER SENT\n");
		printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
		printf("Seq: %d\n", t.sequence);
		printf("ACK: %d\n", t.ack);
		printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
		printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
		printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d\n", s_bit, f_bit, t.window);
		printf("CHECKSUM: 0x%04X\n", t.checksum);
		printf("OPTIONS: %d\n DATA: 0\n", t.options);
		printf("\n\n");
	}
	if(run == 4)
	{
		n = read(createSocket, buffer, sizeof(buffer));
		if(n < 0) error("ERROR on reading from socket");

		t.sequence = buffer[2];
		t.ack = buffer[3];
		f_bit = buffer[4];
		a_bit = buffer[5];
		t.checksum = buffer[7];

		// PRINT OUT WHAT THE CLIENT SENDS TO SERVER
		printf("\n\nCLOSING TCP\n");
		printf("CLIENT -> SERVER SENT\n");
		printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
		printf("Seq: %d\n", t.sequence);
		printf("ACK: %d\n", t.ack);
		printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
		printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
		printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d\n", s_bit, f_bit, t.window);
		printf("CHECKSUM: 0x%04X\n", t.checksum);
		printf("OPTIONS: %d\n DATA: 0\n", t.options);
		printf("\n\n");

		int *buffer2 = (int*) malloc(BUFFSIZE * sizeof(int));    // Buffer where everything will be saved
		n = read(createSocket, buffer2, sizeof(buffer2));
		if(n < 0) error("ERROR on reading from socket");
		
		t.sequence = buffer[2];
		t.ack = buffer[3];
		f_bit = buffer[4];
		a_bit = buffer[5];
		t.checksum = buffer[7];

		// PRINT OUT WHAT THE CLIENT SENDS TO SERVER
		printf("\n\nCLOSING TCP\n");
		printf("CLIENT -> SERVER SENT\n");
		printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
		printf("Seq: %d\n", t.sequence);
		printf("ACK: %d\n", t.ack);
		printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
		printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
		printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d\n", s_bit, f_bit, t.window);
		printf("CHECKSUM: 0x%04X\n", t.checksum);
		printf("OPTIONS: %d\n DATA: 0\n", t.options);
		printf("\n\n");


		t.ack = t.sequence;
		t.sequence = t.sequence + 1;
		a_bit = 1;

		buffer[2] = t.sequence;
		buffer[3] = t.ack;
		buffer[5] = a_bit;

		memcpy(cksum_arr, &t, 24); 		//Copying 24 bytes
		for (i=0;i<12;i++)              // Compute sum
			sum = sum + cksum_arr[i];

		cksum = sum >> 16;             	// Fold once
		sum = sum & 0x0000FFFF; 
		sum = cksum + sum;

		cksum = sum >> 16;             	// Fold once more
		sum = sum & 0x0000FFFF;
		cksum = cksum + sum;
		cksum = 0xFFFF^cksum;

		t.checksum = cksum;

		buffer[7] = t.checksum;

		// Send to the server
		n = write(createSocket, buffer, sizeof(buffer));
		if(n < 0) error("ERROR writing to socket");
	}
	

	close(createSocket);
	return 0;
}
