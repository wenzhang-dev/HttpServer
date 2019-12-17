HttpServer
==========
A webserver developed using modern c++

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

# Introduction
该项目参考muduo实现了一个多线程静态的web服务器。HttpServer使用c++11实现，支持head、post和get请求，使用<br>
epoll ET边沿触发提高实时性。HttpServer支持短连接、长连接，和采用timerfd实现的应用层心跳。

# Environment
Ubuntu 16.04, i5-8G<br>
gcc-5.4, g++-5.4

# Usage
* 编译<br>
    make -j4<br>
    cd webbench && make
* 运行<br>
    ./run.sh<br>
    ./webbench.sh
    
# Architecture
该项目采用reactor+NIO+thread pool+epoll ET的模型设计而成。<br>
![architecture](!pic/architecture.png)

# Performance Test
使用webbench，测试non-keepalive和keepalive Http连接下的表现。<br>
* 10k 长连接测试
![test1](pic/10k-keepalive-8threads.png)

* 1k 短链接测试
![test2](pic/1k-non-keepalive-4threads.png)

# Others
* 代码统计<br>
![code](pic/code.png)

* 内存泄露检测<br>
使用valgrid中memcheck工具，检测HttpServer内存泄露情况。<br>
检测方法: ./memcheck.sh<br>

* 监控<br>
使用top命令和/proc/${PID}/中内省，检测HttpServer运行时CPU利用率，内存以及文件描述符使用情况。<br>
使用方法: ./monitor.sh<br>
