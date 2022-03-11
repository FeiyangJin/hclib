import numpy as np
import matplotlib.pyplot as plt


def main():
    benchmark = ["health", "knapsack", "matmul", "sort", "strassen", "sparselu", "poisson"]
    time = [23.11, 10.98, 20.24, 84.93, 66.18, 40.63, 95.91]
    width = 0.2
    space = 1
    groups = [time]
    desc = ['Time Overhead']
    xticks = benchmark
    index = np.arange(len(xticks)) * width * (len(groups) + space)
    plt.clf()
    fig, ax = plt.subplots()
    bar1 = ax.bar(index + 0 * width, time, width=width)
    for x,y in zip(index + 0 * width, time):
        label = "{:.2f}".format(y)
        ax.annotate(label, (x,y), textcoords="offset points", xytext=(0,5), ha='center', fontsize=6)
    ax.set_xticks(index)
    ax.set_xticklabels(xticks)
    ax.set_xlabel("Benchmarks")
    ax.set_ylabel("Time Overhead (\u2715)")
    ax.set_yscale('log')
    ax.set_ylim([1, 1000])
    ax.legend([bar1], desc, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=4, fancybox=True, shadow=False)
    fig.savefig("over.pdf")
if __name__== "__main__":
    main()
