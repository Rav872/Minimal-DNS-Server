#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define DOMAIN_SIZE 128

char * parse(char *buffer, int n_bytes){

	char *host = NULL;

	host  = (char *)malloc(sizeof(char)*DOMAIN_SIZE);
	if (host == NULL) {
		printf("Malloc Failed\n");
		exit(EXIT_FAILURE);
	}
	
	if ( n_bytes < DOMAIN_SIZE)
		strncpy(host, buffer, DOMAIN_SIZE);
	else
		printf("Input Domain is bigger\n");

	return host;
}

int create_sock_and_read(unsigned int port) {

	int len = 0, n_bytes = 0;
	char buffer[256] = {0} , *hostname = NULL;
	struct sockaddr_in caddr = {0};
	struct sockaddr_in saddr = {0} ;

	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sock_fd < 0) { 
		printf("Socket Creation failed\n");
		exit(EXIT_FAILURE);
	}
	
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(port); 

	if(bind(sock_fd, (const struct sockaddr *)&saddr, sizeof(saddr)) < 0 ) {
		printf("Socket Bind failed\n");
		exit(EXIT_FAILURE);
	}

	printf("Server Listening on %d\n", port);
	n_bytes = recvfrom(sock_fd, buffer, sizeof(buffer), MSG_WAITALL,
		(struct sockaddr *) &caddr, &len);
	
	buffer[n_bytes] = '\0';
	hostname = parse(buffer, n_bytes);
	dns(hostname, sock_fd, len, caddr);
	free(hostname);
}

int dns(char *hostname, int sock_fd, int len, struct sockaddr_in caddr) {
	
	int ret = 0;
	char IP[100] = {0};
	struct addrinfo hints = {0};
	struct addrinfo *result, *rp;
	void *test;

	printf("Hostname : %s\n", hostname);
	hints.ai_family = AF_UNSPEC;    
	hints.ai_socktype = SOCK_DGRAM; 
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	ret = getaddrinfo((const char *)hostname, "domain", &hints, &result);
	if (ret != 0) {
		perror("getaddrinfo:\n");
		exit(EXIT_FAILURE);
	}
	
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_family != AF_INET)
			continue;
		test = &((struct sockaddr_in *) rp->ai_addr)->sin_addr;

		inet_ntop(rp->ai_family, test, IP, 100 ); 
		printf("Address: %s\n ", IP);
	}
	close(sock_fd);
}

int main(int argc, char *argv[]) {

	unsigned int port = 0;

	if(argc != 2 || (port < 1024 && port > 65535)) {	
		printf(" <-- Enter Valid port --> \n");
		printf("Usage: %s <port>\n", argv[0]);
		return -1;
	}
	port = atoi(argv[1]);
	
	create_sock_and_read(port);

return 0;
}
