#ifndef code_HttpServer_h
#define code_HttpServer_h

#include <memory>
#include <vector>

#include "InetAddress.h"

namespace webserver
{
	
class Channel;
class EventLoop;
class EventLoopThreadPool;

class HttpServer
{
public:
	HttpServer(EventLoop *loop, const InetAddress &addr, int numThreads);
	~HttpServer();
	
	void start();
	void acceptor();
	
private:
	EventLoop *mainLoop_;
	int numThreads_;
	std::unique_ptr<EventLoopThreadPool> threadPool_;
	int listenFd_;
	std::shared_ptr<Channel> acceptChannel_;
	bool started_;
	int idleFd_;
};

}//namespace webserver

#endif
