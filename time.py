import subprocess
import os
from timeit import default_timer
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator

from tqdm import tqdm

def parse_arguments():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('executable', type=str)
    parser.add_argument('--repeat', '-r', type=int, default=10)
    parser.add_argument('--max_cores', '-m', type=int, default=os.cpu_count())
    parser.add_argument('--show', action='store_true')

    return parser.parse_args()


def time_subprocess(args, repeat):
    ts = np.zeros(repeat)
    for i in tqdm(range(len(ts))):
        start = default_timer()
        subprocess.run(args)
        end = default_timer()
        ts[i] = end - start

    mean = np.mean(ts)
    lower_error = mean - np.min(ts)
    upper_error = np.max(ts) - mean
    return mean, lower_error, upper_error


def unzip(x):
    return zip(*x)

if __name__ == '__main__':
    args = parse_arguments()

    num_cores = range(1, args.max_cores + 1)
    times = np.zeros(args.max_cores)
    lower_error = np.zeros(args.max_cores)
    upper_error = np.zeros(args.max_cores)

    times, lower_error, upper_error = map(np.array, unzip(
        time_subprocess([args.executable, str(cores)], args.repeat)
        for cores in num_cores))

    print(times, lower_error, upper_error)

    ax = plt.gca()
    ax.xaxis.set_major_locator(MaxNLocator(integer=True))
    ax.plot(num_cores, num_cores, '--')
    ax.errorbar(num_cores, times[0] / times,
                 yerr=[times[0] * upper_error/times**2,
                       times[0] * lower_error/times**2],
                 fmt='.')
    plt.xlabel('Threads per kernel')
    plt.ylabel('Speedup')
    plt.legend(['Perfect', 'Actual'])
    plt.title(args.executable + " scaling")
    plt.savefig(args.executable+".png")
    if args.show:
        plt.show()
