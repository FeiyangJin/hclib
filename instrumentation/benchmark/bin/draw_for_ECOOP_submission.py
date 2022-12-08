 #!/usr/bin/python
import numpy as np

import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt


def main():
    benchmark = ["health", "knapsack", "matmul", "sort", "strassen", "sparselu", "poisson"]
    time = [21.13, 4.96, 20.53, 42.13, 29.13, 28.07, 46.10]
    width = 0.5
    space = 1
    groups = [time]
    desc = ['Time Overhead']
    xticks = benchmark
    index = np.arange(len(xticks)) * width * (len(groups) + space)
    plt.clf()
    fig, ax = plt.subplots()
    bar1 = ax.bar(index + 0 * width, time, width=width, align='edge', color='orange')
    for x,y in zip(index + 0 * width, time):
        label = "{:.2f}".format(y)
        ax.annotate(label, (x,y), textcoords="offset points", xytext=(12,5), ha='center', fontsize=10)
    ax.set_xticks(index + width/2)
    ax.set_xticklabels(xticks)
    ax.set_xlabel("Benchmarks")
    ax.set_ylabel("Time Overhead (x)")
    ax.set_yscale('log')
    ax.set_ylim([1, 100])
    ax.legend([bar1], desc, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=4, fancybox=True, shadow=False)
    fig.savefig("over.pdf")
if __name__== "__main__":
    main()
