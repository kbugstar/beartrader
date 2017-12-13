#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include "CCSVRow.h"

using std::cout;
using std::endl;
using namespace boost::filesystem;

int main(int argc, char** argv)
{
	path dir(argv[1]);
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
					int64_t total_a = 0;
					int64_t total_b = 0;
					std::ifstream file(x.path().string());
					bool is_beyond = true;
					for (CSVIterator loop(file); loop != CSVIterator(); ++loop)
					{
						if ((*loop)[0] == "")
							continue;
						int64_t a = std::atoi((*loop)[4].c_str());
						int64_t b = std::atoi((*loop)[3].c_str()) * 100;
						//cout << (*loop)[2] << "\t" << (*loop)[3] << endl;
						int64_t c = int64_t(std::atof((*loop)[2].c_str()) * 100);
						total_a += a;
						total_b += b;
						if (b != 0)
						{
							if (c < a * 100 / b)
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
}