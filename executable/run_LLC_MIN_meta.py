import os
import sys
sys.path.insert(0, '/data/i-am-mkbera/')
from utilities.parallel_prog import run_parallel_progs as rpp
import argparse

# parser = argparse.ArgumentParser(description='run pin tool')
# parser.add_argument('--size',  type=str, default=None,
#                     help='size of the benchmark suite')
# parser.add_argument('--cache-size', type=int, default=None,
#                      help='size of LLC in MB')
# args = parser.parse_args()
# assert(args.size != None)

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

sizes = [
	'simsmall',
	'simmedium',
]

levels = [
	'LLC',
	'L2',
]

cache_sizes = [4,8]

exe = './LLC_MIN_meta'

cmd_list = []
for p in progs:
	for s in sizes:
		for cs in cache_sizes:
			for l in levels:
				input_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}trace'.format(l)
				output_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}meta/{}MB'.format(l,cs)
				history_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}_sharing_history/{}MB'.format(l,cs)

				input_file = '{}/{}/{}.{}trace'.format(input_folder, s, p, l)
				output_file =  '{}/{}/{}.{}meta'.format(output_folder, s, p, l)
				sharing_history_file = '{}/{}/{}.{}_sharing_history'.format(history_folder, s, p, l)
				LLC_NUMSET = cs * 1024

				command = '{} {} {} {} {}'.format(exe, input_file, output_file, sharing_history_file, LLC_NUMSET)
				cmd_list.append(command)

rpp(cmd_list, 17)
