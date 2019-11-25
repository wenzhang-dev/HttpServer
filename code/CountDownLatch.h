#ifndef code_CountDownLatch_h
#define code_CountDownLatch_h

#include <mutex>
#include <condition_variable>

#include "noncopyable.h"

namespace webserver
{

class CountDownLatch : noncopyable
{
public:
	explicit CountDownLatch(int count);
	
	void wait();
	void countDown();
	int getCount();
	
private:
	mutable std::mutex mutex_;
	std::condition_variable condition_;
	int count_;
};

}

#endif
