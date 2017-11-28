import sys
import os
import argparse
import colorsys
import matplotlib.pyplot as plt

def plot(fname, color, label, truncate_x=None):
    x_vals = []
    y_vals = []
    with open(fname) as f:
        for idx, line in enumerate(f):
            score = line.strip()
            score = float(score)
            y_vals.append(score)
            x_vals.append(idx / 2)
    x_vals = x_vals[:truncate_x]
    y_vals = y_vals[:truncate_x]
    plt.plot(x_vals, y_vals, color=color, label=label)

def get_random_color(i, n):
    hue = (360//n*i)/360.0
    sat = 0.5
    light = 0.5
    r,g,b = colorsys.hls_to_rgb(hue, light, sat)
    return '#%02x%02x%02x' % (int(r*255), int(g*255), int(b*255))

def label(string):
    return string.split(',')

if __name__ == '__main__':
    PARSER = argparse.ArgumentParser(description='Plot gain curves from CAL record list')
    PARSER.add_argument('files', help='Path of record list(<topic>.record.list) file', nargs='+')
    PARSER.add_argument('--labels', help='legend labels (comma separated, one for each curve)', default=None, type=label)
    PARSER.add_argument('--title', help='curve title', default=None, type=str)
    CLI = PARSER.parse_args()

    if CLI.labels is not None and len(CLI.labels) != len(CLI.files):
        sys.stderr.write('Error: Number of labels much match the number of files!')
        sys.exit(1)

    for idx, fname in enumerate(CLI.files):
        plot(
            fname, 
            get_random_color(idx, len(CLI.files)),
            "file-%d" % idx if CLI.labels is None else CLI.labels[idx]
        )
    plt.xlabel("effort")
    plt.ylabel("recall")
    if CLI.title is not None:
        plt.suptitle(CLI.title, fontsize=14, fontweight='bold')

    ax = plt.subplot(111)
    box = ax.get_position()
    ax.set_position([box.x0, box.y0 + box.height * 0.1,
                 box.width, box.height * 0.9])
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.05),
          fancybox=True, shadow=True, ncol=5)
    plt.show()
