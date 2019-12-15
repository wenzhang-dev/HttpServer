#include "HttpManager.h"

#include <strings.h>

#include <cassert>

#include "Timer.h"
#include "EventLoop.h"
#include "Channel.h"
#include "HttpHandler.h"
#include "HttpConnection.h"
#include "macros.h"
#include "config.h"

namespace webserver
{

HttpManager::HttpManager(EventLoop *loop)
	: loop_(loop),
	  timer_(new Timer(loop_))
{
	timer_->setTimerExpireCallback(std::bind(&HttpManager::handleExpireEvent, this));
	timer_->addPeriodicTimer(3);
}

HttpManager::~HttpManager()
{
	
}

void HttpManager::addNewHttpConnection(SP_HttpHandler handler)
{
	SP_Channel &channel = handler->connection_->getChannel();
	httpMap.insert(make_pair(channel, handler));
	
	handler->newConnection();
}

void HttpManager::handler(SP_Channel &channel)
{
	if(likely(channel->isReading() && httpMap.count(channel)))
	{
		SP_HttpHandler &it = httpMap[channel];
		it->handleHttpReq();
	}
}

void HttpManager::flushKeepAlive(SP_Channel channel, HttpManager::TimerNode &node)
{
	struct timeval time;
	
	bzero(&time, sizeof(time));
	int ret = gettimeofday(&time, nullptr);
	if(unlikely(ret<0)) 
	{
		perror("gettimeofday\n");
	}
	
	time.tv_sec += MAX_HTTPEXPIRETIME;
	
	if(keepAliveSet_.count(channel))
	{
		/* 刷新超时时间 */
		keepAliveList_.erase(node);
	}
	else 
	{
		/* 增加新KeepAlive连接 */
		keepAliveSet_.insert(channel);
		assert(keepAliveSet_.count(channel));
	}
	
	TimerNode it = 
			keepAliveList_.insert(keepAliveList_.end(), make_pair(channel, time));
	assert(it == --keepAliveList_.end());
	node = it;
}

void HttpManager::handleExpireEvent()
{
	struct timeval time;
	
	bzero(&time, sizeof(time));
	int ret = gettimeofday(&time, nullptr);
	if(unlikely(ret<0)) 
	{
		perror("gettimeofday\n");
	}
	
	/* 清理所有超时连接 */
	/* 不能边遍历，边删除 */
	std::list<Entry>::iterator it;
	for(it=keepAliveList_.begin(); it!=keepAliveList_.end(); ++it)
	{
		if(it->second.tv_sec > time.tv_sec)	
			break;
		
		//主动断开连接时，在delHttpConnection中将其移除
		//assert(keepAliveSet_.count(it->first));
		if(keepAliveSet_.count(it->first))
		{
			keepAliveSet_.erase(it->first);
			
			/* 类间依赖过重，不太理想 */
			/* 关闭超时连接 */
			SP_HttpHandler &handler = httpMap[it->first];
			handler->connection_->setState(HttpConnection::kDisconnected);
			handler->connection_->handleClose();
		}
	}
	keepAliveList_.erase(keepAliveList_.begin(), it);
}

}