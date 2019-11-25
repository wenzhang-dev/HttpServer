#include "CountDownLatch.h"

namespace webserver
{
	
CountDownLatch::CountDownLatch(int count)
	: count_(count)
{}

void CountDownLatch::wait()
{
	std::unique_lock<std::mutex> lock(mutex_);
	/* spurious wakeup */
	while(count_ > 0)
		condition_.wait(lock);
}

void CountDownLatch::countDown()
{
	std::unique_lock<std::mutex> lock(mutex_);
	--count_;
	if(count_ == 0)
		condition_.notify_all();
}

}
