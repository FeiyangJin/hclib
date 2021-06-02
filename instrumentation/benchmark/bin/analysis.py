import re
import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def main():
    if (len(sys.argv) <= 1):
        print("analysis.py path_to_csv_file")
        sys.exit(0)

    #set up color
    # tableau20 = [(31, 119, 180), (174, 199, 232), (255, 127, 14), (255, 187, 120),
    #          (44, 160, 44), (152, 223, 138), (214, 39, 40), (255, 152, 150),
    #          (148, 103, 189), (197, 176, 213), (140, 86, 75), (196, 156, 148),
    #          (227, 119, 194), (247, 182, 210), (127, 127, 127), (199, 199, 199),
    #          (188, 189, 34), (219, 219, 141), (23, 190, 207), (158, 218, 229)]
    # for i in range(len(tableau20)):
    #     red, g, b = tableau20[i]
    #     tableau20[i] = (red / 255., g / 255., b / 255.)
    # colors = tableau20
    
    #preprocess data
    csv_path = sys.argv[1]
    data = pd.read_csv(csv_path)
    data_organized = {}
    unit = {}
    data_for_figure = {}
    prog = re.compile(r'(\w+)-(\w+)-(\d+)\s*\((\w+)\)')
    for col in data.columns:
        result = prog.match(col)
        if (not result):
            continue
        metric = result[2]
        category = result[1]
        if (not data_organized.get(metric)):
            data_organized[metric] = {}
        if (not data_organized[metric].get(category)):
            data_organized[metric][category] = []
        data_organized[metric][category].append(data[col])
        if (not unit.get(metric)):
            unit[metric] = result[4]
    for m, met in data_organized.items():
        data_for_figure[m] = {}
        for c, cat in met.items():
            avg = np.average(cat, axis=0)
            data_for_figure[m][c] = avg
    
    #draw figures
    width = 0.2
    space = 1
    groups = ['Orig', 'Rd']
    desc = ['Native', "Race Detection"]
    xticks = data[data.columns[0]].astype(str)
    index = np.arange(len(xticks)) * width * (len(groups) + space)
    for m, met in data_for_figure.items():
        # metric figure
        plt.clf()
        fig, ax = plt.subplots()
        bars = []
        ylim = 0
        for i, g in enumerate(groups):
            bar = ax.bar(index + i * width, met[g], width=width)
            for x,y in zip(index + i * width, met[g]):
                label = "{:.2f}".format(y)
                ax.annotate(label, (x,y), textcoords="offset points", xytext=(0,5), ha='center', fontsize=6)
            bars.append(bar)
            max_y = np.max(met[g])
            ylim = max_y if ylim < max_y else ylim
        ylim_round = 1
        while ylim > ylim_round:
            ylim_round *= 10
        ax.set_xticks(index + width * len(groups) / 2)
        ax.set_xticklabels(xticks)
        ax.set_xlabel("Benchmarks")
        ax.set_ylabel("{} ({})".format(m, unit[m]))
        ax.set_yscale('log')
        ax.set_ylim([0.01, ylim_round * 10])
        ax.legend(bars, desc, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=4, fancybox=True, shadow=False)
        fig.savefig("{}.pdf".format(m.lower()))

        # overhead figure
        plt.clf()
        fig, ax = plt.subplots()
        bars = []
        ylim = 0
        for i, g in enumerate(groups):
            val = met[g]/met[groups[0]]
            bar = ax.bar(index + i * width, val, width=width)
            for x,y in zip(index + i * width, val):
                label = "{:.2f}".format(y)
                ax.annotate(label, (x,y), textcoords="offset points", xytext=(0,5), ha='center', fontsize=6)
            bars.append(bar)
            max_y = np.max(val)
            ylim = max_y if ylim < max_y else ylim
        ylim_round = 1
        while ylim > ylim_round:
            ylim_round *= 10
        ax.set_xticks(index + width * len(groups) / 2)
        ax.set_xticklabels(xticks)
        ax.set_xlabel("Benchmarks")
        ax.set_ylabel("{} Overhead (\u2715)".format(m))
        ax.set_yscale('log')
        ax.set_ylim([0.1, ylim_round])
        ax.legend(bars, desc, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=4, fancybox=True, shadow=False)
        fig.savefig("{}_overhead.pdf".format(m.lower()))



if __name__ == "__main__":
    main()