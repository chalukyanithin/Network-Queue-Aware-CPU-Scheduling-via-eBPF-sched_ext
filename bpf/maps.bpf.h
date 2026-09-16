#ifndef __MAPS_BPF_H__
#define __MAPS_BPF_H__

#include <vmlinux.h>
#include <bpf/bpf_helpers.h>

/* Maps active socket pointer (sk) to consuming Process ID */
struct {
	    __uint(type, BPF_MAP_TYPE_HASH);
	        __uint(max_entries, 65536);
	            __type(key, __u64);   /* struct sock * */
	                __type(value, __u32); /* PID */
	                } sock_to_pid_map SEC(".maps");

	                /* Maps PID to current observed receive queue length */
	                struct {
	                	    __uint(type, BPF_MAP_TYPE_HASH);
	                	        __uint(max_entries, 16384);
	                	            __type(key, __u32);   /* PID */
	                	                __type(value, __u32); /* qlen */
	                	                } net_queue_map SEC(".maps");

	                	                /* Bounded token bucket for rate-limiting preferential dispatch */
	                	                struct {
	                	                	    __uint(type, BPF_MAP_TYPE_HASH);
	                	                	        __uint(max_entries, 16384);
	                	                	            __type(key, __u32);   /* PID */
	                	                	                __type(value, __u32); /* Remaining tokens */
	                	                	                } token_bucket_map SEC(".maps");

	                	                	                #endif /* __MAPS_BPF_H__ */
	                	                }
	                }
}
