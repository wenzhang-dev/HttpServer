#!/bin/bash
i=0
times=$2

while [ $i -le $times ]
do
	let 'i++'
	./$1
done
