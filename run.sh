#!/bin/bash

# 12w打开文件上限
ulimit -n 120000

# 无限制coredump文件大小
ulimit -c unlimited

./HttpServer > run.log

