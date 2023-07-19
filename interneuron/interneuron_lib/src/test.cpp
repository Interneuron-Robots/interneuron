/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-19 09:43:46
 * @LastEditTime: 2023-07-19 10:05:00
 * @LastEditors: Sauron
 */
#include "interneuron_lib/time_point_manager.hpp"
#include <iostream>
#include "interneuron_lib/tools.hpp"
int main(){
	std::cout<<interneuron::get_timestamp_in_ns()<<std::endl;
	return 0;
}