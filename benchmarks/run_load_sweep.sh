#!/bin/bash

ITERATIONS=10
CLIENT_COUNTS=(10 50 100 250 500)
BASE_DIR="./sweep_results"

sudo cpupower frequency-set --governor performance > /dev/null 2>&1

for clients in "${CLIENT_COUNTS[@]}"; do
    OUTPUT_DIR="${BASE_DIR}/clients_${clients}"
        mkdir -p $OUTPUT_DIR
            echo "=== Running Sweep for $clients Clients ==="

                for i in $(seq 1 $ITERATIONS); do
                        killall -9 redis-server stress-ng 2>/dev/null
                                sleep 1

                                        redis-server --port 6379 --save "" --appendonly no --protected-mode no > /dev/null 2>&1 &
                                                REDIS_PID=$!
                                                        sleep 2

                                                                stress-ng --cpu $(nproc) --timeout 30s > /dev/null 2>&1 &
                                                                        STRESS_PID=$!
                                                                                sleep 1

                                                                                        redis-benchmark -h 127.0.0.1 -p 6379 -c $clients -n 100000 -t get,set --csv \
                                                                                                    > "$OUTPUT_DIR/redis_run_${i}.csv"

                                                                                                            kill -9 $REDIS_PID 2>/dev/null
                                                                                                                    kill -9 $STRESS_PID 2>/dev/null
                                                                                                                            wait $REDIS_PID 2>/dev/null
                                                                                                                                    wait $STRESS_PID 2>/dev/null
                                                                                                                                            sleep 2
                                                                                                                                                done
                                                                                                                                                done
                                                                                                                                                echo "=== Load Sweep Complete ==="
