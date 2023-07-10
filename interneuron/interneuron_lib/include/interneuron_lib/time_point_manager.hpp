/*
 * @Description:
 * @Author: Sauron
 * @Date: 2023-05-10 16:33:52
 * @LastEditTime: 2023-07-09 17:14:34
 * @LastEditors: Sauron
 */

#ifndef INTERNEURON_LIB__INTERNEURON_MONITOR_HPP_
#define INTERNEURON_LIB__INTERNEURON_MONITOR_HPP_
//#include "interneuron_msgs/msg/interneuron_header.hpp"
#include <vector>
#include <string>
#include "time_point.hpp"
#include "visibility_control.hpp"

namespace interneuron
{

	class TimePointManager
	{
	public:
		// Static access method. This function is how we access the singleton object.
		static TimePointManager &getInstance()
		{
			static TimePointManager instance; // Guaranteed to be destroyed.
							  // Instantiated on first use.
			return instance;
		}

		// Delete copy constructor and assignment operator.
		// This is important, because we don't want to accidentally make copies of the singleton.
		TimePointManager(const TimePointManager &) = delete;		// Copy constructor
		TimePointManager &operator=(const TimePointManager &) = delete; // Copy assignment operator
		TimePointManager(TimePointManager &&) = delete;			// Move constructor
		TimePointManager &operator=(TimePointManager &&) = delete;	// Move assignment operator

		INTERNEURON_PUBLIC
		std::shared_ptr<TimePoint> add_timepoint(const std::string &key, const std::vector<std::string> &sensor_names);

// we cannot use message_info directly, so it's rmw, rcl and rclcpp's job to provide needed information
		INTERNEURON_PUBLIC
		std::shared_ptr<TimePoint> get_timepoint(const std::string &key);

		// init_source should be invoked for all the sensors during the initialization
		// key should be the sensor's publisher's ID+'sen', remain_time could be 0
		INTERNEURON_PUBLIC
		void init_source(std::string&sensor_name, uint64_t deadline, uint64_t remain_time = 0);

		INTERNEURON_PUBLIC
		bool update_remain_time(std::string sensor_name, uint64_t finish_time, uint8_t x=30);
		INTERNEURON_PUBLIC
		uint64_t get_remain_time(std::string sensor_name);

	private:
		// Private constructor so that no objects can be created.
		TimePointManager()
		{
			// Initialization of the singleton, if needed
		}
std::mutex mtx_;
// we use shared_ptr, so we dont need to worry about the memory management(this is for get_timepoint())
		//std::map<std::string, std::map<std::string, std::shared_ptr<TimePoint>>> time_points_; // topic_name + node_name -> timepoint
		std::map<std::string, std::shared_ptr<TimePoint>> time_points_;// key is the sub/pub_id + a string

		//the following maps are used to store the information for all the chains
		std::map<std::string, uint64_t> remain_times_;//key is the sensor name
		std::map<std::string, uint64_t> deadlines_;//key is the sensor name
	};
}
#endif