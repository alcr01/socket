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
    short int flags;                    // Combined fields of d, e, and f
    short int window;                   // 16 bit receive window for flow control
    short int checksum;                 // 16 bit checksum
    int sequence;                       // 32 bit sequence number
    int ack;                            // 32 bit acknowledgement number
    int options;                        // 32 bit options
};

int main(){
    // Variables
    int createSocket, newSocket;
    int *buffer = (int*) malloc(BUFFSIZE * sizeof(int));    // Buffer where everything will be saved
    struct sockaddr_in addr;            // Clients connection   
    struct sockaddr_in servaddr;        // Servers connection
    socklen_t addrlen;
    struct tCPSegment t;
    unsigned short int cksum_arr[12];   // Checksum
    unsigned int sum = 0, i, cksum; 
    int s_bit = 0, a_bit = 0, f_bit = 0;
    int run = 1, n = 0;           


    // Creates the socket and checks if it created successfully
    if((createSocket = socket(AF_INET, SOCK_STREAM, 0)) > 0)
        printf("Socket created!\n");
    
    // Something to do with sockets and port access (binding)
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(2200);

        //Binds the port to socket
    if(bind(createSocket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        error("ERROR on binding");
    printf("Succesfully Bound!\n");
    // Checks if socket is listening on the port
    if(listen(createSocket, 10) < 0)
        error("ERROR on Listening!");
    printf("Listening...\n");
    // Checks if the client is able to connect
    addrlen = sizeof(addr);
    if((newSocket = accept(createSocket, (struct sockaddr *) &addr, &addrlen)) < 0) 
        error("ERROR on accept!");
    printf("Client connected!\n");

    if(run == 1)
    {
        n = read(newSocket, buffer, sizeof(buffer));
        if(n < 0) error("ERROR reading to socket");

        t.sourcePort = 2200;        // Source Port
        t.destPort = buffer[0];     // Destination Port
        t.sequence = buffer[2];     // Sequence Number
        t.ack = buffer[2] + 1;     // Ack number
            // SYN AND ACK BITS
        s_bit = buffer[4];          // SYNBIT
        a_bit = 1;                  // ACKBIT
        t.window = buffer[6];       // Window
        t.checksum = buffer[7];     // Checksum
        t.options = buffer[8];      // Options

        // Runs Checksum
        memcpy(cksum_arr, &t, 24);          //Copying 24 bytes
        for (i=0;i<12;i++)                  // Compute sum
            sum = sum + cksum_arr[i];

        cksum = sum >> 16;                  // Fold once
        sum = sum & 0x0000FFFF; 
        sum = cksum + sum;

        cksum = sum >> 16;                  // Fold once more
        sum = sum & 0x0000FFFF;
        cksum = cksum + sum;
        cksum = 0xFFFF^cksum;

        t.checksum = cksum;
        run++;                      // Goes to the next run of the server

        buffer[2] = t.sequence;
        buffer[3] = t.ack;
        buffer[4] = s_bit;
        buffer[5] = a_bit;
        buffer[7] = t.checksum;

        // PRINTING
        printf("SERVER SENDING TO CLIENT\n");
        printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
        printf("Seq: %d\n", t.sequence);
        printf("ACK: %d\n", t.ack);
        printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
        printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
        printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d", s_bit, f_bit, t.window);
        printf("CHECKSUM: 0x%04X\n", t.checksum);
        printf("OPTIONS: %d\n DATA: 0\n", t.options);
        printf("\n\n");

        n = write(newSocket, buffer, sizeof(buffer));
        if(n < 0) error("ERROR on writing to socket");

    }
    if(run == 2)
    {
        n = read(newSocket, buffer, sizeof(buffer));
        if(n < 0) error("ERROR reading to socket");

        t.sequence = buffer[2];     // Sequence Number
        t.ack = t.sequence + 1;     // Ack number
            // FIN AND ACK BITS
        a_bit = 1;                  // ACKBIT

        // PRINTING
        printf("FINAL RESPONSE FROM CLIENT\n");
        printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
        printf("Seq: %d\n", t.sequence);
        printf("ACK: %d\n", t.ack);
        printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
        printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
        printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d", s_bit, f_bit, t.window);
        printf("CHECKSUM: 0x%04X\n", t.checksum);
        printf("OPTIONS: %d\n DATA: 0\n", t.options);
        printf("\n\n");
        run++;
    }

/* CLOSING TCP CONNECTION */
    printf("\n\nCLOSING TCP CONNECTION\n");
    if(run == 3)
    {
        n = read(newSocket, buffer, sizeof(buffer));
        if(n < 0) error("ERROR reading to socket");  

        srand(time(NULL));                   // Randomize the random seed
        t.sequence = random() % BUFFSIZE;
        t.ack = buffer[2] + 1;
        a_bit = 1;

        buffer[1] = t.sequence;
        buffer[2] = t.ack;
        buffer[5] = a_bit;

        // Runs Checksum
        memcpy(cksum_arr, &t, 24);          //Copying 24 bytes
        for (i=0;i<12;i++)                  // Compute sum
            sum = sum + cksum_arr[i];

        cksum = sum >> 16;                  // Fold once
        sum = sum & 0x0000FFFF; 
        sum = cksum + sum;

        cksum = sum >> 16;                  // Fold once more
        sum = sum & 0x0000FFFF;
        cksum = cksum + sum;
        cksum = 0xFFFF^cksum;

        t.checksum = cksum;
        run++;                      // Goes to the next run of the server

        // PRINTING
        printf("SERVER SENDING TO CLIENT\n");
        printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
        printf("Seq: %d\n", t.sequence);
        printf("ACK: %d\n", t.ack);
        printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
        printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
        printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d", s_bit, f_bit, t.window);
        printf("CHECKSUM: 0x%04X\n", t.checksum);
        printf("OPTIONS: %d\n DATA: 0\n", t.options);
        printf("\n\n");

        n = write(newSocket, buffer, sizeof(buffer));
        if(n < 0) error("ERROR on writing to socket");

        srand(time(NULL));                   // Randomize the random seed
        t.sequence = random() % BUFFSIZE;
        t.ack = buffer[2] + 1;
        f_bit = 1;

        // Runs Checksum
        memcpy(cksum_arr, &t, 24);          //Copying 24 bytes
        for (i=0;i<12;i++)                  // Compute sum
            sum = sum + cksum_arr[i];

        cksum = sum >> 16;                  // Fold once
        sum = sum & 0x0000FFFF; 
        sum = cksum + sum;

        cksum = sum >> 16;                  // Fold once more
        sum = sum & 0x0000FFFF;
        cksum = cksum + sum;
        cksum = 0xFFFF^cksum;

        t.checksum = cksum;

        buffer[2] = t.sequence;
        buffer[4] = f_bit;
        buffer[5] = a_bit;

        n = write(newSocket, buffer, sizeof(buffer));
        if(n < 0) error("ERROR on writing to socket");


        // PRINTING
        printf("\nSECOND CLOSE ACK\n");
        printf("SERVER SENDING TO CLIENT\n");
        printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
        printf("Seq: %d\n", t.sequence);
        printf("ACK: %d\n", t.ack);
        printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
        printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
        printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d", s_bit, f_bit, t.window);
        printf("CHECKSUM: 0x%04X\n", t.checksum);
        printf("OPTIONS: %d\n DATA: 0\n", t.options);
        printf("\n\n");

    }
    if(run == 4)
    {
        n = read(newSocket, buffer, sizeof(buffer));
        if(n < 0) error("ERROR reading to socket"); 

        t.sequence = buffer[2];
        t.ack = buffer[3];
        a_bit = buffer[5];



        // PRINTING
        printf("\nFINAL CLOSE ACK\n");
        printf("SERVER RECIEVING FROM CLIENT\n");
        printf("Source: %d, Dest: %d\n", t.sourcePort, t.destPort); // Printing all values
        printf("Seq: %d\n", t.sequence);
        printf("ACK: %d\n", t.ack);
        printf("DATA OFFSET: 5\n\tRESERV: 0 \n\tURGFLAG: 0 \n");
        printf("\tACKFLAG: %d\n\tPSH: 0 \n\tRST: 0 \n", a_bit);
        printf("\tSYN: %d \n\tFINBIT: %d \n\tWINDOW: %d", s_bit, f_bit, t.window);
        printf("CHECKSUM: 0x%04X\n", t.checksum);
        printf("OPTIONS: %d\n DATA: 0\n", t.options);
        printf("\n\n");

    }


    // Closes all sockets creates
    close(createSocket);
    close(newSocket);
    // Releases all the memory used
    free(buffer);
    free(buffer2);
    return 0;
}