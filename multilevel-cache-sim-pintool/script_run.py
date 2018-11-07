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
	command = 'time python run.py --tool tools/addrtrace --exe {} --size simsmall'.format(p)
	os.system(command)
	rename = 'mv output/addrtrace.out output/simsmall/addrtrace_{}.out'.format(p)
	os.system(rename)


