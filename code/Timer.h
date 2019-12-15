#ifndef code_Timer_h
#define code_Timer_h

#include <memory>
#include <functional>

namespace webserver
{

class Channel;
class EventLoop;

class Timer
{
public:
	typedef std::function<void ()> callback;
	Timer(EventLoop *loop);
	~Timer();
	
	void addTimer(int nS);			/* 设置定时 */
	void reset(void);				/* 重置定时器 */
	void addPeriodicTimer(int nS);
	
	/* 超时调用函数 */
	void setTimerExpireCallback(callback cb)
	{ expireCallback_ = std::move(cb); }
	
	void readCallback();
	
private:
	EventLoop *loop_;
	int timerFd_;
	std::shared_ptr<Channel> timerChannel_;
	int timeout_;
	callback expireCallback_;	/* timerfd可读时被调用 */
};

}// namespace webserver

#endif
