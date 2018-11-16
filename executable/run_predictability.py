import os
import sys
sys.path.insert(0, '/data/i-am-mkbera/')
from utilities.parallel_prog import run_parallel_progs as rpp
import argparse

from itertools import product as zip_prod
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

exe = './predictability'

num_bits = [2,3,4]
prev_nbr = [0,1]
prev_nbr_num_bits = [1,2,3,4]
next_nbr = [0,1]
next_nbr_num_bits = [1,2,3,4]


cmd_list = []
for x in zip_prod(progs, sizes, levels, cache_sizes):
	for y in zip_prod(num_bits, prev_nbr, prev_nbr_num_bits, 
		next_nbr, next_nbr_num_bits):
	
		p, s, l, cs = x
		nb, pn, pnnb, nn, nnnb = y

		if pn == 0:
			if pnnb == 1:
				pnnb = 0
			else:
				continue
		if nn == 0:
			if nnnb == 1:
				nnnb = 0
			else:
				continue
				
		input_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}_sharing_history/{}MB'.format(l,cs)
		output_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}analysis/{}MB'.format(l,cs)

		file_name = '{}-{}-{}-{}-{}-{}'.format(p, nb, pn, pnnb, nn, nnnb)

		input_file = '{}/{}/{}.{}_sharing_history'.format(input_folder, s, p, l)
		output_file = '{}/{}/{}.{}analysis'.format(output_folder, s, file_name, l)


		command = '{} {} {} {} {} {} {} {}'.format(
			exe, # executable
			input_file, # input file name
			output_file, # output file name 
			nb, # num bits
			pn, # prev nbr
			pnnb, # prev nbr num bits
			nn, # next nbr
			nnnb, # next nbr num bits
			)

		cmd_list.append(command)

rpp(cmd_list, 10)