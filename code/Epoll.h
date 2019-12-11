#ifndef code_Epoll_h
#define code_Epoll_h

#include <vector>
#include <memory>
#include <unordered_map>

#include <sys/epoll.h>

#include "noncopyable.h"

namespace webserver
{

class Channel;

/* 
 * io-multiplex
 * oligation: add/modify/delete concerned events and 
 *            return activate events
 * owner: EventLoop, guaranteed by unique_ptr
*/
class Epoll : noncopyable
{
public:
	typedef std::shared_ptr<Channel> SP_Channel;
	typedef std::vector<SP_Channel> ChannelVector;
	
	Epoll();
	~Epoll();
	
	ChannelVector poll(int timeout);
	
	void updateChannel(SP_Channel &channel);
	void removeChannel(SP_Channel &channel);

private:
	/* internel function, be invoked by update and remove channels */
	int updateEvent(SP_Channel &channel, int operation);
	
private:
	int epollFd_;
	
	/* be used for epoll */
	std::vector<epoll_event> events_;	
	
	/* The mapping of the file descriptor to the channel */
	/* record those monitored file descriptors */
	std::unordered_map<int, SP_Channel> channelMap_;
	
	static const int kInitEventSize = 16;
};

} //namespace webserver 


#endif
