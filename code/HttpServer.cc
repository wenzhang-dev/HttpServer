#include "HttpServer.h"

#include <cassert>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "HttpHandler.h"
#include "Channel.h"
#include "macros.h"
#include "utils.h"

// main loop need a acceptor
// dispatch new connections to other event loops
// HttpAck.cc HttpReq.cc
/* auto-machine, setStateTrackCallback: each http requests is divided to some stage handler ?? */
/* 应用层keepalive机制 */
namespace webserver
{
	
HttpServer::HttpServer(EventLoop *loop, const InetAddress &addr, int numThreads)
	: mainLoop_(loop),
	  numThreads_(numThreads),
	  threadPool_(new EventLoopThreadPool(mainLoop_, numThreads_)),
	  listenFd_(utils::SocketBindListen(addr)),
	  acceptChannel_(new Channel(listenFd_, mainLoop_)),
	  started_(false),
	  idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
	assert(listenFd_ > 0);
	assert(idleFd_ > 0);
	
	utils::setReuseAddr(listenFd_, true);
}

HttpServer::~HttpServer()
{
	assert(!started_);	
}

void HttpServer::start()
{
	assert(!started_);
	started_ = true;
	
	/* main loop be used to accept new connections */
	acceptChannel_->setReadCallback(std::bind(&HttpServer::acceptor, this));
	acceptChannel_->enableReading();
	
	threadPool_->start();
}

void HttpServer::acceptor()
{
	InetAddress addr(0);
	int connfd;
	
	//edge trigger mode
	while((connfd = utils::AcceptNb(listenFd_, addr)) > 0 || errno == EMFILE)
	{
		/* File descriptor exhausted */
		if(unlikely(errno == EMFILE))
		{
			::close(idleFd_);
			idleFd_ = ::accept(listenFd_, NULL, NULL);
			::close(idleFd_);
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
			continue;
		}
		
		EventLoop *loop = threadPool_->getNextLoop();

		std::shared_ptr<HttpHandler> handler(new HttpHandler(loop, connfd));
		loop->queueInLoop(std::bind(&EventLoop::addHttpConnection, loop, handler));	
		
		printf("fd=%d, %s\n", connfd, addr.toIpPortString().c_str());
	}
}

}//namespace webserver
