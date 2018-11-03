import os


exe = './L1L2cache_LRU'

input_folder = '../../../pin-3.6/source/tools/multilevel-cache-sim-pintool/output'
output_folder = '../../../data/multilevel-cache-sim/LLCtrace'

input_file =  '{}/test/addrtrace.out'.format(input_folder) 
output_file = '{}/temp.LLCtrace'.format(output_folder)

command = '{} {} {}'.format(exe, input_file, output_file)

os.system(command)

