import os


compile_L1L2cache_LRU = 'gcc -O3 cache/L1L2cache_LRU.c -o executable/L1L2cache_LRU'
compile_tasks = [
	# 'gcc -O3 cache/L1L2cache_LRU.c -o executable/L1L2cache_LRU',
	'gcc -O3 cache/LLC_MIN.c -o executable/LLC_MIN',
]

for c in compile_tasks:	
	os.system(c)