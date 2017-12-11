#!python

import os 
import sys
import pandas

def avg_pattern(df):
	total_volume = 0
	total_amount = 0
	beyond = True
	for x in df:
		total_amount += x['amount']
		total_volume += x['volume']
		if x['volume'] != 0:
			total_amount // total_volume > x['price']
			beyond = False

def main():
	ticks_path = sys.argv[1]
	dir_list = os.walk(ticks_path)
	matched = []
	for root, dirs, files in dir_list:
		df = pandas.read_csv(os.path.join(root, files))
		if avg_pattern(df):
			matched.append(files)

	print()


if __name__ == '__main__':
	main()