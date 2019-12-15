#ifndef code_HttpHandler_h
#define code_HttpHandler_h

#include <memory>
#include <map>

#include "HttpManager.h"

namespace webserver
{

class EventLoop;
class HttpConnection;
class HttpManager;

/* 持有HttpConnection */
/* 负责解析Http协议，并给予Http应答 */
class HttpHandler : public std::enable_shared_from_this<HttpHandler>
{
public:
	/* google c++ format */
	enum HttpVersion { kHttpV10, kHttpV11, kHttpUnkown, kVersionSize };
	enum HttpMethod { kGet, kPost, kHead, kOtherMethods, kMethodSize };
	enum HttpState { kStart, kPraseUrl, kPraseHeader, kPraseBody, kPraseDone, kResponse, kStateSize };
	
	static const char *kMethod[];
	static const char *kVersion[];
	
	HttpHandler(EventLoop *loop, int connfd);
	~HttpHandler();

	void newConnection(); /* 被main loop调用 */
	void handleHttpReq();

private:
	int praseUrl(std::string &buf, int bpos);
	int praseHeader(std::string &buf, int bpos);
	int praseBody(std::string &buf, int bpos);
	void responseReq();
	void keepAliveHandle();
	void badRequest(int num, const std::string &note);
	void onRequest(const std::string &body);
	
	void setMethod(const std::string &method)
	{
		method_ = kOtherMethods;
		for(int i=0; i<kMethodSize-1; ++i)
		{
			if(method == kMethod[i])
			{
				method_ = static_cast<HttpMethod>(i);
				return ;
			}
		}
	}
	
	void setPath(const std::string &path) { path_ = path; }
	void setVersion(const std::string &version)
	{
		version_ = kHttpUnkown;
		for(int i=0; i<kVersionSize-1; ++i)
		{
			if(version == kVersion[i])
			{
				version_ = static_cast<HttpVersion>(i);
				return ;
			}
		}
	}
	
	void setHeader(const std::string &key, const std::string &value) 
	{ 
		header_[key] = value; 
	}
	
private:
	EventLoop *loop_;
	int connfd_;
	std::unique_ptr<HttpConnection> connection_;
	
	HttpState state_;
	HttpMethod method_;
	HttpVersion version_;
	std::map<std::string, std::string> header_;
	std::string path_;
	std::string body_;
	bool keepAlive_;
	
	/* 变量类型不大理想 */
	HttpManager::TimerNode timerNode_;
	
	friend class HttpManager;
};

}

#endif
