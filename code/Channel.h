#ifndef code_Channel_h
#define code_Channel_h

#include <memory>
#include <functional>

#include <sys/epoll.h>

namespace webserver
{

class EventLoop;
struct channelHash;
struct channelCmp;

/* event dispatcher
 * obligation: handle activate events in this file descriptor 
 * owner: EventLoop, and each file descriptor corresponds to a Channel
 */
class Channel : public std::enable_shared_from_this<Channel>
{
public:
	typedef std::function<void ()> EventCallback;

	Channel(int fd, EventLoop *loop);
	~Channel()
	{ 
		//printf("dtor channel\n"); 
	}
	
	/* event dispatcher */
	void handleEvent();
	
	void setReadCallback(EventCallback cb)
	{ readCallback_ = std::move(cb); }
	void setWriteCallback(EventCallback cb)
	{ writeCallback_ = std::move(cb); }
	void setCloseCallback(EventCallback cb)
	{ closeCallback_ = std::move(cb); }
	void setErrorCallback(EventCallback cb)
	{ errorCallback_ = std::move(cb); }
	
	int getFd() const { return fd_; }
	int events() const { return events_; }
	int revents() const { return revents_; }
	void set_revents(int revt) { revents_ = revt; }
	bool isNoneEvent() const { return events_ == kNoneEvent; }
	
	void enableReading() 
	{ events_ |= (kReadEvent | EPOLLET); update(); }
	
	void disableReading() 
	{ events_ &= ~kReadEvent; update(); }
	
	void enableWriting() 
	{ events_ |= (kWriteEvent | EPOLLET); update(); }
	
	void disableWriting() 
	{ events_ &= ~kWriteEvent; update(); }
	
	void disableAll() 
	{ events_ = kNoneEvent; update(); }
	
	bool isWriting() const 
	{ return revents_ & kWriteEvent; }
	
	bool isReading() const 
	{ return revents_ & kReadEvent; }

	bool isEnableReading() const 
	{ return events_ & kReadEvent; }

	bool isEnableWriting() const
	{ return events_ & kWriteEvent; }

	EventLoop *ownerLoop() const 
	{ return loop_; }
	
private:
	void update();
	
private:
	const int fd_;
	int events_;
	int revents_;
	EventLoop * const loop_;
	
	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;
	
	/* ET mode */
	static const int kNoneEvent = 0;
	static const int kReadEvent = EPOLLIN | EPOLLPRI;
	static const int kWriteEvent = EPOLLOUT;
};

struct channelHash
{
	std::size_t operator()(const std::shared_ptr<Channel> &key) const
	{ 
		return std::hash<int>()(key->getFd());
	}
};

struct channelCmp
{
	bool operator()(const std::shared_ptr<Channel> &lhs, 
	                const std::shared_ptr<Channel> &rhs) const
	{
		return lhs->getFd() < rhs->getFd();
	}
};

} //namespace webserver

#endif
