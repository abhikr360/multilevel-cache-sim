import os
import sys
sys.path.insert(0, '/data/i-am-mkbera/')
from utilities.parallel_prog import run_parallel_progs as rpp

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

	input_file = '{}/simmedium/{}.LLCtrace'.format(input_folder, p)
	output_file =  '{}/simmedium/{}.LLCmeta'.format(output_folder, p)
	sharing_history_file = '{}/simmedium/{}.LLC_sharing_history'.format(history_folder, p)

	command = '{} {} {} {}'.format(exe, input_file, output_file, sharing_history_file)
	cmd_list.append(command)

rpp(cmd_list, 10)
