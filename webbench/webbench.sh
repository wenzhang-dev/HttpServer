#!/bin/bash

# non keep-alive 10000并发访问60秒 Http1.1
./webbench -c 10000 -t 60 -2 --get http://192.168.1.4:20000/hello > 10k-non-keepalive.log

# keep-alive 10000并发访问60秒 HTTP1.1
./webbench -a -c 10000 -t 60 -2 --get http://192.168.1.4:20000/hello > 10k-keepalive.log

