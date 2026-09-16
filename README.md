# Network-Queue Aware CPU Scheduling via eBPF & sched_ext

A proof-of-concept Linux CPU scheduling subsystem that mitigates tail latency for network-bound event loops and worker pools under heavy multi-core CPU contention.

Built on Linux 6.12's extensible scheduling class (**`sched_ext`**) and **eBPF**, the system bridges the architectural gap between the TCP/IP stack and CPU dispatching without modifying the kernel source tree or breaking standard POSIX socket compatibility.

---

## Architectural Problem: The SoftIRQ Telemetry Gap

In commodity Linux, process scheduling (CFS / EEVDF) and packet processing operate independently:
* Default schedulers allocate CPU runtime based on execution history, lag, and virtual deadlines.
* Incoming network packets are processed asynchronously inside software interrupt contexts (**SoftIRQs**) via NAPI polling and placed into socket receive queues (`sk_receive_queue`).
* When competing against CPU-bound background processes ("noisy neighbors"), network threads waiting on `epoll` or blocking calls experience significant runqueue queuing delays, resulting in severe $P_{99}$ tail-latency degradation.

Because SoftIRQ executes in an asynchronous interrupt context—temporarily borrowing the task structure of whichever process happens to be executing—standard inspection hooks like `bpf_get_current_pid_tgid()` return the interrupted task rather than the owning socket recipient.

---

## Solution Overview

This project implements a cross-layer telemetry bridge and a rate-limited dispatch engine:

```text
[ Ingress Packets ]
        │
        ▼ (SoftIRQ Context)
[ fentry/tcp_data_queue ] ────► Queries sock_to_pid_map ────► Updates net_queue_map (qlen)
        │
[ User Process ] ◄────────────────────────────────────────────────────┤
tcp_recvmsg() ──► [ fentry/tcp_recvmsg ] ──► Updates sock_to_pid_map│
        ▼
[ scx_bpfland_enqueue() ]
        │
        ┌─────────────────┴─────────────────┐
        ▼                                   ▼
[ q > 0 && Tokens > 0 ]              [ Fallback / Exhausted ]
        │                                   │
        ▼                                   ▼
SCX_DSQ_LOCAL                       SCX_DSQ_GLOBAL
(Core-Local Dispatch)               (Fair-Share Queue)
```

1. **Dual-Map Telemetry Bridge:**
   * **Stage 1 (User Context):** An `fentry/tcp_recvmsg` hook maps the active socket kernel pointer (`struct sock *`) to the calling thread's ID/PID in `sock_to_pid_map`.
   * **Stage 2 (SoftIRQ Context):** An `fentry/tcp_data_queue` hook uses the socket pointer to identify the owning thread, reads the receive queue depth (`sk_receive_queue.qlen`) via BPF CO-RE, and updates `net_queue_map`.
   * **Cleanup:** An `fentry/tcp_close` tracepoint deterministic deletes entries to prevent stale pointer re-use.

2. **Bounded Token-Bucket Dispatch:**
   * Raw `q > 0` checks introduce denial-of-service / priority inversion risks if a socket is deliberately flooded.
   * The scheduler tracks per-task token quotas in `token_bucket_map`.
   * Tasks with pending network backlog consume a token and route to the local dispatch queue (`SCX_DSQ_LOCAL`) for low-latency scheduling.
   * If the token budget is exhausted or no backlog exists, the task falls back to standard fair-share scheduling via `SCX_DSQ_GLOBAL`.

---

## Empirical Benchmark Highlights

Evaluations conducted under 100% all-core CPU contention (`stress-ng` matrix multiplication) across 10 independent iterations:

* **Redis (Single-Threaded Event Loop):**
  * **Mean $P_{99}$ Latency:** Reduced from **11.8 ms** (EEVDF) to **5.0 ms** (**-57.6%**)[cite: 5].
  * **$P_{99}$ Variance:** Reduced from $\sigma = 3.2\,\text{ms}$ to $\sigma = 0.5\,\text{ms}$ (**84.3% tighter consistency**)[cite: 5].
  * **Throughput:** Increased from **15,400** to **23,500 req/sec** (**+52.6%**)[cite: 5].
* **Memcached (Multi-Threaded Worker Pool):**
  * **Mean $P_{99}$ Latency:** Reduced by **23.1%** (from 37.11 ms to 28.54 ms)[cite: 2, 3].
  * **Tradeoff:** A marginal throughput dip of 1.9% (35,208 to 34,513 req/sec) and a modest increase in median ($P_{50}$) latency (3.48 ms to 4.06 ms)[cite: 2, 3], highlighting the system-level cost of rescuing worst-case tail packets.
* **Comparison with Native `SO_BUSY_POLL`:**
  * Native Linux busy polling (50 µs) collapsed under multi-core CPU contention, causing $P_{99}$ spikes up to 133 ms and dropping throughput to ~12,097 req/sec[cite: 4] due to execution quanta being wasted in spin-loops.

> **Note on Environment:** Testing was conducted within a virtualized Linux development environment. Benchmarks demonstrate the execution mechanics of the *scheduler pathway* under contention, rather than bare-metal physical NIC interrupt steering.

---

## Repository Structure

```text
├── bpf/
│   ├── telemetry.bpf.c       # eBPF fentry probes (recvmsg, data_queue, close)
│   └── maps.bpf.h            # BPF map definitions (sock_to_pid, net_queue, tokens)
├── sched/
│   └── scx_bpfland.bpf.c     # struct_ops scheduler enqueue and dispatch logic
├── benchmarks/
│   ├── run_rigorous.sh       # 10-iteration Redis benchmark harness under stress-ng
│   ├── run_memcached.sh      # memtier_benchmark harness for multi-threaded testing
│   └── run_load_sweep.sh     # Client scaling load-sweep script
├── scripts/
│   └── generate_graphs.py    # Python/matplotlib scripts for IEEE-style latency plots
└── README.md
```

## Prerequisites

* **Operating System:** Ubuntu 24.04 LTS or Fedora 40
* **Kernel:** Linux >= 6.12 compiled with `CONFIG_SCHED_CLASS_EXT=y`
* **Toolchain:**
  ```bash
  sudo apt-get install -y clang llvm libbpf-dev bpftool make
  ```

- **Workload Drivers:** Redis (`redis-server`, `redis-benchmark`), Memcached, `memtier_benchmark`, and `stress-ng`.

## Quick Start

### 1. Build the BPF Probes and Scheduler

```bash
git clone https://github.com/<your-username>/network-queue-aware-scheduler.git
cd network-queue-aware-scheduler
make
```

### 2. Run the Custom Scheduler

Ensure the current user has `CAP_SYS_ADMIN` or run as root:

```bash
sudo ./bin/scx_bpfland
```

### 3. Run Contention Benchmarks

In a second terminal:

```bash
# Benchmark Redis under 100% all-core CPU contention
chmod +x benchmarks/run_rigorous.sh
./benchmarks/run_rigorous.sh
```

## Academic & Implementation Disclosures

- **Workload Scope:** Designed primarily for event-loop datastores and dedicated worker-pool models where socket ownership is stable at the PID/TID boundary.

- **FD Multiplexing:** Applications multiplexing thousands of heterogeneous sockets over a single thread (e.g., Envoy/Nginx) require socket priority tagging (`SO_PRIORITY`) to avoid indiscriminate thread boosting.

- **Safety:** Relies on upstream `sched_ext` safety boundaries. Any verification failure or runtime abort triggers automatic fallback to default EEVDF.

## License

This project is licensed under the [GPL-2.0-only](https://www.google.com/search?q=LICENSE&authuser=4) for kernel BPF components and [MIT](https://www.google.com/search?q=LICENSE-MIT&authuser=4) for user-space scaffolding.
