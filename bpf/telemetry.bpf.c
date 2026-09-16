#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "maps.bpf.h"

char LICENSE[] SEC("license") = "GPL";

/* Hook: synchronous user-space socket receive */
SEC("fentry/tcp_recvmsg")
int BPF_PROG(trace_tcp_recvmsg, struct sock *sk) {
	    __u64 sk_ptr = (__u64)sk;
	        __u64 pid_tgid = bpf_get_current_pid_tgid();
	            __u32 pid = pid_tgid >> 32;

	                bpf_map_update_elem(&sock_to_pid_map, &sk_ptr, &pid, BPF_ANY);
	                    return 0;
	                    }

	                    /* Hook: asynchronous SoftIRQ packet enqueue */
	                    SEC("fentry/tcp_data_queue")
	                    int BPF_PROG(trace_tcp_data_queue, struct sock *sk) {
	                    	    __u64 sk_ptr = (__u64)sk;
	                    	        __u32 *pid_ptr = bpf_map_lookup_elem(&sock_to_pid_map, &sk_ptr);

	                    	            if (!pid_ptr)
	                    	                    return 0;

	                    	                        __u32 qlen = BPF_CORE_READ(sk, sk_receive_queue.qlen);
	                    	                            bpf_map_update_elem(&net_queue_map, pid_ptr, &qlen, BPF_ANY);
	                    	                                return 0;
	                    	                                }

	                    	                                /* Hook: socket teardown cleanup */
	                    	                                SEC("fentry/tcp_close")
	                    	                                int BPF_PROG(trace_tcp_close, struct sock *sk) {
	                    	                                	    __u64 sk_ptr = (__u64)sk;
	                    	                                	        bpf_map_delete_elem(&sock_to_pid_map, &sk_ptr);
	                    	                                	            return 0;
	                    	                                	            }
	                    	                                }
	                    }
}
