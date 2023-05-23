/*
 * @Description:
 * @Author: Sauron
 * @Date: 2023-05-10 16:33:52
 * @LastEditTime: 2023-05-22 23:15:09
 * @LastEditors: Sauron
 */

#ifndef INTERNEURON_LIB__INTERNEURON_MONITOR_HPP_
#define INTERNEURON_LIB__INTERNEURON_MONITOR_HPP_
#include "interneuron_msgs/msg/interneuron_header.hpp"
#include <vector>
#include <string>
#include "time_point.hpp"
#include "visibility_control.hpp"

namespace interneuron
{
	enum class Advice
	{
		START,
		END
	};

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
		bool add_timepoint(const std::string &topic_name, const std::string &node_name, std::vector<std::string> &sensor_names);
		
// we cannot use message_info directly, so it's rmw, rcl and rclcpp's job to provide needed information
		INTERNEURON_PUBLIC
		std::shared_ptr<TimePoint> get_timepoint(const std::string &topic_name, const std::string &node_name);

// this function is provided for convenience and is thread-safe, it will use default parameters(x) and algorithm to update timepoint, you should use get_timepoint() and update_time() if you want to customize the update process
		INTERNEURON_PUBLIC
		bool default_update(const std::string &topic_name, const std::string &node_name);

	private:
		// Private constructor so that no objects can be created.
		TimePointManager()
		{
			// Initialization of the singleton, if needed
		}
std::mutex mtx_;
// we use shared_ptr, so we dont need to worry about the memory management(this is for get_timepoint())
		std::map<std::string, std::map<std::string, std::shared_ptr<TimePoint>>> time_points_; // topic_name + node_name -> timepoint
	};
}
#endif