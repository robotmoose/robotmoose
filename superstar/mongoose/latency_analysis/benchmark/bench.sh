#!/bin/bash

host="localhost"
url="http://$host:8080/superstar"
while [ true ]
do
	time wget -4 -nv -O foo "$url" || exit 1
done
