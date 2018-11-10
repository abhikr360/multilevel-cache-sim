from utilities.parallel_prog import run_parallel_progs
import os
benchmarks = [
	'blackscholes',
	'bodytrack',
	'canneal',
	# 'dedup', ERROR
	'facesim',
	'ferret',
	'fluidanimate',
	'freqmine',
	'raytrace',
	'streamcluster',
	'swaptions',
	'vips',
	# 'x264', ERROR
]


# source_cmd = 'source /data/i-am-mkbera/parsec-3.0/env.sh'
# os.system(source_cmd)
# path = '/data/i-am-mkbera/parsec-3.0/bin/parsecmgmt'
commands = []
for bm in benchmarks:
	build_cmd = 'parsecmgmt -a build -p {}'.format(bm)
	# os.system(build_cmd)
	commands.append(build_cmd)

run_parallel_progs(commands, 12)


