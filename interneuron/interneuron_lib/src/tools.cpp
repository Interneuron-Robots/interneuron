/*
 * @Description:
 * @Author: Sauron
 * @Date: 2023-07-18 21:57:37
 * @LastEditTime: 2023-07-31 11:23:55
 * @LastEditors: Sauron
 */
#include "interneuron_lib/tools.hpp"

namespace interneuron
{

	INTERNEURON_PUBLIC
	uint64_t get_timestamp_in_ns()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	INTERNEURON_PUBLIC
	std::ofstream create_file(const std::string &path, const std::string &file_name)
	{
		if (std::filesystem::create_directories(path))
		{
			std::cout << "Successfully created " << path << '\n';
		}
		else
		{
			std::cout << "Cannot create " << path << ", some directories may already exist or operation is not permitted.\n";
		}
		std::ofstream file(path + '/' + file_name, std::ios::out | std::ios::app);
		assert(file.is_open());
		return file;
	}
}