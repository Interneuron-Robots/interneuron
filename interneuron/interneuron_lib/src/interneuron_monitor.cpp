/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-10 17:26:18
 * @LastEditTime: 2023-05-11 23:18:31
 * @LastEditors: Sauron
 */
#include "interneuron_lib/interneuron_monitor.hpp"
#include <iostream>

namespace interneuron{
	bool init_timepoint(rclcpp::Node& node, const std::string & topic_name, std::vector<std::string>& sensor_names){
		std::cout << "init_timepoint" << std::endl;
		if(node.init_timepoint(topic_name, sensor_names)){
			std::cout<<"success"<<std::endl;
		}
		return true;
	}
	uint64_t start_time(interneuron_msgs::msg::InterneuronHeader& header){
		std::cout << "start_time" <<header.time_pieces[0].start_time << std::endl;
		return header.time_pieces[0].start_time;
	}
}