NUM_BITS = [2,4]
# prev_nbr = [0,1]
# prev_nbr_num_bits = [1,2]
# next_nbr = [0,1]
NBR = [0,1]
# next_nbr_num_bits = [1,2]

import argparse

parser = argparse.ArgumentParser(description='')
parser.add_argument('--size', type=str, default=None)
parser.add_argument('--level', type=str, default=None)
parser.add_argument('--cache-size', type=int, default=None)
args = parser.parse_args()
size = args.size
level = args.level
cache_size = args.cache_size
assert(size is not None)
assert(level is not None)
assert(cache_size is not None)

progs = [
	'blackscholes',
	'bodytrack',
	'canneal',
	'facesim',
	'ferret',
	'fluidanimate',
	'freqmine',
	'raytrace',
	'streamcluster',
	'vips', 
]


table = []

header_row = ['program',  'number of bits [block, prev block, next block]',  'predictability index', 'conditional entropy']
table.append(header_row)

input_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}analysis/{}MB/{}'.format(level, cache_size, size)
output_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}_latex/{}MB/{}'.format(level, cache_size, size)
out_file_name = 'analysis.{}_latex'.format(level)
output_file = '{}/{}'.format(output_folder, out_file_name)

for p in progs:
	for nbr in NBR:
		for num_bits in NUM_BITS:
			# print('MARK', nbr, num_bits)
			if nbr == 0:
				nbr_bits = 0
				in_file_name = '{}-{}-0-0-0-0.{}analysis_global'.format(p, num_bits ,level)
			else:
				nbr_bits = int(num_bits / 2)
				in_file_name = '{}-{}-1-{}-1-{}.{}analysis_global'.format(p, num_bits, nbr_bits, nbr_bits ,level)
				
			input_file = '{}/{}'.format(input_folder, in_file_name)
			row = []
			row.append(p) # program name
			number_of_bits = [num_bits, nbr_bits, nbr_bits]
			row.append(number_of_bits)

			f = open(input_file, "r")
			data = f.read().split('\n')
			data = data[0]
			data = data.split()
			pred = float(data[0])
			entropy = float(data[1])
			row.append(pred)
			row.append(entropy)

			table.append(row)



def save_table(table):
	f = open(output_file, 'w')
	data = ''
	for i in range(len(table)):
		data += table[i]
		data += '\n'
	f.write(data)

def generate_latex_table (table):
	n_rows = len(table)
	tom = []
	tom.append('\\begin{table}[!hbt]')
	tom.append('\\caption{{ {} analysis: {} {} MB}} \\label{{ table:{}analysis_{}_{} }}'.format(level, size, cache_size, level, size, cache_size))
	tom.append('\\begin{center}')
	tom.append('\\begin{tabular}{|p {2cm}|p {1.5cm}|p {1.5cm}|p {1.5cm}| }')
	tom.append('\\hline')

	for i in range(n_rows):
		if i == 0:
			head_row = table[i]
			entry = ''
			for j in range(len(head_row)):
				entry += ' \\textbf{{ {} }} '.format(head_row[j])
				if j == len(head_row)-1:
					entry += ' \\\\ '
				else:
					entry += ' & '
			tom.append(entry)
		else:
			row = table[i]
			entry = ''
			for j in range(len(row)):
				entry += ' {} '.format(row[j])
				if j == len(head_row)-1:
					entry += ' \\\\ '
				else:
					entry += ' & '
			tom.append(entry)				

		tom.append('\\hline')

	tom.append('\\end{tabular}')
	tom.append('\\end{center}')
	tom.append('\\end{table}')

	return tom


latex_table = generate_latex_table(table)
save_table(latex_table)

