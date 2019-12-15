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
	与LT相比，ET模型是通过减少系统调用来达到提高并行效率
	ET模型更容易丢失事件
	ET模型下，每次读写缓冲区必须处理干净
	对于socket监听套接字，在ET模型下，需要循环执行accept，直到返回EAGAIN 
	
缓冲区buffer问题
	非阻塞IO，tcp连接需要发送和接收缓冲
	
优雅关闭socket方法
	shutdown关闭发送或接受半部
	
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

emplace_back vs. push_back
	两者都可以在vector、deque、string等数据结构后添加一个元素。
	当使用push_back时，添加一个临时变量（右值），push_back会新构造一个对象，在放入容器。
	使用emplace_back时，可以使用临时对象，在原地构造，没有赋值和移动操作。
	push_back也重载了右值引用参数，可以使用std::move减少一个构造和拷贝操作。
	
std:move
	将左值强制转换为右值，并将对象所有权转移。
	vector<string> vec;
	string str("Hello");
	vec.push_back(move(str)); //不应该再使用str，其值未定义
	
类职责划分
	EventLoop 事件循环类
		调用epoll，返回活跃事件
		根据活跃事件，回调处理函数
		
	Epoll 检测活跃事件类
		根据检测的fd的活跃事件，调用epoll
		装填Channel，返回活跃事件
		
	Channel 事件分发类
		固定检测某一个fd，注册每类事件的回调函数

muduo设计EpollPoller::fillActiveChannel存在的问题
	针对每个监控事件，对epoll_event的data域，设置为指向Channel的指针
	
	针对Channel的管理过于分散？？
	
c++函数返回值不是右值
	一般情况c++编译器将为函数的返回值做RVO优化，较少一个对象构造和析构

shared_from_this
	在类对象中返回该对象的shared_ptr方法。但该类需要继承enable_shared_from_this<class>。

errno线程安全
	man 3 errno，可以看到errno在Posix标准中要求为线程安全。

使用linux nc命令可以测试连接和通信 tcp/udp均可
	nc -nv 127.0.0.1 20000  client
	nc -lvp 80              server
	-u 使用udp协议
	-p 指定端口
	-n 指定ip地址
	-l 监听模式
	-v 显示执行过程
	
学习http协议，可以使用
下面两个命令可以获得http响应包
1、使用curl命令：
　　curl "http://www.baidu.com" 如果这里的URL指向的是一个文件或者一幅图都可以直接下载到本地
　　curl -i "http://www.baidu.com" 显示全部信息
　　curl -l "http://www.baidu.com" 只显示头部信息
　　curl -v "http://www.baidu.com" 显示get请求全过程解析

2、使用wget命令：
　　wget "http://www.baidu.com"也可以

使用tcpdump -i eth0 -vvv port 80
	抓包
	
Channel分布
	每个EventLoop有一个wakeupChannel，用于其他Eventloop来唤醒
	每个HttpConnection有一个Channel
	main EventLoop有一个监听Channel

find_first_of和find区别
	find作用为查找子串
	find_first_of返回匹配任意一个字符的位置。
	
最棘手的问题
	epoll中EPOLLOUT重复触发，取消监听EPOLLOUT事件也是如此
	添加EPOLLONESHOT也是如此

调用shutdown也会触发读0，而此时应该发送完数据后才关闭连接