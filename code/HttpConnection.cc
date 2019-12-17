#include "HttpConnection.h"

#include <cassert>

#include "Channel.h"
#include "EventLoop.h"
#include "utils.h"

namespace webserver
{

HttpConnection::HttpConnection(EventLoop *loop, int connfd)
	: loop_(loop),
	  connfd_(connfd),
	  channel_(new Channel(connfd, loop_)),
	  state_(kConnected)
{
	assert(connfd > 0);
}

HttpConnection::~HttpConnection()
{
	//printf("dtor HttpConnection\n");
}

void HttpConnection::setDefaultCallback()
{
	channel_->setReadCallback(std::bind(&HttpConnection::handleRead, this));
	channel_->setWriteCallback(std::bind(&HttpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&HttpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&HttpConnection::handleError, this));
}

/* client发送数据 */
/* HttpConnection自动读入缓冲区 */
void HttpConnection::handleRead(void)
{
	assert(loop_->isInLoopThread());
	
	bool isZero = false;
	int bytes = utils::readn(connfd_, __in_buffer, isZero);
	if(bytes < 0)
	{
		state_ = kError;
		handleError();
		return ;
	}
	else if(bytes == 0 && state_ == kConnected)	
	{
		/* 第一次请求到来，但读到字节数为0 */
		/* request abort */
		state_ = kDisconnected;
		handleClose();
		return ;
	}
	else if(isZero)
	{
		/* 正常收到数据，对端可能调用shutdown或close */
		state_ = kDisConnecting;
		channel_->disableReading();
		utils::Shutdown(connfd_, SHUT_RD);	/* 关闭读半部 */
		/* HttpHandler需尝试发送应答后，再关闭连接 */
		return ;
	}

	/* everything is right */
	state_ = kHandle;
}

void HttpConnection::handleWrite(void)
{
	assert(loop_->isInLoopThread());
	int bytes = utils::writen(connfd_, __out_buffer);
	if(__out_buffer.size() == 0) 
	{
		/* 关闭写监控 */
		channel_->disableWriting();
		/* 对端已关闭写半部 */
		/* 此时，发送完应答，就可以关闭连接 */
		if(state_ == kDisConnecting)
		{
			state_ = kDisconnected;		
			handleClose();
			return ;
		}
	}
	
	if(bytes < 0)	/* 对端可能关闭连接 */
	{
		state_ = kDisconnected;
		handleClose();
	}
}

void HttpConnection::handleClose(void)
{
	assert(loop_->isInLoopThread());
	/* Channel有信号可直接触发 */
	//assert(state_ == kDisconnected);	
	
	/* 对方已关闭连接，web server直接关闭该套接字 */
	std::shared_ptr<HttpHandler> guard = holder_.lock();
	if(guard == nullptr) return ;	//TODO:
	
	channel_->disableAll();
	loop_->removeChannel(channel_);
	
	/* 关闭文件描述符 */
	//utils::Close(connfd_);//文件描述符绑定在Channel上了
}

void HttpConnection::handleError(void)
{
	assert(loop_->isInLoopThread());
	
	/* 读写已关闭的文件，触发异常 */
	state_ = kDisconnected;		
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
	
	/* 使能写监控 */
	channel_->enableWriting();
}

void HttpConnection::send(const std::string &data)
{
	this->send(static_cast<const void *>(data.data()),
	           static_cast<int>(data.size()));
}

void HttpConnection::shutdown(int how)
{
	utils::Shutdown(connfd_, how);
}

}