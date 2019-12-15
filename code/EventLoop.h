#ifndef code_EventLoop_h
#define code_EventLoop_h

#include <mutex>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include "CurrentThread.h"
#include "HttpManager.h"

namespace webserver
{

class Epoll;
class Channel;
class HttpHandler;
class HttpManager;

class EventLoop
{
public:
	typedef std::function<void ()> Functor;
	typedef std::shared_ptr<Channel> SP_Channel;
	typedef std::vector<SP_Channel> ChannelVector;
	typedef std::shared_ptr<HttpHandler> SP_HttpHandler;
	
	EventLoop();
	~EventLoop();
	
	void loop();
	void quit();
	
	/* run callback immdiately in the loop thread */
	void runInLoop(Functor &&cb);
	
	/* queue callback in the loop thread */
	void queueInLoop(Functor &&cb);
	
	/* assert whether in the loop thread or not */
	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
	
	/* internel usage */
	void updateChannel(SP_Channel channel);
	void removeChannel(SP_Channel channel);
	
	void doPendingFunctors();
	
	void wakeupRead();
	void wakeup();
	
	static EventLoop* getEventLoopOfCurrentThread();
	
	/* support Http */
	void addHttpConnection(SP_HttpHandler handler);
	void flushKeepAlive(SP_Channel &channel, HttpManager::TimerNode &node);
	
private:
	bool looping_;
	bool quit_;
	pid_t threadId_;
	std::unique_ptr<Epoll> poller_;
	int wakeupFd_;
	std::shared_ptr<Channel> wakeupChannel_;
	ChannelVector activateChannels_;
	bool callingPendingFucntors_;
	std::vector<Functor> pendingFunctors_;
	
	std::mutex mutex_;	/* be used by functor vector */
	
	/* 由事件循环处理各个Http请求 */
	/* 先调用Channel的handleEvent，接受数据 */
	/* 各个事件循环管理Http连接（通断，清理） */
	std::unique_ptr<HttpManager> manager_;
};

}

#endif
