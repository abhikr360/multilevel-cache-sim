import os


exe = './predictability'

input_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/LLC_sharing_history'
output_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/LLCanalysis'


input_file = '{}/simsmall/canneal.LLC_sharing_history'.format(input_folder)
output_file = '{}/simsmall/canneal.LLCanalysis'

num_bits = 2
prev_nbr = 1
next_nbr = 1
prev_nbr_num_bits = 2
next_nbr_num_bits = 2

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

os.system(command)