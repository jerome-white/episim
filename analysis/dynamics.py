import sys
from scipy import constants
from pathlib import Path
from argparse import ArgumentParser

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick

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

gridspec_kw = {
    'hspace': 0.1,
    'height_ratios': [1, 0.25],
}

for (n, g) in df.groupby(by, sort=False):
    (fig, (top, bottom)) = plt.subplots(nrows=2,
                                        sharex=True,
                                        gridspec_kw=gridspec_kw)

    view = (g
            .drop(columns=[rtime, by])
            .set_index('offset')
            .sort_index())
    view.plot(grid=True, ax=top)
    top.set_ylabel('People')

    view = (view
            .sum(axis='columns')
            .to_frame()
            .apply(lambda x: (x - x.iloc[0]) / x.iloc[0]))
    view.plot(grid=True, legend=False, ax=bottom)
    bottom.yaxis.set_major_formatter(mtick.PercentFormatter(xmax=1))
    bottom.set_ylabel('Pop. chg')

    fname = args.output.joinpath('{:04d}'.format(n)).with_suffix('.png')
    plt.xlabel('Days')
    plt.suptitle('Tile {}'.format(n))
    plt.savefig(fname)
    plt.close(fig)
