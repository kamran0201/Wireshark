#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <poll.h>

#define MAX_LENGTH 1501
#define BACKLOG 1

int main(int argc, char *argv[]){
	struct timeval after, before;
	char buf[MAX_LENGTH];
	int status, sockfd, listenfd, syn_size, recvport;
	long long recvd_bytes, total_received = 0;
	struct sockaddr_in server_addr;
	struct sockaddr_storage their_addr; // connector's address information

	// args = sender_port delay loss tcp_variant file_name
	recvport = atoi(argv[1]);		// 1st arg = receiver's port

	// socket creation and binding taken from geeksforgeeks and Beej's book
	bzero(&server_addr, sizeof(server_addr));
	// Set address = own address, port = server's port.
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// Own address
	server_addr.sin_port = htons(recvport);	// Convert host to network encoding
	server_addr.sin_family = AF_INET;
	

	// Create a TCP socket
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){	// TCP connection = SOCK_STREAM
		printf("Socket could not be created\n");
		return -1;
	}
		
	// Bind socket to port
	if((bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr))) < 0){
		printf("Connection binding could not be established\n");
		perror("socket:");
		close(listenfd);
		return -1;
	}

	// Listen for incoming connections
	if(listen(listenfd, BACKLOG) < 0){
		printf("Cannot listen\n");
		return -1;
	}

	// Accept incoming connection and break the loop (assumption : there will be only one incoming connection)
	while(1){
		syn_size = sizeof their_addr;
		sockfd = accept(listenfd, (struct sockaddr*) &their_addr, &syn_size);
		if(sockfd >= 0){
			break;
		}
	}
	// Close listening socket. We don't need it any more
	status = close(listenfd);
	if(status != 0){
		printf("listenfd not close properly! %d\n",status);
	}


	// Find current time just when TCP connection is established and server starts sending packets
	gettimeofday( &before, NULL);
		
	// Make file pointer to store received data and clear the given file first
	FILE *fp = fopen("recv.txt", "w");
	if (fp == NULL){
		printf("Error opening receiver.txt!\n");
		exit(1);
	}
	fclose(fp);
	
	// While bytes received, append to file
	while(1){
		recvd_bytes = recv(sockfd, buf, MAX_LENGTH - 1, 0);
		if(recvd_bytes > 0){
			buf[recvd_bytes] = '\0';
			fp = fopen("recv.txt", "a");
			fprintf(fp, "%s", buf);
			fclose(fp);
			memset(buf, '\0', sizeof buf);
			total_received += recvd_bytes;
		}
		if(recvd_bytes <= 0){
			break;
		}
	}
	
	// Find time when data has stopped
	gettimeofday( &after, NULL);
	
	// Close TCP connection
	shutdown(sockfd, 2);
	status = close(sockfd);
	if(status != 0){
		printf("sockfd not close properly! %d\n",status);
	}
	
	// Calculate time difference in microseconds. 
	float difference = ((after.tv_sec - before.tv_sec)* 1000000L) + after.tv_usec - before.tv_usec;
	FILE *throughput = fopen(argv[5], "a");
	
	// Print bytes transferred, time taken : This will be used in calculating average throughput
	// Also print delay, loss and TCP variant for ease
	fprintf(throughput, "%lld %f %s %s %s\n", total_received, difference, argv[2], argv[3], argv[4]);
	
	return 0;
}
