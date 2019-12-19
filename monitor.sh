#!/bin/bash

# 监控HttpServer各线程的CPU利用率以及内存占用

ProgName=HttpServer
SleepTime=3

mainPid=$(ps -ef | grep ${ProgName} | grep -v 'grep' | awk '{print $2}')
pids=$(ls /proc/${mainPid}/task | xargs)
threadNum=$(cat /proc/${mainPid}/status | grep Threads | awk -F" " '{print $2}')
echo "monitored process pid=${pids}, ${threadNum} threads"

echo -e "PID      CPU(%)   MEM(KB)  Fds"

while [ 1 ]
do
	# check alive
	if [ ! -d "/proc/${mainPid}" ]; then
		echo "main Process was killed"
		echo "[done]!"
		break
	fi
	
	for pid in ${pids}
	do
		cpuRate=$(top -bn 1 -p ${pid} | awk -v pid=${pid} '{if ($1 == pid) print $9}')	
		memValue=$(cat /proc/${pid}/status | grep RSS | awk -F" " '{print $2}') #kB
		fdNum=$(ls /proc/${pid}/fd -l 2> /dev/null | grep "^l" | wc -l)
		
		printf "%-8s %-8.2f %-8s %-8s\n" ${pid} ${cpuRate} ${memValue} ${fdNum}
	done

	echo -ne "\r\033[${threadNum}A"
	
	sleep ${SleepTime}
	
done
exit 0
