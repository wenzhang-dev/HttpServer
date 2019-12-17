#ifndef code_utils_h
#define code_utils_h

#include "InetAddress.h"

namespace webserver
{

namespace utils
{

int Socket(int family, int type, int proto);
void Bind(int sockfd, const struct sockaddr_in &addr);
void Listen(int sockfd, int backlog);
int SocketBindListen(const webserver::InetAddress &addr);
void Close(int sockfd);

int AcceptNb(int sockfd, webserver::InetAddress &addr);
int readn(int sockfd, std::string &io_buf, bool &isZero);
int writen(int sockfd, std::string &io_buf);

void setReuseAddr(int sockfd, bool on);
void Shutdown(int sockfd, int how);

void IgnoreSigpipe();

} //namespace utils

} //namespace webserver

#endif
