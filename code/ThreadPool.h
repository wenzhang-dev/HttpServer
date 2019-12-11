#ifndef code_ThreadPool_h
#define code_ThreadPool_h

#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

#include "Thread.h"
#include "noncopyable.h"

namespace webserver
{

class ThreadPool : noncopyable
{
public:
	typedef std::function<void ()> Task;
	ThreadPool(int threadNum, int maxQueueSize);
	~ThreadPool();
	
	void start();
	void stop();
	int addTask(const Task &task);
	
private:
	void runInThread();
	Task take();
	
private:
	std::mutex mutex_;
	std::condition_variable condition_;
	std::vector<std::unique_ptr<webserver::Thread>> threads_;
	std::deque<Task> queue_;	/* fifo */
	int maxQueueSize_;
	int maxThreadSize_;
	bool running_;
};

} //webserver

#endif
