import os


compile_L1L2cache_LRU = 'gcc -O3 cache/L1L2cache_LRU.c -o executable/L1L2cache_LRU'
os.system(compile_L1L2cache_LRU)