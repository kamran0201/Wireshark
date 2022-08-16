#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <poll.h>

#define MAX_LENGTH 1500	// MTU = 1500 bytes. Fill up this buffer completely and then send
#define BACKLOG 1

int main(int argc, char *argv[]){
	char tcp_buff[100], buf[MAX_LENGTH];
	int tcp_len = sizeof tcp_buff, status, sockfd, recvport, len, sent_bytes;
	struct sockaddr_in server_addr;		
	
	// args = sender_port delay loss tcp_variant file_name
	recvport = atoi(argv[1]);		// 1st arg = receiver's port

	// socket creation and binding taken from geeksforgeeks and Beej's book	
	bzero(&server_addr, sizeof(server_addr));
	// Set address = own address, port = server's port.
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// Own address
	server_addr.sin_port = htons(recvport);	// Convert host to network encoding
	server_addr.sin_family = AF_INET;

	// Create a TCP socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){	// TCP connection = SOCK_STREAM
		printf("Socket could not be created\n");
		return -1;
	}

	// Get TCP variant
	if (getsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, tcp_buff, &tcp_len) != 0)
	{
		printf("TCP Variant could not be found!\n");
		return -1;
	}
	// Copy TCP variant from argv[4] to tcp_buff
	strcpy(tcp_buff, argv[4]);
	tcp_len = strlen(tcp_buff);
	// Set TCP variant as passed in command line argument
	if (setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, tcp_buff, tcp_len) != 0)
	{
		printf("TCP Variant could not be set!\n");
		return -1;
	}
	// Check TCP Variant
	tcp_len = sizeof tcp_buff;
	if (getsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, tcp_buff, &tcp_len) != 0)
	{
		printf("TCP Variant could not be found!\n");
		return -1;
	}
	// If set variant != argv[4], terminate! TCP variant could not be set to what was required
	if(strcmp(tcp_buff, argv[4]) != 0){
		printf("TCP Variant could not be set!\n");
		return -1;
	}
	
	
	// Connect to server
	while(connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){
		printf("Failed to connect\n");
		exit(1);
	}

	// Open file
	int fd = open("send.txt", O_RDONLY);
	if(fd < 0){
		printf("File send.txt could not be opened\n");
		return -1;
	}

	// Send MTU length packets till file gets over
	while(1){
		int bytes_read = read(fd, buf, sizeof buf);
		if(bytes_read == 0){
			break;
		}
		if(bytes_read > 0){
			sent_bytes = send(sockfd, buf, bytes_read, 0);
		}
	}

	// Close the connection
	shutdown(sockfd, 2);
	status = close(sockfd);
	if(status != 0){
		printf("listenfd not close properly! %d\n",status);
	}

	return 0;
}
