import os
import sys
import logging
from scipy import constants
from pathlib import Path
from argparse import ArgumentParser
from multiprocessing import Pool, JoinableQueue

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick

lvl = os.environ.get('PYTHONLOGLEVEL', 'WARNING').upper()
fmt = '[ %(asctime)s %(levelname)s %(process)d ] %(message)s'
logging.basicConfig(format=fmt,
                    datefmt="%d %H:%M:%S",
                    level=lvl)
logging.getLogger('matplotlib').setLevel(logging.CRITICAL)

class TimeConversion:
    def __init__(self, rtime):
        self.rtime = rtime

    def __call__(self, x):
        diff = x[self.rtime] - x[self.rtime].min()
        return diff.apply(lambda x: x * constants.milli / constants.day)

def func(queue, by, rtime, output):
    gridspec_kw = {
        'hspace': 0.1,
        'height_ratios': [1, 0.25],
    }

    while True:
        (name, group) = queue.get()
        logging.info(name)

        (fig, (top, bottom)) = plt.subplots(nrows=2,
                                            sharex=True,
                                            gridspec_kw=gridspec_kw)

        view = (group
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

        fname = output.joinpath('{:04d}'.format(name)).with_suffix('.png')
        plt.xlabel('Days')
        plt.suptitle('Tile {}'.format(name))
        plt.savefig(fname)
        plt.close(fig)

        queue.task_done()

arguments = ArgumentParser()
arguments.add_argument('--master', type=int, default=0)
arguments.add_argument('--output', type=Path)
arguments.add_argument('--workers', type=int)
args = arguments.parse_args()

if args.output is None or not args.output.is_dir():
    logging.critical('output={} does not exist'.format(args.output))
    sys.exit(os.EX_OSFILE)

by = 'lp'
rtime = 'virtual_time'
queue = JoinableQueue()
initargs = (
    queue,
    by,
    rtime,
    args.output,
)

with Pool(args.workers, func, initargs):
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

    for i in df.groupby(by, sort=False):
        queue.put(i)
    queue.join()
