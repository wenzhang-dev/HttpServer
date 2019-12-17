#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(server_addr);
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_port = htons(20000);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
	server_addr.sin_family = AF_INET;
	
	connect(sockfd, (sockaddr *)&server_addr, addrlen);
	
	std::string packet = "POST / HTTP/1.1\r\n";
	packet += "Host: www.baidu.com\r\n";
	packet += "User-Agent: curl/7.47.0\r\n";
	packet += "Accept: */*\r\n";
	packet += "Content-length: 20\r\n\r\n";
	packet += "Hello, I'm a client!";
	
	ssize_t num = write(sockfd, packet.c_str(), (int)packet.size());
	(void)num;
	printf("%s\n", packet.c_str());
	
	//sleep(3);
	
	char buffer[1024]={0};
	num = read(sockfd, buffer, sizeof(buffer));
	(void)num;
	printf("Http Ack:\n%s\n", buffer);
	
	shutdown(sockfd, SHUT_WR);
	shutdown(sockfd, SHUT_WR);
	
	//sleep(15);
	
	close(sockfd);
	printf("End\n");
	
#if 0
	char buf[4096];
	while(fgets(buf, sizeof(buf), stdin))
	{
		if(!strncmp(buf, "quit", 4))
			break;
		
		ssize_t num = write(sockfd, buf, strlen(buf));
		(void)num;
		
		printf("send msg:%s", buf);
		
		memset(buf, 0, sizeof(buf));
	}
	shutdown(sockfd, SHUT_WR);
	close(sockfd);
#endif

	return 0;
}
