#include "InetAddress.h"

#include <strings.h>

#include <netinet/in.h>
#include <arpa/inet.h>

namespace webserver
{
	
InetAddress::InetAddress(uint16_t port)
{
	bzero(&addr_, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(port);
	addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

InetAddress::InetAddress(const std::string &ip, uint16_t port)
{
	bzero(&addr_, sizeof(addr_));
	
	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}

std::string InetAddress::toIpString() const
{
	char ip[INET_ADDRSTRLEN];
	
	inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof(ip));
	return std::string(ip);
}

std::string InetAddress::toIpPortString() const
{
	char ip[INET_ADDRSTRLEN];
	
	inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof(ip));
	uint16_t port = ntohs(addr_.sin_port);
	return std::string(ip) + ":" + std::to_string(port);
}

}//namespace webserver