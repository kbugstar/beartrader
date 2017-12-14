#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>

//#include "csv.h"
#include "CCSVRow.h"

using std::cout;
using std::endl;
using namespace boost::filesystem;


int convert_str_to_bin(char* file_path)
{
	std::ifstream file(file_path);

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
		if (b != 0)
		{
			if (c < total_a / total_b)
			{
				is_beyond = false;
				break;
			}
		}
	}

}


int main(int argc, char** argv)
{
	boost::timer t;
	path dir(argv[1]);
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
			for (auto& x : directory_iterator(dir))
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
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
	}
	cout << "Timstamp:" << t.elapsed() << "total:" << total << endl;
}