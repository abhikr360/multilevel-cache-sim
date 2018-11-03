import os


exe = './LLC_MIN'

input_folder = '../../../data/multilevel-cache-sim/LLCtrace'
output_folder = '../../../data/multilevel-cache-sim/LLCmeta'

input_file = '{}/temp.LLCtrace'.format(input_folder)
output_file =  '{}/temp.LLCmeta'.format(input_folder) 

command = '{} {} {}'.format(exe, input_file, output_file)

os.system(command)

