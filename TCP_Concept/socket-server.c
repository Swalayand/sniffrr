// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080

int server_fd, new_socket, valread;
char buffer[1024] = {0};
char *hello = "HTTP 200 OK\r\n\r\n Hai";

void comm(){
	ssize_t send_d;
	printf("comm called\n"); fflush(stdout);
	while(1){
		sleep(1);
		send_d = send(new_socket , hello , strlen(hello) , MSG_NOSIGNAL ); //https://stackoverflow.com/a/1705705
		if (send_d < 1) return;
		// printf("Hello from client line:%d\n", __LINE__);
		// valread = read( new_socket , buffer, 1024);
		// if (valread < 1) return;
		// printf("[%s] %d %ld line:%d\n", buffer, valread,send_d, __LINE__);
		// printf("%s line:%d\n", buffer, __LINE__);
	}
}
int main(int argc, char const *argv[])
{
	// int server_fd, new_socket, valread;

	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	// char buffer[1024] = {0};
	// char *hello = "HTTP 200 OK\r\n\r\n Hai";
	

	// Creating socket file descriptor : all about unix (device, etc) is file
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	
	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	
	/*
  while(1){
	
		Sampai baris ini, client dan server sudah melaksanakan protokol 
		yang disepakati. Paradigma pemrograman setelahnya (akses database,
		OOP, dsb) akan diimplementasikan di tengah proses pertukaran data
		ini
		
		valread = read( new_socket , buffer, 1024);
		printf("%s line:%d\n",buffer, __LINE__ );
		send_d = send(new_socket , hello , strlen(hello) , 0 );
		printf("%ld %d line:%d\n", send_d, valread, __LINE__);
		printf("Hello message sent from server\n");
		//close(new_socket);
  }
	*/
	while(1){
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		comm();
	}
	
	printf("while done:%d\n", __LINE__); fflush(stdout);

	return 0;
}

