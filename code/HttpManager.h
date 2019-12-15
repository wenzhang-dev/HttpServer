#ifndef code_HttpManager_h
#define code_HttpManager_h

#include <sys/time.h>

#include <memory>
#include <functional>
#include <list>
#include <unordered_set>
#include <unordered_map>

#include "Channel.h"

namespace webserver
{
	
class HttpHandler;
class EventLoop;
class Channel;
class Timer;

/* 职责：管理所有Http连接和处理 */
/* Http处理由HttpHandler完成 */
/* Http连接使用一个单独的定时器来完成 */
/* 对于KeepAlive连接，超时断开连接(RST) */
class HttpManager
{
public:
	typedef std::shared_ptr<Channel> SP_Channel;
	typedef std::shared_ptr<HttpHandler> SP_HttpHandler;
	typedef std::pair<SP_Channel, struct timeval> Entry;
	typedef std::list<Entry>::iterator TimerNode;
	
	HttpManager(EventLoop *loop);
	~HttpManager();
	
	/* 根据channel,调用对应的HttpHandler */
	void handler(SP_Channel &channel);
	
	/* 插入HttpMap */
	void addNewHttpConnection(SP_HttpHandler hander);
	
	/* 删除某个Http连接 */
	void delHttpConnection(SP_Channel channel)
	{ 
		httpMap.erase(channel); 
		
		//Keep-Alive处理
		if(keepAliveSet_.count(channel))
		{
			keepAliveSet_.erase(channel);
		}
	}
	
	/* 更新 Http KeepAlive连接超时时间*/
	void flushKeepAlive(SP_Channel channel, HttpManager::TimerNode &node);
	
	/* 超时回调函数，用于清理超时连接 */
	void handleExpireEvent();
	
private:
	EventLoop *loop_;
	std::unique_ptr<Timer> timer_;
	
	/* 记录所有Http连接 */
	std::unordered_map<SP_Channel, SP_HttpHandler, channelHash> httpMap;
	
	/* 记录keepalive Http连接 */
	std::list<Entry> keepAliveList_;
	std::unordered_set<SP_Channel, channelHash> keepAliveSet_;
};	
	
}

/* 每个Http连接都有一个结构存储，它们的超时时间 */
/* 以便，在相同Tcp连接中的多个Http请求，可以刷新超时时间 */
/* HttpManager得有一个结构按照Http超时时间排序得序列 */
/* 每个Timer溢出时，回调函数，并处理超时连接 */
/* 使用STL list数据结构，每个Http连接持有一个指向list的迭代器 */

#endif
