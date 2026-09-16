#!/bin/bash

echo "--- Starting Memcached ---"
memcached -p 11211 -t 4 -u nobody > /dev/null 2>&1 &
MEM_PID=$!
sleep 2

echo "--- Starting CPU Stress ---"
stress-ng --cpu $(nproc) --timeout 30s > /dev/null 2>&1 &
STRESS_PID=$!
sleep 1

echo "--- Running memtier_benchmark ---"
memtier_benchmark -s 127.0.0.1 -p 11211 -P memcache_binary -c 50 -t 4 -n 10000 --ratio=1:1 -x 1 

echo "--- Cleaning up ---"
kill $MEM_PID 2>/dev/null
wait $STRESS_PID 2>/dev/null
