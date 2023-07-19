/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-18 21:57:37
 * @LastEditTime: 2023-07-19 09:56:03
 * @LastEditors: Sauron
 */
#include "interneuron_lib/tools.hpp"

namespace interneuron{

	INTERNEURON_PUBLIC
	uint64_t get_timestamp_in_ns()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	INTERNEURON_PUBLIC
	bool dirExists(const char *path)
	{
		struct stat info;

		if (stat(path, &info) != 0)
		{
			return false; // can't access path
		}
		else if (info.st_mode & S_IFDIR)
		{
			return true; // path is a directory
		}
		else
		{
			return false; // path is not a directory
		}
	}
	
	INTERNEURON_PUBLIC
	std::ofstream create_file(const std::string& path, const std::string &file_name)
	{
		if(interneuron::dirExists(path.c_str()) == false)
		{
			std::cout<<"[tools]path:"<<path<<" doesn't exist, create it"<<std::endl;
			if(mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
			{
				std::cout<<"[tools]mkdir failed, error:"<<strerror(errno)<<std::endl;
				assert(false);
			}
		}
		std::ofstream file(path+'/'+file_name, std::ios::out | std::ios::app);
		assert(file.is_open());
		return file;
	}
}