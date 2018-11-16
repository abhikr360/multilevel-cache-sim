from itertools import product as zip_prod
import sys
sys.path.insert(0, '/data/i-am-mkbera/')
from utilities.parallel_prog import run_parallel_progs as rpp

sizes = [
	'simsmall',
	'simmedium',
]

levels = [
	'LLC',
	'L2',
]

cache_sizes = [
	4,
	8,
]

cmd_list = []
for x in zip_prod(sizes, levels, cache_sizes):
	s, l, cs = x

	command = 'python LLCanalysis_table.py --size {} --level {} --cache-size {}'.format(s, l, cs)
	cmd_list.append(command)

rpp(cmd_list, 2)


