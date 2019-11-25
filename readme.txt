Web Server构思

google C++编程规范
	代码风格检查 cpplint
	
modern C++，C++11为主，C++14为辅
	stl、boost

尝试使用OO设计方法，可以使用一些设计模式GOF
	template mothod

多线程，高并发，c100k
	充分发挥多核优势

同步与互斥原语
	mutex、cond

IO模式
	reactor vs. proactor
	前者是同步IO，后者是异步IO，需要os支持异步功能

异步IO模式
	epoll vs. select
	电平触发LT vs. 边沿触发ET
	
缓冲区buffer问题
	非阻塞IO，tcp连接需要发送和接收缓冲
	
优雅关闭socket方法
	shutdown关闭发送，或接受半部
	
并发数限制
	程序总是占用一个空闲描述符，用于断开新连接
	
线程池
	多个reactor，增强系统处理能力
	提供负载均衡，可在不同工作线程中动态分配任务

清理空闲连接
	定时器定时处理闲置的连接

异步日志

压力测试
	webbench
	python脚本完成自动化测试

内存泄露检查
	valgrid工具
	RAII，用类来管理资源


查阅其他方面资料
	memcached、leveldb、rocksdb、redis、nginx
	libevent、libuv
	boost.asio
	

c++11标准库提供原语和线程的支持
std::mutex lock_;
std::unique_lock(std::mutex) lock(lock_);
std::lock_guard

std::thread thread_

std::atomic 原子运算

std::mutex vs. pthread_mutex_t
	在posix平台，std::mutex是对pthread_mutex_t的一层很薄的封装
	lock_guard仅持有mutex的一个引用，并且在析构时unlock

desicion：原语使用std中的mutex和lock_guard
		  线程使用posix，自己实现。（std不支持线程工作工作函数分配以及线程池）

为什么在thread类中使用latch？
	为了在Thread构造函数中，就拿到正确得tid值。
	所以必须等到Thread被调度时，才能拿到tid值。

thread_local vs. __thread？
	thread_local在std中实现(可修饰类)，__thread在gcc中实现（POD类型）。
	两者都是线程私有变量，在线程创建时被构造。

posix gettid?
	posix不提供gettid封装
	可以使用syscall(SYS_gettid)来做





	

