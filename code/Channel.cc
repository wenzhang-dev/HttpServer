#include "Channel.h"

#include "EventLoop.h"
#include "utils.h"

namespace webserver
{

/* The file descriptor is bound to the Channel */
Channel::Channel(int fd, EventLoop *loop)
	: fd_(fd),
	  events_(0),
	  revents_(0),
	  loop_(loop)
{}

/* Channel负责关闭文件描述符 */
Channel::~Channel()
{
	//printf("dtor channel\n");
	utils::Close(fd_);
}

/* event dispatcher */
void Channel::handleEvent()
{
	if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
	{
		if(closeCallback_) closeCallback_();
		return ;
	}
	
	if(revents_ & EPOLLERR)
	{
		if(errorCallback_) errorCallback_();
		return ;
	}
	
	if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if(readCallback_) readCallback_();
	}
	
	if(revents_ & EPOLLOUT)
	{
		if(writeCallback_) writeCallback_();
	}
}

/* update state of the Channel in poller */
/* may be ineffecient */
void Channel::update(void)
{
	loop_->updateChannel(shared_from_this());
}

} //webserver
