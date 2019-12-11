#include <iostream>
#include <cassert>
#include <functional>
#include "EventLoopThread.h"

void printStr(const std::string &str)
{
	printf("%s\n", str.c_str());
}

class Foo
{
public:
	void printStr()
	{
		printf("this is foo\n");
	}
};

int main(int argc, char *argv[])
{
	webserver::EventLoopThread elt1(std::bind(&printStr, "Hello, World"));
	webserver::EventLoop *loop1 = elt1.startLoop();
	
	Foo foo;
	webserver::EventLoopThread elt2(std::bind(&Foo::printStr, &foo));
	webserver::EventLoop *loop2 = elt2.startLoop();
	
	assert(loop1 != loop2);
	printf("Done!\n");
	
	return 0;
}
