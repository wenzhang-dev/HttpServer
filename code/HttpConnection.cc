#include "HttpConnection.h"

#include <cassert>

#include "Channel.h"
#include "EventLoop.h"
#include "utils.h"

/* 先完成读写功能!!! */
/* 进一步：由HttpConnection关闭连接（非keep-alive） */
namespace webserver
{

HttpConnection::HttpConnection(EventLoop *loop, int connfd)
	: loop_(loop),
	  connfd_(connfd),
	  channel_(new Channel(connfd, loop_)),
	  state(Connected)
{
	assert(connfd > 0);
}

HttpConnection::~HttpConnection()
{
	printf("dtor HttpConnection\n");
}

void HttpConnection::setDefaultCallback()
{
	channel_->setReadCallback(std::bind(&HttpConnection::handleRead, this));
	channel_->setWriteCallback(std::bind(&HttpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&HttpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&HttpConnection::handleError, this));
	
	//channel_->enableReading();
}

/* client发送数据 */
/* HttpConnection自动读入缓冲区 */
void HttpConnection::handleRead(void)
{
	printf("handleRead\n");

	assert(loop_->isInLoopThread());
	
	int bytes = utils::readn(connfd_, __in_buffer);
	if(bytes < 0)
	{
		state = Error;
		handleError();
		return ;
	}
	
#if 0
	else if(bytes == 0)
	{
		state = DisConnected;
		handleClose();
		return ;
	}
#endif

	state = RecvReq;
}

void HttpConnection::handleWrite(void)
{
	printf("handleWrite\n");
	
	assert(loop_->isInLoopThread());
	int bytes = utils::writen(connfd_, __out_buffer);
	/* 关闭写监控 */
	if(__out_buffer.size() == 0)
	{
		channel_->disableWriting();
	}
	
	if(bytes < 0)
	{
		state = Error;
		channel_->disableWriting();
		handleError();
	}
	
	state = SendRsp;
}

void HttpConnection::handleClose(void)
{
	printf("handleClose\n");
	
	assert(loop_->isInLoopThread());
	/* 对方已关闭连接，web server直接关闭该套接字 */
	std::shared_ptr<HttpHandler> guard = holder_.lock();
	if(guard == nullptr) return ;	//TODO:
	
	channel_->disableAll();
	loop_->removeChannel(channel_);
}

void HttpConnection::handleError(void)
{
	printf("handleError\n");
	
	assert(loop_->isInLoopThread());
	/* TODO: */
	handleClose();
}

void HttpConnection::send(const void *data, int len)
{
	assert(loop_->isInLoopThread());
	const char *ptr = static_cast<const char *>(data);
	
	/* inefficient */
	int size = static_cast<int>(__out_buffer.size());
	__out_buffer.resize(size+len);
	std::copy(ptr, ptr+len, __out_buffer.begin()+size);
	
	//printf("write buffer:%s\n", __out_buffer.c_str());
	
	/* 使能写监控 */
	channel_->enableWriting();
	
}

void HttpConnection::send(const std::string &data)
{
	this->send(static_cast<const void *>(data.data()),
	           static_cast<int>(data.size()));
}

}