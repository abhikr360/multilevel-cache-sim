import os


exe = './LLC_MIN_meta'

input_folder = '../../../data/multilevel-cache-sim/LLCtrace'
output_folder = '../../../data/multilevel-cache-sim/LLCmeta'

input_file = '{}/temp.LLCtrace'.format(input_folder)
output_file =  '{}/temp.LLCmeta'.format(input_folder) 

command = '{} {} {}'.format(exe, input_file, output_file)
print(command)
os.system(command)

