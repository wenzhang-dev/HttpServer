#ifndef code_InetAddress_h
#define code_InetAddress_h

#include <string>

#include <netinet/in.h>

namespace webserver
{

class InetAddress
{
public:
	explicit InetAddress(uint16_t port);
	InetAddress(const std::string &ip, uint16_t port);
	InetAddress(const sockaddr_in &addr) { addr_ = addr; }
	
	const struct sockaddr_in &get() const { return addr_; }
	void set(const struct sockaddr_in &addr) { addr_ = addr; }
	
	std::string toIpString() const;
	std::string toIpPortString() const;
	
private:
	struct sockaddr_in addr_;
};

}

#endif
