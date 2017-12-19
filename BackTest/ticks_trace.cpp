#include <iostream>
#include <fstream>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/make_shared.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#ifdef _WIN32 
#include <Windows.h>
#endif // _WIN32

//#include "csv.h"
#include "CCSVRow.h"

using std::cout;
using std::endl;

#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <iostream>

#include <boost/atomic.hpp>

typedef struct _tick_trade_data
{
	uint32_t tick;
	uint32_t price;
	uint32_t amount;
	uint32_t volume;
}tick_trade_data;

// boost::shared_ptr<int[]> sh_arr2 = boost::make_shared<int[]>(30);
// typedef boost::shared_ptr<char[]> sh_tick_data_array;

class WriteFileBlock
{
public:
	WriteFileBlock(const char* file_path, size_t file_size);
	~WriteFileBlock();
	
	bool read_line(std::string& line_stock_info);
	bool write_to_file();
	bool read_file(const char* file_path);

private:
	char* _buffer;
	size_t _size;
	std::string _file_path;
	size_t _file_size;
	size_t _offset;
};

WriteFileBlock::WriteFileBlock(const char* file_path, size_t file_size)
{
	_file_path = file_path;
	_file_size = file_size;
	_buffer = NULL;
	_size = 0;
	_offset = 0;
}

WriteFileBlock::~WriteFileBlock()
{
	if (_buffer && _size)
	{
		delete[] _buffer;
	}
}

bool WriteFileBlock::read_line(std::string& line_stock_info)
{
	std::stringstream   lineStream(line_stock_info);
	std::string         cell;
	tick_trade_data		tick_data;
	//std::vector<std::string> stock_info;

	for ( int i = 0; i <= 4 && std::getline(lineStream, cell, ','); i++)
	{
		//stock_info.push_back(cell);
		switch (i)
		{
			case 0:
			{
				if (cell.empty())
				{
					// skip columns
					return false;
				}
				break;
			}
			case 1:
			{
				boost::posix_time::time_duration td(boost::posix_time::duration_from_string(cell));
				tick_data.tick = td.total_seconds();
				break;
			}
			case 2:
			{
				tick_data.price = uint32_t(std::atof(cell.c_str()) * 100);
				break;
			}
			case 3:
			{
				tick_data.volume = std::atoi(cell.c_str());
				break;
			}
			case 4:
			{
				tick_data.amount = std::atoi(cell.c_str());
				break;
			}
			default:
				break;
		}
	}

	if (_buffer == NULL)
	{
		_buffer = new char[_file_size];
		_size = _file_size;
		_offset = 0;
	}
	if (_offset + sizeof(tick_data) > _size)
	{
		_size += 10240;
		char* new_buffer = new char[_size];
		memcpy(new_buffer, _buffer, _offset);
		delete[] _buffer;
		_buffer = new_buffer;
	}
	memcpy(_buffer + _offset, &tick_data, sizeof(tick_data));
	_offset += sizeof(tick_data);

	return true;
}

bool WriteFileBlock::read_file(const char* file_path)
{
	std::ifstream file(file_path);
	std::string line_stock_info;
	std::string last_line;

	while (std::getline(file, line_stock_info) && file.good())
	{
		if (last_line.compare(line_stock_info) == 0)
		{
			continue;
		}
		read_line(line_stock_info);
		last_line.swap(line_stock_info);
	}
	return true;
}

bool WriteFileBlock::write_to_file()
{
	std::ofstream out_file(_file_path, std::ios::out | std::ios::binary);
	tick_trade_data* top_tick = (tick_trade_data*)_buffer;
	size_t count = _offset / sizeof(tick_trade_data);
	tick_trade_data* botton_tick = &top_tick[count-1];
	if (top_tick->tick > botton_tick->tick)
	{
		// revert
		tick_trade_data* revert_buf = (tick_trade_data*)new char[_offset];
		for (size_t i = 0; i < count; i++)
		{
			revert_buf[i] = top_tick[count - 1 - i];
		}
		out_file.write((const char*)revert_buf, _offset);
		delete[] revert_buf;
 	}
	else
	{
		out_file.write(_buffer, _offset);
	}
	return true;
}

typedef std::list<WriteFileBlock*> sh_tick_data_list;
typedef std::list<std::pair<boost::filesystem::path*, boost::filesystem::path*>> sh_file_path_list;

HANDLE tick_data_semaphore;
std::mutex sh_tick_data_list_lock_;
sh_tick_data_list sh_tick_data_list_;

HANDLE tick_file_semaphore;
std::mutex sh_tick_file_list_lock_;
sh_file_path_list sh_tick_file_list_;

std::atomic<bool> enum_done_(false);
std::atomic<bool> work_done_(false);


void writing_thread()
{

	while (true)
	{
		::WaitForSingleObject(tick_data_semaphore, INFINITE);

		WriteFileBlock* _data_block = nullptr;

		sh_tick_data_list_lock_.lock();
		if (sh_tick_data_list_.size() > 0)
		{
			_data_block = sh_tick_data_list_.front();
			sh_tick_data_list_.pop_front();
		}
		sh_tick_data_list_lock_.unlock();
		
		if (_data_block == nullptr)
			break;

		_data_block->write_to_file();
		delete _data_block;
		/*
		while (spsc_queue_.pop(_data_block))
		{
			_data_block->write_to_file();
			delete _data_block;
		}
		*/
	}
}

int convert_str_to_bin_multi(const boost::filesystem::path& file_path, boost::filesystem::path& out_dir)
{
	uint32_t lines = 0;
//	std::ifstream file(file_path.string());
//	std::string line_stock_info;
//	std::string last_line;
	WriteFileBlock* _data_block = new WriteFileBlock((out_dir / file_path.filename()).string().c_str(), file_path.size());

	_data_block->read_file(file_path.string().c_str());
	/*
	while (std::getline(file, line_stock_info) && file.good())
	{
		if (last_line.compare(line_stock_info) == 0)
		{
			continue;
		}
		_data_block->read_line(line_stock_info);
		lines++;
		last_line.swap(line_stock_info);
	}
	*/

//	spsc_queue_.push(_data_block);
	sh_tick_data_list_lock_.lock();
	sh_tick_data_list_.push_back(_data_block);
	sh_tick_data_list_lock_.unlock();
	::ReleaseSemaphore(tick_data_semaphore, 1, NULL);
	return lines;
}

void enuming_thread(const boost::filesystem::path& dir, const boost::filesystem::path& out_dir)
{
	if (!exists(out_dir))
	{
		boost::filesystem::create_directories(out_dir);
	}

	for (auto& x : boost::filesystem::directory_iterator(dir))
	{
		//cout << x << endl;
		if (is_regular_file(x.path()))
		{
			sh_tick_file_list_lock_.lock();
			sh_tick_file_list_.push_back(std::make_pair(new boost::filesystem::path(x.path()), \
				new boost::filesystem::path(out_dir)));
			sh_tick_file_list_lock_.unlock();
			long count = 0;
			::ReleaseSemaphore(tick_file_semaphore, 1, &count);
			if (count > 1000)
			{
				Sleep(500);
			}
		}
		else if (is_directory(x.path()))
		{
			enuming_thread(x.path(), out_dir / x.path().filename());
		}
	}
}


void reading_thread()
{
	while (true)
	{
		::WaitForSingleObject(tick_file_semaphore, INFINITE);
		boost::filesystem::path* src = nullptr;
		boost::filesystem::path* dst = nullptr;
		sh_tick_file_list_lock_.lock();
		if (sh_tick_file_list_.size() > 0)
		{
			src = sh_tick_file_list_.front().first;
			dst = sh_tick_file_list_.front().second;
			sh_tick_file_list_.pop_front();
		}
		sh_tick_file_list_lock_.unlock();

		if (src == nullptr)
			break;
		convert_str_to_bin_multi(*src, *dst);
		delete src;
		delete dst;
	}
}

int convert_str_to_bin(const boost::filesystem::path& file_path, boost::filesystem::path& out_dir)
{
	uint32_t lines = 0;
	std::ifstream file(file_path.string());
	std::ofstream out_file((out_dir / file_path.filename()).string(), std::ios::out | std::ios::binary);
	std::string line_stock_info;
	char* buf = new char[40960];
	uint32_t i = 0;
	tick_trade_data* tick_data = (tick_trade_data*)buf;

	while (std::getline(file, line_stock_info) && file.good())
	{
		std::stringstream   lineStream(line_stock_info);
		std::string         cell;
		std::vector<std::string> stock_info;

		while (std::getline(lineStream, cell, ','))
		{
			stock_info.push_back(cell);
		}
		//split(line_stock_info, ",", &stock_info);
		if (stock_info.size() == 0 || stock_info[0] == "")
			continue;
		boost::posix_time::time_duration td(boost::posix_time::duration_from_string(stock_info[1]));
		tick_data->tick = td.total_seconds();
		tick_data->amount = std::atoi(stock_info[4].c_str());
		tick_data->volume = std::atoi(stock_info[3].c_str());
		tick_data->price = uint32_t(std::atof(stock_info[2].c_str()) * 100);

		tick_data++;
		i++;

		if (i == 40960 / sizeof(*tick_data))
		{
			out_file.write((const char*)buf, 40960 / sizeof(*tick_data) * sizeof(*tick_data));
			i = 0;
			tick_data = (tick_trade_data*)buf;
		}

		lines++;
	}
	if (i != 0)
	{
		out_file.write((const char*)buf, sizeof(*tick_data) * i);
		out_file.close();
	}
	delete[] buf;
	return lines;
}

bool avg_pattern_symbol(const boost::filesystem::path& file_path)
{
	std::ifstream input(file_path.string());
	uint64_t total_amount = 0;
	uint64_t total_volume = 0;
	tick_trade_data tick_data;
	while (input.good() && input.read((char*)&tick_data, sizeof(tick_data)))
	{
		total_amount += tick_data.amount;
		total_volume += tick_data.volume;
		if (tick_data.volume != 0)
		{
			if (tick_data.price < total_amount / total_volume)
			{
				return false;
			}
		}
	}
	return true;
}

int main(int argc, char** argv)
{
	uint32_t total = 0;
	boost::timer t;
	boost::filesystem::path dir(argv[1]);
	boost::filesystem::path out_dir(argv[2]);


	if (!exists(dir))
	{
		return -1;
	}

	if (!is_directory(dir))
	{
		return -1;
	}

// 	for (auto& x : boost::filesystem::directory_iterator(dir))
// 	{
// 		//cout << x << endl;
// 		if (is_regular_file(x.path()))
// 		{
// 			convert_str_to_bin(x.path(), out_dir);
// 		}
// 		total++;
// 	}

	tick_file_semaphore = ::CreateSemaphore(NULL, 0, 1024, NULL);
	tick_data_semaphore = ::CreateSemaphore(NULL, 0, 1024, NULL);
	std::thread enumer(enuming_thread, dir, out_dir);

	std::list<std::thread*> read_thread_group;
	std::list<std::thread*> write_thread_group;
	for (int i = 0; i < 8; i++)
	{
		read_thread_group.push_back(new std::thread(reading_thread));
		write_thread_group.push_back(new std::thread(writing_thread));
	}

	enumer.join();
	::ReleaseSemaphore(tick_file_semaphore, 8, NULL);

	for (auto thread : read_thread_group)
	{
		thread->join();
		delete thread;
	}
	::ReleaseSemaphore(tick_data_semaphore, 8, NULL);

	for (auto thread : write_thread_group)
	{
		thread->join();
		delete thread;
	}
	::CloseHandle(tick_data_semaphore);
	::CloseHandle(tick_file_semaphore);
	cout << "Timstamp:" << t.elapsed() << "total:" << total << endl;
	return 0;
}



int main2(int argc, char** argv)
{
	uint32_t total = 0;
	boost::timer t;
	boost::filesystem::path dir(argv[1]);
	//boost::filesystem::path out_dir(argv[2]);


	if (!exists(dir))
	{
		return -1;
	}

// 	if (!exists(out_dir))
// 	{
// 		boost::filesystem::create_directories(out_dir);
// 	}
// 
// 	if (!is_directory(out_dir))
// 	{
// 		return -1;
// 	}

	for (auto& x : boost::filesystem::directory_iterator(dir))
	{
		//cout << x << endl;
		if (is_regular_file(x.path()))
		{
			//convert_str_to_bin(x.path(), out_dir);
			if (avg_pattern_symbol(x.path()))
			{
				cout << "symbol:" << x << endl;
			}
		}
		total++;
	}


	cout << "Timstamp:" << t.elapsed() << "total:" << total << endl;
	return 0;
}


int main1(int argc, char** argv)
{
	boost::timer t;
	boost::filesystem::path dir(argv[1]);
	int total = 0;
	try 
	{
		if (!exists(dir))
		{
			return -1;
		}

		if (is_regular_file(dir))
		{
			cout << dir << " size is " << file_size(dir) << '\n';
		}
		else
		{
			for (auto& x : boost::filesystem::directory_iterator(dir))
			{
				//cout << x << endl;
				if (is_regular_file(x))
				{
					total++;
//					cout << "analyze:" << x << endl;
					int64_t total_a = 0;
					int64_t total_b = 0;
					std::ifstream file(x.path().string());
					bool is_beyond = true;
					std::string line_stock_info;
					while (std::getline(file, line_stock_info) && file.good())
					{
						std::stringstream   lineStream(line_stock_info);
						std::string         cell;
						std::vector<std::string> stock_info;

						while (std::getline(lineStream, cell, ','))
						{
							stock_info.push_back(cell);
						}
						//split(line_stock_info, ",", &stock_info);
						if (stock_info.size() == 0 || stock_info[0] == "")
							continue;
						int64_t a = std::atoi(stock_info[4].c_str()) * 100;
						int64_t b = std::atoi(stock_info[3].c_str()) * 100;
						int64_t c = int64_t(std::atof(stock_info[2].c_str()) * 100);
						total_a += a;
						total_b += b;
						if (b != 0)
						{
							if (c < total_a / total_b)
							{
								is_beyond = false;
								break;
							}
						}
					}
					if (is_beyond)
					{
						cout << "symbol:" << x << endl;
					}
				}
			}
		}

	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << ex.what() << '\n';
	}
	cout << "Timstamp:" << t.elapsed() << "total:" << total << endl;
	return 0;
}