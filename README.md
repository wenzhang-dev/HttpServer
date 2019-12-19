HttpServer Project
==========
A webserver developed using modern c++<br>

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

# Introduction
该项目参考muduo实现了一个多线程静态的web服务器。HttpServer采用c++11编写，支持Http中head、post和get<br>
请求，使用epoll ET边沿触发来提高实时性。HttpServer支持短连接、长连接，并采用timerfd实现了应用层心跳。<br>

# Environment
Ubuntu 16.04 i5-8G<br>
gcc-5.4 g++-5.4<br>

# Usage
* 编译<br>
    make -j4<br>
    cd webbench && make<br>
* 运行<br>
    ./run.sh<br>
    ./webbench.sh
    
# Architecture
该项目采用经典的reactor+NIO+thread pool+epoll ET模型设计而成。利用多核的优势，可显著提升实时响应能力。<br>
![architecture](!pic/1.svg)

# Performance Test
使用linux压测工具webbench，分别测试10k连接下，keepAlive和non-keepAlive请求。<br>
60s本地压力测试，响应能力为15 million requests/min, 传输速度为34.64 M/s 。<br>
60s同网段内测试，响应能力为0.75 million requests/min, 传输速度为1.67 M/s。<br>

# Others
* 代码统计<br>
![code](pic/code.png)

* 内存泄露检测<br>
使用valgrid中memcheck工具，检测HttpServer内存泄露情况。<br>
测试方法: ./memcheck.sh<br>

* 资源监控<br>
使用top命令和/proc/${PID}/中内省，检测HttpServer运行时CPU利用率，内存以及文件描述符使用情况。<br>
使用方法: ./monitor.sh<br>
  
