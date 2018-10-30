
# path = '/data/i-am-mkbera/parsec-3.0/bin/parsecmgmt'
# exe = '{} -a run -p blackscholes'.format(path)
# exe = './prog2 8'
import os
import argparse

parser = argparse.ArgumentParser(description='run pin tool')
parser.add_argument('--tool',  type=str, default=None,
                    help='pin tool')
parser.add_argument('--exe',  type=str, default='blackscholes',
                    help='executable to be instrumented')
parser.add_argument('--size',  type=str, default='test',
                    help='size of the benchmark suite')
args = parser.parse_args()
assert(args.tool is not None)

try:
	copy_tool_command = 'cp {}.cpp pin_tool.cpp'.format(args.tool)
	os.system(copy_tool_command)
except:
	print('*** tool not found ***')
	exit()

# rm_old_builds = 'rm obj-intel64/*'
# os.system(rm_old_builds)

pin_build = 'make obj-intel64/pin_tool.so'
os.system(pin_build)


pin_path = '/data/i-am-mkbera/pin-3.6'
pin_tool_path = '{}/source/tools/multilevel-cache-sim-pintool'.format(pin_path)
pin_command = '{}/pin -t {}/obj-intel64/pin_tool.so -- '.format(pin_path, pin_tool_path, args.tool)
command = "parsecmgmt -a run -p {} -i {} -n 8 -s '{}'".format(args.exe, args.size, pin_command)
os.system(command)
