import os
import sys
sys.path.insert(0, '/data/i-am-mkbera/')
from utilities.parallel_prog import run_parallel_progs as rpp
import argparse

parser = argparse.ArgumentParser(description='run pin tool')
parser.add_argument('--size',  type=str, default=None,
                    help='size of the benchmark suite')
args = parser.parse_args()
assert(args.size != None)

progs = [
         'blackscholes',
         'bodytrack',
         'canneal',
         'facesim',
         'ferret',
         'fluidanimate',
         'freqmine',
         'raytrace',
         'streamcluster',
         'vips', 
         # 'swaptions', ERROR build
         # 'dedup', ERROR build
         # 'x264', ERROR run
 ]


exe = './LLC_MIN_meta'

input_folder = '../../../data/multilevel-cache-sim/LLCtrace'
output_folder = '../../../data/multilevel-cache-sim/LLCmeta'
history_folder = '../../../data/multilevel-cache-sim/LLC_sharing_history'

cmd_list = []

for p in progs:

	input_file = '{}/{}/{}.LLCtrace'.format(input_folder, args.size, p)
	output_file =  '{}/{}/{}.LLCmeta'.format(output_folder, args.size, p)
	sharing_history_file = '{}/{}/{}.LLC_sharing_history'.format(history_folder, args.size, p)

	command = '{} {} {} {}'.format(exe, input_file, output_file, sharing_history_file)
	cmd_list.append(command)

rpp(cmd_list, 10)
