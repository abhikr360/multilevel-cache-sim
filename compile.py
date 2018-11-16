import os

compile_tasks = [
	'gcc -O3 -w cache/L1L2cache_LRU.c -o executable/L1L2cache_LRU',
	'gcc -O3 -w cache/LLC_MIN.c -o executable/LLC_MIN',
	'gcc -O3 -w cache/LLC_MIN_meta.c -o executable/LLC_MIN_meta',
	'gcc -O3 -w analysis/predictability.c -o executable/predictability -lm',
	'gcc -O3 -w cache/L1cache_LRU.c -o executable/L1cache_LRU',
]

for c in compile_tasks:	
	os.system(c)
