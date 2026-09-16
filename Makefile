CLANG ?= clang
LLVM_STRIP ?= llvm-strip
BPFTOOL ?= bpftool
CFLAGS := -g -O2 -target bpf -D__TARGET_ARCH_x86

all: bpf/vmlinux.h bpf/telemetry.bpf.o sched/scx_bpfland.bpf.o

bpf/vmlinux.h:
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

	bpf/telemetry.bpf.o: bpf/telemetry.bpf.c bpf/maps.bpf.h bpf/vmlinux.h
		$(CLANG) $(CFLAGS) -I bpf -c $< -o $@
			$(LLVM_STRIP) -g $@

			sched/scx_bpfland.bpf.o: sched/scx_bpfland.bpf.c bpf/maps.bpf.h bpf/vmlinux.h
				$(CLANG) $(CFLAGS) -I bpf -c $< -o $@
					$(LLVM_STRIP) -g $@

					clean:
						rm -f bpf/*.o sched/*.o bpf/vmlinux.h

						.PHONY: all clean
