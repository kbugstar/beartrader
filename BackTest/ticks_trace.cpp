#include <iostream>
#include <fstream>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/thread/condition.hpp>
#include <thread>

//#include "csv.h"
#include "CCSVRow.h"

using std::cout;
using std::endl;

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
		char* new_buffer = new char[10240];
		memcpy(new_buffer, _buffer, _offset);
		delete[] _buffer;
		_buffer = new_buffer;
	}
	memcpy(_buffer + _offset, &tick_data, sizeof(tick_data));
	_offset += sizeof(tick_data);

	return true;
}

bool WriteFileBlock::write_to_file()
{
	std::ofstream out_file(_file_path);
	out_file.write(_buffer, _offset);
	return true;
}

typedef std::list<WriteFileBlock*> sh_tick_data_list;
boost::mutex sh_tick_data_list_lock_;
sh_tick_data_list sh_tick_data_list_;
bool work_done = false;
boost::condition sh_tick_data_condition_;

void writing_thread()
{
	while (true)
	{
		WriteFileBlock* _data_block = nullptr;
		sh_tick_data_list_lock_.lock();
		if (sh_tick_data_list_.size() > 0)
		{
			_data_block = sh_tick_data_list_.front();
			sh_tick_data_list_.pop_front();
		}
		sh_tick_data_list_lock_.unlock();

		if (_data_block == nullptr && work_done)
		{
			break;
		}

		_data_block->write_to_file();

		delete _data_block;
	}
}

int convert_str_to_bin_multi(const boost::filesystem::path& file_path, boost::filesystem::path& out_dir)
{
	uint32_t lines = 0;
	std::ifstream file(file_path.string());
	std::string line_stock_info;
	WriteFileBlock* _data_block = new WriteFileBlock((out_dir / file_path.filename()).string().c_str(), file_path.size());

	while (std::getline(file, line_stock_info) && file.good())
	{
		_data_block->read_line(line_stock_info);
		lines++;
	}

	sh_tick_data_list_lock_.lock();
	sh_tick_data_list_.push_back(_data_block);
	sh_tick_data_list_lock_.unlock();
	return lines;
}

void rading_thread(boost::filesystem::path& dir, boost::filesystem::path& out_dir)
{
	for (auto& x : boost::filesystem::directory_iterator(dir))
	{
		//cout << x << endl;
		if (is_regular_file(x.path()))
		{
			convert_str_to_bin_multi(x.path(), out_dir);
		}
	}
	work_done = true;
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

	if (!exists(out_dir))
	{
		boost::filesystem::create_directories(out_dir);
	}

	if (!is_directory(out_dir))
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
	std::thread reader(rading_thread, dir, out_dir);
	std::thread writer(writing_thread);

	reader.join();
	writer.join();

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