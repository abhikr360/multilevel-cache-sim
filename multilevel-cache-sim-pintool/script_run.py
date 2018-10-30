import os

progs = [
	'blackscholes',
	'bodytrack',
	'canneal',
	'dedup',
	'facesim',
	'ferret',
	'fluidanimate',
	'freqmine',
	'raytrace',
	'streamcluster',
	'swaptions',
	'vips',
	'x264',
]

for p in progs:
	command = 'time python run.py --tool tools/addrtrace --size simmedium --exe {}'.format(p)
	os.system(command)
	rename = 'mv output/addrtrace.out output/addrtrace_{}.out'.format(p)
	os.system(rename)

	