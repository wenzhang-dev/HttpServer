#include "utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include "config.h"
#include "macros.h"

namespace webserver
{

namespace utils
{
	
typedef struct sockaddr SA;
const SA* sockaddr_cast(const struct sockaddr_in* addr)
{
	return static_cast<const SA*>(reinterpret_cast<const void*>(addr));
}

SA* sockaddr_cast(struct sockaddr_in* addr)
{
	return static_cast<SA*>(reinterpret_cast<void*>(addr));
}

int Socket(int family, int type, int proto)
{
	int sockfd = -1;
	sockfd = ::socket(family, type, proto);
	if(unlikely(sockfd < 0))
	{
		perror("Socket");
	}
	return sockfd;
}

void Bind(int sockfd, const struct sockaddr_in &addr)
{
	int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof(addr));
	if(unlikely(ret < 0))
	{
		perror("Bind");
	}
}

void Listen(int sockfd, int backlog)
{
	int ret = ::listen(sockfd, backlog);
	if(unlikely(ret < 0))
	{
		perror("Listen");
	}
}

int SocketBindListen(const webserver::InetAddress &addr)
{
	int sockfd = Socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | 
	                    SOCK_CLOEXEC, IPPROTO_TCP);
						
	Bind(sockfd, addr.get());
	Listen(sockfd, SOCKET_MAXBACKLOG);
	
	return sockfd;
}

int AcceptNb(int sockfd, webserver::InetAddress &addr)
{
	struct sockaddr_in acceptAddr;
	socklen_t addrLen = sizeof(acceptAddr);
	
	int connfd = ::accept4(sockfd, sockaddr_cast(&acceptAddr), 
	                       &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
	
	addr.set(acceptAddr);
	
	return connfd;
}

void Close(int sockfd)
{
	int ret = ::close(sockfd);
	if(unlikely(ret < 0))
	{
		perror("Close");
	}
}

void setTcpNoDelay(int sockfd, bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
               &optval, sizeof optval);
}

void setReuseAddr(int sockfd, bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof optval);
}

void Shutdown(int sockfd, int how)
{
	int ret = ::shutdown(sockfd, how);
	if(unlikely(ret<0))
	{
		perror("shutdown");
	}
}

/* ET mode */
/* 读写直到EAGAIN */
int readn(int sockfd, std::string &io_buf, bool &isZero)
{
	char buf[MAX_BUFSIZE];
	int nbytes;
	int totalSize = 0;
	
	while(true)
	{
		if((nbytes = ::read(sockfd, buf, MAX_BUFSIZE)) <= 0)
		{
			if(errno == EINTR) continue;
			if(errno == EAGAIN) return totalSize;
			if(nbytes == 0)	/* 读0 */
			{
				isZero = true;
				break;
			}
			
			return -1;
		}
		
		totalSize += nbytes;
		io_buf += std::string(buf, buf+nbytes);
	}

	return totalSize;
}

int writen(int sockfd, std::string &io_buf)
{
	int nbytes;
	int totalSize = 0;
	int bufSize = static_cast<int>(io_buf.size());;
	const char *pstr = reinterpret_cast<const char *>(io_buf.data());
	
	while(totalSize < bufSize)
	{
		if((nbytes = ::write(sockfd, pstr +	totalSize, 
		                     bufSize-totalSize)) < 0)
		{
			if(errno == EINTR) continue;
			if(errno == EAGAIN) break;
			//if(errno == EPIPE)
			
			io_buf.clear();
			return -1;
		}
		totalSize += nbytes;
	}
	
	if(totalSize == bufSize) 
	{
		io_buf.clear();
	}
	else 
	{
		io_buf = io_buf.substr(totalSize);
	}
	
	return totalSize;
}

}//namespace utils

}//namespace webserver
