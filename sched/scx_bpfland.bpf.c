#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include "../bpf/maps.bpf.h"

char LICENSE[] SEC("license") = "GPL";

/* sched_ext default constants */
#define SCX_DSQ_GLOBAL 0
#define SCX_DSQ_LOCAL  1
#define SCX_SLICE_DFL  0

/* Fallback prototype if vmlinux.h lacks scx kfuncs */
extern void scx_bpf_dispatch(struct task_struct *p, __u64 dsq_id,
                            __u64 slice, __u64 enq_flags) __ksym;

                            SEC("struct_ops/bpfland_enqueue")
                            void BPF_PROG(bpfland_enqueue, struct task_struct *p, __u64 enq_flags) {
                            	    __u32 pid = p->pid;
                            	        __u32 *qlen = bpf_map_lookup_elem(&net_queue_map, &pid);
                            	            __u32 *tokens = bpf_map_lookup_elem(&token_bucket_map, &pid);

                            	                /* Preferential dispatch when queue has data and tokens remain */
                            	                    if (qlen && *qlen > 0) {
                            	                    	        if (tokens && *tokens > 0) {
                            	                    	        	            __sync_fetch_and_sub(tokens, 1);
                            	                    	        	                        scx_bpf_dispatch(p, SCX_DSQ_LOCAL, SCX_SLICE_DFL, enq_flags);
                            	                    	        	                                    return;
                            	                    	        	                                            }
                            	                    	        	                                                }

                            	                    	        	                                                    /* Standard fair-share fallback */
                            	                    	        	                                                        scx_bpf_dispatch(p, SCX_DSQ_GLOBAL, SCX_SLICE_DFL, enq_flags);
                            	                    	        	                                                        }
                            	                    	        }
                            	                    }
                            }
