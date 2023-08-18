/*
 * @Description:
 * @Author: Sauron
 * @Date: 2023-07-17 16:38:11
 * @LastEditTime: 2023-07-31 11:24:14
 * @LastEditors: Sauron
 */
#ifndef INTERNEURON__TOOLS_HPP_
#define INTERNEURON__TOOLS_HPP_
#include <iostream>
#include <cstring>
#include <errno.h>
#include <chrono>
#include <fstream>
#include <cassert>
#include <filesystem>
#include "visibility_control.hpp"
namespace interneuron
{
	INTERNEURON_PUBLIC
	uint64_t get_timestamp_in_ns();

	INTERNEURON_PUBLIC
	std::ofstream create_file(const std::string& path, const std::string &file_name);
	
}
#endif // INTERNEURON__TOOLS_HPP_