#ifndef code_Thread_h
#define code_Thread_h

#include <pthread.h>

#include <functional>
#include <atomic>

#include "CountDownLatch.h"
#include "noncopyable.h"
#include "CurrentThread.h"

namespace webserver
{

class Thread : noncopyable
{
public:
	typedef std::function<void()> ThreadFunc;
	explicit Thread(const ThreadFunc &func, const std::string &name = std::string());
	~Thread();
	
	void start();
	bool started() const { return started_; }
	int join();
	pid_t tid() const { return tid_; }
	const std::string &name() const { return name_; }
	
	static int numCreated() { return threadNum_; }
	
private:
	void setDefaultName();
	
private:
	pthread_t pthreadId_;
	pid_t tid_;
	ThreadFunc func_;
	std::string name_;
	bool started_;
	bool joined_;
	/* make sure to get the correct tid after the thread runs */
	CountDownLatch latch_;
	
	static std::atomic<int> threadNum_;
};
	
} //namespace webserver


#endif