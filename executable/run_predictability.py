import os
import sys
sys.path.insert(0, '/data/i-am-mkbera/')
from utilities.parallel_prog import run_parallel_progs as rpp
import argparse

progs = [
			# 'blackscholes',
			# 'bodytrack',
			'canneal',
			# 'facesim',
			# 'ferret',
			# 'fluidanimate',
			# 'freqmine',
			# 'raytrace',
			# 'streamcluster',
			# 'vips', 

			# 'swaptions', ERROR build
			# 'dedup', ERROR build
			# 'x264', ERROR run
]

sizes = [
	'simsmall',
	# 'simmedium',
]

levels = [
	'LLC',
	# 'L2',
]

# cache_sizes = [4,8]
cache_sizes = [4]

exe = './predictability'


cmd_list = []
for p in progs:
	for s in sizes:
		for l in levels:
			for cs in cache_sizes:

				num_bits = 2
				prev_nbr = 1
				prev_nbr_num_bits = 2
				next_nbr = 1
				next_nbr_num_bits = 2
				
				input_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}_sharing_history/{}MB'.format(l,cs)
				output_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}analysis/{}MB'.format(l,cs)

				file_name = '{}-{}-{}-{}-{}-{}'.format(p, num_bits, prev_nbr, prev_nbr_num_bits, next_nbr, next_nbr_num_bits)

				input_file = '{}/{}/{}.{}_sharing_history'.format(input_folder, s, p, l)
				output_file = '{}/{}/{}.{}analysis'.format(output_folder, s, file_name, l)


				command = '{} {} {} {} {} {} {} {}'.format(
					exe,
					input_file,
					output_file,
					num_bits,
					prev_nbr,
					prev_nbr_num_bits,
					next_nbr,
					next_nbr_num_bits,
					)

				cmd_list.append(command)

rpp(cmd_list, 17)