import sys
from scipy import constants
from pathlib import Path
from argparse import ArgumentParser

import pandas as pd
import matplotlib.pyplot as plt

class TimeConversion:
    def __init__(self, rtime):
        self.rtime = rtime

    def __call__(self, x):
        diff = x[self.rtime] - x[self.rtime].min()
        return diff.apply(lambda x: x * constants.milli / constants.day)

arguments = ArgumentParser()
arguments.add_argument('--master', type=int, default=0)
arguments.add_argument('--output', type=Path)
args = arguments.parse_args()

by = 'lp'
rtime = 'virtual_time'
usecols = [
    rtime,
    by,
    'susceptible',
    'infected',
    'recovered',
]
df = (pd
      .read_csv(sys.stdin, usecols=usecols)
      .assign(offset=TimeConversion(rtime)))

for (n, g) in df.groupby(by, sort=False):
    fig = plt.gcf()
    view = (g
            .drop(columns=[rtime, by])
            .set_index('offset')
            .sort_index())
    view.plot(grid=True, title=n)

    fname = args.output.joinpath('{:04d}'.format(n)).with_suffix('.png')
    plt.xlabel('Days')
    plt.ylabel('People')
    plt.savefig(fname)
    plt.close(fig)
