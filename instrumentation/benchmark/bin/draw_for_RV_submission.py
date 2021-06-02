import numpy as np
import matplotlib.pyplot as plt


def main():
    benchmark = ["sparselu", "poisson", "fib", "nqueens", "health", "matmul", "life"]
    time = [1.8, 15.8, 20.7, 24.4, 50.6, 80.5, 14.2]
    memory = [6.9, 12.8, 13.5, 11.3, 24.7, 104.5, 9.1]
    width = 0.2
    space = 1
    groups = [time, memory]
    desc = ['Time Overhead', "Memory Overhead"]
    xticks = benchmark
    index = np.arange(len(xticks)) * width * (len(groups) + space)
    plt.clf()
    fig, ax = plt.subplots()
    bar1 = ax.bar(index + 0 * width, time, width=width)
    for x,y in zip(index + 0 * width, time):
        label = "{:.2f}".format(y)
        ax.annotate(label, (x,y), textcoords="offset points", xytext=(0,5), ha='center', fontsize=6)
    bar2 = ax.bar(index + 1 * width, memory, width=width)
    for x,y in zip(index + 1 * width, memory):
        label = "{:.2f}".format(y)
        ax.annotate(label, (x,y), textcoords="offset points", xytext=(0,5), ha='center', fontsize=6)
    ax.set_xticks(index + width * len(groups) / 2)
    ax.set_xticklabels(xticks)
    ax.set_xlabel("Benchmarks")
    ax.set_ylabel("Overhead (\u2715)")
    ax.set_yscale('log')
    ax.set_ylim([1, 1000])
    ax.legend([bar1, bar2], desc, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=4, fancybox=True, shadow=False)
    fig.savefig("over.pdf")
if __name__== "__main__":
    main()