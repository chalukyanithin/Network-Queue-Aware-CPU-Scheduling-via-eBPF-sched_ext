import matplotlib.pyplot as plt
import numpy as np

# Configure styling for IEEE publication compatibility
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.size'] = 11

def plot_tail_latency():
    labels = ['Default (EEVDF)', 'Custom scx_bpfland']
        means = [11.8, 5.0]
            ci = [2.3, 0.4]

                x = np.arange(len(labels))
                    fig, ax = plt.subplots(figsize=(6, 4))
                        bars = ax.bar(x, means, yerr=ci, capsize=5, color=['#7f7f7f', '#1f77b4'], edgecolor='black', width=0.5)

                            ax.set_ylabel('P99 Tail Latency (ms)')
                                ax.set_title('Redis P99 Latency Under 100% CPU Contention')
                                    ax.set_xticks(x)
                                        ax.set_xticklabels(labels)
                                            ax.grid(axis='y', linestyle='--', alpha=0.7)

                                                plt.tight_layout()
                                                    plt.savefig('ieee_latency_chart.pdf')
                                                        print("Generated: ieee_latency_chart.pdf")

                                                        def plot_throughput():
                                                            labels = ['Default (EEVDF)', 'Custom scx_bpfland']
                                                                means = [15400, 23500]
                                                                    ci = [1120, 680]

                                                                        x = np.arange(len(labels))
                                                                            fig, ax = plt.subplots(figsize=(6, 4))
                                                                                bars = ax.bar(x, means, yerr=ci, capsize=5, color=['#7f7f7f', '#2ca02c'], edgecolor='black', width=0.5)

                                                                                    ax.set_ylabel('Throughput (req/sec)')
                                                                                        ax.set_title('Redis Transaction Throughput Under Contention')
                                                                                            ax.set_xticks(x)
                                                                                                ax.set_xticklabels(labels)
                                                                                                    ax.grid(axis='y', linestyle='--', alpha=0.7)

                                                                                                        plt.tight_layout()
                                                                                                            plt.savefig('ieee_throughput_chart.pdf')
                                                                                                                print("Generated: ieee_throughput_chart.pdf")

                                                                                                                if __name__ == '__main__':
                                                                                                                    plot_tail_latency()
                                                                                                                        plot_throughput()
