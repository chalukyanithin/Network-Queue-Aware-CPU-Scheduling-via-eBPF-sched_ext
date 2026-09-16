#!/bin/bash

ITERATIONS=10
CLIENTS=50
REQUESTS=100000

sudo cpupower frequency-set --governor performance > /dev/null 2>&1

echo "=== Starting 10-Iteration Contention Benchmark ==="

for i in $(seq 1 $ITERATIONS); do
    echo "--- Iteration $i of $ITERATIONS ---"

            killall -9 redis-server stress-ng 2>/dev/null
                sleep 1

                    redis-server --port 6379 --save "" --appendonly no --protected-mode no > /dev/null 2>&1 &
                        REDIS_PID=$!
                            sleep 2

                                stress-ng --cpu $(nproc) --cpu-method matrixprod --timeout 60s > /dev/null 2>&1 &
                                    STRESS_PID=$!
                                        sleep 1

                                            redis-benchmark -h 127.0.0.1 -p 6379 -c $CLIENTS -n $REQUESTS -t get,set --csv

                                                kill -9 $REDIS_PID 2>/dev/null
                                                    kill -9 $STRESS_PID 2>/dev/null
                                                        wait $REDIS_PID 2>/dev/null
                                                            wait $STRESS_PID 2>/dev/null
                                                                sleep 2
                                                                done

                                                                echo "=== Benchmark Complete ==="
