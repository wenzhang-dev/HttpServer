HttpServer Project
==========
A webserver developed using modern c++<br>

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

# Introduction
This project implements a multithreaded static web server with reference to Muduo. HttpServer is implemented with C++ 11, it supports head, post and get requests, and uses epoll ET edge trigger to improve real-time performance. HttpServer supports short connection, long connection and application layer heartbeat implemented by timerfd.

# Environment
Ubuntu 16.04;<br>
i5-8G;<br>
gcc-5.4;<br>
g++-5.4;<br>

# Usage
* compilation<br>
    make -j4<br>
    cd webbench && make<br>
* Function<br>
    ./run.sh<br>
    ./webbench.sh
    
# Architecture
The project is designed with the model of reactor + NIO + thread pool + epoll ET.<br>
![architecture](!pic/architecture.png)

# Performance Test
This project uses webbench to test the performance of non-keepalive and keepalive HTTP connections<br>
* 10k Long connection test<br>
![test1](pic/10k-keepalive-8threads.png)

* 1k Short link test<br>
![test2](pic/1k-non-keepalive-4threads.png)

# Others
* Code statistics<br>
![code](pic/code.png)

* Memory leak detection<br>
This project uses MemCheck tool in valgrid to detect httpServer memory leak.<br>
Test method: ./memcheck.sh<br>

* Monitor<br>
This project uses the top command and /proc/${PID}/ introspection to detect httpserver runtime CPU utilization, memory, and file descriptor usage.<br>
Usage method: ./monitor.sh<br>
