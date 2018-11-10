import argparse

parser = argparse.ArgumentParser(description='driver script for run.py')
parser.add_argument('--size',  type=str, default='test',
                    help='size of the benchmark suite')
args = parser.parse_args()


import os

progs = [
	'blackscholes',
	'bodytrack',
	'canneal',
	# 'dedup',
	'facesim',
	'ferret',
	'fluidanimate',
	'freqmine',
	'raytrace',
	'streamcluster',
	'swaptions',
	'vips',
	# 'x264',
]

# progs = [ 'canneal', ]
for p in progs:
	command = 'time python run.py --tool tools/addrtrace --exe {} --size {}'.format(p, args.size)
	os.system(command)
	rename = 'mv output/addrtrace.out output/{}/addrtrace_{}.out'.format(args.size, p)
	os.system(rename)
	

