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


exe = './L1L2cache_LRU'

input_folder = '../../../pin-3.6/source/tools/multilevel-cache-sim-pintool/output'
output_folder = '../../../data/multilevel-cache-sim/LLCtrace'


commands = []
for p in progs:

	input_file =  '{}/simmedium/addrtrace_{}.out'.format(input_folder, p) 
	output_file = '{}/simmedium/{}.LLCtrace'.format(output_folder, p)

	cmd = '{} {} {}'.format(exe, input_file, output_file)

	commands.append(cmd)

rpp(commands, 10)



