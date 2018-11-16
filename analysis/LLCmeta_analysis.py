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

header_row = ['program',  'cache fills [shared, private, no-use]',  'hits [shared, private]', 'reuse [shared, private]']
table.append(header_row)


input_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}meta/{}MB/{}'.format(level, cache_size, size)
output_folder = '/data/i-am-mkbera/data/multilevel-cache-sim/{}_latex/{}MB/{}'.format(level, cache_size, size)
out_file_name = 'meta.{}_latex'.format(level)
output_file = '{}/{}'.format(output_folder, out_file_name)
for p in progs:
	in_file_name = '{}.{}meta'.format(p, level)

	input_file = '{}/{}'.format(input_folder, in_file_name)
	row = []
	row.append(p) # program name

	f = open(input_file, "r")
	data = f.read().split('\n')
	data[1] = data[1].split()
	fills = [int(d) for d in data[1]]
	
	data[2] = data[2].split()
	hits = [int(d) for d in data[2]]

	data[3] = data[3].split()
	reuse = [int(d) for d in data[3]]

	row.append(fills) 
	row.append(hits)
	row.append(reuse)

	table.append(row)

# \begin{table}[!hbt]
# \caption{configuration 1}\label{table:config_1}
# \begin{center}
# \begin{tabular}{ |p {1cm}|p {1cm}|p {1cm} |p {2cm}|}
# \hline
#  \textbf{cache} & \textbf{size} & \textbf{assoc} & \textbf{replacement policy} \\ 
#  \hline
#  L1 & 32 KB & 8 way & LRU\\  
#  \hline
#  L2 & 128 KB & 8 way & LRU \\
#  \hline
#  LLC (offline) & 4 MB / 8 MB & 16 way & MIN\\
#  \hline
# \end{tabular}
# \end{center}
# \end{table}

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
	tom.append('\\caption{{ {} metadata: {} {} MB}} \\label{{ table:{}metadata_{}_{} }}'.format(level, size, cache_size, level, size, cache_size))
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

