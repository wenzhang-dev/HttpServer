#include "HttpHandler.h"

#include <cassert>

#include "Channel.h"
#include "HttpConnection.h"
#include "EventLoop.h"
#include "macros.h"

namespace webserver
{

const char *HttpHandler::kMethod[] = {"GET", "POST", "HEAD", "Unknown"};
const char *HttpHandler::kVersion[] = {"HTTP/1.0", "HTTP/1.1", "Unknown"};

HttpHandler::HttpHandler(EventLoop *loop, int connfd)
	: loop_(loop),
	  connfd_(connfd),
	  connection_(new HttpConnection(loop_, connfd_)),
	  state_(kStart)
{
	assert(connfd_ > 0);
}

HttpHandler::~HttpHandler()
{
	printf("dtor HttpHandler\n");
}

//在当前event loop
//仅被调用一次
void HttpHandler::newConnection()
{
	std::shared_ptr<Channel> channel = connection_->getChannel();
	loop_->addHttpConnection(shared_from_this(), channel);
	
	connection_->setDefaultCallback();
	connection_->setHolder(shared_from_this());
	channel->enableReading();
}

/* HTTP 1.1: 多个Http请求，不能重叠执行 */
void HttpHandler::handleHttpReq()
{
	printf("handleHttpReq\n");
		
	/* ROV优化, 不必忧心效率 */
	std::string buffer = connection_->getRecvBuffer();
	//printf("buffer empty: %s\n", buffer.size()==0?"True":"False");
	
	/* bpos当前位置，epos下一行位置 */
	int bpos = 0;
	int epos = praseUrl(buffer, bpos);
	if(unlikely(epos < 0))
	{
		//错误处理, 设置状态 bad request
		state_ = kPraseUrl;
		goto __err;
	}
	
	bpos = epos;
	epos = praseHeader(buffer, bpos);
	if(unlikely(epos < 0))
	{
		//错误处理, 设置状态 bad request
		state_ = kPraseHeader;
		goto __err;
	}
	
	bpos = epos;
	epos = praseBody(buffer, bpos);
	if(unlikely(epos < 0))
	{
		//错误处理, 设置状态 bad request
		state_ = kPraseBody;
		goto __err;
	}
	state_ = kPraseDone;
	
__err:
	/* 根据解析状态，返回结果 */
	responseReq();
	
	/* keepAlive? 连接处理：断开 or 保持 */
	
}

/* 解析请求行，发生错误时返回-1，否则返回Header的索引位置 */
int HttpHandler::praseUrl(std::string &buf, int bpos)
{
	/* 提取请求行 */
	std::string::size_type epos = buf.find("\r\n", bpos);
	if(epos == std::string::npos) return -1;
	
	/* 解析请求方法 */
	std::string::size_type space = buf.find(" ", bpos);
	if(space == std::string::npos || space > epos) return -1;
	
	setMethod(buf.substr(bpos, space-bpos));
	printf("method:%s\n", kMethod[method_]);
	if(kMethod[method_] == std::string("Unknown")) return -1;
	
	/* 解析请求资源路径 */
	bpos = space+1;
	space = buf.find(" ", bpos);
	if(space == std::string::npos || space > epos) return -1;
	
	setPath(buf.substr(bpos, space-bpos));
	printf("path:%s\n", path_.c_str());
	
	/* 解析Http版本号 */
	bpos = space+1;
	setVersion(buf.substr(bpos, epos-bpos));
	printf("version:%s\n", kVersion[version_]);
	if(kVersion[version_] == std::string("Unknown")) return -1;
	
	return epos+2;
}

/* 解析Header，发生错误时返回-1，否则返回Body的索引位置 */
int HttpHandler::praseHeader(std::string &buf, int bpos)
{
	std::string::size_type epos = bpos;
	
	while(static_cast<int>(epos = buf.find("\r\n", bpos)) != bpos)
	{
		//printf("%s\n", buf.substr(bpos, epos-bpos).c_str());
		
		std::string::size_type sep = buf.find(":", bpos);
		if(sep == std::string::npos || sep > epos) return -1;
		
		while(buf[bpos] == ' ') bpos++;
		std::string key(buf.substr(bpos,sep-bpos));
		
		sep += 1;
		while(buf[sep] == ' ') sep++;
		std::string value(buf.substr(sep, epos-sep));
		
		setHeader(key, value);
		
		bpos = epos+2;
	}
	
	for(auto &p : header_)
	{
		printf("%s: %s\n", p.first.c_str(), p.second.c_str());
	}

	return epos+2;
}

/* 解析Body，发生错误时返回-1，否则返回0 */
int HttpHandler::praseBody(std::string &buf, int bpos)
{
	/* 非Post，不解析body */
	if(method_ != kPost) return 0;
	
	/* Body长度 */
	if(!header_.count("Content-length")) return -1;
	
	int bodyLen = stoi(header_["Content-length"]);
	if(bodyLen < static_cast<int>(buf.size())-bpos) return -1;
	
	/* inefficient!! */
	body_ = buf.substr(bpos);
	printf("body:%s\n", body_.c_str());

	return 0;
}

void HttpHandler::responseReq()
{
	std::string header;
	/* 根据解析状态，响应Http请求 */
	if(state_ != kPraseDone)
	{
		//bad request 400
		header += "HTTP/1.1 400 Bad Request\r\n\r\n";
		connection_->send(header);
		
		return ;
	}
	
	/* 解析请求资源 */
	if(path_ == "/")
	{
		header += "HTTP/1.1 200 OK\r\n";
		connection_->send(header);
	}
	
	
	//404 Not Found
	
}

}