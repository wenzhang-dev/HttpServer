#!/bin/bash

# 使用valgrind中memcheck工具
# 生成日志文件valgrind_Server.log
G_SLICE=always-malloc G_DEBUG=gc-friendly  valgrind -v --tool=memcheck --leak-check=full --num-callers=40 --log-file=valgrind_Server.log ./HttpServer

