#!python

import os 
import sys
import pandas
import multiprocessing

def avg_pattern(df):
	total_volume = 0
	total_amount = 0
	beyond = True
	df.sort_values('time', inplace=True)
	for i in range(df.shape[0]):
		total_amount += df.iloc[i]['amount']
		total_volume += df.iloc[i]['volume'] * 100
		if df.iloc[i]['volume'] != 0:
			if total_amount // total_volume > df.iloc[i]['price']:
				beyond = False
				break
	return beyond

def worker(paths, result):
	symbol = None
	while True:
		if len(paths) == 0:
			break
		else:
			symbol = paths.pop()
		print(symbol)
		df = pandas.read_csv(symbol)
		if avg_pattern(df):
			result.append(symbol)

def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]

def main():
	ticks_path = sys.argv[1]
	dir_list = os.walk(ticks_path)
	matched = []
	for root, dirs, files in dir_list:
		for file in files:
			print('check %s' % file)
			df = pandas.read_csv(os.path.join(root, file))
			if avg_pattern(df):
				matched.append(file)
	print(matched)
			#path_list.append(os.path.join(root, file))
#	process_pool = []
#	result_list = []
#	for i in chunks(path_list, 5):
#		result = list()
#		result_list.append(result)
#		#sub_list = path_list[chunk_size*i:chunk_size*i + chunk_size]
#		print(i)
#		process_pool.append(multiprocessing.Process(target=worker, args=(i, result)))
#	for x in process_pool:
#		x.start()
#	for x in process_pool:
#		x.join()
	#print(result_list)


if __name__ == '__main__':
	main()