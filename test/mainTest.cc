#include <iostream>
#include "HttpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"

/* 8核16线程 */
int main(int argc, char *argv[])
{
	webserver::InetAddress self_addr(20000);
	webserver::EventLoop mainLoop;
	
	webserver::HttpServer server(&mainLoop, self_addr, 3);
	server.start();
	
	mainLoop.loop();
	
	return 0;
}